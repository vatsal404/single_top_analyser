import sys
sys.path.insert(0, "/uscms_data/d3/vsinha/torch_libs")

import uproot
import awkward as ak
import numpy as np
import pandas as pd
import optuna
import os
import matplotlib.pyplot as plt
import joblib
import gc

import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, TensorDataset

from sklearn.model_selection import train_test_split, StratifiedShuffleSplit
from sklearn.metrics import roc_curve, roc_auc_score, confusion_matrix, ConfusionMatrixDisplay
from sklearn.preprocessing import StandardScaler, label_binarize

# ============================================
# Configuration
# ============================================

region_configs = {
    "1j1t": {
        "signal": [
            "/eos/uscms/store/user/vsinha/combined/TWminusto2L2Nu/TWminusto2L2Nu_nominal.root",
            "/eos/uscms/store/user/vsinha/combined/TbarWplusto2L2Nu/TbarWplusto2L2Nu_nominal.root",
        ],
        "background_dilept": [
            "/eos/uscms/store/user/vsinha/combined/TTbar_Dilept/TTbar_Dilept_nominal.root",
        ],
        "background_semilept": [
            "/eos/uscms/store/user/vsinha/combined/TTbar_SemiLept/TTbar_SemiLept_nominal.root",
        ],
        "region_flag":  "region_1j1t",
        "channel_flag": "eu_channel",
    }
}
# Class label mapping
#   0 -> tW signal
#   1 -> TTbar Dileptonic
#   2 -> TTbar Semi-Leptonic

CLASS_NAMES  = {0: "Signal (tW)",  1: r"$t\bar{t}$ Dilept",  2: r"$t\bar{t}$ SemiLept"}
CLASS_COLORS = {0: "steelblue",    1: "tomato",                2: "forestgreen"}

tree_name = "outputTree"

input_vars = [
    "leading_lepton_pt",
    "dilepton_del_phi",
    "leptons_invariant_mass",
    "dilepton_jet_pt",
    "Selected_loosejet_leadingpt",
    "leading_lepton_jet_pt",
    "dilepton_jet_mass",
    "sphericity",
    "aplanery",
    "delR_leadinglepton_jet",
    "delR_ele_muon",
    "subleading_lepton_pt",
    "HT",
    "ST",
    "MT2",
    "centrality",
    "lepton_pt_asymmetry",
#    "PuppiMET_pt_corr",
#    "PuppiMET_phi_corr",
    "mbl_min",
    "max_delR",
    "min_delR",
#    "Selected_loosejet_leadingbtag",
#    "Selected_jet_leading_btag",
    "datasetId"
]

N_FEATURES = len(input_vars)
N_CLASSES  = 3

# ============================================
# Sampling fraction
# ============================================

SAMPLE_FRACTION = 0.5

# ============================================
# Optuna settings
# ============================================

OPTUNA_TUNE_FRACTION = 0.1
OPTUNA_N_TRIALS      = 20
OPTUNA_ALPHA         = 1.0

# ============================================
# DNN training settings
# ============================================

BATCH_SIZE  = 4096
MAX_EPOCHS  = 100
PATIENCE    = 10
INFER_CHUNK = 50_000

DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")
print(f"\n🖥️  Using device: {DEVICE}")

# ============================================
# Output directories
# ============================================

os.makedirs("plots/roc_classifier",     exist_ok=True)
os.makedirs("plots/confusion_matrix",   exist_ok=True)
os.makedirs("plots/dnn_output",         exist_ok=True)
os.makedirs("plots/correlation_matrix", exist_ok=True)

# ============================================
# Helper: load ROOT files
# NOTE: also loads "event" branch for odd/even splitting
# ============================================

def load_files(file_list, label, region_flag, channel_flag):
    dfs = []
    required_branches = input_vars + [region_flag, channel_flag, "evWeight", "event"]

    for f in file_list:
        print(f"\n📂 Opening file: {f}")
        total_events = 0

        for arrays in uproot.iterate(
            f"{f}:{tree_name}",
            expressions=required_branches,
            library="ak",
            step_size="100 MB",
        ):
            mask = (arrays[region_flag] == True) & (arrays[channel_flag] == True)
            arrays     = arrays[mask]
            n_selected = len(arrays)
            total_events += n_selected
            if n_selected == 0:
                continue

            data_dict = {}
            for var in input_vars + ["evWeight", "event"]:
                try:
                    data_dict[var] = ak.to_numpy(arrays[var])
                except Exception as e:
                    print(f"\n❌ Failed converting branch: {var}  Reason: {e}")
                    sys.exit(1)

            df_chunk = pd.DataFrame(data_dict)
            df_chunk["label"] = label
            dfs.append(df_chunk)

        print(f"   → Events after selection: {total_events}")

    return pd.concat(dfs, ignore_index=True)

# ============================================
# Helper: DNN architecture
# ============================================

class DNN(nn.Module):
    def __init__(self, input_dim, hidden_layers, dropout_rate):
        super().__init__()
        layers = []
        in_dim = input_dim
        for h in hidden_layers:
            layers += [
                nn.Linear(in_dim, h),
                nn.BatchNorm1d(h),
                nn.ReLU(),
                nn.Dropout(dropout_rate),
            ]
            in_dim = h
        layers.append(nn.Linear(in_dim, N_CLASSES))
        self.net = nn.Sequential(*layers)

    def forward(self, x):
        return self.net(x)

# ============================================
# Helper: weighted cross-entropy loss
# ============================================

def weighted_ce_loss(logits, labels, weights):
    ce = nn.CrossEntropyLoss(reduction="none")(logits, labels)
    return (ce * weights).sum() / weights.sum()

# ============================================
# Helper: one mini-batch training epoch
# ============================================

def train_epoch(model, loader, optimizer):
    model.train()
    total_loss = 0.0
    for X_batch, y_batch, w_batch in loader:
        X_batch = X_batch.to(DEVICE)
        y_batch = y_batch.to(DEVICE)
        w_batch = w_batch.to(DEVICE)
        optimizer.zero_grad()
        logits = model(X_batch)
        loss   = weighted_ce_loss(logits, y_batch, w_batch)
        loss.backward()
        optimizer.step()
        total_loss += loss.item()
    return total_loss / len(loader)

# ============================================
# Helper: full-batch training step (Optuna)
# ============================================

def train_epoch_fullbatch(model, X_t, y_t, w_t, optimizer):
    model.train()
    optimizer.zero_grad()
    logits = model(X_t)
    loss   = weighted_ce_loss(logits, y_t, w_t)
    loss.backward()
    optimizer.step()
    return loss.item()

# ============================================
# Helper: macro OvR AUC
# ============================================

def macro_ovr_auc(y_true, proba, weights):
    y_bin = label_binarize(y_true, classes=[0, 1, 2])
    aucs  = [
        roc_auc_score(y_bin[:, c], proba[:, c], sample_weight=weights)
        for c in range(N_CLASSES)
    ]
    return float(np.mean(aucs))

# ============================================
# Helper: chunked inference
# ============================================

def predict_proba(model, X_np):
    model.eval()
    chunks = []
    with torch.no_grad():
        for i in range(0, len(X_np), INFER_CHUNK):
            X_chunk = torch.tensor(X_np[i : i + INFER_CHUNK]).to(DEVICE)
            logits  = model(X_chunk)
            proba   = torch.softmax(logits, dim=1).cpu().numpy()
            chunks.append(proba)
    return np.concatenate(chunks, axis=0)

# ============================================
# Helper: make DataLoader
# ============================================

def make_loader(X_np, y_np, w_np, shuffle=True):
    ds = TensorDataset(
        torch.tensor(X_np, dtype=torch.float32),
        torch.tensor(y_np, dtype=torch.long),
        torch.tensor(w_np, dtype=torch.float32),
    )
    return DataLoader(ds, batch_size=BATCH_SIZE, shuffle=shuffle,
                      num_workers=0, pin_memory=False)

# ============================================
# Helper: pairwise ROC
# ============================================

def pairwise_roc(y_true, proba, w, class_a, class_b):
    mask   = (y_true == class_a) | (y_true == class_b)
    y_pair = (y_true[mask] == class_a).astype(int)
    w_pair = w[mask]
    p_a    = proba[mask, class_a]
    p_b    = proba[mask, class_b]
    score  = p_a / (p_a + p_b + 1e-12)
    fpr, tpr, _ = roc_curve(y_pair, score, sample_weight=w_pair)
    auc = roc_auc_score(y_pair, score, sample_weight=w_pair)
    return fpr, tpr, auc

def ovr_roc(y_bin_col, proba_col, w):
    fpr, tpr, _ = roc_curve(y_bin_col, proba_col, sample_weight=w)
    auc = roc_auc_score(y_bin_col, proba_col, sample_weight=w)
    return fpr, tpr, auc

# ============================================
# Helper: correlation matrix plot
# ============================================

def plot_corr_matrix(corr, title, outpath):
    n = len(input_vars)
    plt.figure(figsize=(20, 18))
    im = plt.imshow(corr, interpolation="nearest", aspect="auto",
                    vmin=-1, vmax=1, cmap="coolwarm")
    plt.colorbar(im, label="Pearson Correlation")
    plt.xticks(range(n), input_vars, rotation=90, fontsize=8)
    plt.yticks(range(n), input_vars, fontsize=8)
    for i in range(n):
        for j in range(n):
            v = corr.iloc[i, j]
            plt.text(j, i, f"{v:.2f}", ha="center", va="center",
                     fontsize=6, fontweight="bold",
                     color="white" if abs(v) > 0.5 else "black")
    plt.title(title, fontsize=16)
    plt.tight_layout()
    plt.savefig(outpath, dpi=300, bbox_inches="tight")
    plt.close()

# ============================================
# Load data (3 classes)
# ============================================

cfg = region_configs["1j1t"]

print("\n================ Loading Signal (label=0) =================")
df_sig = load_files(cfg["signal"], 0, cfg["region_flag"], cfg["channel_flag"])

print("\n================ Loading TTbar Dilept (label=1) =================")
df_bkg1 = load_files(cfg["background_dilept"], 1, cfg["region_flag"], cfg["channel_flag"])

print("\n================ Loading TTbar SemiLept (label=2) =================")
df_bkg2 = load_files(cfg["background_semilept"], 2, cfg["region_flag"], cfg["channel_flag"])

print(f"\nRaw signal events          : {len(df_sig)}")
print(f"Raw bkg1 (dilept) events   : {len(df_bkg1)}")
print(f"Raw bkg2 (semilept) events : {len(df_bkg2)}")

# ============================================
# Sub-sample (preserve class ratios)
# ============================================

if SAMPLE_FRACTION < 1.0:
    df_sig  = df_sig.sample(frac=SAMPLE_FRACTION,  random_state=42).reset_index(drop=True)
    df_bkg1 = df_bkg1.sample(frac=SAMPLE_FRACTION, random_state=42).reset_index(drop=True)
    df_bkg2 = df_bkg2.sample(frac=SAMPLE_FRACTION, random_state=42).reset_index(drop=True)
    print(f"\n✂️  Sub-sampled to {SAMPLE_FRACTION*100:.0f}%:")
    print(f"   Signal : {len(df_sig)}   Bkg1 : {len(df_bkg1)}   Bkg2 : {len(df_bkg2)}")

# ============================================
# Correlation matrices (before concat)
# ============================================

CORR_SAMPLE = 20_000
corr_sig  = df_sig[input_vars].sample(min(CORR_SAMPLE, len(df_sig)),   random_state=0).corr(method="pearson")
corr_bkg1 = df_bkg1[input_vars].sample(min(CORR_SAMPLE, len(df_bkg1)), random_state=0).corr(method="pearson")
corr_bkg2 = df_bkg2[input_vars].sample(min(CORR_SAMPLE, len(df_bkg2)), random_state=0).corr(method="pearson")

# ============================================
# Concat + clean
# ============================================

df = pd.concat([df_sig, df_bkg1, df_bkg2], ignore_index=True)
del df_sig, df_bkg1, df_bkg2
gc.collect()

print("\nTotal events before cleaning:", len(df))
df = df.replace([np.inf, -np.inf], np.nan).dropna()
print("Total events after  cleaning:", len(df))

# Positive-weight filter
mask = df["evWeight"] > 0
df   = df[mask].reset_index(drop=True)
print(f"\n⚖️  Events after positive-weight filter: {len(df)}")

# ============================================
# Odd / Even split into two independent DataFrames
#
#   df_even — only events where event % 2 == 0
#   df_odd  — only events where event % 2 == 1
#
#   Each DNN is trained AND tested entirely
#   within its own parity subset using a
#   standard 70/30 train_test_split.
#
#   You then apply externally:
#     even DNN  →  score odd  events  (fit template)
#     odd DNN   →  score even events  (fit template)
#
# ============================================

df_even = df[df["event"] % 2 == 0].reset_index(drop=True)
df_odd  = df[df["event"] % 2 == 1].reset_index(drop=True)

del df
gc.collect()

print(f"\n🔀 Odd/Even event split:")
print(f"   Even events : {len(df_even)}")
print(f"   Odd events  : {len(df_odd)}")

# ============================================
# Main loop — one fully independent DNN per parity
# ============================================

FOLDS = {
    "even": df_even,
    "odd":  df_odd,
}

for fold_name, df_parity in FOLDS.items():
    tag = f"1j1t_emu_{fold_name}"
    print(f"\n{'='*60}")
    print(f"  FOLD: {fold_name.upper()}  —  {len(df_parity)} events")
    print(f"  Train/test split entirely within {fold_name} events")
    print(f"{'='*60}")

    X = df_parity[input_vars].values.astype(np.float32)
    y = df_parity["label"].values.astype(np.int64)
    w = df_parity["evWeight"].values.astype(np.float32)

    # ------------------------------------------
    # 70 / 30 train–test split within this parity
    # ------------------------------------------

    X_train, X_test, y_train, y_test, w_train, w_test = train_test_split(
        X, y, w, test_size=0.3, random_state=42, stratify=y
    )

    print(f"\n   Train events : {len(X_train)}")
    print(f"   Test events  : {len(X_test)}")

    # ------------------------------------------
    # Feature standardisation (fit on train only)
    # ------------------------------------------

    scaler  = StandardScaler()
    X_train = scaler.fit_transform(X_train).astype(np.float32)
    X_test  = scaler.transform(X_test).astype(np.float32)

    scaler_path = f"DNN_scaler_multiclass_1j1t_{fold_name}.pkl"
    joblib.dump(scaler, scaler_path)
    print(f"\n💾 Scaler saved as {scaler_path}")

    # ------------------------------------------
    # Weight normalisation — equalise class sums
    # ------------------------------------------

    w_train = w_train.copy()
    w_test  = w_test.copy()

    class_sums = {c: w_train[y_train == c].sum() for c in range(N_CLASSES)}
    max_sum    = max(class_sums.values())

    print(f"\n⚖️  Weight normalisation [{fold_name}] (max sum = {max_sum:.3f}):")
    for c, s in class_sums.items():
        scale = max_sum / s
        print(f"   Class {c} ({CLASS_NAMES[c]}): sum={s:.3f}  scale={scale:.3f}")
        w_train[y_train == c] *= scale
        w_test[y_test   == c] *= scale

    # Normalise mean to ~1
    w_train = (w_train / w_train.mean()).astype(np.float32)
    w_test  = (w_test  / w_test.mean()).astype(np.float32)

    # ------------------------------------------
    # Optuna subsample (from this fold's train/test)
    # ------------------------------------------

    optuna.logging.set_verbosity(optuna.logging.WARNING)

    sss = StratifiedShuffleSplit(n_splits=1, test_size=(1.0 - OPTUNA_TUNE_FRACTION), random_state=42)
    tune_idx, _ = next(sss.split(X_train, y_train))
    X_tune, y_tune, w_tune = X_train[tune_idx], y_train[tune_idx], w_train[tune_idx]

    sss2 = StratifiedShuffleSplit(n_splits=1, test_size=(1.0 - OPTUNA_TUNE_FRACTION), random_state=42)
    tune_test_idx, _ = next(sss2.split(X_test, y_test))
    X_tune_test = X_test[tune_test_idx]
    y_tune_test = y_test[tune_test_idx]
    w_tune_test = w_test[tune_test_idx]

    print(f"\n🔬 Optuna tuning subset [{fold_name}]:")
    print(f"   Tune train events : {len(X_tune)}  ({OPTUNA_TUNE_FRACTION*100:.0f}% of train)")
    print(f"   Tune test  events : {len(X_tune_test)}")
    print(f"\n🔎 Starting Optuna search [{fold_name}] ({OPTUNA_N_TRIALS} trials)...")

    def objective(trial):
        n_layers     = trial.suggest_int  ("n_layers",     2, 4)
        n_units      = trial.suggest_int  ("n_units",      64, 512, step=64)
        dropout      = trial.suggest_float("dropout",      0.0, 0.5)
        lr           = trial.suggest_float("lr",           1e-4, 1e-2, log=True)
        weight_decay = trial.suggest_float("weight_decay", 1e-5, 1e-2, log=True)

        hidden = [n_units] * n_layers
        model  = DNN(N_FEATURES, hidden, dropout).to(DEVICE)
        opt    = optim.Adam(model.parameters(), lr=lr, weight_decay=weight_decay)

        X_t = torch.tensor(X_tune,      dtype=torch.float32).to(DEVICE)
        y_t = torch.tensor(y_tune,      dtype=torch.long).to(DEVICE)
        w_t = torch.tensor(w_tune,      dtype=torch.float32).to(DEVICE)
        X_v = torch.tensor(X_tune_test, dtype=torch.float32)
        y_v = y_tune_test
        w_v = w_tune_test

        best_val_auc = 0.0
        patience_ctr = 0

        for epoch in range(MAX_EPOCHS):
            train_epoch_fullbatch(model, X_t, y_t, w_t, opt)

            with torch.no_grad():
                logits_v = model(X_v.to(DEVICE))
                proba_v  = torch.softmax(logits_v, dim=1).cpu().numpy()

            val_auc = macro_ovr_auc(y_v, proba_v, w_v)

            if val_auc > best_val_auc:
                best_val_auc = val_auc
                patience_ctr = 0
            else:
                patience_ctr += 1
                if patience_ctr >= PATIENCE:
                    break

            trial.report(val_auc, epoch)
            if trial.should_prune():
                raise optuna.exceptions.TrialPruned()

        with torch.no_grad():
            proba_t = torch.softmax(model(X_t), dim=1).cpu().numpy()
        train_auc = macro_ovr_auc(y_tune, proba_t, w_tune)
        gap = abs(train_auc - best_val_auc)
        return best_val_auc - OPTUNA_ALPHA * gap

    pruner = optuna.pruners.MedianPruner(n_startup_trials=5, n_warmup_steps=10)
    study  = optuna.create_study(direction="maximize", pruner=pruner)
    study.optimize(objective, n_trials=OPTUNA_N_TRIALS, show_progress_bar=True)

    print(f"\n✅ Optuna best penalised score [{fold_name}]: {study.best_value:.4f}")
    print(f"✅ Optuna best params          [{fold_name}]: {study.best_params}")

    # Optuna history plot
    trials      = [t.number for t in study.trials if t.value is not None]
    scores      = [t.value  for t in study.trials if t.value is not None]
    best_so_far = np.maximum.accumulate(scores)
    plt.figure(figsize=(9, 5))
    plt.scatter(trials, scores,      s=15, alpha=0.5, color="steelblue", label="Trial penalised score")
    plt.plot   (trials, best_so_far, color="tomato",  linewidth=2,       label="Best score so far")
    plt.xlabel("Trial Number",                                              fontsize=13)
    plt.ylabel(r"Macro OvR AUC $-$ $\alpha$·|Train$-$Test gap|",         fontsize=13)
    plt.title(
        rf"tW vs $t\bar{{t}}$ — Optuna History (1j1t, $e\mu$, {fold_name})",
        fontsize=13
    )
    plt.legend(fontsize=12); plt.grid(); plt.tight_layout()
    plt.savefig(f"plots/roc_classifier/optuna_search_history_{tag}.png", dpi=150)
    plt.close()
    print(f"📈 Optuna search history saved [{fold_name}]")

    best_p = study.best_params
    best_params = {
        "n_layers"     : best_p.get("n_layers",     3),
        "n_units"      : best_p.get("n_units",      256),
        "dropout"      : best_p.get("dropout",      0.3),
        "lr"           : best_p.get("lr",           1e-3),
        "weight_decay" : best_p.get("weight_decay", 1e-4),
    }

    # ------------------------------------------
    # Free Optuna subsets before final training
    # ------------------------------------------

    del X_tune, y_tune, w_tune, X_tune_test, y_tune_test, w_tune_test
    gc.collect()

    # ------------------------------------------
    # Train final DNN on full training set of this parity
    # ------------------------------------------

    print(f"\n🚀 Training final DNN [{fold_name}] on full {fold_name} training set...")
    print(f"   Architecture : {best_params['n_layers']} layers × {best_params['n_units']} units")
    print(f"   Dropout      : {best_params['dropout']:.4f}")
    print(f"   LR           : {best_params['lr']:.6f}")
    print(f"   Weight decay : {best_params['weight_decay']:.2e}")

    hidden_final = [best_params["n_units"]] * best_params["n_layers"]
    final_model  = DNN(N_FEATURES, hidden_final, best_params["dropout"]).to(DEVICE)
    final_opt    = optim.Adam(
        final_model.parameters(),
        lr           = best_params["lr"],
        weight_decay = best_params["weight_decay"],
    )

    scheduler = optim.lr_scheduler.ReduceLROnPlateau(
        final_opt, mode="max", factor=0.5, patience=5
    )

    train_loader = make_loader(X_train, y_train, w_train)

    train_losses, train_aucs, test_aucs = [], [], []
    best_test_auc = 0.0
    best_state    = None
    patience_ctr  = 0

    AUC_EVAL_EVERY = 5

    for epoch in range(MAX_EPOCHS):
        loss = train_epoch(final_model, train_loader, final_opt)
        train_losses.append(loss)

        if (epoch + 1) % AUC_EVAL_EVERY == 0 or epoch == 0:
            proba_tr = predict_proba(final_model, X_train)
            proba_te = predict_proba(final_model, X_test)
            tr_auc   = macro_ovr_auc(y_train, proba_tr, w_train)
            te_auc   = macro_ovr_auc(y_test,  proba_te, w_test)
            train_aucs.append(tr_auc)
            test_aucs.append(te_auc)
            print(f"   Epoch {epoch+1:3d} | loss={loss:.4f} | train AUC={tr_auc:.4f} | test AUC={te_auc:.4f}")

            scheduler.step(te_auc)

            if te_auc > best_test_auc:
                best_test_auc = te_auc
                best_state    = {k: v.cpu().clone() for k, v in final_model.state_dict().items()}
                patience_ctr  = 0
            else:
                patience_ctr += 1
                if patience_ctr >= PATIENCE:
                    print(f"   ⏹  Early stopping at epoch {epoch+1}")
                    break

    final_model.load_state_dict(best_state)
    print(f"\n✅ Best test macro OvR AUC [{fold_name}]: {best_test_auc:.4f}")

    # ------------------------------------------
    # Loss curve
    # ------------------------------------------

    plt.figure(figsize=(8, 5))
    plt.plot(train_losses, label="Train loss")
    plt.xlabel("Epoch",            fontsize=13)
    plt.ylabel("Weighted CE Loss", fontsize=13)
    plt.title(
        rf"tW vs $t\bar{{t}}$ — DNN Training Loss (1j1t, $e\mu$, {fold_name})",
        fontsize=13
    )
    plt.legend(fontsize=12); plt.grid(); plt.tight_layout()
    plt.savefig(f"plots/roc_classifier/loss_vs_epoch_{tag}.png", dpi=150)
    plt.close()
    print(f"📈 Loss curve saved [{fold_name}]")

    # ------------------------------------------
    # AUC curve
    # ------------------------------------------

    plt.figure(figsize=(8, 5))
    plt.plot(train_aucs, label="Train macro OvR AUC")
    plt.plot(test_aucs,  label="Test  macro OvR AUC")
    plt.xlabel("AUC Eval Step",  fontsize=13)
    plt.ylabel("Macro OvR AUC", fontsize=13)
    plt.title(
        rf"tW vs $t\bar{{t}}$ — DNN AUC per Epoch (1j1t, $e\mu$, {fold_name})",
        fontsize=13
    )
    plt.legend(fontsize=12); plt.grid(); plt.tight_layout()
    plt.savefig(f"plots/roc_classifier/auc_vs_epoch_{tag}.png", dpi=150)
    plt.close()
    print(f"📈 AUC curve saved [{fold_name}]")

    # ------------------------------------------
    # Final predictions
    # ------------------------------------------

    proba_test  = predict_proba(final_model, X_test)
    proba_train = predict_proba(final_model, X_train)

    y_test_bin  = label_binarize(y_test,  classes=[0, 1, 2])
    y_train_bin = label_binarize(y_train, classes=[0, 1, 2])

    # ------------------------------------------
    # Per-class OvR AUC printout
    # ------------------------------------------

    print(f"\n{'='*55}")
    print(f"Per-class One-vs-Rest AUC [{fold_name}]")
    print(f"{'='*55}")
    for c in range(N_CLASSES):
        auc_tr = roc_auc_score(y_train_bin[:, c], proba_train[:, c], sample_weight=w_train)
        auc_te = roc_auc_score(y_test_bin[:,  c], proba_test[:,  c], sample_weight=w_test)
        print(f"  Class {c} ({CLASS_NAMES[c]}):  Train={auc_tr:.4f}  |  Test={auc_te:.4f}")

    macro_tr = macro_ovr_auc(y_train, proba_train, w_train)
    macro_te = macro_ovr_auc(y_test,  proba_test,  w_test)
    print(f"\n  Macro-average OvR AUC  →  Train={macro_tr:.4f}  |  Test={macro_te:.4f}")

    # ------------------------------------------
    # ROC Plot 1 — Pairwise 3-subplot
    # ------------------------------------------

    pairwise_cfgs = [
        {"class_a": 0, "class_b": 1,
         "title"  : rf"tW vs $t\bar{{t}}$ Dilept (1j1t, $e\mu$, {fold_name})",
         "color"  : CLASS_COLORS[0],
         "label_a": "tW",                   "label_b": r"$t\bar{t}$ Dilept"},
        {"class_a": 0, "class_b": 2,
         "title"  : rf"tW vs $t\bar{{t}}$ SemiLept (1j1t, $e\mu$, {fold_name})",
         "color"  : CLASS_COLORS[0],
         "label_a": "tW",                   "label_b": r"$t\bar{t}$ SemiLept"},
        {"class_a": 1, "class_b": 2,
         "title"  : rf"$t\bar{{t}}$ Dilept vs $t\bar{{t}}$ SemiLept (1j1t, $e\mu$, {fold_name})",
         "color"  : CLASS_COLORS[1],
         "label_a": r"$t\bar{t}$ Dilept",  "label_b": r"$t\bar{t}$ SemiLept"},
    ]

    fig, axes = plt.subplots(1, 3, figsize=(19, 6))
    for ax, pcfg in zip(axes, pairwise_cfgs):
        ca, cb = pcfg["class_a"], pcfg["class_b"]
        for split, y_s, proba_s, w_s, ls in [
            ("Train", y_train, proba_train, w_train, "--"),
            ("Test",  y_test,  proba_test,  w_test,  "-"),
        ]:
            fpr, tpr, auc = pairwise_roc(y_s, proba_s, w_s, ca, cb)
            ax.plot(fpr, tpr, color=pcfg["color"], linestyle=ls,
                    label=f"{split}  AUC = {auc:.3f}")
        ax.plot([0, 1], [0, 1], "k--", linewidth=0.8, label="Random")
        ax.set_xlabel(f"FPR  [{pcfg['label_b']} efficiency]", fontsize=11)
        ax.set_ylabel(f"TPR  [{pcfg['label_a']} efficiency]", fontsize=11)
        ax.set_title(pcfg["title"], fontsize=11)
        ax.legend(fontsize=10); ax.grid()

    plt.suptitle(
        rf"DNN — Pairwise ROC Curves (1j1t, $e\mu$, {fold_name})", fontsize=13
    )
    plt.tight_layout()
    plt.savefig(f"plots/roc_classifier/roc_pairwise_3panels_{tag}.png",
                dpi=150, bbox_inches="tight")
    plt.close()
    print(f"📈 Pairwise ROC (3-panel) saved [{fold_name}]")

    # ------------------------------------------
    # ROC Plot 2 — All 3 OvR combined
    # ------------------------------------------

    plt.figure(figsize=(9, 6))
    for c in range(N_CLASSES):
        for split, y_bin, proba_s, w_s, ls in [
            ("Train", y_train_bin, proba_train, w_train, "--"),
            ("Test",  y_test_bin,  proba_test,  w_test,  "-"),
        ]:
            fpr, tpr, auc = ovr_roc(y_bin[:, c], proba_s[:, c], w_s)
            plt.plot(fpr, tpr, color=CLASS_COLORS[c], linestyle=ls,
                     label=f"{CLASS_NAMES[c]} {split} AUC={auc:.3f}")
    plt.plot([0, 1], [0, 1], "k--", linewidth=0.8, label="Random")
    plt.xlabel("False Positive Rate (1 − Specificity)", fontsize=13)
    plt.ylabel("True Positive Rate (Sensitivity)",      fontsize=13)
    plt.title(rf"DNN — All OvR ROC Curves (1j1t, $e\mu$, {fold_name})", fontsize=12)
    plt.legend(fontsize=10); plt.grid(); plt.tight_layout()
    plt.savefig(f"plots/roc_classifier/roc_all_ovr_combined_{tag}.png", dpi=150)
    plt.close()
    print(f"📈 Combined OvR ROC saved [{fold_name}]")

    # ------------------------------------------
    # Confusion matrix
    # ------------------------------------------

    y_pred_labels = np.argmax(proba_test, axis=1)
    cm = confusion_matrix(y_test, y_pred_labels, sample_weight=w_test)

    fig, ax = plt.subplots(figsize=(7, 6))
    disp = ConfusionMatrixDisplay(
        cm,
        display_labels=["Signal (tW)", r"$t\bar{t}$ Dilept", r"$t\bar{t}$ SemiLept"],
    )
    disp.plot(ax=ax, colorbar=True, cmap="Blues")
    ax.set_title(rf"DNN — Confusion Matrix (1j1t, $e\mu$, {fold_name})", fontsize=12)
    plt.tight_layout()
    plt.savefig(f"plots/confusion_matrix/confusion_matrix_{tag}.png", dpi=150)
    plt.close()
    print(f"📊 Confusion matrix saved [{fold_name}]")

    # ------------------------------------------
    # DNN score distributions
    # ------------------------------------------

    bins = np.linspace(0, 1, 41)
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    for c in range(N_CLASSES):
        ax = axes[c]
        for cls, color in CLASS_COLORS.items():
            ax.hist(proba_train[y_train == cls, c], bins=bins, density=True,
                    histtype="step", weights=w_train[y_train == cls],
                    color=color, linewidth=1.5, label=f"Train — {CLASS_NAMES[cls]}")
            ax.hist(proba_test[y_test == cls, c], bins=bins, density=True,
                    histtype="stepfilled", alpha=0.35, weights=w_test[y_test == cls],
                    color=color, label=f"Test  — {CLASS_NAMES[cls]}")
        ax.set_xlabel(f"P(class = {c})",   fontsize=12)
        ax.set_ylabel("Normalised Events", fontsize=12)
        ax.set_title(f"Output Node: {CLASS_NAMES[c]}", fontsize=11)
        ax.legend(fontsize=9); ax.grid()
    plt.suptitle(
        rf"DNN — Output Score Distributions (1j1t, $e\mu$, {fold_name})", fontsize=13
    )
    plt.tight_layout()
    plt.savefig(f"plots/dnn_output/dnn_score_distributions_{tag}.png", dpi=150)
    plt.close()
    print(f"📊 DNN score distributions saved [{fold_name}]")

    # ------------------------------------------
    # Save model weights
    # ------------------------------------------

    model_path = f"DNN_multiclass_1j1t_{fold_name}.pt"
    torch.save(final_model.state_dict(), model_path)
    print(f"\n💾 Model weights saved as {model_path}")
    print(f"💾 Scaler already saved as {scaler_path}")

    # Free large arrays before next fold
    del X_train, X_test, y_train, y_test, w_train, w_test
    del proba_train, proba_test, final_model, train_loader
    gc.collect()
    if DEVICE.type == "cuda":
        torch.cuda.empty_cache()

# ============================================
# Correlation matrices (once, shared across folds)
# ============================================

plot_corr_matrix(corr_sig,
    "Signal (tW) — Input Variable Correlation Matrix",
    "plots/correlation_matrix/signal_correlation_matrix_multiclass_1j1t_emu.png")
print("📊 Signal correlation matrix saved")

plot_corr_matrix(corr_bkg1,
    r"$t\bar{t}$ Dileptonic — Input Variable Correlation Matrix",
    "plots/correlation_matrix/bkg1_dilept_correlation_matrix_multiclass_1j1t_emu.png")
print("📊 Bkg1 (Dilept) correlation matrix saved")

plot_corr_matrix(corr_bkg2,
    r"$t\bar{t}$ Semi-Leptonic — Input Variable Correlation Matrix",
    "plots/correlation_matrix/bkg2_semilept_correlation_matrix_multiclass_1j1t_emu.png")
print("📊 Bkg2 (SemiLept) correlation matrix saved")

# ============================================
# Summary
# ============================================

print("\n" + "="*60)
print("✅  All done!")
print("   Models saved:")
print("     DNN_multiclass_1j1t_even.pt  +  DNN_scaler_multiclass_1j1t_even.pkl")
print("       → trained & tested on even events only")
print("     DNN_multiclass_1j1t_odd.pt   +  DNN_scaler_multiclass_1j1t_odd.pkl")
print("       → trained & tested on odd events only")
print()
print("   To extract fit templates, apply externally:")
print("     even DNN  →  score odd  events")
print("     odd DNN   →  score even events")
print("="*60)
