import uproot
import numpy as np
import matplotlib.pyplot as plt
import os

# === User Config ===
directory = "merged/"  # Path where all ROOT files are stored

# Full list of variables to plot
variables = [
    "goodElectrons_leading_pt_2j1b", "goodElectrons_leading_pt_2j0b", "goodElectrons_leading_pt_3j2b",
    "goodmuons_leading_pt_2j0b", "goodmuons_leading_pt_2j1b", "goodmuons_leading_pt_3j2b",
    "goodElectrons_leading_eta_2j1b", "goodElectrons_leading_eta_2j0b", "goodElectrons_leading_eta_3j2b",
    "goodmuons_leading_eta_2j0b", "goodmuons_leading_eta_2j1b", "goodmuons_leading_eta_3j2b",
    "goodElectrons_leading_phi_2j1b", "goodElectrons_leading_phi_2j0b", "goodElectrons_leading_phi_3j2b",
    "goodmuons_leading_phi_2j0b", "goodmuons_leading_phi_2j1b", "goodmuons_leading_phi_3j2b",
    "goodjet_e_phi_2j1b", "goodjet_e_phi_2j0b", "goodjet_e_phi_3j2b",
    "goodjet_u_phi_2j1b", "goodjet_u_phi_2j0b", "goodjet_u_phi_3j2b",
    "goodjet_e_eta_2j1b", "goodjet_e_eta_2j0b", "goodjet_e_eta_3j2b",
    "goodjet_u_eta_2j1b", "goodjet_u_eta_2j0b", "goodjet_u_eta_3j2b",
    "goodjet_e_pt_2j1b", "goodjet_e_pt_2j0b", "goodjet_e_pt_3j2b",
    "goodjet_u_pt_2j1b", "goodjet_u_pt_2j0b", "goodjet_u_pt_3j2b",
    "goodMET_e_pt_2j1b", "goodMET_e_pt_2j0b", "goodMET_e_pt_3j2b",
    "goodMET_u_pt_2j1b", "goodMET_u_pt_2j0b", "goodMET_u_pt_3j2b",
    "goodMET_e_phi_2j1b", "goodMET_e_phi_2j0b", "goodMET_e_phi_3j2b",
    "goodMET_u_phi_2j1b", "goodMET_u_phi_2j0b", "goodMET_u_phi_3j2b"
]

# Define file groups (MC + Data)
file_groups = {
    "ttbar": ["TTbar_SemiLept.root", "TTbar_Dilept.root"],
    "wjets": ["WtoLNu.root"],
    "signal": ["TbarBQ_t_channel.root", "TBbarQ_t_channel.root"],
    "drell_yan": ["DYjetsM10to50.root", "DYJetsM50.root"],
    "other": ["ZZto2L2Q.root", "ZZto2LNu.root", "ZZto4L.root", "WWto2L2Nu.root", "WZto3LNu.root",
              "WWW_4F.root", "WWZ_4F.root", "WZZ.root", "ZZZ.root",
              "TTGJets_PTG-10to100.root", "TTGJets_PTG-100to200.root", "TTGJets_PTG-200.root",
              "TTLNu-1Jets.root"],
    "schannel": ["TbarBtoLminusNuB_s_channel.root", "TBbartoLplusNuBbar_s_channel.root"],
    "single_tW": ["TbarWplusto2L2Nu.root", "TWminusto2L2Nu.root",
                  "TbarWplustoLNu2Q.root", "TWminustoLNu2Q.root"]
}

data_file = os.path.join(directory, "data.root")

colors = {
    "ttbar": "red",
    "wjets": "blue",
    "signal": "green",
    "drell_yan": "orange",
    "other": "gray",
    "schannel": "purple",
    "single_tW": "brown"
}

# === Loop over all variables ===
for variable in variables:
    print(f"\n=== Processing variable: {variable} ===")

    # Dynamic binning selection based on variable name
    var_lower = variable.lower()
    if "pt" in var_lower:
        bins = np.linspace(0, 200, 40)  # pT: 0–200 GeV, 40 bins
    elif "eta" in var_lower or "phi" in var_lower:
        bins = np.linspace(-5, 5, 40)  # eta/phi: -5 to 5, 40 bins
    else:
        bins = np.linspace(0, 300, 30)  # default binning

    # Store histograms
    hist_data = {}

    # --- Loop through MC groups ---
    for group_name, files in file_groups.items():
        values = []
        weights = []
        print(f"Processing group: {group_name}")
        for filename in files:
            filepath = os.path.join(directory, filename)
            if not os.path.exists(filepath):
                print(f"  [Warning] File not found: {filepath}")
                continue
            try:
                with uproot.open(filepath) as file:
                    tree = file["outputTree"]
                    available_branches = tree.keys()

                    if variable not in available_branches:
                        print(f"  [Skip] {variable} not in {filename}")
                        continue
                    if "evWeight" not in available_branches:
                        print(f"  [Skip] evWeight not in {filename}")
                        continue

                    arr = tree.arrays([variable, "evWeight"], library="np")
                    if len(arr[variable]) == 0:
                        print(f"  [Skip] No entries in {variable} in {filename}")
                        continue

                    values.append(arr[variable])
                    weights.append(arr["evWeight"])
            except Exception as e:
                print(f"  [Error] Failed to read {filename}: {e}")
                continue

        if values:
            all_vals = np.concatenate(values)
            all_weights = np.concatenate(weights)
            hist, _ = np.histogram(all_vals, bins=bins, weights=all_weights)
            hist_data[group_name] = hist
        else:
            print(f"  [Note] No data collected for group {group_name}")
            hist_data[group_name] = np.zeros(len(bins) - 1)

    # === Load Data ===
    print(f"Processing group: data")
    try:
        with uproot.open(data_file) as file:
            tree = file["outputTree"]
            available_branches = tree.keys()

            if variable not in available_branches:
                print(f"  [Error] Variable {variable} not found in data file.")
                data_hist = np.zeros(len(bins) - 1)
            else:
                arr = tree.arrays([variable], library="np")
                if len(arr[variable]) == 0:
                    print(f"  [Note] No entries in data file.")
                    data_hist = np.zeros(len(bins) - 1)
                else:
                    data_vals = arr[variable]
                    data_hist, _ = np.histogram(data_vals, bins=bins)
    except Exception as e:
        print(f"  [Error] Failed to read data file: {e}")
        data_hist = np.zeros(len(bins) - 1)

    # === Plot Stacked Histogram and Ratio ===
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10), sharex=True,
                                   gridspec_kw={'height_ratios': [3, 1]})

    # --- Stack Plot ---
    bottom = np.zeros_like(bins[:-1])
    for group_name in file_groups.keys():
        ax1.bar(bins[:-1], hist_data[group_name], width=np.diff(bins), bottom=bottom,
                color=colors[group_name], label=group_name, align='edge')
        bottom += hist_data[group_name]

    # Overlay data as points
    bin_centers = 0.5 * (bins[:-1] + bins[1:])
    ax1.errorbar(bin_centers, data_hist, yerr=np.sqrt(data_hist), fmt='o', color='black', label='Data')

    ax1.set_ylabel("Events")
    ax1.set_title(f"Stacked Histogram of {variable}")
    ax1.legend()
    ax1.grid(True)

    # --- Ratio Plot ---
    mc_total = np.sum([hist_data[group] for group in file_groups], axis=0)
    ratio = np.divide(data_hist, mc_total, out=np.zeros_like(data_hist, dtype=float), where=mc_total > 0)
    ratio_err = np.divide(np.sqrt(data_hist), mc_total, out=np.zeros_like(data_hist, dtype=float), where=mc_total > 0)

    ax2.errorbar(bin_centers, ratio, yerr=ratio_err, fmt='o', color='black')
    ax2.axhline(1.0, color='red', linestyle='--')
    ax2.set_ylabel("Data / MC")
    ax2.set_xlabel(variable)
    ax2.grid(True)
    ax2.set_ylim(0, 2)

    plt.tight_layout()
    output_name = f"stacked_ratio_{variable}.png"
    plt.savefig(output_name)
    plt.close(fig)
    print(f" Saved plot with ratio as {output_name}")

