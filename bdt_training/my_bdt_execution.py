import uproot
import xgboost
import numpy as np
from tqdm import tqdm
import os
import ROOT
from ROOT import TH1D, TFile

# Configuration
MODEL_PATH = "xgboost_model.model"
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

OUTPUT_FILE = "combined_bdt1_electron_results.root"
HIST_FILE = "bdt1_electron_histograms.root"
FEATURE_BRANCHES = ["top_mass", "specJet_leading_eta", "bdt_delR", "bdt_deltaEta", "Wboson_transversMass"]#,"bdt_WHelicity","bdt_eventShape", "MET_pt_corr"]
WEIGHT_BRANCHES = ["genWeight", "evWeight"]
FILTER_BRANCHES = ["electronChannel", "region_2j1t"]
CHUNK_SIZE = 100000
BDT_BINS = 32
BDT_RANGE = (0, 1)

# Normalization parameters
LUMINOSITY = 41480.0  # Integrated luminosity in pb^-1

# Cross-sections and sum of genWeights
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

SUM_GEN_WEIGHTS = {
    "PROCESSED_ST_tchannel_antitop_5f": 4462868882.059891,
    "PROCESSED_ST_tchannel_top_5f": 9090911402.50351,
    "PROCESSED_TTbar-channel_top_UL17": 63817760.0,
    "PROCESSED_WJetsToLNu_0J_UL17": 9552834599026.219,
    "PROCESSED_WJetsToLNu_1J_UL17": 6026429164907.4375,
    "PROCESSED_WJetsToLNu_2J_UL17": 2350902331638.5547,
    "PROCESSED_ST_tW_top_5f": 183284892.384979,
    "PROCESSED_ST_tW_antitop_5f": 184446306.893539,
    "PROCESSED_DYJetsToLL_M-50_UL17": 97436981.0,
    "PROCESSED_DYJetsToLL_M-10to50_UL17": 68480179.0
}

def process_files():
    # Load the XGBoost model
    print("Loading BDT model...")
    model = xgboost.Booster()
    model.load_model(MODEL_PATH)

    # Create TH1D histograms
    signal_hist = TH1D("signal", "Signal (Single Top t-channel);BDT Score;Events", BDT_BINS, BDT_RANGE[0], BDT_RANGE[1])
    ttbar_hist = TH1D("ttbar", "TTbar and tW;BDT Score;Events", BDT_BINS, BDT_RANGE[0], BDT_RANGE[1])
    wjets_hist = TH1D("wjets", "W+Jets and Other;BDT Score;Events", BDT_BINS, BDT_RANGE[0], BDT_RANGE[1])
    data_hist = TH1D("data", "Data;BDT Score;Events", BDT_BINS, BDT_RANGE[0], BDT_RANGE[1])

    # Create output files
    output_file = uproot.recreate(OUTPUT_FILE)
    hist_file = TFile(HIST_FILE, "RECREATE")

    try:
        for input_file in INPUT_FILES:
            file_name = os.path.basename(input_file)
            tree_name = os.path.splitext(file_name)[0]
            sample_key = tree_name.replace("PROCESSED_", "PROCESSED_")

            print(f"\nProcessing {file_name} -> {tree_name}...")

            # Check if this is data
            is_data = file_name == "data.root"

            # Determine sample category
            is_signal = "ST_tchannel" in file_name
            is_ttbar = "TTbar" in file_name or "tW" in file_name
            
            print(f"Sample categorization: Data={is_data}, Signal={is_signal}, TTbar={is_ttbar}")

            # Open input file
            with uproot.open(input_file) as input_file:
                input_tree = input_file["outputTree"]

                # Prepare output data containers
                output_data = {branch: [] for branch in FEATURE_BRANCHES}
                output_data["BDTscore"] = []
                
                # For MC we need weight branches, for data we don't
                if not is_data:
                    for branch in WEIGHT_BRANCHES:
                        output_data[branch] = []
                    output_data["weight"] = []

                # Process in chunks
                required_branches = FILTER_BRANCHES + FEATURE_BRANCHES
                if not is_data:
                    required_branches += WEIGHT_BRANCHES
                
                for chunk in tqdm(input_tree.iterate(
                    library="np",
                    step_size=CHUNK_SIZE,
                    filter_name=required_branches
                ), desc="Processing events"):
                    # Apply event selection
                    mask = (chunk["electronChannel"] == 1) & (chunk["region_2j1t"] == 1)
                    if not np.any(mask):
                        continue

                    # Get filtered features
                    filtered_features = {branch: chunk[branch][mask] for branch in FEATURE_BRANCHES}
                    
                    # Calculate BDT scores
                    features_array = np.column_stack([filtered_features[branch] for branch in FEATURE_BRANCHES])
                    dmatrix = xgboost.DMatrix(features_array)
                    bdt_scores = model.predict(dmatrix)
                    
                    # Handle weights differently for data vs MC
                    if is_data:
                        # Data - no weight, just fill with weight=1.0
                        for score in bdt_scores:
                            data_hist.Fill(score, 1.0)
                    else:
                        # MC - apply normalization and weights
                        # Calculate normalization factor for MC
                        if sample_key not in CROSS_SECTIONS or sample_key not in SUM_GEN_WEIGHTS:
                            raise ValueError(f"Missing cross-section or sum weights for {sample_key}")
                        
                        norm_factor = (LUMINOSITY * CROSS_SECTIONS[sample_key]) / SUM_GEN_WEIGHTS[sample_key]
                        
                        # Get filtered weights
                        filtered_weights = {branch: chunk[branch][mask] for branch in WEIGHT_BRANCHES}
                        weights = norm_factor * filtered_weights["genWeight"] * filtered_weights["evWeight"]
                        
                        # Fill appropriate histogram based on file categorization
                        if is_signal:
                            # Signal (t-channel single top) samples
                           # print(f"Filling signal histogram with {len(bdt_scores)} events")
                            for score, weight in zip(bdt_scores, weights):
                                signal_hist.Fill(score, weight)
                        elif is_ttbar:
                            # TTbar and tW samples
                           # print(f"Filling ttbar histogram with {len(bdt_scores)} events")
                            for score, weight in zip(bdt_scores, weights):
                                ttbar_hist.Fill(score, weight)
                        else:
                            # All other background samples (W+Jets, DY)
                            #print(f"Filling wjets histogram with {len(bdt_scores)} events")
                            for score, weight in zip(bdt_scores, weights):
                                wjets_hist.Fill(score, weight)
                                
                        # Store weight-related fields for MC
                        for branch in WEIGHT_BRANCHES:
                            output_data[branch].append(filtered_weights[branch])
                        output_data["weight"].append(weights)

                    # Store common results for output tree
                    for branch in FEATURE_BRANCHES:
                        output_data[branch].append(filtered_features[branch])
                    output_data["BDTscore"].append(bdt_scores)

                # Combine all chunks if any events passed selection
                if output_data["BDTscore"]:
                    branches_to_concat = FEATURE_BRANCHES + ["BDTscore"]
                    if not is_data:
                        branches_to_concat += WEIGHT_BRANCHES + ["weight"]
                        
                    final_output = {
                        branch: np.concatenate(output_data[branch])
                        for branch in branches_to_concat if branch in output_data
                    }
                    output_file[tree_name] = final_output
                    print(f"Created tree '{tree_name}' with {len(final_output['BDTscore'])} events")
                else:
                    print("No events passed selection criteria")

        # Write histograms to file and print entries
        print(f"\nSignal histogram entries: {signal_hist.GetEntries()}")
        print(f"TTbar histogram entries: {ttbar_hist.GetEntries()}")
        print(f"W+Jets histogram entries: {wjets_hist.GetEntries()}")
        print(f"Data histogram entries: {data_hist.GetEntries()}")
        
        signal_hist.Write()
        ttbar_hist.Write()
        wjets_hist.Write()
        data_hist.Write()
        print(f"\nHistograms saved to {HIST_FILE}")

    finally:
        # Ensure files are properly closed
        output_file.close()
        hist_file.Close()

    print("\nAll files processed successfully!")
    print(f"Results saved to {OUTPUT_FILE}")
    print(f"Histograms saved to {HIST_FILE}")

if __name__ == "__main__":
    process_files()
