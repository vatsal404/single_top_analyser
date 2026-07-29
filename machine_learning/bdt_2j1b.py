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
# Classification mode configuration
#
#   "multiclass" → 3-class discrimination
#       0 = tW signal
#       1 = TTbar Dileptonic
#       2 = TTbar Semi-Leptonic
#
#   "binary" → 2-class discrimination
#       0 = tW signal
#       1 = dominant background only (choose via BINARY_BACKGROUND)
#
#   BINARY_BACKGROUND is only used when CLASSIFICATION_MODE == "binary".
#   Set it to whichever background you consider dominant in the region/
#   channel you are training on:
#       "background_dilept"     → TTbar Dileptonic
#       "background_semilept"   → TTbar Semi-Leptonic
# ============================================

CLASSIFICATION_MODE = "binary"          # "binary" or "multiclass"
BINARY_BACKGROUND   = "background_dilept"

if CLASSIFICATION_MODE not in ("binary", "multiclass"):
    sys.exit(f"❌ Unknown CLASSIFICATION_MODE: {CLASSIFICATION_MODE!r} (use 'binary' or 'multiclass')")

if CLASSIFICATION_MODE == "binary" and BINARY_BACKGROUND not in ("background_dilept", "background_semilept"):
    sys.exit(f"❌ Unknown BINARY_BACKGROUND: {BINARY_BACKGROUND!r}")

region_configs = {
    "2j1t": {
        "signal": [
            "/eos/uscms/store/user/vsinha/Result_2024/systemetics/TWminusto2L2Nu/TWminusto2L2Nu_nominal.root",
            "/eos/uscms/store/user/vsinha/Result_2024/systemetics/TbarWplusto2L2Nu/TbarWplusto2L2Nu_nominal.root",
        ],
        "background_dilept": [
            "/eos/uscms/store/user/vsinha/Result_2024/systemetics/TTbar_Dilept/TTbar_Dilept_nominal.root",
        ],
        "background_semilept": [
            "/eos/uscms/store/user/vsinha/combined/TTbar_SemiLept/TTbar_SemiLept_nominal.root",
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
#    "aplanery",
    "delR_leadinglepton_jet",
    "delR_ele_muon",
    "subleading_lepton_pt",
#    "HT",
    "ST",
    "MT2",
    "centrality",
    "lepton_pt_asymmetry",
#    "PuppiMET_pt_corr",
#    "PuppiMET_phi_corr",
    "mbl_min",
    "delR_dilepton_jet",
    "Selected_jet_subleading_pt",
#    "max_delR",
    "min_delR",
#    "jet_pt_assymmetry",
    "jet_pt_ratio",
#    "Selected_loosejet_leadingbtag",
#    "Selected_jet_leading_btag",
#    "Selected_jet_subleading_btag"
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
            arrays = arrays[mask]
            n_selected = len(arrays)
            total_events += n_selected

            if n_selected == 0:
                continue

            data_dict = {}
            for var in input_vars + ["evWeight", "event"]:
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
# Class configuration (depends on CLASSIFICATION_MODE)
#
#   classes_to_load : dict  label -> (name, file_list)
#   CLASS_NAMES     : dict  label -> pretty name (for titles/legends)
#   CLASS_COLORS    : dict  label -> plot color
#   CLASS_LIST      : sorted list of labels, e.g. [0, 1] or [0, 1, 2]
#   NUM_CLASSES     : len(CLASS_LIST)
#   MODEL_PREFIX    : prefix used for saved model filenames
#   MODE_TAG        : short tag folded into plot filenames
# ============================================

cfg = region_configs["2j1t"]

if CLASSIFICATION_MODE == "binary":
    bkg_pretty = r"$t\bar{t}$ Dilept" if BINARY_BACKGROUND == "background_dilept" else r"$t\bar{t}$ SemiLept"
    classes_to_load = {
        0: ("signal", cfg["signal"]),
        1: (BINARY_BACKGROUND, cfg[BINARY_BACKGROUND]),
    }
    CLASS_NAMES = {
        0: "Signal (tW)",
        1: f"Background ({bkg_pretty})",
    }
    CLASS_COLORS = {
        0: "steelblue",
        1: "tomato",
    }
    MODEL_PREFIX = "BDT_binary"
    MODE_TAG     = f"binary_{BINARY_BACKGROUND}"
else:
    classes_to_load = {
        0: ("signal", cfg["signal"]),
        1: ("background_dilept", cfg["background_dilept"]),
        2: ("background_semilept", cfg["background_semilept"]),
    }
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
    MODEL_PREFIX = "BDT_multiclass"
    MODE_TAG     = "multiclass"

CLASS_LIST  = sorted(classes_to_load.keys())
NUM_CLASSES = len(CLASS_LIST)

print(f"\n🏷️  Classification mode : {CLASSIFICATION_MODE.upper()}")
for c in CLASS_LIST:
    print(f"   Class {c}: {CLASS_NAMES[c]}")

# ============================================
# Load data
# ============================================

dfs_by_class = {}
for label, (name, file_list) in classes_to_load.items():
    print(f"\n================ Loading {CLASS_NAMES[label]} (label={label}) =================")
    dfs_by_class[label] = load_files(file_list, label, cfg["region_flag"], cfg["channel_flag"])
    print(f"Raw events for class {label} ({CLASS_NAMES[label]}): {len(dfs_by_class[label])}")

# ============================================
# Sub-sample (preserve class ratios)
# ============================================

if SAMPLE_FRACTION < 1.0:
    print(f"\n✂️  Sub-sampling to {SAMPLE_FRACTION*100:.0f}% of each class:")
    for label in CLASS_LIST:
        dfs_by_class[label] = dfs_by_class[label].sample(
            frac=SAMPLE_FRACTION, random_state=42
        ).reset_index(drop=True)
        print(f"   Class {label} ({CLASS_NAMES[label]}) events: {len(dfs_by_class[label])}")

df = pd.concat([dfs_by_class[c] for c in CLASS_LIST], ignore_index=True)
print("\nTotal events before cleaning:", len(df))

# ============================================
# Cleaning
# ============================================

df = df.replace([np.inf, -np.inf], np.nan).dropna()
print("Total events after  cleaning:", len(df))

# ============================================
# Features / labels / weights — positive weights only
# ============================================

mask = df["evWeight"] > 0
df = df[mask].reset_index(drop=True)
print(f"\n⚖️  Events after positive-weight filter: {len(df)}")

# ============================================
# Odd / Even split into two independent DataFrames
#
#   df_even — only events where event % 2 == 0
#   df_odd  — only events where event % 2 == 1
#
#   Each BDT is trained AND tested entirely
#   within its own parity subset using a
#   standard 70/30 train_test_split.
#
#   You then apply:
#     even BDT  →  odd events   (fit template)
#     odd BDT   →  even events  (fit template)
#
# ============================================

df_even = df[df["event"] % 2 == 0].reset_index(drop=True)
df_odd  = df[df["event"] % 2 == 1].reset_index(drop=True)

print(f"\n🔀 Odd/Even event split:")
print(f"   Even events : {len(df_even)}")
print(f"   Odd events  : {len(df_odd)}")

# ============================================
# Correlation matrices (per class, full dataset)
# ============================================

corr_by_class = {c: dfs_by_class[c][input_vars].corr(method="pearson") for c in CLASS_LIST}

# ============================================
# Helper: normalise class weights
# ============================================

def normalise_weights(w, y):
    w = w.copy()
    class_sums = {c: w[y == c].sum() for c in CLASS_LIST}
    max_sum = max(class_sums.values())
    print(f"   Weight normalisation (max sum = {max_sum:.3f}):")
    for c, s in class_sums.items():
        scale = max_sum / s
        print(f"     Class {c} ({CLASS_NAMES[c]}): sum={s:.3f}  scale={scale:.3f}")
        w[y == c] *= scale
    return w

# ============================================
# Helper: AUC used as the Optuna objective score
#   - binary   -> single ROC AUC on the positive class
#   - multiclass -> macro-averaged One-vs-Rest AUC
# ============================================

def score_auc(y_true, proba, sample_weight):
    if NUM_CLASSES == 2:
        return float(roc_auc_score(y_true, proba[:, 1], sample_weight=sample_weight))
    y_bin = label_binarize(y_true, classes=CLASS_LIST)
    aucs = [
        roc_auc_score(y_bin[:, i], proba[:, i], sample_weight=sample_weight)
        for i in range(NUM_CLASSES)
    ]
    return float(np.mean(aucs))

# ============================================
# Helper: OvR / binary ROC curve
# ============================================

def ovr_roc(y_bin_col, proba_col, sample_weight):
    fpr, tpr, _ = roc_curve(y_bin_col, proba_col, sample_weight=sample_weight)
    auc = roc_auc_score(y_bin_col, proba_col, sample_weight=sample_weight)
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
# XGBoost objective / eval-metric names for this mode
# ============================================

if NUM_CLASSES == 2:
    XGB_OBJECTIVE  = "binary:logistic"
    XGB_EVAL_METRIC = ["logloss", "error"]
    LOSS_KEY, ERROR_KEY = "logloss", "error"
else:
    XGB_OBJECTIVE  = "multi:softprob"
    XGB_EVAL_METRIC = ["mlogloss", "merror"]
    LOSS_KEY, ERROR_KEY = "mlogloss", "merror"

# ============================================
# Main loop — one fully independent BDT per parity
#
#   fold="even":
#     df_parity = df_even
#     70% train / 30% test, all from even events
#
#   fold="odd":
#     df_parity = df_odd
#     70% train / 30% test, all from odd events
# ============================================

FOLDS = {
    "even": df_even,
    "odd":  df_odd,
}

for fold_name, df_parity in FOLDS.items():
    tag = f"{MODE_TAG}_2j1t_emu_{fold_name}"
    print(f"\n{'='*60}")
    print(f"  FOLD: {fold_name.upper()}  —  {len(df_parity)} events  ({CLASSIFICATION_MODE})")
    print(f"  Train/test split entirely within {fold_name} events")
    print(f"{'='*60}")

    X = df_parity[input_vars]
    y = df_parity["label"]
    w = df_parity["evWeight"]

    # ------------------------------------------
    # 70 / 30 train–test split within this parity
    # ------------------------------------------

    X_train, X_test, y_train, y_test, w_train, w_test = train_test_split(
        X, y, w, test_size=0.3, random_state=42, stratify=y
    )

    X_train = X_train.reset_index(drop=True)
    X_test  = X_test.reset_index(drop=True)
    y_train = y_train.reset_index(drop=True)
    y_test  = y_test.reset_index(drop=True)
    w_train = w_train.reset_index(drop=True)
    w_test  = w_test.reset_index(drop=True)

    print(f"\n   Train events : {len(X_train)}")
    print(f"   Test events  : {len(X_test)}")

    # ------------------------------------------
    # Weight normalisation
    # ------------------------------------------

    print(f"\n⚖️  Normalising train weights [{fold_name}]:")
    w_train = normalise_weights(w_train, y_train)

    print(f"\n⚖️  Normalising test weights [{fold_name}]:")
    w_test = normalise_weights(w_test, y_test)

    print(f"\n⚖️  After normalisation [{fold_name}]:")
    for c in CLASS_LIST:
        print(f"   Class {c} weight sum (train): {w_train[y_train == c].sum():.3f}")

    # ------------------------------------------
    # Optuna subsample (from within this fold's training set)
    # ------------------------------------------

    sss = StratifiedShuffleSplit(
        n_splits=1, test_size=(1.0 - OPTUNA_TUNE_FRACTION), random_state=42
    )
    tune_idx, _ = next(sss.split(X_train, y_train))

    X_tune = X_train.iloc[tune_idx].reset_index(drop=True)
    y_tune = y_train.iloc[tune_idx].reset_index(drop=True)
    w_tune = w_train.iloc[tune_idx].reset_index(drop=True)

    sss_test = StratifiedShuffleSplit(
        n_splits=1, test_size=(1.0 - OPTUNA_TUNE_FRACTION), random_state=42
    )
    tune_test_idx, _ = next(sss_test.split(X_test, y_test))

    X_tune_test = X_test.iloc[tune_test_idx].reset_index(drop=True)
    y_tune_test = y_test.iloc[tune_test_idx].reset_index(drop=True)
    w_tune_test = w_test.iloc[tune_test_idx].reset_index(drop=True)

    print(f"\n🔬 Optuna tuning subset [{fold_name}]: "
          f"{len(X_tune)} train / {len(X_tune_test)} test events")

    # ------------------------------------------
    # Optuna objective
    # ------------------------------------------

    print(f"\n🔎 Starting Optuna search [{fold_name}] ({OPTUNA_N_TRIALS} trials)...")
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
            "objective"             : XGB_OBJECTIVE,
            "eval_metric"           : XGB_EVAL_METRIC,
            "tree_method"           : "hist",
            "device"                : XGB_DEVICE,
            "n_jobs"                : -1,
            "early_stopping_rounds" : 20,
        }
        if NUM_CLASSES > 2:
            params["num_class"] = NUM_CLASSES

        clf = xgb.XGBClassifier(**params)
        clf.fit(
            X_tune, y_tune,
            sample_weight          = w_tune,
            eval_set               = [(X_tune_test, y_tune_test)],
            sample_weight_eval_set = [w_tune_test],
            verbose                = False,
        )

        proba_tr = clf.predict_proba(X_tune)
        proba_te = clf.predict_proba(X_tune_test)

        auc_train = score_auc(y_tune,      proba_tr, w_tune)
        auc_test  = score_auc(y_tune_test, proba_te, w_tune_test)

        gap = abs(auc_train - auc_test)
        return auc_test - OPTUNA_ALPHA * gap

    study = optuna.create_study(direction="maximize")
    study.optimize(objective, n_trials=OPTUNA_N_TRIALS, show_progress_bar=True)

    print(f"\n✅ Optuna best penalised score [{fold_name}]: {study.best_value:.4f}")
    print(f"✅ Optuna best params          [{fold_name}]: {study.best_params}")

    # ------------------------------------------
    # Optuna history plot
    # ------------------------------------------

    trials      = [t.number for t in study.trials]
    scores      = [t.value  for t in study.trials]
    best_so_far = np.maximum.accumulate(scores)

    plt.figure(figsize=(9, 5))
    plt.scatter(trials, scores,      s=15, alpha=0.5, color="steelblue", label="Trial penalised score")
    plt.plot   (trials, best_so_far, color="tomato",  linewidth=2,       label="Best score so far")
    plt.xlabel("Trial Number",                          fontsize=13)
    plt.ylabel(r"AUC $-$ $\alpha$·|Train$-$Test gap|", fontsize=13)
    plt.title(
        rf"tW vs $t\bar{{t}}$ — Optuna History (2j1t, $e\mu$, {fold_name}, {CLASSIFICATION_MODE})",
        fontsize=13
    )
    plt.legend(fontsize=12)
    plt.grid()
    plt.tight_layout()
    plt.savefig(f"plots/roc_classifier/optuna_search_history_{tag}.png", dpi=150)
    plt.close()
    print(f"📈 Optuna search history saved [{fold_name}]")

    # ------------------------------------------
    # Train final model on full training set of this parity
    # ------------------------------------------

    print(f"\n🚀 Training final {CLASSIFICATION_MODE} BDT [{fold_name}] on full {fold_name} training set...")

    final_params = {k: v for k, v in study.best_params.items() if k != "early_stopping_rounds"}

    model_kwargs = dict(
        **final_params,
        objective    = XGB_OBJECTIVE,
        eval_metric  = XGB_EVAL_METRIC,
        tree_method  = "hist",
        device       = XGB_DEVICE,
        n_jobs       = -1,
    )
    if NUM_CLASSES > 2:
        model_kwargs["num_class"] = NUM_CLASSES

    model = xgb.XGBClassifier(**model_kwargs)

    model.fit(
        X_train, y_train,
        sample_weight          = w_train,
        eval_set               = [(X_train, y_train), (X_test, y_test)],
        sample_weight_eval_set = [w_train, w_test],
        verbose                = 50,
    )

    # ------------------------------------------
    # Loss / error curves
    # ------------------------------------------

    results = model.evals_result()
    epochs  = len(results["validation_0"][LOSS_KEY])
    x_axis  = range(epochs)

    fig, axes = plt.subplots(1, 2, figsize=(14, 5))

    axes[0].plot(x_axis, results["validation_0"][LOSS_KEY], label="Train")
    axes[0].plot(x_axis, results["validation_1"][LOSS_KEY], label="Test")
    axes[0].set_xlabel("Boosting Round", fontsize=12)
    axes[0].set_ylabel("Log Loss" if NUM_CLASSES == 2 else "Multiclass Log Loss", fontsize=12)
    axes[0].set_title(
        rf"tW vs $t\bar{{t}}$ — Log Loss (2j1t, $e\mu$, {fold_name})", fontsize=12
    )
    axes[0].legend(fontsize=11)
    axes[0].grid()

    axes[1].plot(x_axis, results["validation_0"][ERROR_KEY], label="Train")
    axes[1].plot(x_axis, results["validation_1"][ERROR_KEY], label="Test")
    axes[1].set_xlabel("Boosting Round", fontsize=12)
    axes[1].set_ylabel("Classification Error", fontsize=12)
    axes[1].set_title(
        rf"tW vs $t\bar{{t}}$ — Error (2j1t, $e\mu$, {fold_name})", fontsize=12
    )
    axes[1].legend(fontsize=11)
    axes[1].grid()

    plt.tight_layout()
    plt.savefig(f"plots/roc_classifier/loss_error_vs_boosting_round_{tag}.png", dpi=150)
    plt.close()
    print(f"📈 Loss and error curves saved [{fold_name}]")

    # ------------------------------------------
    # Predict probabilities
    # ------------------------------------------

    proba_test  = model.predict_proba(X_test)
    proba_train = model.predict_proba(X_train)

    if NUM_CLASSES == 2:
        # keep everything downstream working off 2-column "one-hot-like"
        # arrays so the rest of the pipeline is mode-agnostic
        y_test_bin  = np.column_stack([1 - y_test.values,  y_test.values])
        y_train_bin = np.column_stack([1 - y_train.values, y_train.values])
    else:
        y_test_bin  = label_binarize(y_test,  classes=CLASS_LIST)
        y_train_bin = label_binarize(y_train, classes=CLASS_LIST)

    # ------------------------------------------
    # Per-class OvR AUC (for binary this is just Signal-AUC == Bkg-AUC)
    # ------------------------------------------

    print(f"\n{'='*50}")
    print(f"Per-class AUC [{fold_name}]")
    print(f"{'='*50}")

    auc_per_class_test  = {}
    auc_per_class_train = {}

    for i, c in enumerate(CLASS_LIST):
        auc_test_c  = roc_auc_score(y_test_bin[:,  i], proba_test[:,  i], sample_weight=w_test)
        auc_train_c = roc_auc_score(y_train_bin[:, i], proba_train[:, i], sample_weight=w_train)
        auc_per_class_test[c]  = auc_test_c
        auc_per_class_train[c] = auc_train_c
        print(f"  Class {c} ({CLASS_NAMES[c]}):  Train AUC={auc_train_c:.4f}  |  Test AUC={auc_test_c:.4f}")

    macro_auc_test  = np.mean(list(auc_per_class_test.values()))
    macro_auc_train = np.mean(list(auc_per_class_train.values()))
    auc_label = "AUC" if NUM_CLASSES == 2 else "Macro-average OvR AUC"
    print(f"\n  {auc_label}  →  Train={macro_auc_train:.4f}  |  Test={macro_auc_test:.4f}")

    # ------------------------------------------
    # ROC curves
    #   binary     -> single ROC curve (signal vs background)
    #   multiclass -> two-panel (signal-vs-rest, then both backgrounds)
    # ------------------------------------------

    if NUM_CLASSES == 2:
        plt.figure(figsize=(7, 6))
        for split, y_bin, proba, w_s, ls in [
            ("Train", y_train_bin, proba_train, w_train, "--"),
            ("Test",  y_test_bin,  proba_test,  w_test,  "-"),
        ]:
            fpr, tpr, auc = ovr_roc(y_bin[:, 1], proba[:, 1], w_s)
            plt.plot(fpr, tpr, color=CLASS_COLORS[0], linestyle=ls, label=f"{split} AUC = {auc:.3f}")
        plt.plot([0, 1], [0, 1], "k--", linewidth=0.8, label="Random")
        plt.xlabel("False Positive Rate (1 − Specificity)", fontsize=12)
        plt.ylabel("True Positive Rate (Sensitivity)",      fontsize=12)
        plt.title(
            rf"Signal (tW) vs {CLASS_NAMES[1]} — ROC (2j1t, $e\mu$, {fold_name})", fontsize=12
        )
        plt.legend(fontsize=11)
        plt.grid()
        plt.tight_layout()
        plt.savefig(f"plots/roc_classifier/roc_signal_vs_bkg_{tag}.png", dpi=150, bbox_inches="tight")
        plt.close()
        print(f"📈 ROC plot saved [{fold_name}]")
    else:
        fig, axes = plt.subplots(1, 2, figsize=(14, 6))

        ax = axes[0]
        for split, y_bin, proba, w_s, ls in [
            ("Train", y_train_bin, proba_train, w_train, "--"),
            ("Test",  y_test_bin,  proba_test,  w_test,  "-"),
        ]:
            fpr, tpr, auc = ovr_roc(y_bin[:, 0], proba[:, 0], w_s)
            ax.plot(fpr, tpr, color=CLASS_COLORS[0], linestyle=ls, label=f"{split} AUC = {auc:.3f}")
        ax.plot([0, 1], [0, 1], "k--", linewidth=0.8, label="Random")
        ax.set_xlabel("False Positive Rate (1 − Specificity)", fontsize=12)
        ax.set_ylabel("True Positive Rate (Sensitivity)",      fontsize=12)
        ax.set_title(rf"Signal (tW) vs Rest — OvR ROC (2j1t, $e\mu$, {fold_name})", fontsize=12)
        ax.legend(fontsize=11)
        ax.grid()

        ax = axes[1]
        for c, lbl in [(1, "Bkg1 (Dilept)"), (2, "Bkg2 (SemiLept)")]:
            for split, y_bin, proba, w_s, ls in [
                ("Train", y_train_bin, proba_train, w_train, "--"),
                ("Test",  y_test_bin,  proba_test,  w_test,  "-"),
            ]:
                fpr, tpr, auc = ovr_roc(y_bin[:, c], proba[:, c], w_s)
                ax.plot(fpr, tpr, color=CLASS_COLORS[c], linestyle=ls,
                        label=f"{lbl} {split} AUC={auc:.3f}")
        ax.plot([0, 1], [0, 1], "k--", linewidth=0.8, label="Random")
        ax.set_xlabel("False Positive Rate (1 − Specificity)", fontsize=12)
        ax.set_ylabel("True Positive Rate (Sensitivity)",      fontsize=12)
        ax.set_title(rf"Background OvR ROC Curves (2j1t, $e\mu$, {fold_name})", fontsize=12)
        ax.legend(fontsize=10)
        ax.grid()

        plt.suptitle(
            rf"tW vs $t\bar{{t}}$ — Multiclass OvR ROC (2j1t, $e\mu$, {fold_name})",
            fontsize=13, y=1.01
        )
        plt.tight_layout()
        plt.savefig(f"plots/roc_classifier/roc_signal_vs_bkg_panels_{tag}.png", dpi=150, bbox_inches="tight")
        plt.close()
        print(f"📈 ROC panel plot saved [{fold_name}]")

        # Combined OvR ROC (multiclass only — for binary the single plot above already shows it)
        plt.figure(figsize=(9, 6))
        for c in CLASS_LIST:
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
        plt.title(rf"tW vs $t\bar{{t}}$ — All OvR ROC (2j1t, $e\mu$, {fold_name})", fontsize=12)
        plt.legend(fontsize=10)
        plt.grid()
        plt.tight_layout()
        plt.savefig(f"plots/roc_classifier/roc_all_ovr_combined_{tag}.png", dpi=150)
        plt.close()
        print(f"📈 Combined OvR ROC plot saved [{fold_name}]")

    # ------------------------------------------
    # Confusion matrix
    # ------------------------------------------

    y_pred_test_label = np.argmax(proba_test, axis=1)
    # map argmax index (0..NUM_CLASSES-1) back to actual CLASS_LIST labels
    y_pred_test_label = np.array([CLASS_LIST[i] for i in y_pred_test_label])
    cm = confusion_matrix(y_test, y_pred_test_label, labels=CLASS_LIST, sample_weight=w_test)

    fig, ax = plt.subplots(figsize=(7, 6) if NUM_CLASSES > 2 else (6, 5))
    disp = ConfusionMatrixDisplay(
        cm,
        display_labels=[CLASS_NAMES[c] for c in CLASS_LIST],
    )
    disp.plot(ax=ax, colorbar=True, cmap="Blues")
    ax.set_title(
        rf"tW vs $t\bar{{t}}$ — Confusion Matrix (2j1t, $e\mu$, {fold_name})", fontsize=12
    )
    plt.tight_layout()
    plt.savefig(f"plots/confusion_matrix/confusion_matrix_{tag}.png", dpi=150)
    plt.close()
    print(f"📊 Confusion matrix saved [{fold_name}]")

    # ------------------------------------------
    # BDT score distributions
    # ------------------------------------------

    bins = np.linspace(0, 1, 41)
    n_panels = NUM_CLASSES
    fig, axes = plt.subplots(1, n_panels, figsize=(6 * n_panels, 5))
    if n_panels == 1:
        axes = [axes]

    for i, c_target in enumerate(CLASS_LIST):
        ax = axes[i]
        for c_true in CLASS_LIST:
            cls_label = CLASS_NAMES[c_true]
            color     = CLASS_COLORS[c_true]
            tr_p = proba_train[y_train == c_true, i]
            te_p = proba_test[y_test == c_true, i]
            w_tr = w_train[y_train == c_true].values
            w_te = w_test[y_test == c_true].values

            ax.hist(tr_p, bins=bins, density=True, histtype="step",
                    weights=w_tr, color=color, linewidth=1.5, label=f"Train — {cls_label}")
            ax.hist(te_p, bins=bins, density=True, histtype="stepfilled", alpha=0.35,
                    weights=w_te, color=color, label=f"Test — {cls_label}")
        ax.set_xlabel(f"P(class = {c_target})", fontsize=12)
        ax.set_ylabel("Normalised Events",  fontsize=12)
        ax.set_title(f"Output Probability for {CLASS_NAMES[c_target]}", fontsize=11)
        ax.legend(fontsize=9)
        ax.grid()

    plt.suptitle(
        rf"tW vs $t\bar{{t}}$ — BDT Score Distributions (2j1t, $e\mu$, {fold_name})",
        fontsize=13
    )
    plt.tight_layout()
    plt.savefig(f"plots/bdt_output/bdt_score_distributions_{tag}.png", dpi=150)
    plt.close()
    print(f"📊 BDT score distributions saved [{fold_name}]")

    # ------------------------------------------
    # Feature importance
    # ------------------------------------------

    importance = model.get_booster().get_score(importance_type="gain")
    total_gain = sum(importance.values())
    normalised = {k: v / total_gain for k, v in importance.items()}
    sorted_imp = dict(sorted(normalised.items(), key=lambda x: x[1]))

    plt.figure(figsize=(8, 6))
    plt.barh(list(sorted_imp.keys()), list(sorted_imp.values()), color="steelblue")
    plt.xlabel("Normalised Mean Gain (fraction of total)", fontsize=13)
    plt.title(
        rf"tW vs $t\bar{{t}}$ — Feature Importance (2j1t, $e\mu$, {fold_name})", fontsize=12
    )
    plt.tight_layout()
    plt.savefig(f"plots/feature_importance/feature_importance_gain_{tag}.png", dpi=150)
    plt.close()
    print(f"📌 Feature importance plot saved [{fold_name}]")

    # ------------------------------------------
    # Save model
    # ------------------------------------------

    model_path = f"{MODEL_PREFIX}_2j1t_{fold_name}.json"
    model.save_model(model_path)
    print(f"\n💾 Model saved as {model_path}")

# ============================================
# Correlation matrices (once, shared across folds)
# ============================================

for c in CLASS_LIST:
    safe_name = CLASS_NAMES[c].replace(" ", "_").replace("(", "").replace(")", "").replace("$", "").replace("\\", "")
    plot_corr_matrix(
        corr_by_class[c],
        f"{CLASS_NAMES[c]} — Input Variable Correlation Matrix",
        f"plots/correlation_matrix/{safe_name}_correlation_matrix_2j1t_emu_{MODE_TAG}.png",
    )
    print(f"📊 {CLASS_NAMES[c]} correlation matrix saved")

# ============================================
# Summary
# ============================================

print("\n" + "="*60)
print(f"✅  All done! ({CLASSIFICATION_MODE.upper()} mode)")
print("   Models saved:")
print(f"     {MODEL_PREFIX}_2j1t_even.json")
print("       → trained & tested on even events only")
print(f"     {MODEL_PREFIX}_2j1t_odd.json")
print("       → trained & tested on odd events only")
print()
print("   To extract fit templates, apply externally:")
print("     even BDT  →  score odd  events")
print("     odd BDT   →  score even events")
print("="*60)
