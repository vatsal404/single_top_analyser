import uproot
import xgboost
import numpy as np
from tqdm import tqdm
import os

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
    
    ]
OUTPUT_FILE = "combined_bdt_results.root"  # Single output file
FEATURE_BRANCHES = ["top_mass", "specJet_leading_eta", "bdt_delR", "bdt_deltaEta", "Wboson_transversMass"]
FILTER_BRANCHES = ["electronChannel", "region_2j1t"]
CHUNK_SIZE = 100000

def process_files():
    # Load the XGBoost model
    print("Loading BDT model...")
    model = xgboost.Booster()
    model.load_model(MODEL_PATH)
    
    # Create output file
    with uproot.recreate(OUTPUT_FILE) as output_file:
        for input_file in INPUT_FILES:
            file_name = os.path.basename(input_file)
            tree_name = os.path.splitext(file_name)[0]  # Use filename as tree name
            
            print(f"\nProcessing {file_name} -> {tree_name}...")
            
            # Open input file
            with uproot.open(input_file) as input_file:
                input_tree = input_file["outputTree"]
                
                # Prepare output data containers
                output_data = {branch: [] for branch in FEATURE_BRANCHES}
                output_data["BDTscore"] = []
                
                # Process in chunks
                for chunk in tqdm(input_tree.iterate(
                    library="np",
                    step_size=CHUNK_SIZE,
                    filter_name=FILTER_BRANCHES + FEATURE_BRANCHES
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
                    
                    # Store results
                    for branch in FEATURE_BRANCHES:
                        output_data[branch].append(filtered_features[branch])
                    output_data["BDTscore"].append(bdt_scores)
                
                # Combine all chunks if any events passed selection
                if output_data["BDTscore"]:
                    final_output = {
                        branch: np.concatenate(output_data[branch]) 
                        for branch in FEATURE_BRANCHES + ["BDTscore"]
                    }
                    
                    # Create new tree in output file
                    output_file[tree_name] = final_output
                    print(f"Created tree '{tree_name}' with {len(final_output['BDTscore'])} events")
                else:
                    print("No events passed selection criteria")

    print("\nAll files processed successfully!")
    print(f"Results saved to {OUTPUT_FILE} with {len(INPUT_FILES)} trees")

if __name__ == "__main__":
    process_files()
