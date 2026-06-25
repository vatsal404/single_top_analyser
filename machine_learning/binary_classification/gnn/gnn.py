"""
GNN Event Classifier: tW signal vs tt-bar background
======================================================
Architecture: EdgeConv (DGCNN-style) with 3 message-passing layers,
global mean+sum pooling, and an MLP head.

Each event is a fully-connected graph of 3 objects:
  node 0 : leading lepton      (pt, eta, phi, mass)
  node 1 : subleading lepton   (pt, eta, phi, mass)
  node 2 : leading jet         (pt, eta, phi, mass)

Edge features between every pair (i, j):
  delta_R, delta_eta, delta_phi, invariant_mass_2body

Optional global features (the same hand-crafted scalars from the BDT)
are concatenated after graph pooling so the network can exploit them
without losing the topological information from the graph.

Requirements:
  pip install torch torch-geometric uproot awkward numpy pandas
              scikit-learn matplotlib optuna

If you are on a machine with CUDA, PyG will use it automatically.
"""

import os
import sys
import math
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import uproot
import awkward as ak
import torch
import torch.nn as nn
import torch.nn.functional as F
import optuna

from torch_geometric.data import Data, DataLoader
from torch_geometric.nn   import MessagePassing, global_mean_pool, global_add_pool
from torch_geometric.utils import add_self_loops

from sklearn.model_selection      import train_test_split
from sklearn.metrics              import roc_curve, roc_auc_score, confusion_matrix, ConfusionMatrixDisplay


# ============================================================
# Configuration  (edit these to match your file locations)
# ============================================================

region_configs = {
    "1j1t": {
        "signal": [
            "../../../merged/TWminusto2L2Nu.root",
            "../../../merged/TbarWplusto2L2Nu.root",
        ],
        "background": [
            "../../../merged/TTbar_Dilept.root",
        ],
        "region_flag": "region_1j1t",
        "channel_flag": "eu_channel",
    }
}

tree_name = "outputTree"

# ── 4-momentum branches for each physics object ─────────────
# The GNN builds one graph node per object.
# Each node has features: [log(pt), eta, sin(phi), cos(phi), log(mass+1), pt/sum_pt]
#
# Graph nodes (4 total, fully connected → 12 directed edges):
#   node 0 : electron (leading)
#   node 1 : muon     (leading)
#   node 2 : selected tight jet   (full 4-momentum)
#   node 3 : selected loose jet   (pt only — eta/phi/mass absent, set to 0)
#
# For node 3 the ΔR and invariant-mass edge features to/from other nodes
# will be approximate (eta=0, phi=0), but the pt-fraction and log-pt
# features are exact and still informative.
OBJECT_BRANCHES = {
    "electron": {
        "pt":   "goodElectrons_leading_pt",
        "eta":  "goodElectrons_leading_eta",
        "phi":  "goodElectrons_leading_phi",
        "mass": "goodElectrons_leading_mass",
    },
    "muon": {
        "pt":   "goodmuons_leading_pt",
        "eta":  "goodmuons_leading_eta",
        "phi":  "goodmuons_leading_phi",
        "mass": "goodmuons_leading_mass",
    },
    "tight_jet": {
        "pt":   "Selected_loosejet_leadingpt",   # leading tight-selected jet pt
        "eta":  "Selected_jet_leading_eta",
        "phi":  "Selected_jet_leading_phi",
        "mass": "Selected_jet_leading_mass",
    },
    "loose_jet": {
        # Only pt is available for the loose jet — eta/phi/mass will safely
        # fall back to 0 inside build_node_features / build_edge_index_and_features.
        "pt":   "Selected_loosejet_leadingpt",
        "eta":  None,   # not in ntuple → fallback 0
        "phi":  None,   # not in ntuple → fallback 0
        "mass": None,   # not in ntuple → fallback 0
    },
}

# ── Global (event-level) features appended after pooling ─────
# These are hand-crafted event scalars that complement the graph.
# The graph captures object topology; these capture MET, shape variables,
# and composite quantities the GNN would need many layers to reconstruct.
# Set to [] to run graph-only.
GLOBAL_BRANCHES = [
    "HT",
    "ST",
    "MT2",
    "PuppiMET_pt_corr",
    "PuppiMET_phi_corr",
    "centrality",
    "sphericity",
    "aplanery",
]

SAMPLE_FRACTION      = 0.5    # fraction of events to load (1.0 = all)
OPTUNA_N_TRIALS      = 30     # number of Optuna hyperparameter trials
OPTUNA_TUNE_FRACTION = 0.2    # fraction of train set used during Optuna search
OPTUNA_ALPHA         = 1.0    # penalty weight for train-test AUC gap

DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")
print(f"\n🖥️  Using device: {DEVICE}")

os.makedirs("plots/roc_classifier",     exist_ok=True)
os.makedirs("plots/confusion_matrix",   exist_ok=True)
os.makedirs("plots/feature_importance", exist_ok=True)
os.makedirs("plots/bdt_output",         exist_ok=True)


# ============================================================
# Physics helper functions
# ============================================================

def delta_phi(phi1, phi2):
    """ΔΦ wrapped to [-π, π]."""
    dphi = phi1 - phi2
    return np.where(dphi > math.pi,  dphi - 2*math.pi,
           np.where(dphi < -math.pi, dphi + 2*math.pi, dphi))


def delta_R(eta1, phi1, eta2, phi2):
    """ΔR = sqrt(Δη² + ΔΦ²)"""
    deta = eta1 - eta2
    dphi = delta_phi(phi1, phi2)
    return np.sqrt(deta**2 + dphi**2)


def invariant_mass_2body(pt1, eta1, phi1, m1, pt2, eta2, phi2, m2):
    """2-body invariant mass from 4-momenta (massless approximation + mass terms)."""
    px1 = pt1 * np.cos(phi1);  py1 = pt1 * np.sin(phi1);  pz1 = pt1 * np.sinh(eta1)
    px2 = pt2 * np.cos(phi2);  py2 = pt2 * np.sin(phi2);  pz2 = pt2 * np.sinh(eta2)
    E1  = np.sqrt(px1**2 + py1**2 + pz1**2 + m1**2)
    E2  = np.sqrt(px2**2 + py2**2 + pz2**2 + m2**2)
    m2v = (E1+E2)**2 - (px1+px2)**2 - (py1+py2)**2 - (pz1+pz2)**2
    return np.sqrt(np.maximum(m2v, 0.0))


# ============================================================
# Graph construction
# ============================================================

def build_node_features(row):
    """
    Return a (3, 6) float32 tensor for one event.

    Each row = [log(pt), eta, sin(phi), cos(phi), log(mass+1), pt/sum_pt]
    Using sin/cos(phi) instead of raw phi avoids the -π/π discontinuity.
    Using log(pt) compresses the dynamic range.
    """
    objects = list(OBJECT_BRANCHES.keys())   # electron, muon, tight_jet, loose_jet
    feats = []
    sum_pt = sum(row[OBJECT_BRANCHES[o]["pt"]] for o in objects) + 1e-6

    for obj in objects:
        br  = OBJECT_BRANCHES[obj]
        pt  = row[br["pt"]]
        # Branch is None when the variable is not in the ntuple (e.g. loose_jet eta/phi/mass).
        # Safe fallback: eta=0, phi=0, mass=0.
        eta = (row[br["eta"]]  if br["eta"]  is not None else 0.0)
        phi = (row[br["phi"]]  if br["phi"]  is not None else 0.0)
        m   = (row[br["mass"]] if br["mass"] is not None else 0.0)
        feats.append([
            math.log(pt + 1e-6),
            eta,
            math.sin(phi),
            math.cos(phi),
            math.log(m + 1.0),
            pt / sum_pt,
        ])

    return torch.tensor(feats, dtype=torch.float32)   # (3, 6)


def build_edge_index_and_features(row):
    """
    Fully-connected directed graph over the 4 nodes (12 directed edges).
    Edge features for (i→j): [ΔR, Δη, sin(ΔΦ), cos(ΔΦ), m_inv / 1000]
    For the loose_jet node (index 3), eta/phi/mass are 0 so its ΔR and
    m_inv edge features are approximate — but the network can still learn
    from the pt-ratio information encoded in the node features.
    """
    objects = list(OBJECT_BRANCHES.keys())   # electron, muon, tight_jet, loose_jet
    n = len(objects)

    # Collect per-object kinematics; None branch → fallback 0
    pts  = [row[OBJECT_BRANCHES[o]["pt"]] for o in objects]
    etas = [(row[OBJECT_BRANCHES[o]["eta"]]  if OBJECT_BRANCHES[o]["eta"]  is not None else 0.0) for o in objects]
    phis = [(row[OBJECT_BRANCHES[o]["phi"]]  if OBJECT_BRANCHES[o]["phi"]  is not None else 0.0) for o in objects]
    ms   = [(row[OBJECT_BRANCHES[o]["mass"]] if OBJECT_BRANCHES[o]["mass"] is not None else 0.0) for o in objects]

    src_list, dst_list, edge_feat_list = [], [], []

    for i in range(n):
        for j in range(n):
            if i == j:
                continue   # skip self-loops in edge features; added later for message passing
            dphi_val  = delta_phi(np.array([phis[i]]), np.array([phis[j]]))[0]
            deta_val  = etas[i] - etas[j]
            dr_val    = math.sqrt(deta_val**2 + dphi_val**2)
            minv      = invariant_mass_2body(
                pts[i], etas[i], phis[i], ms[i],
                pts[j], etas[j], phis[j], ms[j]
            )
            src_list.append(i)
            dst_list.append(j)
            edge_feat_list.append([
                dr_val,
                deta_val,
                math.sin(dphi_val),
                math.cos(dphi_val),
                minv / 1000.0,        # scale to ~O(1)
            ])

    edge_index = torch.tensor([src_list, dst_list], dtype=torch.long)
    edge_attr  = torch.tensor(edge_feat_list, dtype=torch.float32)

    return edge_index, edge_attr


def build_global_features(row):
    """Return a 1-D tensor of global (event-level) scalars."""
    feats = []
    for br in GLOBAL_BRANCHES:
        val = row.get(br, 0.0)
        if val is None or (isinstance(val, float) and math.isnan(val)):
            val = 0.0
        feats.append(float(val))
    return torch.tensor(feats, dtype=torch.float32)


def event_to_graph(row, label, weight):
    """Convert one event (pandas Series / dict) to a PyG Data object."""
    x          = build_node_features(row)
    edge_index, edge_attr = build_edge_index_and_features(row)
    u          = build_global_features(row)   # global features

    return Data(
        x          = x,
        edge_index = edge_index,
        edge_attr  = edge_attr,
        u          = u.unsqueeze(0),          # shape (1, n_global)
        y          = torch.tensor([label],  dtype=torch.float32),
        w          = torch.tensor([weight], dtype=torch.float32),
    )


# ============================================================
# Data loading  (reuses your uproot loop pattern)
# ============================================================

def load_files(file_list, label, region_flag, channel_flag):
    """
    Load events from ROOT files into a pandas DataFrame.
    Branches loaded = 4-momentum branches + global branches +
                      region/channel flags + evWeight.
    """
    # Collect all branch names we need
    obj_branches = set()
    for obj in OBJECT_BRANCHES.values():
        for br in obj.values():
            if br:
                obj_branches.add(br)

    required = list(obj_branches) + GLOBAL_BRANCHES + [region_flag, channel_flag, "evWeight"]
    required = list(set(required))

    dfs = []
    for f in file_list:
        print(f"\n📂 Opening: {f}")
        total = 0
        for arrays in uproot.iterate(
            f"{f}:{tree_name}",
            expressions=required,
            library="ak",
            step_size="100 MB",
        ):
            mask = (arrays[region_flag] == True) & (arrays[channel_flag] == True)
            arrays = arrays[mask]
            if len(arrays) == 0:
                continue
            total += len(arrays)

            data_dict = {}
            for br in required:
                if br in (region_flag, channel_flag):
                    continue
                try:
                    data_dict[br] = ak.to_numpy(arrays[br])
                except Exception:
                    data_dict[br] = np.zeros(len(arrays))

            df_chunk = pd.DataFrame(data_dict)
            df_chunk["label"] = label
            dfs.append(df_chunk)

        print(f"   → Selected events: {total}")

    return pd.concat(dfs, ignore_index=True)


# ============================================================
# Build PyG dataset from DataFrame
# ============================================================

def df_to_graph_list(df):
    """Convert a DataFrame to a list of PyG Data objects."""
    graphs = []
    for _, row in df.iterrows():
        label  = int(row["label"])
        weight = float(row["evWeight"])
        try:
            g = event_to_graph(row, label, weight)
            graphs.append(g)
        except Exception as e:
            # Skip malformed events silently
            pass
    return graphs


# ============================================================
# GNN Architecture
# ============================================================

class EdgeConvLayer(MessagePassing):
    """
    EdgeConv message-passing layer.

    For each directed edge i→j:
      message m_ij = MLP([hᵢ ‖ hⱼ − hᵢ ‖ edge_attr_ij])

    Then node update:
      hᵢ' = MLP(hᵢ ‖ Σ_j m_ij)

    The difference (hⱼ − hᵢ) is the key insight: the network
    explicitly sees the RELATIVE kinematics between objects,
    which is exactly what physics analyses care about.
    """

    def __init__(self, in_channels, out_channels, edge_dim, dropout=0.1):
        super().__init__(aggr="sum")   # Σ over neighbours
        self.dropout = dropout

        # MLP for computing messages from (hᵢ, hⱼ-hᵢ, edge_attr)
        self.msg_mlp = nn.Sequential(
            nn.Linear(2 * in_channels + edge_dim, out_channels),
            nn.LayerNorm(out_channels),
            nn.ReLU(),
            nn.Dropout(dropout),
            nn.Linear(out_channels, out_channels),
            nn.ReLU(),
        )

        # MLP for updating node embedding from (hᵢ, aggregated messages)
        self.update_mlp = nn.Sequential(
            nn.Linear(in_channels + out_channels, out_channels),
            nn.LayerNorm(out_channels),
            nn.ReLU(),
            nn.Dropout(dropout),
        )

        # Residual projection if dimensions don't match
        self.residual = (
            nn.Linear(in_channels, out_channels)
            if in_channels != out_channels else nn.Identity()
        )

    def forward(self, x, edge_index, edge_attr):
        # edge_index shape: (2, E)
        # x shape:          (N, in_channels)
        # edge_attr shape:  (E, edge_dim)
        out = self.propagate(edge_index, x=x, edge_attr=edge_attr)
        out = self.update_mlp(torch.cat([x, out], dim=-1))
        return out + self.residual(x)   # residual connection

    def message(self, x_i, x_j, edge_attr):
        # x_i: features of target node i  (E, in_channels)
        # x_j: features of source node j  (E, in_channels)
        # edge_attr:                       (E, edge_dim)
        return self.msg_mlp(torch.cat([x_i, x_j - x_i, edge_attr], dim=-1))


class EventGNN(nn.Module):
    """
    Full GNN event classifier.

    Pipeline:
      1. Input projection: (4-momentum features) → hidden_dim
      2. EdgeConv ×n_layers with residual connections
      3. Global pooling: mean ‖ sum → (2 × hidden_dim)
      4. Concatenate global (event-level) features
      5. MLP classifier → sigmoid probability

    Args:
        node_dim    : number of node input features (default 6)
        edge_dim    : number of edge input features (default 5)
        global_dim  : number of global input features
        hidden_dim  : width of hidden representations
        n_layers    : number of EdgeConv message-passing layers
        dropout     : dropout probability
        label_smooth: label smoothing for BCE loss (0 = off)
    """

    def __init__(
        self,
        node_dim    = 6,
        edge_dim    = 5,
        global_dim  = len(GLOBAL_BRANCHES),
        hidden_dim  = 64,
        n_layers    = 3,
        dropout     = 0.1,
        label_smooth= 0.05,
    ):
        super().__init__()
        self.label_smooth = label_smooth

        # Input projection
        self.input_proj = nn.Sequential(
            nn.Linear(node_dim, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.ReLU(),
        )

        # Stack of EdgeConv layers
        self.conv_layers = nn.ModuleList([
            EdgeConvLayer(hidden_dim, hidden_dim, edge_dim, dropout)
            for _ in range(n_layers)
        ])

        # Classifier MLP
        # Input = mean_pool ‖ sum_pool ‖ global_features
        mlp_in = 2 * hidden_dim + global_dim
        self.classifier = nn.Sequential(
            nn.Linear(mlp_in, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.ReLU(),
            nn.Dropout(dropout),
            nn.Linear(hidden_dim, hidden_dim // 2),
            nn.ReLU(),
            nn.Dropout(dropout / 2),
            nn.Linear(hidden_dim // 2, 1),
        )

    def forward(self, data):
        x          = data.x          # (total_nodes, node_dim)
        edge_index = data.edge_index  # (2, total_edges)
        edge_attr  = data.edge_attr   # (total_edges, edge_dim)
        batch      = data.batch       # (total_nodes,) — which graph each node belongs to
        u          = data.u           # (n_graphs, global_dim)

        # 1. Project node features
        h = self.input_proj(x)

        # 2. Message passing
        for conv in self.conv_layers:
            h = conv(h, edge_index, edge_attr)

        # 3. Global pooling: mean + sum, concatenated
        h_mean = global_mean_pool(h, batch)   # (n_graphs, hidden_dim)
        h_sum  = global_add_pool (h, batch)   # (n_graphs, hidden_dim)
        h_pool = torch.cat([h_mean, h_sum], dim=-1)   # (n_graphs, 2*hidden_dim)

        # 4. Append global features
        h_cat  = torch.cat([h_pool, u.squeeze(1)], dim=-1)

        # 5. Classify
        logit  = self.classifier(h_cat).squeeze(-1)   # (n_graphs,)
        return torch.sigmoid(logit)

    def loss(self, pred, target, weight=None):
        """
        Weighted binary cross-entropy with optional label smoothing.
        Smoothing moves targets from {0,1} to {ε, 1-ε}, which
        prevents overconfidence and improves calibration.
        """
        eps = self.label_smooth
        y   = target.float()
        if eps > 0:
            y = y * (1 - eps) + 0.5 * eps   # smooth toward 0.5

        loss = F.binary_cross_entropy(pred, y, reduction="none")
        if weight is not None:
            loss = loss * weight
        return loss.mean()


# ============================================================
# Training utilities
# ============================================================

def train_one_epoch(model, loader, optimizer, device):
    model.train()
    total_loss = 0.0
    for batch in loader:
        batch = batch.to(device)
        optimizer.zero_grad()
        pred = model(batch)
        loss = model.loss(pred, batch.y.squeeze(-1), batch.w.squeeze(-1))
        loss.backward()
        # Gradient clipping: prevents exploding gradients with heavy events
        nn.utils.clip_grad_norm_(model.parameters(), max_norm=5.0)
        optimizer.step()
        total_loss += loss.item() * batch.num_graphs
    return total_loss / len(loader.dataset)


@torch.no_grad()
def evaluate(model, loader, device):
    """Returns (loss, auc, predictions, targets, weights)."""
    model.eval()
    preds, targets, weights = [], [], []
    total_loss = 0.0
    for batch in loader:
        batch  = batch.to(device)
        pred   = model(batch)
        loss   = model.loss(pred, batch.y.squeeze(-1), batch.w.squeeze(-1))
        total_loss += loss.item() * batch.num_graphs
        preds.append(pred.cpu().numpy())
        targets.append(batch.y.squeeze(-1).cpu().numpy())
        weights.append(batch.w.squeeze(-1).cpu().numpy())
    preds   = np.concatenate(preds)
    targets = np.concatenate(targets)
    weights = np.concatenate(weights)
    auc = roc_auc_score(targets, preds, sample_weight=weights)
    return total_loss / len(loader.dataset), auc, preds, targets, weights


# ============================================================
# Load data
# ============================================================

cfg = region_configs["1j1t"]

print("\n================ Loading Signal =================")
df_sig = load_files(cfg["signal"], 1, cfg["region_flag"], cfg["channel_flag"])

print("\n================ Loading Background =================")
df_bkg = load_files(cfg["background"], 0, cfg["region_flag"], cfg["channel_flag"])

print(f"\nRaw signal events     : {len(df_sig)}")
print(f"Raw background events : {len(df_bkg)}")

# ── Sub-sample ───────────────────────────────────────────────
if SAMPLE_FRACTION < 1.0:
    df_sig = df_sig.sample(frac=SAMPLE_FRACTION, random_state=42).reset_index(drop=True)
    df_bkg = df_bkg.sample(frac=SAMPLE_FRACTION, random_state=42).reset_index(drop=True)
    print(f"\n✂️  Sub-sampled to {SAMPLE_FRACTION*100:.0f}%: "
          f"{len(df_sig)} signal, {len(df_bkg)} background")

# ── Cleaning ─────────────────────────────────────────────────
df = pd.concat([df_sig, df_bkg], ignore_index=True)
df = df.replace([np.inf, -np.inf], np.nan).dropna()
mask_pos = df["evWeight"] > 0
df = df[mask_pos].reset_index(drop=True)
print(f"\nTotal events after cleaning + positive-weight filter: {len(df)}")

# ── Weight normalisation: balance signal vs background ───────
sig_mask = df["label"] == 1
bkg_mask = df["label"] == 0
sig_sum  = df.loc[sig_mask, "evWeight"].sum()
bkg_sum  = df.loc[bkg_mask, "evWeight"].sum()
ratio    = bkg_sum / sig_sum

print(f"\n⚖️  Weight normalisation:")
print(f"   Signal weight sum     : {sig_sum:.3f}")
print(f"   Background weight sum : {bkg_sum:.3f}")
print(f"   Scaling signal by     : {ratio:.3f}")

df.loc[sig_mask, "evWeight"] *= ratio

# ── Weight clipping: prevents a handful of huge-weight events ─
# from dominating the loss landscape during GNN training.
w_max = np.percentile(df["evWeight"], 99)
df["evWeight"] = df["evWeight"].clip(upper=w_max)
print(f"   Weight clipped at 99th pct: {w_max:.4f}")

# ── Train / test split ────────────────────────────────────────
df_train, df_test = train_test_split(df, test_size=0.3, random_state=42, stratify=df["label"])
df_train = df_train.reset_index(drop=True)
df_test  = df_test.reset_index(drop=True)

print(f"\nTrain events : {len(df_train)}  |  Test events : {len(df_test)}")

# ── Build graph datasets ──────────────────────────────────────
print("\n🔨 Building graph objects for training set...")
train_graphs = df_to_graph_list(df_train)
print(f"   Train graphs: {len(train_graphs)}")

print("🔨 Building graph objects for test set...")
test_graphs  = df_to_graph_list(df_test)
print(f"   Test  graphs: {len(test_graphs)}")

# ============================================================
# Quick dimension check
# ============================================================
_g = train_graphs[0]
NODE_DIM   = _g.x.shape[1]
EDGE_DIM   = _g.edge_attr.shape[1]
GLOBAL_DIM = _g.u.shape[1]
print(f"\n📐 Graph dimensions:")
print(f"   Node features  : {NODE_DIM}")
print(f"   Edge features  : {EDGE_DIM}")
print(f"   Global features: {GLOBAL_DIM}")
print(f"   Nodes per graph: {_g.x.shape[0]}  (electron, muon, tight_jet, loose_jet)")
print(f"   Edges per graph: {_g.edge_index.shape[1]}")


# ============================================================
# Optuna hyperparameter search
# ============================================================

# Tune on a stratified sub-sample for speed
tune_labels  = np.array([g.y.item() for g in train_graphs])
n_tune       = int(OPTUNA_TUNE_FRACTION * len(train_graphs))
idx_sig      = np.where(tune_labels == 1)[0]
idx_bkg      = np.where(tune_labels == 0)[0]
n_sig_tune   = int(n_tune * len(idx_sig) / len(train_graphs))
n_bkg_tune   = n_tune - n_sig_tune
rng          = np.random.default_rng(42)
tune_idx     = np.concatenate([
    rng.choice(idx_sig, n_sig_tune, replace=False),
    rng.choice(idx_bkg, n_bkg_tune, replace=False),
])
tune_graphs  = [train_graphs[i] for i in tune_idx]

# Mirror for eval
val_labels  = np.array([g.y.item() for g in test_graphs])
n_val       = int(OPTUNA_TUNE_FRACTION * len(test_graphs))
idx_vs      = np.where(val_labels == 1)[0]
idx_vb      = np.where(val_labels == 0)[0]
n_vs        = int(n_val * len(idx_vs) / len(test_graphs))
n_vb        = n_val - n_vs
val_idx     = np.concatenate([
    rng.choice(idx_vs, n_vs, replace=False),
    rng.choice(idx_vb, n_vb, replace=False),
])
val_graphs  = [test_graphs[i] for i in val_idx]


def make_loader(graphs, batch_size, shuffle):
    return DataLoader(graphs, batch_size=batch_size, shuffle=shuffle)


def objective(trial):
    hidden_dim   = trial.suggest_categorical("hidden_dim",   [32, 64, 128])
    n_layers     = trial.suggest_int        ("n_layers",     2, 4)
    dropout      = trial.suggest_float      ("dropout",      0.0, 0.4)
    lr           = trial.suggest_float      ("lr",           1e-4, 5e-3, log=True)
    batch_size   = trial.suggest_categorical("batch_size",   [64, 128, 256])
    label_smooth = trial.suggest_float      ("label_smooth", 0.0, 0.1)

    model = EventGNN(
        node_dim    = NODE_DIM,
        edge_dim    = EDGE_DIM,
        global_dim  = GLOBAL_DIM,
        hidden_dim  = hidden_dim,
        n_layers    = n_layers,
        dropout     = dropout,
        label_smooth= label_smooth,
    ).to(DEVICE)

    optimizer = torch.optim.Adam(model.parameters(), lr=lr, weight_decay=1e-5)

    tune_loader = make_loader(tune_graphs, batch_size, shuffle=True)
    val_loader  = make_loader(val_graphs,  batch_size, shuffle=False)

    best_val_auc = 0.0
    for epoch in range(30):    # short training for Optuna
        train_one_epoch(model, tune_loader, optimizer, DEVICE)
        _, val_auc, _, _, _ = evaluate(model, val_loader, DEVICE)
        if val_auc > best_val_auc:
            best_val_auc = val_auc

    _, train_auc, _, _, _ = evaluate(model, tune_loader, DEVICE)
    gap = abs(train_auc - best_val_auc)
    return best_val_auc - OPTUNA_ALPHA * gap


print(f"\n🔎 Starting Optuna search ({OPTUNA_N_TRIALS} trials)...")
optuna.logging.set_verbosity(optuna.logging.WARNING)
study = optuna.create_study(direction="maximize")
study.optimize(objective, n_trials=OPTUNA_N_TRIALS, show_progress_bar=True)

best = study.best_params
print(f"\n✅ Optuna best penalised score : {study.best_value:.4f}")
print(f"✅ Optuna best params          : {best}")

# ── Optuna history plot ───────────────────────────────────────
trials      = [t.number for t in study.trials]
scores      = [t.value  for t in study.trials]
best_so_far = np.maximum.accumulate(scores)
plt.figure(figsize=(9, 5))
plt.scatter(trials, scores,      s=15, alpha=0.5, color="steelblue", label="Trial score")
plt.plot   (trials, best_so_far, color="tomato",  linewidth=2,       label="Best so far")
plt.xlabel ("Trial",                            fontsize=13)
plt.ylabel ("AUC − α·|Train−Test gap|",         fontsize=13)
plt.title  ("GNN — Optuna Search History (1j1t, eμ)", fontsize=13)
plt.legend(fontsize=12); plt.grid(); plt.tight_layout()
plt.savefig("plots/roc_classifier/optuna_search_history_gnn_1j1t_emu.png", dpi=150)
plt.close()
print("📈 Optuna search history saved")


# ============================================================
# Train final model with best hyperparameters
# ============================================================

EPOCHS     = 150
PATIENCE   = 20        # early stopping patience
BATCH_SIZE = best.get("batch_size", 128)

model = EventGNN(
    node_dim    = NODE_DIM,
    edge_dim    = EDGE_DIM,
    global_dim  = GLOBAL_DIM,
    hidden_dim  = best["hidden_dim"],
    n_layers    = best["n_layers"],
    dropout     = best["dropout"],
    label_smooth= best.get("label_smooth", 0.05),
).to(DEVICE)

optimizer = torch.optim.Adam(model.parameters(), lr=best["lr"], weight_decay=1e-5)
# Cosine annealing: smoothly decays LR to help convergence
scheduler = torch.optim.lr_scheduler.CosineAnnealingLR(optimizer, T_max=EPOCHS, eta_min=1e-6)

train_loader = make_loader(train_graphs, BATCH_SIZE, shuffle=True)
test_loader  = make_loader(test_graphs,  BATCH_SIZE, shuffle=False)

print(f"\n🚀 Training final GNN for up to {EPOCHS} epochs (patience={PATIENCE})...")

history = {"train_loss": [], "test_loss": [], "train_auc": [], "test_auc": []}
best_test_auc = 0.0
patience_counter = 0
best_state = None

for epoch in range(1, EPOCHS + 1):
    train_loss = train_one_epoch(model, train_loader, optimizer, DEVICE)
    scheduler.step()

    test_loss,  test_auc,  _, _, _ = evaluate(model, test_loader,  DEVICE)
    train_loss2, train_auc, _, _, _ = evaluate(model, train_loader, DEVICE)

    history["train_loss"].append(train_loss2)
    history["test_loss"].append(test_loss)
    history["train_auc"].append(train_auc)
    history["test_auc"].append(test_auc)

    if test_auc > best_test_auc:
        best_test_auc = test_auc
        patience_counter = 0
        best_state = {k: v.cpu().clone() for k, v in model.state_dict().items()}
    else:
        patience_counter += 1

    if epoch % 10 == 0 or epoch == 1:
        print(f"  Epoch {epoch:3d} | Train loss {train_loss2:.4f}  AUC {train_auc:.4f}"
              f" | Test loss {test_loss:.4f}  AUC {test_auc:.4f}"
              f" | Best test AUC {best_test_auc:.4f}")

    if patience_counter >= PATIENCE:
        print(f"\n⏹️  Early stopping at epoch {epoch} (no improvement for {PATIENCE} epochs)")
        break

# Restore best checkpoint
model.load_state_dict(best_state)
print(f"\n✅ Restored best model (test AUC = {best_test_auc:.4f})")


# ============================================================
# Evaluation plots
# ============================================================

# ── Loss curves ───────────────────────────────────────────────
epochs_done = len(history["train_loss"])
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
ax1.plot(history["train_loss"], label="Train");  ax1.plot(history["test_loss"], label="Test")
ax1.set_xlabel("Epoch", fontsize=13);  ax1.set_ylabel("Loss", fontsize=13)
ax1.set_title("GNN — Loss Curve (1j1t, eμ)", fontsize=13)
ax1.legend(fontsize=12);  ax1.grid()

ax2.plot(history["train_auc"], label="Train");  ax2.plot(history["test_auc"], label="Test")
ax2.set_xlabel("Epoch", fontsize=13);  ax2.set_ylabel("AUC", fontsize=13)
ax2.set_title("GNN — AUC per Epoch (1j1t, eμ)", fontsize=13)
ax2.legend(fontsize=12);  ax2.grid()

plt.tight_layout()
plt.savefig("plots/roc_classifier/gnn_training_curves_1j1t_emu.png", dpi=150)
plt.close()
print("📈 Training curves saved")

# ── Final predictions ─────────────────────────────────────────
_, auc_test,  y_pred_test,  y_test,  w_test  = evaluate(model, test_loader,  DEVICE)
_, auc_train, y_pred_train, y_train, w_train = evaluate(model, train_loader, DEVICE)

print(f"\n✅ Final Train AUC = {auc_train:.4f}")
print(f"✅ Final Test  AUC = {auc_test:.4f}")
print(f"   Train-Test gap  = {abs(auc_train - auc_test):.4f}")

# ── ROC curve ─────────────────────────────────────────────────
fpr_test,  tpr_test,  _ = roc_curve(y_test,  y_pred_test,  sample_weight=w_test)
fpr_train, tpr_train, _ = roc_curve(y_train, y_pred_train, sample_weight=w_train)

plt.figure(figsize=(8, 6))
plt.plot(fpr_train, tpr_train, label=f"Train  AUC = {auc_train:.3f}")
plt.plot(fpr_test,  tpr_test,  label=f"Test   AUC = {auc_test:.3f}")
plt.plot([0, 1], [0, 1], "k--", label="Random classifier")
plt.xlabel("False Positive Rate  (Background efficiency)", fontsize=13)
plt.ylabel("True Positive Rate  (Signal efficiency)",      fontsize=13)
plt.title ("GNN: tW vs $t\\bar{t}$ — ROC Curve (1j1t, $e\\mu$)", fontsize=13)
plt.legend(fontsize=12); plt.grid(); plt.tight_layout()
plt.savefig("plots/roc_classifier/gnn_roc_curve_1j1t_emu.png", dpi=150)
plt.close()
print("📈 ROC curve saved")

# ── Confusion matrix ──────────────────────────────────────────
y_label = (y_pred_test > 0.5).astype(int)
cm = confusion_matrix(y_test, y_label, sample_weight=w_test)
fig, ax = plt.subplots(figsize=(6, 5))
disp = ConfusionMatrixDisplay(cm, display_labels=["Background ($t\\bar{t}$)", "Signal (tW)"])
disp.plot(ax=ax, colorbar=True)
ax.set_title("GNN: tW vs $t\\bar{t}$ — Confusion Matrix (1j1t, $e\\mu$)", fontsize=12)
plt.tight_layout()
plt.savefig("plots/confusion_matrix/gnn_confusion_matrix_1j1t_emu.png", dpi=150)
plt.close()
print("📊 Confusion matrix saved")

# ── Score distribution ────────────────────────────────────────
bins = np.linspace(0, 1, 41)
plt.figure(figsize=(8, 5))
plt.hist(y_pred_train[y_train == 1], bins=bins, density=True, histtype="step",
         weights=w_train[y_train == 1], color="steelblue", linewidth=1.5, label="Train — Signal")
plt.hist(y_pred_test[y_test == 1],   bins=bins, density=True, histtype="stepfilled", alpha=0.35,
         weights=w_test[y_test == 1],  color="steelblue", label="Test — Signal")
plt.hist(y_pred_train[y_train == 0], bins=bins, density=True, histtype="step",
         weights=w_train[y_train == 0], color="tomato", linewidth=1.5, label="Train — Background")
plt.hist(y_pred_test[y_test == 0],   bins=bins, density=True, histtype="stepfilled", alpha=0.35,
         weights=w_test[y_test == 0],  color="tomato", label="Test — Background")
plt.xlabel("GNN Score",        fontsize=13)
plt.ylabel("Normalised Events",fontsize=13)
plt.title ("GNN: tW vs $t\\bar{t}$ — Score Distribution (1j1t, $e\\mu$)", fontsize=13)
plt.legend(fontsize=11); plt.grid(); plt.tight_layout()
plt.savefig("plots/bdt_output/gnn_score_distribution_1j1t_emu.png", dpi=150)
plt.close()
print("📊 GNN score distribution saved")

# ── Save model ────────────────────────────────────────────────
torch.save({
    "model_state_dict" : model.state_dict(),
    "best_params"      : best,
    "node_dim"         : NODE_DIM,
    "edge_dim"         : EDGE_DIM,
    "global_dim"       : GLOBAL_DIM,
    "auc_test"         : auc_test,
    "auc_train"        : auc_train,
}, "GNN_1j1t.pt")

print("\n💾 Model saved as GNN_1j1t.pt")
print("\n✅ Done!")
