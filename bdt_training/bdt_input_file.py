import uproot
import numpy as np

# Define the input files and output files
signal_files = [
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tchannel_antitop_5f.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tchannel_top_5f.root",
#   "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_TTbar-channel_top_UL17.root",

    ]
background_files = [
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_TTbar-channel_top_UL17.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_1J_UL17.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_0J_UL17.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_2J_UL17.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tW_antitop_5f.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tW_top_5f.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_DYJetsToLL_M-10to50_UL17.root",
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_DYJetsToLL_M-50_UL17.root"
    ]
signal_output_file = "signal_electron.root"  # Output file for signal events
background_output_file = "background_electron.root"  # Output file for background events

# Define the tree name and variables to read
tree_name = "outputTree"
bdt_variables = ["top_mass", "specJet_leading_eta", "bdt_delR", "bdt_deltaEta","Wboson_transversMass"]#,"bdt_WHelicity","bdt_eventShape", "MET_pt_corr"]
boolean_variables = ["region_2j1t", "muonChannel", "electronChannel", "region_2j0t", "region_3j2t","QCDelectronChannel","QCDmuonChannel","Wboson_transversMass>50"]

# Define file-specific event selection criteria
event_selection = {
    # For signal files
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tchannel_antitop_5f.root": {
#        ("region_2j1t", "muonChannel","Wboson_transversMass>50"): 200000,
        ("region_2j1t", "electronChannel","Wboson_transversMass>50"): 30000,
    },
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tchannel_top_5f.root": {
       # ("muonChannel", "region_2j0t","Wboson_transversMass>50"): 200000,
        ("region_2j1t", "electronChannel","Wboson_transversMass>50"): 30000,
    },
#    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_TTbar-channel_top_UL17.root": {
#        ("region_2j1t", "muonChannel"): 50000,
#        ("region_2j1t", "electronChannel"): 30000,
#        ("region_3j2t", "muonChannel"): 3000,
#        ("region_3j2t", "electronChannel"): 2000,
#    },

    # For background files
   "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_TTbar-channel_top_UL17.root": {
        #("region_2j1t", "muonChannel","Wboson_transversMass>50"): 950000,
        ("region_2j1t", "electronChannel","Wboson_transversMass>50"): 30000,
        #("region_3j2t", "muonChannel","Wboson_transversMass>50"): 5000,
        ("region_3j2t", "electronChannel","Wboson_transversMass>50"): 2000,
    },
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_1J_UL17.root": {
        #("region_2j1t", "muonChannel","Wboson_transversMass>50"): 30000,
        ("region_2j1t", "electronChannel","Wboson_transversMass>50"): 10000,
        #("region_2j0t", "muonChannel","Wboson_transversMass>50"): 3000,
        ("region_2j0t", "electronChannel","Wboson_transversMass>50"): 1000,
    },
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_0J_UL17.root": {
        #("region_2j1t", "muonChannel","Wboson_transversMass>50"): 30000,
        ("region_2j1t", "electronChannel","Wboson_transversMass>50"): 10000,
        #("region_2j0t", "muonChannel","Wboson_transversMass>50"): 4000,
        ("region_2j0t", "electronChannel","Wboson_transversMass>50"): 1000,
    },
    "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_2J_UL17.root": {
        #("region_2j1t", "muonChannel","Wboson_transversMass>50"): 30000,
        ("region_2j1t", "electronChannel","Wboson_transversMass>50"): 10000,
        #("region_2j0t", "muonChannel","Wboson_transversMass>50"): 3000,
        ("region_2j0t", "electronChannel","Wboson_transversMass>50"):1000,
    },

   "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tW_antitop_5f.root": {
        #("region_2j1t", "muonChannel","Wboson_transversMass>50"): 49000,
        ("region_2j1t", "electronChannel","Wboson_transversMass>50"): 30000,
        #("region_3j2t", "muonChannel","Wboson_transversMass>50"): 1000,
        ("region_3j2t", "electronChannel","Wboson_transversMass>50"): 2000,
    },

   "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tW_top_5f.root": {
        #("region_2j1t", "muonChannel","Wboson_transversMass>50"): 49000,
        ("region_2j1t", "electronChannel","Wboson_transversMass>50"): 30000,
        #("region_3j2t", "muonChannel","Wboson_transversMass>50"): 1000,
        ("region_3j2t", "electronChannel","Wboson_transversMass>50"): 2000,
    },
   "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_DYJetsToLL_M-10to50_UL17.root": {
        #("region_2j1t", "muonChannel","Wboson_transversMass>50"): 48000,
        ("region_2j1t", "electronChannel","Wboson_transversMass>50"): 30000,
        #("region_3j2t", "muonChannel","Wboson_transversMass>50"): 2000,
        ("region_3j2t", "electronChannel","Wboson_transversMass>50"): 2000,
    },
   "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_DYJetsToLL_M-50_UL17.root": {
        #("region_2j1t", "muonChannel","Wboson_transversMass>50"): 48000,
        ("region_2j1t", "electronChannel","Wboson_transversMass>50"): 30000,
        #("region_3j2t", "muonChannel","Wboson_transversMass>50"): 2000,
        ("region_3j2t", "electronChannel","Wboson_transversMass>50"): 2000,
    },

    
}

# Function to process files and fill the output tree
def process_files(input_files, output_file):
    # Initialize dictionaries to store selected events
    selected_events = {var: np.array([], dtype=np.float32) for var in bdt_variables}

    # Loop over input files
    for file in input_files:
        with uproot.open(file) as input_file:
            tree = input_file[tree_name]

            # Read the BDT variables and boolean variables
            data = tree.arrays([*bdt_variables, *boolean_variables], library="np")

            # Get the event selection criteria for the current file
            file_selection = event_selection.get(file, {})
            for condition, num_events in file_selection.items():
                # Create a mask for the current condition
                mask = np.ones(len(data[boolean_variables[0]]), dtype=bool)
                for bool_var in condition:
                    mask &= data[bool_var]

                # Select the required number of events
                selected_indices = np.where(mask)[0][:num_events]
                for var in bdt_variables:
                    selected_events[var] = np.append(selected_events[var], data[var][selected_indices])

    # Write the selected events to the output tree
    with uproot.recreate(output_file) as output_file:
        output_file["outputTree"] = selected_events

# Process signal files and save to signal.root
process_files(signal_files, signal_output_file)

# Process background files and save to background.root
process_files(background_files, background_output_file)

print(f"Signal events saved to {signal_output_file}")
print(f"Background events saved to {background_output_file}")
