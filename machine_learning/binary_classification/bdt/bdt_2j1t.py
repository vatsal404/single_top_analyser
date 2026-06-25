
import uproot
import awkward as ak
import numpy as np
import pandas as pd
import xgboost as xgb
import optuna
import os
import sys
import matplotlib.pyplot as plt

from sklearn.model_selection import train_test_split, StratifiedShuffleSplit
from sklearn.metrics import roc_curve, roc_auc_score, confusion_matrix, ConfusionMatrixDisplay

# ============================================
# Configuration
# ============================================

region_configs = {
    "2j1t": {
        "signal": [
            "../../../merged/TWminusto2L2Nu.root",
            "../../../merged/TbarWplusto2L2Nu.root",
        ],
        "background": [
            "../../../merged/TTbar_Dilept.root",
        ],
        "region_flag": "region_2j1t",
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
    "delR_dilepton_jet",
    "Selected_jet_subleading_pt",
    "max_delR",
    "min_delR",
    "jet_pt_assymmetry",
    "jet_pt_ratio"


]

# ============================================
# Sampling fraction
# — Set to 0.5 to train on 50% of all events.
#   Change to 1.0 to use everything.
# ============================================

SAMPLE_FRACTION = 0.5

# ============================================
# Optuna tuning fraction
# — Fraction of the training set used for
#   hyperparameter search (speeds up Optuna).
#   Final model always trains on the full
#   training set.
# ============================================

OPTUNA_TUNE_FRACTION = 0.15
OPTUNA_N_TRIALS      = 40
OPTUNA_ALPHA         = 1.0   # Penalty for train-test AUC gap

# ============================================
# Output directories
# ============================================

os.makedirs("plots/roc_classifier",     exist_ok=True)
os.makedirs("plots/confusion_matrix",   exist_ok=True)
os.makedirs("plots/feature_importance", exist_ok=True)
os.makedirs("plots/bdt_output",         exist_ok=True)

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

cfg = region_configs["2j1t"]

print("\n================ Loading Signal =================")
df_sig = load_files(cfg["signal"], 1, cfg["region_flag"], cfg["channel_flag"])

print("\n================ Loading Background =================")
df_bkg = load_files(cfg["background"], 0, cfg["region_flag"], cfg["channel_flag"])

print(f"\nRaw signal events     : {len(df_sig)}")
print(f"Raw background events : {len(df_bkg)}")

# ============================================
# Half-sample: draw SAMPLE_FRACTION from
# signal and background independently so
# the class ratio is preserved exactly.
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
# Weight normalization — fix class imbalance
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
# — Draw a stratified subset of the training
#   set for fast hyperparameter search.
#   The final model trains on the full
#   X_train / y_train / w_train.
# ============================================

sss = StratifiedShuffleSplit(n_splits=1, test_size=(1.0 - OPTUNA_TUNE_FRACTION), random_state=42)
tune_idx, _ = next(sss.split(X_train, y_train))

X_tune = X_train.iloc[tune_idx].reset_index(drop=True)
y_tune = y_train.iloc[tune_idx].reset_index(drop=True)
w_tune = w_train.iloc[tune_idx].reset_index(drop=True)

# Mirror test set at the same fraction for a fair Optuna eval set
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
        "n_estimators"          : trial.suggest_int  ("n_estimators",     100, 300),
        "learning_rate"         : trial.suggest_float("learning_rate",    0.005, 0.05, log=True),
        "max_depth"             : trial.suggest_int  ("max_depth",        2, 4),
        "subsample"             : trial.suggest_float("subsample",        0.5, 0.8),
        "colsample_bytree"      : trial.suggest_float("colsample_bytree", 0.4, 0.7),
        "min_child_weight"      : trial.suggest_int  ("min_child_weight", 5, 20),
        "gamma"                 : trial.suggest_float("gamma",            0.5, 3.0),
        "reg_lambda"            : trial.suggest_float("reg_lambda",       1.0, 8.0),
        "objective"             : "binary:logistic",
        "eval_metric"           : "auc",
        "tree_method"           : "hist",   # Much faster on large datasets
        "n_jobs"                : 2,
        "early_stopping_rounds" : 20,
    }

    clf = xgb.XGBClassifier(**params)
    clf.fit(
        X_tune, y_tune,
        sample_weight          = w_tune,
        eval_set               = [(X_tune_test, y_tune_test)],
        sample_weight_eval_set = [w_tune_test],
        verbose                = False
    )

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
plt.title  ("tW vs $t\\bar{t}$ — Optuna Search History (2j1t, $e\\mu$)", fontsize=13)
plt.legend (fontsize=12)
plt.grid()
plt.tight_layout()
plt.savefig("plots/roc_classifier/optuna_search_history_2j1t_emu.png", dpi=150)
plt.close()
print("📈 Optuna search history saved")

# ============================================
# Train final BDT on full training set
# with best Optuna hyperparameters
# ============================================

print("\n🚀 Training final BDT on full training set with best hyperparameters...")

final_params = {k: v for k, v in study.best_params.items() if k != "early_stopping_rounds"}

model = xgb.XGBClassifier(
    **final_params,
    eval_metric  = ["logloss", "auc"],
    objective    = "binary:logistic",
    tree_method  = "hist",
    n_jobs       = 2,
)

model.fit(
    X_train, y_train,
    sample_weight          = w_train,
    eval_set               = [(X_train, y_train), (X_test, y_test)],
    sample_weight_eval_set = [w_train, w_test],
    verbose                = 50
)

# ============================================
# Loss curve (logloss)
# ============================================

results = model.evals_result()
epochs  = len(results["validation_0"]["logloss"])
x_axis  = range(epochs)

plt.figure(figsize=(8, 5))
plt.plot(x_axis, results["validation_0"]["logloss"], label="Train")
plt.plot(x_axis, results["validation_1"]["logloss"], label="Test")
plt.xlabel("Boosting Round", fontsize=13)
plt.ylabel("Log Loss",       fontsize=13)
plt.title ("tW vs $t\\bar{t}$ — XGBoost Log Loss per Boosting Round (2j1t, $e\\mu$)", fontsize=13)
plt.legend(fontsize=12)
plt.grid()
plt.tight_layout()
plt.savefig("plots/roc_classifier/logloss_vs_boosting_round_2j1t_emu.png", dpi=150)
plt.close()
print("📈 Loss curve saved")

# ============================================
# AUC curve
# ============================================

plt.figure(figsize=(8, 5))
plt.plot(x_axis, results["validation_0"]["auc"], label="Train")
plt.plot(x_axis, results["validation_1"]["auc"], label="Test")
plt.xlabel("Boosting Round", fontsize=13)
plt.ylabel("AUC",            fontsize=13)
plt.title ("tW vs $t\\bar{t}$ — XGBoost AUC per Boosting Round (2j1t, $e\\mu$)", fontsize=13)
plt.legend(fontsize=12)
plt.grid()
plt.tight_layout()
plt.savefig("plots/roc_classifier/auc_vs_boosting_round_2j1t_emu.png", dpi=150)
plt.close()
print("📈 AUC curve saved")

# ============================================
# ROC curve (train + test)
# ============================================

y_pred_test  = model.predict_proba(X_test)[:, 1]
y_pred_train = model.predict_proba(X_train)[:, 1]

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
plt.title ("tW vs $t\\bar{t}$ — ROC Curve (2j1t, $e\\mu$)", fontsize=13)
plt.legend(fontsize=12)
plt.grid()
plt.tight_layout()
plt.savefig("plots/roc_classifier/roc_curve_train_vs_test_2j1t_emu.png", dpi=150)
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
    "tW vs $t\\bar{t}$ — Confusion Matrix (2j1t, $e\\mu$)",
    fontsize=12
)
plt.tight_layout()
plt.savefig("plots/confusion_matrix/confusion_matrix_2j1t_emu.png", dpi=150)
plt.close()
print("📊 Confusion matrix saved")

# ============================================
# BDT score distribution (signal vs background)
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

plt.xlabel("BDT Score",         fontsize=13)
plt.ylabel("Normalised Events", fontsize=13)
plt.title ("tW vs $t\\bar{t}$ — BDT Score Distribution (2j1t, $e\\mu$)", fontsize=13)
plt.legend(fontsize=11)
plt.grid()
plt.tight_layout()
plt.savefig("plots/bdt_output/bdt_score_distribution_2j1t_emu.png", dpi=150)
plt.close()
print("📊 BDT score distribution saved")

# ============================================
# Feature importance by gain (normalised)
# ============================================

importance = model.get_booster().get_score(importance_type="gain")
total_gain = sum(importance.values())
normalised = {k: v / total_gain for k, v in importance.items()}
sorted_imp = dict(sorted(normalised.items(), key=lambda x: x[1]))

plt.figure(figsize=(8, 6))
plt.barh(list(sorted_imp.keys()), list(sorted_imp.values()), color="steelblue")
plt.xlabel("Normalised Mean Gain (fraction of total)", fontsize=13)
plt.title ("tW vs $t\\bar{t}$ — Feature Importance by Gain (2j1t, $e\\mu$)", fontsize=13)
plt.tight_layout()
plt.savefig("plots/feature_importance/feature_importance_gain_2j1t_emu.png", dpi=150)
plt.close()
print("📌 Feature importance plot saved")

# ============================================
# Save model
# ============================================

model.save_model("BDT_2j1t.json")
print("\n💾 Model saved as BDT_2j1t.json")
