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
from sklearn.metrics import (
    roc_curve, roc_auc_score,
    confusion_matrix, ConfusionMatrixDisplay
)
from sklearn.preprocessing import label_binarize

# ============================================
# GPU detection
# ============================================

try:
    test_model = xgb.XGBClassifier(tree_method="hist", device="cuda")
    GPU_AVAILABLE = True
    print("\n🚀 CUDA GPU detected -> using GPU training")
except Exception:
    GPU_AVAILABLE = False
    print("\n💻 No CUDA GPU detected -> using CPU")

XGB_DEVICE = "cuda" if GPU_AVAILABLE else "cpu"

# ============================================
# Class label mapping
#   0 → tW signal  (TWminusto2L2Nu + TbarWplusto2L2Nu)
#   1 → TTbar Dileptonic
#   2 → TTbar Semi-Leptonic
# ============================================

CLASS_NAMES = {
    0: "Signal (tW)",
    1: r"Bkg 1 ($t\bar{t}$ Dilept)",
    2: r"Bkg 2 ($t\bar{t}$ SemiLept)",
}

CLASS_COLORS = {
    0: "steelblue",
    1: "tomato",
    2: "forestgreen",
}

region_configs = {
    "2j1t": {
        "signal": [
            "../../../merged/TWminusto2L2Nu.root",
            "../../../merged/TbarWplusto2L2Nu.root",
        ],
        "background_dilept": [
            "../../../merged/TTbar_Dilept.root",
        ],
        "background_semilept": [
            "../../../merged/TTbar_SemiLept.root",
        ],
        "region_flag":  "region_2j1t",
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
# Sampling / Optuna configuration
# ============================================

SAMPLE_FRACTION      = 0.5
OPTUNA_TUNE_FRACTION = 0.15
OPTUNA_N_TRIALS      = 40
OPTUNA_ALPHA         = 1.0   # Penalty for train-test AUC gap

# ============================================
# Output directories
# ============================================

for d in [
    "plots/roc_classifier",
    "plots/confusion_matrix",
    "plots/feature_importance",
    "plots/bdt_output",
    "plots/correlation_matrix",
]:
    os.makedirs(d, exist_ok=True)

# ============================================
# Helper: load ROOT files into a DataFrame
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
            step_size="100 MB",
        ):
            mask = (arrays[region_flag] == True) & (arrays[channel_flag] == True)
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
                    sys.exit(1)

            df_chunk = pd.DataFrame(data_dict)
            df_chunk["label"] = label
            dfs.append(df_chunk)

        print(f"   → Events after selection: {total_events}")

    return pd.concat(dfs, ignore_index=True)

# ============================================
# Load data (3 classes)
# ============================================

cfg = region_configs["2j1t"]

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
    print(f"\n✂️  Sub-sampled to {SAMPLE_FRACTION*100:.0f}% of each class:")
    print(f"   Signal events     : {len(df_sig)}")
    print(f"   Bkg1 events       : {len(df_bkg1)}")
    print(f"   Bkg2 events       : {len(df_bkg2)}")

df = pd.concat([df_sig, df_bkg1, df_bkg2], ignore_index=True)
print("\nTotal events before cleaning:", len(df))

# ============================================
# Cleaning
# ============================================

df = df.replace([np.inf, -np.inf], np.nan).dropna()
print("Total events after  cleaning:", len(df))

# ============================================
# Features / labels / weights
# ============================================

X = df[input_vars]
y = df["label"]
w = df["evWeight"]

mask = w > 0
X, y, w = X[mask], y[mask], w[mask]

print(f"\n⚖️  Events after positive-weight filter: {len(X)}")

# ============================================
# Correlation matrices (per class)
# ============================================

corr_sig  = df_sig[input_vars].corr(method="pearson")
corr_bkg1 = df_bkg1[input_vars].corr(method="pearson")
corr_bkg2 = df_bkg2[input_vars].corr(method="pearson")

# ============================================
# Train / test split
# ============================================

X_train, X_test, y_train, y_test, w_train, w_test = train_test_split(
    X, y, w, test_size=0.3, random_state=42, stratify=y
)

# ============================================
# Weight normalisation — equalise class sums
#   Scale each class so its weight sum equals
#   the largest class weight sum in training.
# ============================================

w_train = w_train.copy()
w_test  = w_test.copy()

class_sums_train = {c: w_train[y_train == c].sum() for c in [0, 1, 2]}
max_sum = max(class_sums_train.values())

print(f"\n⚖️  Weight normalisation (equalise classes to max sum = {max_sum:.3f}):")
for c, s in class_sums_train.items():
    scale = max_sum / s
    print(f"   Class {c} ({CLASS_NAMES[c]}): sum={s:.3f}  scale={scale:.3f}")
    w_train[y_train == c] *= scale
    w_test[y_test   == c] *= scale

print("\n⚖️  After normalisation:")
for c in [0, 1, 2]:
    print(f"   Class {c} weight sum (train): {w_train[y_train == c].sum():.3f}")

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

print(f"\n🔬 Optuna tuning subset: {len(X_tune)} train / {len(X_tune_test)} test events")

# ============================================
# Optuna objective
# ============================================

print(f"\n🔎 Starting Optuna search ({OPTUNA_N_TRIALS} trials)...")
optuna.logging.set_verbosity(optuna.logging.WARNING)


def _macro_ovr_auc(y_true, proba, sample_weight):
    """Weighted macro-average OvR AUC across 3 classes."""
    y_bin = label_binarize(y_true, classes=[0, 1, 2])
    aucs = []
    for c in range(3):
        aucs.append(
            roc_auc_score(y_bin[:, c], proba[:, c], sample_weight=sample_weight)
        )
    return float(np.mean(aucs))


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
        "objective"             : "multi:softprob",
        "num_class"             : 3,
        "eval_metric"           : "mlogloss",
        "tree_method"           : "hist",
        "device"                : XGB_DEVICE,
        "n_jobs"                : -1,
        "early_stopping_rounds" : 20,
    }

    clf = xgb.XGBClassifier(**params)
    clf.fit(
        X_tune, y_tune,
        sample_weight          = w_tune,
        eval_set               = [(X_tune_test, y_tune_test)],
        sample_weight_eval_set = [w_tune_test],
        verbose                = False,
    )

    proba_train = clf.predict_proba(X_tune)
    proba_test  = clf.predict_proba(X_tune_test)

    auc_train = _macro_ovr_auc(y_tune,      proba_train, w_tune)
    auc_test  = _macro_ovr_auc(y_tune_test, proba_test,  w_tune_test)

    gap = abs(auc_train - auc_test)
    return auc_test - OPTUNA_ALPHA * gap


study = optuna.create_study(direction="maximize")
study.optimize(objective, n_trials=OPTUNA_N_TRIALS, show_progress_bar=True)

print(f"\n✅ Optuna best penalised score : {study.best_value:.4f}")
print(f"✅ Optuna best params          : {study.best_params}")

# ============================================
# Optuna history plot
# ============================================

trials      = [t.number for t in study.trials]
scores      = [t.value  for t in study.trials]
best_so_far = np.maximum.accumulate(scores)

plt.figure(figsize=(9, 5))
plt.scatter(trials, scores,      s=15, alpha=0.5, color="steelblue", label="Trial penalised score")
plt.plot   (trials, best_so_far, color="tomato",  linewidth=2,       label="Best score so far")
plt.xlabel("Trial Number",                          fontsize=13)
plt.ylabel(r"Macro OvR AUC $-$ $\alpha$·|Train$-$Test gap|", fontsize=13)
plt.title (r"tW vs $t\bar{t}$ Multiclass — Optuna History (2j1t, $e\mu$)", fontsize=13)
plt.legend(fontsize=12)
plt.grid()
plt.tight_layout()
plt.savefig("plots/roc_classifier/optuna_search_history_2j1t_emu.png", dpi=150)
plt.close()
print("📈 Optuna search history saved")

# ============================================
# Train final model on full training set
# ============================================

print("\n🚀 Training final multiclass BDT on full training set...")

final_params = {k: v for k, v in study.best_params.items() if k != "early_stopping_rounds"}

model = xgb.XGBClassifier(
    **final_params,
    objective    = "multi:softprob",
    num_class    = 3,
    eval_metric  = ["mlogloss", "merror"],
    tree_method  = "hist",
    device       = XGB_DEVICE,
    n_jobs       = -1,
)

model.fit(
    X_train, y_train,
    sample_weight          = w_train,
    eval_set               = [(X_train, y_train), (X_test, y_test)],
    sample_weight_eval_set = [w_train, w_test],
    verbose                = 50,
)

# ============================================
# Loss / error curves
# ============================================

results = model.evals_result()
epochs  = len(results["validation_0"]["mlogloss"])
x_axis  = range(epochs)

fig, axes = plt.subplots(1, 2, figsize=(14, 5))

axes[0].plot(x_axis, results["validation_0"]["mlogloss"], label="Train")
axes[0].plot(x_axis, results["validation_1"]["mlogloss"], label="Test")
axes[0].set_xlabel("Boosting Round", fontsize=12)
axes[0].set_ylabel("Multiclass Log Loss", fontsize=12)
axes[0].set_title(r"tW vs $t\bar{t}$ Multiclass — Log Loss (2j1t, $e\mu$)", fontsize=12)
axes[0].legend(fontsize=11)
axes[0].grid()

axes[1].plot(x_axis, results["validation_0"]["merror"], label="Train")
axes[1].plot(x_axis, results["validation_1"]["merror"], label="Test")
axes[1].set_xlabel("Boosting Round", fontsize=12)
axes[1].set_ylabel("Classification Error", fontsize=12)
axes[1].set_title(r"tW vs $t\bar{t}$ Multiclass — Error (2j1t, $e\mu$)", fontsize=12)
axes[1].legend(fontsize=11)
axes[1].grid()

plt.tight_layout()
plt.savefig("plots/roc_classifier/loss_error_vs_boosting_round_2j1t_emu.png", dpi=150)
plt.close()
print("📈 Loss and error curves saved")

# ============================================
# Predict probabilities
# ============================================

proba_test  = model.predict_proba(X_test)   # shape (N, 3)
proba_train = model.predict_proba(X_train)  # shape (N, 3)

y_test_bin  = label_binarize(y_test,  classes=[0, 1, 2])
y_train_bin = label_binarize(y_train, classes=[0, 1, 2])

# ============================================
# Per-class OvR AUC
# ============================================

print("\n" + "="*50)
print("Per-class One-vs-Rest AUC")
print("="*50)

auc_per_class_test  = {}
auc_per_class_train = {}

for c in range(3):
    auc_test_c  = roc_auc_score(y_test_bin[:,  c], proba_test[:,  c], sample_weight=w_test)
    auc_train_c = roc_auc_score(y_train_bin[:, c], proba_train[:, c], sample_weight=w_train)
    auc_per_class_test[c]  = auc_test_c
    auc_per_class_train[c] = auc_train_c
    print(f"  Class {c} ({CLASS_NAMES[c]}):  Train AUC={auc_train_c:.4f}  |  Test AUC={auc_test_c:.4f}")

macro_auc_test  = np.mean(list(auc_per_class_test.values()))
macro_auc_train = np.mean(list(auc_per_class_train.values()))
print(f"\n  Macro-average OvR AUC  →  Train={macro_auc_train:.4f}  |  Test={macro_auc_test:.4f}")

# ============================================
# Helper: compute OvR ROC curve for one class
# ============================================

def ovr_roc(y_bin_col, proba_col, sample_weight):
    fpr, tpr, _ = roc_curve(y_bin_col, proba_col, sample_weight=sample_weight)
    auc = roc_auc_score(y_bin_col, proba_col, sample_weight=sample_weight)
    return fpr, tpr, auc

# ============================================
# Helper: pairwise ROC for class A vs class B
#   Uses only the events belonging to A or B.
#   "Positive" = class A (score = P(A) / (P(A)+P(B))).
# ============================================

def pairwise_roc(y_true, proba, w, class_a, class_b):
    """
    Compute ROC curve and AUC for class_a vs class_b.
    Only events from those two classes are used.
    The score is the re-normalised probability of class_a.
    """
    mask = (y_true == class_a) | (y_true == class_b)
    y_pair = (y_true[mask] == class_a).astype(int)  # 1=A, 0=B
    w_pair = w[mask]
    # Re-normalise: score = P(A) / (P(A) + P(B))
    p_a = proba[mask, class_a]
    p_b = proba[mask, class_b]
    score = p_a / (p_a + p_b + 1e-12)
    fpr, tpr, _ = roc_curve(y_pair, score, sample_weight=w_pair)
    auc = roc_auc_score(y_pair, score, sample_weight=w_pair)
    return fpr, tpr, auc


# ============================================
# ROC Plot 1 — Pairwise 3-subplot figure
#   Panel 1: tW vs TTbar Dilept
#   Panel 2: tW vs TTbar SemiLept
#   Panel 3: TTbar Dilept vs TTbar SemiLept
# ============================================

pairwise_configs = [
    {
        "class_a"  : 0,
        "class_b"  : 1,
        "title"    : r"tW vs $t\bar{t}$ Dilept (2j1t, $e\mu$)",
        "color_a"  : CLASS_COLORS[0],
        "label_a"  : "tW",
        "label_b"  : r"$t\bar{t}$ Dilept",
    },
    {
        "class_a"  : 0,
        "class_b"  : 2,
        "title"    : r"tW vs $t\bar{t}$ SemiLept (2j1t, $e\mu$)",
        "color_a"  : CLASS_COLORS[0],
        "label_a"  : "tW",
        "label_b"  : r"$t\bar{t}$ SemiLept",
    },
    {
        "class_a"  : 1,
        "class_b"  : 2,
        "title"    : r"$t\bar{t}$ Dilept vs $t\bar{t}$ SemiLept (2j1t, $e\mu$)",
        "color_a"  : CLASS_COLORS[1],
        "label_a"  : r"$t\bar{t}$ Dilept",
        "label_b"  : r"$t\bar{t}$ SemiLept",
    },
]

fig, axes = plt.subplots(1, 3, figsize=(19, 6))

for ax, cfg_pair in zip(axes, pairwise_configs):
    ca   = cfg_pair["class_a"]
    cb   = cfg_pair["class_b"]
    col  = cfg_pair["color_a"]

    for split, y_s, proba_s, w_s, ls in [
        ("Train", y_train.values, proba_train, w_train.values, "--"),
        ("Test",  y_test.values,  proba_test,  w_test.values,  "-"),
    ]:
        fpr, tpr, auc = pairwise_roc(y_s, proba_s, w_s, ca, cb)
        ax.plot(fpr, tpr, color=col, linestyle=ls,
                label=f"{split}  AUC = {auc:.3f}")

    ax.plot([0, 1], [0, 1], "k--", linewidth=0.8, label="Random")
    ax.set_xlabel(
        f"FPR  [{cfg_pair['label_b']} efficiency]",
        fontsize=11
    )
    ax.set_ylabel(
        f"TPR  [{cfg_pair['label_a']} efficiency]",
        fontsize=11
    )
    ax.set_title(cfg_pair["title"], fontsize=11)
    ax.legend(fontsize=10)
    ax.grid()

plt.suptitle(
    r"Pairwise ROC Curves — tW vs $t\bar{t}$ Dilept vs $t\bar{t}$ SemiLept (2j1t, $e\mu$)",
    fontsize=13
)
plt.tight_layout()
plt.savefig("plots/roc_classifier/roc_pairwise_3panels_2j1t_emu.png", dpi=150, bbox_inches="tight")
plt.close()
print("📈 Pairwise ROC (3-panel) plot saved")

# ============================================
# ROC Plot 2 — All 3 OvR curves on one axes
#   (combined plot, train dashed / test solid)
# ============================================

plt.figure(figsize=(9, 6))

for c in range(3):
    for split, y_bin, proba, w_s, ls in [
        ("Train", y_train_bin, proba_train, w_train, "--"),
        ("Test",  y_test_bin,  proba_test,  w_test,  "-"),
    ]:
        fpr, tpr, auc = ovr_roc(y_bin[:, c], proba[:, c], w_s)
        lbl = f"{CLASS_NAMES[c].split('(')[1].rstrip(')')} {split} AUC={auc:.3f}"
        plt.plot(fpr, tpr, color=CLASS_COLORS[c], linestyle=ls, label=lbl)

plt.plot([0, 1], [0, 1], "k--", linewidth=0.8, label="Random")
plt.xlabel("False Positive Rate (1 − Specificity)", fontsize=13)
plt.ylabel("True Positive Rate (Sensitivity)",      fontsize=13)
plt.title(
    r"tW vs $t\bar{t}$ Dilept vs $t\bar{t}$ SemiLept — All OvR ROC (2j1t, $e\mu$)",
    fontsize=12
)
plt.legend(fontsize=10)
plt.grid()
plt.tight_layout()
plt.savefig("plots/roc_classifier/roc_all_ovr_combined_2j1t_emu.png", dpi=150)
plt.close()
print("📈 Combined OvR ROC plot saved")

# ============================================
# Confusion matrix (threshold = argmax)
# ============================================

y_pred_test_label = np.argmax(proba_test, axis=1)

cm = confusion_matrix(y_test, y_pred_test_label, sample_weight=w_test)

fig, ax = plt.subplots(figsize=(7, 6))
disp = ConfusionMatrixDisplay(
    cm,
    display_labels=[
        "Signal (tW)",
        r"$t\bar{t}$ Dilept",
        r"$t\bar{t}$ SemiLept",
    ],
)
disp.plot(ax=ax, colorbar=True, cmap="Blues")
ax.set_title(
    r"tW vs $t\bar{t}$ Multiclass — Confusion Matrix (2j1t, $e\mu$)",
    fontsize=12
)
plt.tight_layout()
plt.savefig("plots/confusion_matrix/confusion_matrix_multiclass_2j1t_emu.png", dpi=150)
plt.close()
print("📊 Confusion matrix saved")

# ============================================
# BDT score distribution — all 3 classes
# (one subplot per class probability output)
# ============================================

bins = np.linspace(0, 1, 41)

fig, axes = plt.subplots(1, 3, figsize=(18, 5))

for c in range(3):
    ax = axes[c]

    for cls_label, color, train_proba, test_proba, w_tr, w_te in [
        (CLASS_NAMES[0], CLASS_COLORS[0], proba_train[y_train == 0, c], proba_test[y_test == 0, c],
         w_train[y_train == 0].values, w_test[y_test == 0].values),
        (CLASS_NAMES[1], CLASS_COLORS[1], proba_train[y_train == 1, c], proba_test[y_test == 1, c],
         w_train[y_train == 1].values, w_test[y_test == 1].values),
        (CLASS_NAMES[2], CLASS_COLORS[2], proba_train[y_train == 2, c], proba_test[y_test == 2, c],
         w_train[y_train == 2].values, w_test[y_test == 2].values),
    ]:
        ax.hist(train_proba, bins=bins, density=True, histtype="step",
                weights=w_tr, color=color, linewidth=1.5, label=f"Train — {cls_label}")
        ax.hist(test_proba, bins=bins, density=True, histtype="stepfilled", alpha=0.35,
                weights=w_te, color=color, label=f"Test — {cls_label}")

    ax.set_xlabel(f"P(class = {c})", fontsize=12)
    ax.set_ylabel("Normalised Events",  fontsize=12)
    ax.set_title(f"Output Probability for {CLASS_NAMES[c]}", fontsize=11)
    ax.legend(fontsize=9)
    ax.grid()

plt.suptitle(
    r"tW vs $t\bar{t}$ Dilept vs $t\bar{t}$ SemiLept — BDT Score Distributions (2j1t, $e\mu$)",
    fontsize=13
)
plt.tight_layout()
plt.savefig("plots/bdt_output/bdt_score_distributions_multiclass_2j1t_emu.png", dpi=150)
plt.close()
print("📊 BDT score distributions (3 panels) saved")

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
plt.title(
    r"tW vs $t\bar{t}$ Multiclass — Feature Importance by Gain (2j1t, $e\mu$)",
    fontsize=12
)
plt.tight_layout()
plt.savefig("plots/feature_importance/feature_importance_gain_multiclass_2j1t_emu.png", dpi=150)
plt.close()
print("📌 Feature importance plot saved")

# ============================================
# Correlation matrices (signal, bkg1, bkg2)
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


plot_corr_matrix(
    corr_sig,
    "Signal (tW) — Input Variable Correlation Matrix",
    "plots/correlation_matrix/signal_correlation_matrix_2j1t_emu.png",
)
print("📊 Signal correlation matrix saved")

plot_corr_matrix(
    corr_bkg1,
    r"$t\bar{t}$ Dileptonic — Input Variable Correlation Matrix",
    "plots/correlation_matrix/bkg1_dilept_correlation_matrix_2j1t_emu.png",
)
print("📊 Bkg1 (Dilept) correlation matrix saved")

plot_corr_matrix(
    corr_bkg2,
    r"$t\bar{t}$ Semi-Leptonic — Input Variable Correlation Matrix",
    "plots/correlation_matrix/bkg2_semilept_correlation_matrix_2j1t_emu.png",
)
print("📊 Bkg2 (SemiLept) correlation matrix saved")

# ============================================
# Save model
# ============================================

model.save_model("BDT_multiclass_2j1t.json")
print("\n💾 Model saved as BDT_multiclass_2j1t.json")
print("\n✅ All done!")
