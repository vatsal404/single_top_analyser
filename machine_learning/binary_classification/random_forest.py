import uproot
import awkward as ak
import numpy as np
import pandas as pd
import optuna
import os
import sys
import matplotlib.pyplot as plt
import joblib

from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split, StratifiedShuffleSplit
from sklearn.metrics import roc_curve, roc_auc_score, confusion_matrix, ConfusionMatrixDisplay

# ============================================
# Configuration
# ============================================

region_configs = {
    "1j1t": {
        "signal": [
            "../../merged/TWminusto2L2Nu.root",
            "../../merged/TbarWplusto2L2Nu.root",
        ],
        "background": [
            "../../merged/TTbar_Dilept.root",
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
]

# ============================================
# Sampling fraction
# ============================================

SAMPLE_FRACTION = 0.5

# ============================================
# Optuna settings
# ============================================

OPTUNA_TUNE_FRACTION = 0.05
OPTUNA_N_TRIALS      = 40
OPTUNA_ALPHA         = 1.0

# ============================================
# Output directories
# ============================================

os.makedirs("plots/roc_classifier",     exist_ok=True)
os.makedirs("plots/confusion_matrix",   exist_ok=True)
os.makedirs("plots/feature_importance", exist_ok=True)
os.makedirs("plots/rf_output",          exist_ok=True)

# ============================================
# Helper function
# ============================================

def load_files(file_list, label, region_flag, channel_flag):

    dfs = []

    required_branches = (
        input_vars
        + [region_flag, channel_flag, "evWeight"]
    )

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
                &
                (arrays[channel_flag] == True)
            )

            arrays = arrays[mask]
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
                    print("\nBranch type:")
                    print(ak.type(arrays[var]))
                    sys.exit(1)

            df_chunk = pd.DataFrame(data_dict)
            df_chunk["label"] = label
            dfs.append(df_chunk)

        print(f"   → Events after selection: {total_events}")

    return pd.concat(dfs, ignore_index=True)

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

X = df[input_vars]
y = df["label"]
w = df["evWeight"]

# ============================================
# Filter out negative / zero weights
# ============================================

mask = w > 0
X    = X[mask]
y    = y[mask]
w    = w[mask]

print(f"\n⚖️  Weight diagnostics (before normalization):")
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
# Weight normalization — only via evWeight,
# no class_weight="balanced" to avoid
# double-counting
# ============================================

w_train = w_train.copy()
w_test  = w_test.copy()

sig_sum_train = w_train[y_train == 1].sum()
bkg_sum_train = w_train[y_train == 0].sum()
ratio         = bkg_sum_train / sig_sum_train

print(f"\n⚖️  Weight normalization:")
print(f"   Signal weight sum (train)     : {sig_sum_train:.3f}")
print(f"   Background weight sum (train) : {bkg_sum_train:.3f}")
print(f"   Reweighting signal by factor  : {ratio:.3f}")

w_train[y_train == 1] *= ratio
w_test[y_test   == 1] *= ratio

print(f"\n⚖️  After normalization:")
print(f"   Signal weight sum (train)     : {w_train[y_train == 1].sum():.3f}")
print(f"   Background weight sum (train) : {w_train[y_train == 0].sum():.3f}")

print(f"\nTrain events : {len(X_train)}")
print(f"Test events  : {len(X_test)}")

# ============================================
# Optuna subsample
# ============================================

sss = StratifiedShuffleSplit(n_splits=1, test_size=(1.0 - OPTUNA_TUNE_FRACTION), random_state=42)
tune_idx, _ = next(sss.split(X_train, y_train))

X_tune = X_train.iloc[tune_idx].reset_index(drop=True)
y_tune = y_train.iloc[tune_idx].reset_index(drop=True)
w_tune = w_train.iloc[tune_idx].reset_index(drop=True)

sss_test = StratifiedShuffleSplit(n_splits=1, test_size=(1.0 - OPTUNA_TUNE_FRACTION), random_state=42)
tune_test_idx, _ = next(sss_test.split(X_test, y_test))

X_tune_test = X_test.iloc[tune_test_idx].reset_index(drop=True)
y_tune_test = y_test.iloc[tune_test_idx].reset_index(drop=True)
w_tune_test = w_test.iloc[tune_test_idx].reset_index(drop=True)

print(f"\n🔬 Optuna tuning subset:")
print(f"   Tune train events : {len(X_tune)}  ({OPTUNA_TUNE_FRACTION*100:.0f}% of train)")
print(f"   Tune test  events : {len(X_tune_test)}")

# ============================================
# Optuna hyperparameter tuning
# ============================================

print(f"\n🔎 Starting Optuna search ({OPTUNA_N_TRIALS} trials)...")
optuna.logging.set_verbosity(optuna.logging.WARNING)

def objective(trial):
    params = {
        "n_estimators"      : trial.suggest_int  ("n_estimators",      50, 200),
        "max_depth"         : trial.suggest_int  ("max_depth",         3, 10),
        "min_samples_split" : trial.suggest_int  ("min_samples_split", 2, 20),
        "min_samples_leaf"  : trial.suggest_int  ("min_samples_leaf",  1, 10),
        "max_features"      : trial.suggest_float("max_features",      0.1, 0.8),
        "bootstrap"         : trial.suggest_categorical("bootstrap",   [True, False]),
        "n_jobs"            : -1,
        "random_state"      : 42,
    }

    clf = RandomForestClassifier(**params)
    clf.fit(X_tune, y_tune, sample_weight=w_tune)

    auc_train = roc_auc_score(
        y_tune, clf.predict_proba(X_tune)[:, 1], sample_weight=w_tune
    )
    auc_test = roc_auc_score(
        y_tune_test, clf.predict_proba(X_tune_test)[:, 1], sample_weight=w_tune_test
    )

    gap = abs(auc_train - auc_test)
    return auc_test - OPTUNA_ALPHA * gap


study = optuna.create_study(direction="maximize")
study.optimize(objective, n_trials=OPTUNA_N_TRIALS, show_progress_bar=True)

print(f"\n✅ Optuna best penalised score : {study.best_value:.4f}")
print(f"✅ Optuna best params          : {study.best_params}")

# ============================================
# Plot Optuna optimisation history
# ============================================

trials      = [t.number for t in study.trials]
scores      = [t.value  for t in study.trials]
best_so_far = np.maximum.accumulate(scores)

plt.figure(figsize=(9, 5))
plt.scatter(trials, scores,      s=15, alpha=0.5, color="steelblue", label="Trial penalised score")
plt.plot   (trials, best_so_far, color="tomato",  linewidth=2,       label="Best score so far")
plt.xlabel ("Trial Number",               fontsize=13)
plt.ylabel ("AUC − α·|Train−Test gap|",   fontsize=13)
plt.title  ("tW vs $t\\bar{t}$ — Optuna Search History (1j1t, $e\\mu$)", fontsize=13)
plt.legend (fontsize=12)
plt.grid()
plt.tight_layout()
plt.savefig("plots/roc_classifier/optuna_search_history_1j1t_emu.png", dpi=150)
plt.close()
print("📈 Optuna search history saved")

# ============================================
# Train final RF on full training set
# ============================================

print("\n🚀 Training final Random Forest on full training set with best hyperparameters...")

model = RandomForestClassifier(
    **study.best_params,
    n_jobs       = -1,
    random_state = 42,
)

model.fit(X_train, y_train, sample_weight=w_train)

# ============================================
# Predict scores — must come before all
# downstream diagnostics and plots
# ============================================

y_pred_test  = model.predict_proba(X_test)[:, 1]
y_pred_train = model.predict_proba(X_train)[:, 1]

# ============================================
# Diagnostic: score distribution sanity check
# ============================================

print("\n📊 Score stats on TEST set:")
print(f"   min    : {y_pred_test.min():.4f}")
print(f"   max    : {y_pred_test.max():.4f}")
print(f"   mean   : {y_pred_test.mean():.4f}")
print(f"   median : {np.median(y_pred_test):.4f}")
print(f"\n   Fraction of test events with score > 0.5 : {(y_pred_test > 0.5).mean():.4f}")
print(f"   Fraction of test events with score > 0.9 : {(y_pred_test > 0.9).mean():.4f}")
print(f"\n   Score mean by true class:")
print(f"   Signal (y=1)     : {y_pred_test[y_test == 1].mean():.4f}")
print(f"   Background (y=0) : {y_pred_test[y_test == 0].mean():.4f}")

# ============================================
# OOB score (only when bootstrap=True)
# ============================================

if study.best_params.get("bootstrap", True):
    model_oob = RandomForestClassifier(
        **study.best_params,
        oob_score    = True,
        n_jobs       = -1,
        random_state = 42,
    )
    model_oob.fit(X_train, y_train, sample_weight=w_train)
    print(f"\n🌲 OOB score (accuracy proxy) : {model_oob.oob_score_:.4f}")

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
ax.set_title(
    "tW vs $t\\bar{t}$ — Confusion Matrix (1j1t, $e\\mu$)",
    fontsize=12
)
plt.tight_layout()
plt.savefig("plots/confusion_matrix/confusion_matrix_1j1t_emu.png", dpi=150)
plt.close()
print("📊 Confusion matrix saved")

# ============================================
# RF score distribution (signal vs background)
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

plt.xlabel("RF Score",          fontsize=13)
plt.ylabel("Normalised Events", fontsize=13)
plt.title ("tW vs $t\\bar{t}$ — RF Score Distribution (1j1t, $e\\mu$)", fontsize=13)
plt.legend(fontsize=11)
plt.grid()
plt.tight_layout()
plt.savefig("plots/rf_output/rf_score_distribution_1j1t_emu.png", dpi=150)
plt.close()
print("📊 RF score distribution saved")

# ============================================
# Feature importance (MDI, normalised)
# ============================================

importances   = model.feature_importances_
sorted_idx    = np.argsort(importances)
sorted_names  = np.array(input_vars)[sorted_idx]
sorted_values = importances[sorted_idx]

plt.figure(figsize=(8, 6))
plt.barh(sorted_names, sorted_values, color="steelblue")
plt.xlabel("Mean Decrease in Impurity (normalised)", fontsize=13)
plt.title ("tW vs $t\\bar{t}$ — Feature Importance by MDI (1j1t, $e\\mu$)", fontsize=13)
plt.tight_layout()
plt.savefig("plots/feature_importance/feature_importance_mdi_1j1t_emu.png", dpi=150)
plt.close()
print("📌 Feature importance plot saved")

# ============================================
# Save model
# ============================================

joblib.dump(model, "RF_1j1t.pkl")
print("\n💾 Model saved as RF_1j1t.pkl")
