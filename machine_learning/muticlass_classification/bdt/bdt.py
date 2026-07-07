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

corr_sig  = df_sig[input_vars].corr(method="pearson")
corr_bkg1 = df_bkg1[input_vars].corr(method="pearson")
corr_bkg2 = df_bkg2[input_vars].corr(method="pearson")

# ============================================
# Helper: normalise class weights
# ============================================

def normalise_weights(w, y):
    w = w.copy()
    class_sums = {c: w[y == c].sum() for c in [0, 1, 2]}
    max_sum = max(class_sums.values())
    print(f"   Weight normalisation (max sum = {max_sum:.3f}):")
    for c, s in class_sums.items():
        scale = max_sum / s
        print(f"     Class {c} ({CLASS_NAMES[c]}): sum={s:.3f}  scale={scale:.3f}")
        w[y == c] *= scale
    return w

# ============================================
# Helper: macro OvR AUC
# ============================================

def _macro_ovr_auc(y_true, proba, sample_weight):
    y_bin = label_binarize(y_true, classes=[0, 1, 2])
    aucs = [
        roc_auc_score(y_bin[:, c], proba[:, c], sample_weight=sample_weight)
        for c in range(3)
    ]
    return float(np.mean(aucs))

# ============================================
# Helper: OvR ROC curve
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
# Main loop — one fully independent BDT per parity
#
#   fold="even":
#     df_parity = df_even
#     70% train / 30% test, all from even events
#     saved as BDT_multiclass_1j1t_even.json
#
#   fold="odd":
#     df_parity = df_odd
#     70% train / 30% test, all from odd events
#     saved as BDT_multiclass_1j1t_odd.json
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
    for c in [0, 1, 2]:
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

        proba_tr = clf.predict_proba(X_tune)
        proba_te = clf.predict_proba(X_tune_test)

        auc_train = _macro_ovr_auc(y_tune,      proba_tr, w_tune)
        auc_test  = _macro_ovr_auc(y_tune_test, proba_te, w_tune_test)

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
    plt.ylabel(r"Macro OvR AUC $-$ $\alpha$·|Train$-$Test gap|", fontsize=13)
    plt.title(
        rf"tW vs $t\bar{{t}}$ — Optuna History (1j1t, $e\mu$, {fold_name})",
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

    print(f"\n🚀 Training final multiclass BDT [{fold_name}] on full {fold_name} training set...")

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

    # ------------------------------------------
    # Loss / error curves
    # ------------------------------------------

    results = model.evals_result()
    epochs  = len(results["validation_0"]["mlogloss"])
    x_axis  = range(epochs)

    fig, axes = plt.subplots(1, 2, figsize=(14, 5))

    axes[0].plot(x_axis, results["validation_0"]["mlogloss"], label="Train")
    axes[0].plot(x_axis, results["validation_1"]["mlogloss"], label="Test")
    axes[0].set_xlabel("Boosting Round", fontsize=12)
    axes[0].set_ylabel("Multiclass Log Loss", fontsize=12)
    axes[0].set_title(
        rf"tW vs $t\bar{{t}}$ — Log Loss (1j1t, $e\mu$, {fold_name})", fontsize=12
    )
    axes[0].legend(fontsize=11)
    axes[0].grid()

    axes[1].plot(x_axis, results["validation_0"]["merror"], label="Train")
    axes[1].plot(x_axis, results["validation_1"]["merror"], label="Test")
    axes[1].set_xlabel("Boosting Round", fontsize=12)
    axes[1].set_ylabel("Classification Error", fontsize=12)
    axes[1].set_title(
        rf"tW vs $t\bar{{t}}$ — Error (1j1t, $e\mu$, {fold_name})", fontsize=12
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

    y_test_bin  = label_binarize(y_test,  classes=[0, 1, 2])
    y_train_bin = label_binarize(y_train, classes=[0, 1, 2])

    # ------------------------------------------
    # Per-class OvR AUC
    # ------------------------------------------

    print(f"\n{'='*50}")
    print(f"Per-class One-vs-Rest AUC [{fold_name}]")
    print(f"{'='*50}")

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

    # ------------------------------------------
    # ROC panels
    # ------------------------------------------

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
    ax.set_title(rf"Signal (tW) vs Rest — OvR ROC (1j1t, $e\mu$, {fold_name})", fontsize=12)
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
    ax.set_title(rf"Background OvR ROC Curves (1j1t, $e\mu$, {fold_name})", fontsize=12)
    ax.legend(fontsize=10)
    ax.grid()

    plt.suptitle(
        rf"tW vs $t\bar{{t}}$ — Multiclass OvR ROC (1j1t, $e\mu$, {fold_name})",
        fontsize=13, y=1.01
    )
    plt.tight_layout()
    plt.savefig(f"plots/roc_classifier/roc_signal_vs_bkg_panels_{tag}.png", dpi=150, bbox_inches="tight")
    plt.close()
    print(f"📈 ROC panel plot saved [{fold_name}]")

    # ------------------------------------------
    # Combined OvR ROC
    # ------------------------------------------

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
    plt.title(rf"tW vs $t\bar{{t}}$ — All OvR ROC (1j1t, $e\mu$, {fold_name})", fontsize=12)
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
    cm = confusion_matrix(y_test, y_pred_test_label, sample_weight=w_test)

    fig, ax = plt.subplots(figsize=(7, 6))
    disp = ConfusionMatrixDisplay(
        cm,
        display_labels=["Signal (tW)", r"$t\bar{t}$ Dilept", r"$t\bar{t}$ SemiLept"],
    )
    disp.plot(ax=ax, colorbar=True, cmap="Blues")
    ax.set_title(
        rf"tW vs $t\bar{{t}}$ — Confusion Matrix (1j1t, $e\mu$, {fold_name})", fontsize=12
    )
    plt.tight_layout()
    plt.savefig(f"plots/confusion_matrix/confusion_matrix_multiclass_{tag}.png", dpi=150)
    plt.close()
    print(f"📊 Confusion matrix saved [{fold_name}]")

    # ------------------------------------------
    # BDT score distributions
    # ------------------------------------------

    bins = np.linspace(0, 1, 41)
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))

    for c in range(3):
        ax = axes[c]
        for cls_label, color, tr_p, te_p, w_tr, w_te in [
            (CLASS_NAMES[0], CLASS_COLORS[0],
             proba_train[y_train == 0, c], proba_test[y_test == 0, c],
             w_train[y_train == 0].values, w_test[y_test == 0].values),
            (CLASS_NAMES[1], CLASS_COLORS[1],
             proba_train[y_train == 1, c], proba_test[y_test == 1, c],
             w_train[y_train == 1].values, w_test[y_test == 1].values),
            (CLASS_NAMES[2], CLASS_COLORS[2],
             proba_train[y_train == 2, c], proba_test[y_test == 2, c],
             w_train[y_train == 2].values, w_test[y_test == 2].values),
        ]:
            ax.hist(tr_p, bins=bins, density=True, histtype="step",
                    weights=w_tr, color=color, linewidth=1.5, label=f"Train — {cls_label}")
            ax.hist(te_p, bins=bins, density=True, histtype="stepfilled", alpha=0.35,
                    weights=w_te, color=color, label=f"Test — {cls_label}")
        ax.set_xlabel(f"P(class = {c})", fontsize=12)
        ax.set_ylabel("Normalised Events",  fontsize=12)
        ax.set_title(f"Output Probability for {CLASS_NAMES[c]}", fontsize=11)
        ax.legend(fontsize=9)
        ax.grid()

    plt.suptitle(
        rf"tW vs $t\bar{{t}}$ — BDT Score Distributions (1j1t, $e\mu$, {fold_name})",
        fontsize=13
    )
    plt.tight_layout()
    plt.savefig(f"plots/bdt_output/bdt_score_distributions_multiclass_{tag}.png", dpi=150)
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
        rf"tW vs $t\bar{{t}}$ — Feature Importance (1j1t, $e\mu$, {fold_name})", fontsize=12
    )
    plt.tight_layout()
    plt.savefig(f"plots/feature_importance/feature_importance_gain_multiclass_{tag}.png", dpi=150)
    plt.close()
    print(f"📌 Feature importance plot saved [{fold_name}]")

    # ------------------------------------------
    # Save model
    # ------------------------------------------

    model_path = f"BDT_multiclass_1j1t_{fold_name}.json"
    model.save_model(model_path)
    print(f"\n💾 Model saved as {model_path}")

# ============================================
# Correlation matrices (once, shared across folds)
# ============================================

plot_corr_matrix(
    corr_sig,
    "Signal (tW) — Input Variable Correlation Matrix",
    "plots/correlation_matrix/signal_correlation_matrix_1j1t_emu.png",
)
print("📊 Signal correlation matrix saved")

plot_corr_matrix(
    corr_bkg1,
    r"$t\bar{t}$ Dileptonic — Input Variable Correlation Matrix",
    "plots/correlation_matrix/bkg1_dilept_correlation_matrix_1j1t_emu.png",
)
print("📊 Bkg1 (Dilept) correlation matrix saved")

plot_corr_matrix(
    corr_bkg2,
    r"$t\bar{t}$ Semi-Leptonic — Input Variable Correlation Matrix",
    "plots/correlation_matrix/bkg2_semilept_correlation_matrix_1j1t_emu.png",
)
print("📊 Bkg2 (SemiLept) correlation matrix saved")

# ============================================
# Summary
# ============================================

print("\n" + "="*60)
print("✅  All done!")
print("   Models saved:")
print("     BDT_multiclass_1j1t_even.json")
print("       → trained & tested on even events only")
print("     BDT_multiclass_1j1t_odd.json")
print("       → trained & tested on odd events only")
print()
print("   To extract fit templates, apply externally:")
print("     even BDT  →  score odd  events")
print("     odd BDT   →  score even events")
print("="*60)
