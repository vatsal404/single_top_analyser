import uproot
import pandas as pd
import numpy as np
from sklearn.metrics import roc_auc_score

# Paths to your signal and background files
signal_files = [
  #  "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tchannel_top_5f.root",
   # "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tchannel_antitop_5f.root"
    # Add more signal files if needed
"/uscms/home/vsinha/nobackup/CMSSW_12_3_4/src/fly/signal.root"
    ]

background_files = [
#    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_0J_UL17.root",
 #   "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_1J_UL17.root",
  #  "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_2J_UL17.root"  # Add more background files if needed
#"/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_TTbar-channel_top_UL17.root"
"/uscms/home/vsinha/nobackup/CMSSW_12_3_4/src/fly/tt_bar.root" 
]

# Tree name
tree_name = "outputTree"

# List of variables to load
variables = [
    "bdt_wboson_muon_2j1t",
     "bdt_top_muon_2j1t",
     "bdt_specJet_2j1t_leading_eta_muon",
     "bdt_delR_muon_2j1t",
     "bdt_deltaEta_muon_2j1t",
     "bdt_eventShape_muon_2j1t",
     "bdt_WHelicity_muon_2j1t",
     "bdt_MET_pt_muon_2j1t"
]

# Function to load data from multiple files
def load_data(files, variables, tree_name):
    data = []
    for file in files:
        with uproot.open(file) as f:
            # Load both the variable data and the event weight (evWeight) branch
            df = f[tree_name].arrays(variables + ["evWeight"], library="pd")
            data.append(df)
    return pd.concat(data, ignore_index=True)

# Load signal and background data
signal_df = load_data(signal_files, variables, tree_name)
background_df = load_data(background_files, variables, tree_name)

# Print the first 100 entries before removing NaNs
print("\n=== First 100 entries of Signal (Before NaN Removal) ===")
print(signal_df.head(100))

print("\n=== First 100 entries of Background (Before NaN Removal) ===")
print(background_df.head(100))

# Drop NaNs across ALL variables at once for both signal and background
#signal_df = signal_df.dropna(subset=variables)
#background_df = background_df.dropna(subset=variables)

mask = (signal_df[variables] == -999).any(axis=1)
signal_df = signal_df[~mask]

mask_bg = (background_df[variables] == -999).any(axis=1)
background_df = background_df[~mask_bg]


# Print the first 100 entries after removing NaNs
print("\n=== First 100 entries of Signal (After NaN Removal) ===")
print(signal_df.head(100))

print("\n=== First 100 entries of Background (After NaN Removal) ===")
print(background_df.head(100))

# Report how much data survived the NaN removal
print(f"Signal entries after NaN removal: {len(signal_df)}")
print(f"Background entries after NaN removal: {len(background_df)}")

# Subsample after NaN removal to match counts
target_size = min(len(signal_df), len(background_df))
signal_df = signal_df.sample(n=target_size, random_state=42)
background_df = background_df.sample(n=target_size, random_state=42)

# To store results
results = []

for var in variables:
    signal_values = signal_df[var]
    background_values = background_df[var]
    signal_weights = signal_df["evWeight"]
    background_weights = background_df["evWeight"]

    # Apply the weights to the signal and background values
    weighted_signal_values = signal_values * signal_weights
    weighted_background_values = background_values * background_weights

    # Number of final entries
    num_signal_entries = len(weighted_signal_values)
    num_background_entries = len(weighted_background_values)

    # Combine for AUC calculation
    combined_values = pd.concat([weighted_signal_values, weighted_background_values], ignore_index=True)
    combined_labels = [1] * num_signal_entries + [0] * num_background_entries

    # Compute AUC
    auc = roc_auc_score(combined_labels, combined_values)

    # Discrimination power in %
    discrimination_power = (auc - 0.5) * 200

    results.append({
        "Variable": var,
        "Signal Entries (after NaN+subsampling)": num_signal_entries,
        "Background Entries (after NaN+subsampling)": num_background_entries,
        "AUC": auc,
        "Discrimination Power (%)": discrimination_power
    })

# Sort by discrimination power
results = sorted(results, key=lambda x: -x["Discrimination Power (%)"])

# Print final table
print("\n=== Variable Discrimination Power Report (After NaN Removal + Subsampling) ===")
print(f"{'Variable':<40}{'Signal':<30}{'Background':<30}{'AUC':<8}{'Power (%)':<12}")
print("-" * 120)
for res in results:
    print(f"{res['Variable']:<40}{res['Signal Entries (after NaN+subsampling)']:<30}{res['Background Entries (after NaN+subsampling)']:<30}{res['AUC']:.4f}   {res['Discrimination Power (%)']:.2f}")

# Optional save
# pd.DataFrame(results).to_csv("discrimination_report_post_nan_and_subsampling.csv", index=False)

