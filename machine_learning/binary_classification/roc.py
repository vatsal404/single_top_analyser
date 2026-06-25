import uproot
import numpy as np
import matplotlib.pyplot as plt
from sklearn.metrics import roc_curve, roc_auc_score

# =============================
# CONFIG
# =============================

tree_name = "outputTree"

variables = [
    "sphericity",
    "aplanery",
    "centrality",
    "leading_lepton_pt",
    "leptons_invariant_mass",
    "dilepton_jet_pt",
    "Selected_loosejet_leadingpt",
    "dilepton_del_phi",
    "Selected_jet_leading_pt",
    "leading_lepton_jet_pt",
    "dilepton_jet_mass",
    "subleading_lepton_pt",
    "delR_dilepton_jet",
    "Selected_jet_subleading_pt",
    "delR_ele_muon",
    "delR_leadinglepton_jet",
]

region_cut_branch = {
    "1j1t": "region_1j1t",
    "2j1t": "region_2j1t",
}

channel_branch = "eu_channel"  # "" to disable

region_configs = {
    "1j1t": {
        "signal": [
            "../merged/TWminusto2L2Nu.root",
            "../merged/TbarWplusto2L2Nu.root",
        ],
        "background": [
            "../merged/TTbar_Dilept.root",
        ],
    },
    "2j1t": {
        "signal": [
            "../merged/TWminusto2L2Nu.root",
            "../merged/TbarWplusto2L2Nu.root",
        ],
        "background": [
            "../merged/TTbar_Dilept.root",
        ],
    },
}

# =============================
# HELPERS
# =============================

def load_var(files, var, region, label):
    vals = []

    for f in files:
        with uproot.open(f) as file:
            tree = file[tree_name]

            branches = [var, region_cut_branch[region]]
            if channel_branch:
                branches.append(channel_branch)

            arr = tree.arrays(branches, library="np")

            mask = arr[region_cut_branch[region]].astype(bool)
            if channel_branch:
                mask &= arr[channel_branch].astype(bool)

            x = arr[var][mask]
            vals.append(x)

    x_all = np.concatenate(vals)
    y_all = np.full(len(x_all), label)

    return x_all, y_all


# =============================
# MAIN
# =============================

for region in ["1j1t", "2j1t"]:
    print(f"\n========== REGION {region} ==========")

    sig_files = region_configs[region]["signal"]
    bkg_files = region_configs[region]["background"]

    for var in variables:
        try:
            x_sig, y_sig = load_var(sig_files, var, region, 1)
            x_bkg, y_bkg = load_var(bkg_files, var, region, 0)

            x = np.concatenate([x_sig, x_bkg])
            y = np.concatenate([y_sig, y_bkg])

            # =============================
            # 🔥 REMOVE NaN/Inf EVENTS HERE
            # =============================
            mask = np.isfinite(x)
            x = x[mask]
            y = y[mask]

            # Safety check
            if len(np.unique(y)) < 2:
                print(f"{var:35s} SKIPPED (only one class after cleaning)")
                continue

            fpr, tpr, _ = roc_curve(y, x)
            auc = roc_auc_score(y, x)

            print(f"{var:35s} AUC = {auc:.4f}  (N = {len(x)})")

            plt.figure()
            plt.plot(fpr, tpr)
            plt.plot([0, 1], [0, 1], "--")
            plt.xlabel("FPR")
            plt.ylabel("TPR")
            plt.title(f"{var} | {region} | AUC={auc:.3f}")
            plt.grid(True)
            plt.savefig(f"roc_{region}_{var}.png")
            plt.close()

        except Exception as e:
            print(f"{var:35s} FAILED ({e})")

