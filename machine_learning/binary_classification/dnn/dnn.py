import sys
sys.path.insert(0, "/uscms_data/d3/vsinha/torch_libs")

import uproot
import awkward as ak
import numpy as np
import pandas as pd
import optuna                  # ← uncomment to re-enable Optuna tuning
import os
import matplotlib.pyplot as plt
import joblib

import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, TensorDataset

from sklearn.model_selection import train_test_split, StratifiedShuffleSplit
from sklearn.metrics import roc_curve, roc_auc_score, confusion_matrix, ConfusionMatrixDisplay
from sklearn.preprocessing import StandardScaler

# ============================================
# Configuration
# ============================================

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
    "PuppiMET_pt_corr",
    "PuppiMET_phi_corr",
    "mbl_min",
    "max_delR",
    "min_delR"
]

# ============================================
# Sampling fraction
# ============================================

SAMPLE_FRACTION = 0.5

# ============================================
# Optuna settings — only used if Optuna
# block below is uncommented
# ============================================

OPTUNA_TUNE_FRACTION = 0.10
OPTUNA_N_TRIALS      = 30
OPTUNA_ALPHA         = 1.0

# ============================================
# DNN training settings
# ============================================

BATCH_SIZE = 4096
MAX_EPOCHS = 100
PATIENCE   = 10

DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")
print(f"\n🖥️  Using device: {DEVICE}")

# ============================================
# Output directories
# ============================================

os.makedirs("plots/roc_classifier",     exist_ok=True)
os.makedirs("plots/confusion_matrix",   exist_ok=True)
os.makedirs("plots/feature_importance", exist_ok=True)
os.makedirs("plots/dnn_output",         exist_ok=True)

# ============================================
# Helper: load ROOT files
# ============================================

def load_files(file_list, label, region_flag, channel_flag):

    dfs = []
    required_branches = input_vars + [region_flag, channel_flag, "evWeight"]

    for f in file_list:
        print(f"\n📂 Opening file: {f}")
        total_events = 0

        for arrays in uproot.iterate(
            f"{f}:{tree_name}",
            expressions=required_branches,
            library="ak",
            step_size="100 MB"
        ):
            mask = (
                (arrays[region_flag] == True)
                & (arrays[channel_flag] == True)
            )
            arrays     = arrays[mask]
            n_selected = len(arrays)
            total_events += n_selected

            if n_selected == 0:
                continue

            data_dict = {}
            for var in input_vars + ["evWeight"]:
                try:
                    data_dict[var] = ak.to_numpy(arrays[var])
                except Exception as e:
                    print(f"\n❌ Failed converting branch: {var}")
                    print(f"Reason: {e}")
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
        layers.append(nn.Linear(in_dim, 1))
        self.net = nn.Sequential(*layers)

    def forward(self, x):
        return self.net(x).squeeze(1)

# ============================================
# Helper: one mini-batch training epoch
# ============================================

def train_epoch(model, loader, optimizer, criterion):
    model.train()
    total_loss = 0.0
    for X_batch, y_batch, w_batch in loader:
        X_batch = X_batch.to(DEVICE)
        y_batch = y_batch.to(DEVICE)
        w_batch = w_batch.to(DEVICE)
        optimizer.zero_grad()
        logits = model(X_batch)
        loss   = (criterion(logits, y_batch) * w_batch).mean()
        loss.backward()
        optimizer.step()
        total_loss += loss.item()
    return total_loss / len(loader)

# ============================================
# Helper: full-batch training step
# (used inside Optuna where subset fits in RAM)
# ============================================

def train_epoch_fullbatch(model, X_t, y_t, w_t, optimizer, criterion):
    model.train()
    optimizer.zero_grad()
    logits = model(X_t)
    loss   = (criterion(logits, y_t) * w_t).mean()
    loss.backward()
    optimizer.step()
    return loss.item()

# ============================================
# Helper: evaluate AUC on a DataLoader
# ============================================

def evaluate_auc(model, loader):
    model.eval()
    all_probs, all_labels, all_weights = [], [], []
    with torch.no_grad():
        for X_batch, y_batch, w_batch in loader:
            logits = model(X_batch.to(DEVICE))
            probs  = torch.sigmoid(logits).cpu().numpy()
            all_probs.append(probs)
            all_labels.append(y_batch.numpy())
            all_weights.append(w_batch.numpy())
    return roc_auc_score(
        np.concatenate(all_labels),
        np.concatenate(all_probs),
        sample_weight=np.concatenate(all_weights)
    )

# ============================================
# Helper: chunked inference to avoid OOM
# ============================================

def predict_proba(model, X_np, chunk_size=50000):
    model.eval()
    all_probs = []
    with torch.no_grad():
        for i in range(0, len(X_np), chunk_size):
            X_chunk = torch.tensor(X_np[i:i + chunk_size]).to(DEVICE)
            logits  = model(X_chunk)
            probs   = torch.sigmoid(logits).cpu().numpy()
            all_probs.append(probs)
    return np.concatenate(all_probs)

# ============================================
# Helper: make DataLoader
# ============================================

def make_loader(X_np, y_np, w_np, shuffle=True):
    ds = TensorDataset(
        torch.tensor(X_np),
        torch.tensor(y_np),
        torch.tensor(w_np),
    )
    return DataLoader(ds, batch_size=BATCH_SIZE, shuffle=shuffle, num_workers=0)

# ============================================
# Load data
# ============================================

cfg = region_configs["1j1t"]

print("\n================ Loading Signal =================")
df_sig = load_files(cfg["signal"], 1, cfg["region_flag"], cfg["channel_flag"])

print("\n================ Loading Background =================")
df_bkg = load_files(cfg["background"], 0, cfg["region_flag"], cfg["channel_flag"])

print(f"\nRaw signal events     : {len(df_sig)}")
print(f"Raw background events : {len(df_bkg)}")

# ============================================
# Sub-sample
# ============================================

if SAMPLE_FRACTION < 1.0:
    df_sig = df_sig.sample(frac=SAMPLE_FRACTION, random_state=42).reset_index(drop=True)
    df_bkg = df_bkg.sample(frac=SAMPLE_FRACTION, random_state=42).reset_index(drop=True)
    print(f"\n✂️  Sub-sampled to {SAMPLE_FRACTION*100:.0f}% of each class:")
    print(f"   Signal events kept     : {len(df_sig)}")
    print(f"   Background events kept : {len(df_bkg)}")

df = pd.concat([df_sig, df_bkg], ignore_index=True)
print("\nTotal events before cleaning:", len(df))

# ============================================
# Cleaning
# ============================================

df = df.replace([np.inf, -np.inf], np.nan)
df = df.dropna()
print("Total events after cleaning:", len(df))

# ============================================
# Features / labels / weights
# ============================================

X = df[input_vars].values.astype(np.float32)
y = df["label"].values.astype(np.float32)
w = df["evWeight"].values.astype(np.float32)

# ============================================
# Filter out negative / zero weights
# ============================================

mask = w > 0
X, y, w = X[mask], y[mask], w[mask]

print(f"\n⚖️  Weight diagnostics:")
print(f"   Events before filter : {len(mask)}")
print(f"   Negative/zero removed: {(~mask).sum()}")
print(f"   Events after filter  : {len(X)}")

# ============================================
# Train / test split
# ============================================

X_train, X_test, y_train, y_test, w_train, w_test = train_test_split(
    X, y, w, test_size=0.3, random_state=42, stratify=y
)

# ============================================
# Feature standardisation
# ============================================

scaler  = StandardScaler()
X_train = scaler.fit_transform(X_train).astype(np.float32)
X_test  = scaler.transform(X_test).astype(np.float32)
joblib.dump(scaler, "DNN_scaler_1j1t.pkl")
print("\n💾 Scaler saved as DNN_scaler_1j1t.pkl")

# ============================================
# Weight normalization
# ============================================

w_train = w_train.copy()
w_test  = w_test.copy()

sig_sum = w_train[y_train == 1].sum()
bkg_sum = w_train[y_train == 0].sum()
ratio   = bkg_sum / sig_sum

print(f"\n⚖️  Weight normalization:")
print(f"   Signal weight sum (train)     : {sig_sum:.3f}")
print(f"   Background weight sum (train) : {bkg_sum:.3f}")
print(f"   Reweighting signal by         : {ratio:.3f}")

w_train[y_train == 1] *= ratio
w_test[y_test   == 1] *= ratio

w_train = (w_train / w_train.mean()).astype(np.float32)
w_test  = (w_test  / w_test.mean()).astype(np.float32)

print(f"\n⚖️  After normalization:")
print(f"   Signal weight sum (train)     : {w_train[y_train == 1].sum():.3f}")
print(f"   Background weight sum (train) : {w_train[y_train == 0].sum():.3f}")
print(f"\nTrain events : {len(X_train)}")
print(f"Test events  : {len(X_test)}")

# ============================================
# ============================================
# OPTUNA HYPERPARAMETER TUNING
# Uncomment this entire block to re-run tuning.
# Requires: import optuna at the top,
#           and OPTUNA_* constants above.
# ============================================
# ============================================

import optuna
optuna.logging.set_verbosity(optuna.logging.WARNING)

sss = StratifiedShuffleSplit(n_splits=1, test_size=(1.0 - OPTUNA_TUNE_FRACTION), random_state=42)
tune_idx, _ = next(sss.split(X_train, y_train))
X_tune, y_tune, w_tune = X_train[tune_idx], y_train[tune_idx], w_train[tune_idx]

sss2 = StratifiedShuffleSplit(n_splits=1, test_size=(1.0 - OPTUNA_TUNE_FRACTION), random_state=42)
tune_test_idx, _ = next(sss2.split(X_test, y_test))
X_tune_test = X_test[tune_test_idx]
y_tune_test = y_test[tune_test_idx]
w_tune_test = w_test[tune_test_idx]

print(f"\n🔬 Optuna tuning subset:")
print(f"   Tune train events : {len(X_tune)}  ({OPTUNA_TUNE_FRACTION*100:.0f}% of train)")
print(f"   Tune test  events : {len(X_tune_test)}")
print(f"\n🔎 Starting Optuna search ({OPTUNA_N_TRIALS} trials)...")

def objective(trial):
    n_layers     = trial.suggest_int  ("n_layers",     2, 3)
    n_units      = trial.suggest_int  ("n_units",      64, 512, step=64)
    dropout      = trial.suggest_float("dropout",      0.0, 0.5)
    lr           = trial.suggest_float("lr",           1e-4, 1e-2, log=True)
    weight_decay = trial.suggest_float("weight_decay", 1e-5, 1e-2, log=True)

    hidden = [n_units] * n_layers
    model  = DNN(len(input_vars), hidden, dropout).to(DEVICE)
    opt    = optim.Adam(model.parameters(), lr=lr, weight_decay=weight_decay)
    crit   = nn.BCEWithLogitsLoss(reduction="none")

    X_t   = torch.tensor(X_tune).to(DEVICE)
    y_t   = torch.tensor(y_tune).to(DEVICE)
    w_t   = torch.tensor(w_tune).to(DEVICE)
    X_v   = torch.tensor(X_tune_test).to(DEVICE)
    y_v   = torch.tensor(y_tune_test)
    w_v   = torch.tensor(w_tune_test)

    best_val_auc = 0.0
    patience_ctr = 0

    for epoch in range(MAX_EPOCHS):
        train_epoch_fullbatch(model, X_t, y_t, w_t, opt, crit)
        with torch.no_grad():
            val_probs = torch.sigmoid(model(X_v.to(DEVICE))).cpu().numpy()
        val_auc = roc_auc_score(y_v.numpy(), val_probs, sample_weight=w_v.numpy())
        if val_auc > best_val_auc:
            best_val_auc = val_auc
            patience_ctr = 0
        else:
            patience_ctr += 1
            if patience_ctr >= PATIENCE:
                break

    with torch.no_grad():
        train_probs = torch.sigmoid(model(X_t)).cpu().numpy()
    train_auc = roc_auc_score(y_tune, train_probs, sample_weight=w_tune)
    gap = abs(train_auc - best_val_auc)
    return best_val_auc - OPTUNA_ALPHA * gap

study = optuna.create_study(direction="maximize")
study.optimize(objective, n_trials=OPTUNA_N_TRIALS, show_progress_bar=True)

print(f"\n✅ Optuna best penalised score : {study.best_value:.4f}")
print(f"✅ Optuna best params          : {study.best_params}")

trials      = [t.number for t in study.trials]
scores      = [t.value  for t in study.trials]
best_so_far = np.maximum.accumulate(scores)
plt.figure(figsize=(9, 5))
plt.scatter(trials, scores,      s=15, alpha=0.5, color="steelblue", label="Trial penalised score")
plt.plot   (trials, best_so_far, color="tomato",  linewidth=2,       label="Best score so far")
plt.xlabel ("Trial Number",             fontsize=13)
plt.ylabel ("AUC − α·|Train−Test gap|", fontsize=13)
plt.title  ("tW vs $t\\bar{t}$ — Optuna Search History (1j1t, $e\\mu$)", fontsize=13)
plt.legend(fontsize=12); plt.grid(); plt.tight_layout()
plt.savefig("plots/roc_classifier/optuna_search_history_1j1t_emu.png", dpi=150)
plt.close()
print("📈 Optuna search history saved")

best_params = study.best_params   # ← feed into the block below

# ============================================
# ============================================
# END OPTUNA BLOCK
# ============================================
# ============================================

# ============================================
# Best hyperparameters from Optuna run
# (paste updated values here after re-tuning)
# ============================================

best_params = {
    "n_layers"     : 2,
    "n_units"      : 512,
    "dropout"      : 0.34666639004624467,
    "lr"           : 0.009453492123423909,
    "weight_decay" : 1.702592452576748e-05,
}

# ============================================
# Train final DNN on full training set
# ============================================

print("\n🚀 Training final DNN on full training set with best hyperparameters...")
print(f"   Architecture : {best_params['n_layers']} layers × {best_params['n_units']} units")
print(f"   Dropout      : {best_params['dropout']:.4f}")
print(f"   LR           : {best_params['lr']:.6f}")
print(f"   Weight decay : {best_params['weight_decay']:.2e}")

hidden_final = [best_params["n_units"]] * best_params["n_layers"]

final_model = DNN(len(input_vars), hidden_final, best_params["dropout"]).to(DEVICE)
final_opt   = optim.Adam(
    final_model.parameters(),
    lr           = best_params["lr"],
    weight_decay = best_params["weight_decay"],
)
final_crit  = nn.BCEWithLogitsLoss(reduction="none")

train_loader = make_loader(X_train, y_train, w_train)
test_loader  = make_loader(X_test,  y_test,  w_test,  shuffle=False)

train_losses, test_aucs, train_aucs = [], [], []
best_test_auc = 0.0
best_state    = None
patience_ctr  = 0

for epoch in range(MAX_EPOCHS):
    loss   = train_epoch(final_model, train_loader, final_opt, final_crit)
    tr_auc = evaluate_auc(final_model, make_loader(X_train, y_train, w_train, shuffle=False))
    te_auc = evaluate_auc(final_model, test_loader)

    train_losses.append(loss)
    train_aucs.append(tr_auc)
    test_aucs.append(te_auc)

    if te_auc > best_test_auc:
        best_test_auc = te_auc
        best_state    = {k: v.cpu().clone() for k, v in final_model.state_dict().items()}
        patience_ctr  = 0
    else:
        patience_ctr += 1

    if (epoch + 1) % 10 == 0:
        print(f"   Epoch {epoch+1:3d} | loss={loss:.4f} | train AUC={tr_auc:.4f} | test AUC={te_auc:.4f}")

    if patience_ctr >= PATIENCE:
        print(f"   ⏹  Early stopping at epoch {epoch+1}")
        break

final_model.load_state_dict(best_state)
print(f"\n✅ Best test AUC during training : {best_test_auc:.4f}")

# ============================================
# Loss curve
# ============================================

plt.figure(figsize=(8, 5))
plt.plot(train_losses, label="Train loss")
plt.xlabel("Epoch",    fontsize=13)
plt.ylabel("BCE Loss", fontsize=13)
plt.title ("tW vs $t\\bar{t}$ — DNN Training Loss (1j1t, $e\\mu$)", fontsize=13)
plt.legend(fontsize=12)
plt.grid()
plt.tight_layout()
plt.savefig("plots/roc_classifier/loss_vs_epoch_1j1t_emu.png", dpi=150)
plt.close()
print("📈 Loss curve saved")

# ============================================
# AUC curve
# ============================================

plt.figure(figsize=(8, 5))
plt.plot(train_aucs, label="Train AUC")
plt.plot(test_aucs,  label="Test  AUC")
plt.xlabel("Epoch", fontsize=13)
plt.ylabel("AUC",   fontsize=13)
plt.title ("tW vs $t\\bar{t}$ — DNN AUC per Epoch (1j1t, $e\\mu$)", fontsize=13)
plt.legend(fontsize=12)
plt.grid()
plt.tight_layout()
plt.savefig("plots/roc_classifier/auc_vs_epoch_1j1t_emu.png", dpi=150)
plt.close()
print("📈 AUC curve saved")

# ============================================
# Predict (chunked to avoid OOM on large sets)
# ============================================

y_pred_test  = predict_proba(final_model, X_test)
y_pred_train = predict_proba(final_model, X_train)

# ============================================
# Diagnostic
# ============================================

print("\n📊 Score stats on TEST set:")
print(f"   min    : {y_pred_test.min():.4f}")
print(f"   max    : {y_pred_test.max():.4f}")
print(f"   mean   : {y_pred_test.mean():.4f}")
print(f"   median : {np.median(y_pred_test):.4f}")
print(f"\n   Fraction with score > 0.5 : {(y_pred_test > 0.5).mean():.4f}")
print(f"   Fraction with score > 0.9 : {(y_pred_test > 0.9).mean():.4f}")
print(f"\n   Score mean by true class:")
print(f"   Signal (y=1)     : {y_pred_test[y_test == 1].mean():.4f}")
print(f"   Background (y=0) : {y_pred_test[y_test == 0].mean():.4f}")

# ============================================
# ROC curve (train + test)
# ============================================

fpr_test,  tpr_test,  _ = roc_curve(y_test,  y_pred_test,  sample_weight=w_test)
fpr_train, tpr_train, _ = roc_curve(y_train, y_pred_train, sample_weight=w_train)

auc_test  = roc_auc_score(y_test,  y_pred_test,  sample_weight=w_test)
auc_train = roc_auc_score(y_train, y_pred_train, sample_weight=w_train)

print(f"\n✅ Final Train AUC = {auc_train:.4f}")
print(f"✅ Final Test  AUC = {auc_test:.4f}")
print(f"   Train-Test gap  = {abs(auc_train - auc_test):.4f}")

plt.figure(figsize=(8, 6))
plt.plot(fpr_train, tpr_train, label=f"Train  AUC = {auc_train:.3f}")
plt.plot(fpr_test,  tpr_test,  label=f"Test   AUC = {auc_test:.3f}")
plt.plot([0, 1], [0, 1], "k--", label="Random classifier")
plt.xlabel("False Positive Rate  (Background efficiency)", fontsize=13)
plt.ylabel("True Positive Rate  (Signal efficiency)",      fontsize=13)
plt.title ("tW vs $t\\bar{t}$ — ROC Curve (1j1t, $e\\mu$)", fontsize=13)
plt.legend(fontsize=12)
plt.grid()
plt.tight_layout()
plt.savefig("plots/roc_classifier/roc_curve_train_vs_test_1j1t_emu.png", dpi=150)
plt.close()
print("📈 ROC curve saved")

# ============================================
# Confusion matrix (threshold = 0.5)
# ============================================

y_label = (y_pred_test > 0.5).astype(int)
cm = confusion_matrix(y_test, y_label, sample_weight=w_test)

fig, ax = plt.subplots(figsize=(6, 5))
disp = ConfusionMatrixDisplay(cm, display_labels=["Background ($t\\bar{t}$)", "Signal (tW)"])
disp.plot(ax=ax, colorbar=True)
ax.set_title("tW vs $t\\bar{t}$ — Confusion Matrix (1j1t, $e\\mu$)", fontsize=12)
plt.tight_layout()
plt.savefig("plots/confusion_matrix/confusion_matrix_1j1t_emu.png", dpi=150)
plt.close()
print("📊 Confusion matrix saved")

# ============================================
# DNN score distribution (signal vs background)
# ============================================

bins = np.linspace(0, 1, 41)

plt.figure(figsize=(8, 5))
plt.hist(y_pred_train[y_train == 1], bins=bins, density=True, histtype="step",
         weights=w_train[y_train == 1], color="steelblue", linewidth=1.5, label="Train — Signal (tW)")
plt.hist(y_pred_test[y_test == 1],   bins=bins, density=True, histtype="stepfilled", alpha=0.35,
         weights=w_test[y_test == 1],  color="steelblue", label="Test — Signal (tW)")
plt.hist(y_pred_train[y_train == 0], bins=bins, density=True, histtype="step",
         weights=w_train[y_train == 0], color="tomato", linewidth=1.5, label="Train — Background ($t\\bar{t}$)")
plt.hist(y_pred_test[y_test == 0],   bins=bins, density=True, histtype="stepfilled", alpha=0.35,
         weights=w_test[y_test == 0],  color="tomato", label="Test — Background ($t\\bar{t}$)")

plt.xlabel("DNN Score",         fontsize=13)
plt.ylabel("Normalised Events", fontsize=13)
plt.title ("tW vs $t\\bar{t}$ — DNN Score Distribution (1j1t, $e\\mu$)", fontsize=13)
plt.legend(fontsize=11)
plt.grid()
plt.tight_layout()
plt.savefig("plots/dnn_output/dnn_score_distribution_1j1t_emu.png", dpi=150)
plt.close()
print("📊 DNN score distribution saved")

# ============================================
# Save model
# ============================================

torch.save(final_model.state_dict(), "DNN_1j1t.pt")
print("\n💾 Model weights saved as DNN_1j1t.pt")
print("💾 Scaler already saved as DNN_scaler_1j1t.pkl")
print("\n✅ Done.")
