import uproot
import numpy as np
import pandas as pd
import xgboost as xgb
from tqdm import tqdm
import awkward as ak

INPUT_FILES = [
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tchannel_antitop_5f.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tchannel_top_5f.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_TTbar-channel_top_UL17.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_0J_UL17.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_1J_UL17.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_2J_UL17.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tW_top_5f.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tW_antitop_5f.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_DYJetsToLL_M-50_UL17.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_DYJetsToLL_M-10to50_UL17.root",
    "data.root"
]

OUTPUT_FILE = "combined_bdt_2j1b_results.root"
HIST_FILE = "eta_ele2j1b_histograms.root"
FEATURE_BRANCHES = ["top_mass", "specJet_leading_eta", "bdt_delR", "bdt_deltaEta", "Wboson_transversMass","bdt_WHelicity","bdt_eventShape", "MET_pt_corr"]

WEIGHT_BRANCHES = ["genWeight", "evWeight"]
FILTER_BRANCHES = ["electronChannel", "region_2j1t", "lepton_charge"]
CHUNK_SIZE = 100000
BDT_BINS = 32  # Number of bins
BDT_RANGE = (-200, 200)  # Range for the histogram

LUMINOSITY = 41480.0

CROSS_SECTIONS = {
    "PROCESSED_ST_tchannel_antitop_5f": 80.95,
    "PROCESSED_ST_tchannel_top_5f": 136.02,
    "PROCESSED_TTbar-channel_top_UL17": 831.76,
    "PROCESSED_WJetsToLNu_0J_UL17": 49670.0,
    "PROCESSED_WJetsToLNu_1J_UL17": 8264.0,
    "PROCESSED_WJetsToLNu_2J_UL17": 3226.0,
    "PROCESSED_ST_tW_top_5f": 35.85,
    "PROCESSED_ST_tW_antitop_5f": 35.85,
    "PROCESSED_DYJetsToLL_M-50_UL17": 4895.0,
    "PROCESSED_DYJetsToLL_M-10to50_UL17": 18610.0
}

# Updated sum of genweights from entries.py output
SUM_GEN_WEIGHTS = {
    "PROCESSED_ST_tchannel_antitop_5f": 4462868882.059891,
    "PROCESSED_ST_tchannel_top_5f": 13808000645.662537,
    "PROCESSED_TTbar-channel_top_UL17": 165610016.0,  # Updated
    "PROCESSED_WJetsToLNu_0J_UL17": 9552834599026.219,
    "PROCESSED_WJetsToLNu_1J_UL17": 8064660889430.6875,
    "PROCESSED_WJetsToLNu_2J_UL17": 2350902331638.5547,
    "PROCESSED_ST_tW_top_5f": 183187615.677055,  # Updated
    "PROCESSED_ST_tW_antitop_5f": 184446306.893539,
    "PROCESSED_DYJetsToLL_M-50_UL17": 98921180.0,  # Updated
    "PROCESSED_DYJetsToLL_M-10to50_UL17": 68480179.0
}

# Load XGBoost models
model1 = xgb.Booster()
model1.load_model("xgboost_model_1.model")
model2 = xgb.Booster()
model2.load_model("xgboost_model_2.model")

# Create a simple dictionary to collect all values and weights for each category
# Using only 4 categories as requested plus data
collected_data = {
    "signal_top": {"values": [], "weights": []},
    "signal_antitop": {"values": [], "weights": []},
    "ttbar": {"values": [], "weights": []},  # Will include ttbar and stw
    "wjets": {"values": [], "weights": []},  # Will include wjets and DY
    "data": {"values": []}
}

# Load a small sample to fit the scaler
print("Loading sample data...")
sample_file = INPUT_FILES[0]
with uproot.open(sample_file)["outputTree"] as tree:
    df_sample = tree.arrays(FEATURE_BRANCHES, library="pd")
    X_sample = df_sample[FEATURE_BRANCHES].to_numpy()

# Now process all files in chunks
print("Processing files...")
for file_path in tqdm(INPUT_FILES):
    is_data = "data.root" in file_path
    file_name = file_path.split("/")[-1].replace(".root", "")
    
    try:
        with uproot.open(file_path)["outputTree"] as tree:
            # First check which branches exist in this file
            available_branches = set(branch for branch in tree.keys())
            
            # Create list of branches to read based on what's available
            branches_to_read = []
            for branch in FEATURE_BRANCHES:
                if branch in available_branches:
                    branches_to_read.append(branch)
                else:
                    print(f"Warning: Branch '{branch}' not found in {file_path}")
            
            for branch in FILTER_BRANCHES:
                if branch in available_branches:
                    branches_to_read.append(branch)
                else:
                    print(f"Warning: Branch '{branch}' not found in {file_path}")
            
            # Only add weight branches for MC files
            if not is_data:
                for branch in WEIGHT_BRANCHES:
                    if branch in available_branches:
                        branches_to_read.append(branch)
                    else:
                        print(f"Warning: Branch '{branch}' not found in {file_path}")
            
            # Check if we have the minimum required branches
            required_features = set(FEATURE_BRANCHES)
            required_filters = set(["region_2j1t", "lepton_charge"])
            required_filters_channel = set(["electronChannel"])
            
            missing_features = required_features - set(branches_to_read)
            missing_filters = required_filters - set(branches_to_read)
            has_channel = any(channel in branches_to_read for channel in required_filters_channel)
            
            if missing_features or missing_filters or not has_channel:
                print(f"Skipping {file_path} due to missing required branches")
                continue
                
            total_events = tree.num_entries
            for start in range(0, total_events, CHUNK_SIZE):
                stop = min(start + CHUNK_SIZE, total_events)
                
                # Read only the branches that exist
                df = tree.arrays(branches_to_read, entry_start=start, entry_stop=stop, library="pd")
                
                # Apply the filter depending on available branches
                filter_condition = (df["region_2j1t"] == 1)
                if "muonChannel" in df.columns and "electronChannel" in df.columns:
                    filter_condition &= ((df["muonChannel"] == 1) | (df["electronChannel"] == 1))
                elif "QCDmuonChannel" in df.columns:
                    filter_condition &= (df["QCDmuonChannel"] == 1)
                elif "electronChannel" in df.columns:
                    filter_condition &= (df["electronChannel"] == 1)
                
                df_filtered = df[filter_condition]
                
                if df_filtered.empty:
                    continue
                
                # Extract features and predict
                X = df_filtered[FEATURE_BRANCHES].to_numpy()
                scores = model1.predict(xgb.DMatrix(X))
                
                # Calculate the final values for histogram filling
                mt = df_filtered["Wboson_transversMass"].to_numpy()
                lepton_charge = df_filtered["lepton_charge"].to_numpy()
                
                # Apply logic vectorized
                val = np.where(mt < 50, mt * lepton_charge,
                               np.where(scores < 0.5, lepton_charge * (50 + 100 * model2.predict(xgb.DMatrix(X))),
                                        lepton_charge * (100 + 100 * scores)))
                
                # Collect values (and weights for MC)
                if is_data:
                    collected_data["data"]["values"].extend(val)
                else:
                    sample = next((k for k in CROSS_SECTIONS if k in file_name), None)
                    if sample is None:
                        print(f"Warning: No cross section info for {file_name}")
                        continue
                    
                    # Calculate weights
                    has_gen_weight = "genWeight" in df_filtered.columns
                    has_ev_weight = "evWeight" in df_filtered.columns
                    
                    if has_gen_weight and has_ev_weight:
                        weight = (df_filtered["genWeight"] * df_filtered["evWeight"]).to_numpy()
                    elif has_gen_weight:
                        weight = df_filtered["genWeight"].to_numpy()
                    elif has_ev_weight:
                        weight = df_filtered["evWeight"].to_numpy()
                    else:
                        weight = np.ones(len(df_filtered))
                    
                    scale = (LUMINOSITY * CROSS_SECTIONS[sample]) / SUM_GEN_WEIGHTS[sample]
                    final_weight = weight * scale
                    
                    # Add to the appropriate consolidated category
                    if "ST_tchannel_top" in file_name:
                        category = "signal_top"
                    elif "ST_tchannel_antitop" in file_name:
                        category = "signal_antitop"
                    elif "TTbar" in file_name or "ST_tW" in file_name:  # Combine ttbar and stw
                        category = "ttbar"
                    elif "WJetsToLNu" in file_name or "DYJets" in file_name:  # Combine wjets and dy
                        category = "wjets"
                    else:
                        print(f"Warning: Unknown category for {file_name}")
                        continue
                    
                    collected_data[category]["values"].extend(val)
                    collected_data[category]["weights"].extend(final_weight)
    except Exception as e:
        print(f"Error processing file {file_path}: {e}")
        continue

# Now create the histograms from the collected data
print("Creating histograms from collected data...")
histograms = {}

for category, data in collected_data.items():
    if category == "data":
        if data["values"]:  # Check if there are any values
            counts, edges = np.histogram(
                data["values"], 
                bins=BDT_BINS, 
                range=BDT_RANGE
            )
            histograms[category] = {"counts": counts, "edges": edges}
    else:
        if data["values"] and data["weights"]:  # Check if there are any values and weights
            counts, edges = np.histogram(
                data["values"], 
                bins=BDT_BINS, 
                range=BDT_RANGE, 
                weights=data["weights"]
            )
            histograms[category] = {"counts": counts, "edges": edges}

# Now save the histograms to a ROOT file
print("Saving histograms to ROOT file...")
import ROOT
from array import array

# Create a ROOT file
root_file = ROOT.TFile(HIST_FILE, "RECREATE")

# Create and fill the TH1 histograms
for category, hist_data in histograms.items():
    # Create a histogram
    hist = ROOT.TH1F(category, category, BDT_BINS, BDT_RANGE[0], BDT_RANGE[1])
    
    # Fill the histogram
    for i, count in enumerate(hist_data["counts"]):
        hist.SetBinContent(i+1, count)  # ROOT bins are 1-indexed
    
    # Write the histogram to the file
    hist.Write()

# Close the file
root_file.Close()
print(f"Histograms successfully saved to {HIST_FILE}")
