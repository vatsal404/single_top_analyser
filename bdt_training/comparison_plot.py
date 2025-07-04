import uproot
import matplotlib.pyplot as plt
import numpy as np
from matplotlib import rcParams

# Set better plot style
rcParams.update({'font.size': 14, 'figure.figsize': (12, 8)})

# Define process groups with colors (customize as needed)
color_map = {
    "kOrange+1": "#ff9900",     # Orange-ish
    "kYellow": "#ffff00",       # Yellow
    "kRed": "#ff0000",          # Red
    "kMagenta": "#ff00ff",      # Magenta
    "kCyan": "#00ffff",         # Cyan
}

# Define process groups
process_groups = {
    "W+Jets": {
        "files": [
            "../histograms/PROCESSED_WJetsToLNu_0J_UL17_hist.root",
            "../histograms/PROCESSED_WJetsToLNu_1J_UL17_hist.root",
            "../histograms/PROCESSED_WJetsToLNu_2J_UL17_hist.root"
        ],
        "color": color_map["kOrange+1"]
    },
    "DY": {
        "files": [
            "../histograms/PROCESSED_DYJetsToLL_M-10to50_UL17_hist.root",
            "../histograms/PROCESSED_DYJetsToLL_M-50_UL17_hist.root"
        ],
        "color": color_map["kYellow"]
    },
    "Single Top (t-channel)": {
        "files": [
            "../histograms/PROCESSED_ST_tchannel_antitop_5f_hist.root",
            "../histograms/PROCESSED_ST_tchannel_top_5f_hist.root"
        ],
        "color": color_map["kRed"]
    },
    "Single Top (tW)": {
        "files": [
            "../histograms/PROCESSED_ST_tW_antitop_5f_hist.root",
            "../histograms/PROCESSED_ST_tW_top_5f_hist.root"
        ],
        "color": color_map["kMagenta"]
    },
    "TTbar": {
        "files": [
            "../histograms/PROCESSED_TTbar-channel_top_UL17_hist.root"
        ],
        "color": color_map["kCyan"]
    },
    "Data": {
        "files": [
            "../histograms/data_hist.root"
        ],
        "color": "black"
    }
}
# ===== ADD YOUR HISTOGRAM NAMES HERE =====
hist_names = [
    "bdt_wboson_muon_2j1t",
    "bdt_MET_pt_muon_2j1t",
    "bdt_WHelicity_muon_2j1t",
    "bdt_eventShape_muon_2j1t",
    "bdt_deltaEta_muon_2j1t",
    "bdt_delR_muon_2j1t",
    "bdt_specJet_2j1t_leading_eta_muon",
    "bdt_top_muon_2j1t"
]

def get_combined_histogram(group, hist_name):
    """Combine histograms from all files in a group for a specific variable"""
    combined_values = None
    edges = None

    for file_path in group['files']:
        try:
            with uproot.open(file_path) as f:
                if hist_name not in f:
                    print(f"Histogram {hist_name} not found in {file_path}")
                    continue

                hist = f[hist_name]
                values, this_edges = hist.to_numpy()

                if combined_values is None:
                    combined_values = values
                    edges = this_edges
                else:
                    if np.array_equal(edges, this_edges):
                        combined_values += values
                    else:
                        print(f"Binning mismatch in {hist_name} for {file_path}")
        except Exception as e:
            print(f"Error loading {hist_name} from {file_path}: {str(e)}")

    return combined_values, edges

# Create plots for each variable
for hist_name in hist_names:
    plt.figure(figsize=(10, 6))
    ax = plt.gca()
    
    # Plot MC processes
    for process, group in process_groups.items():
        if process == 'Data': continue  # Handle data separately
        
        values, edges = get_combined_histogram(group, hist_name)
        
        if values is not None and edges is not None:
            # Normalize to unit area
            norm = values.sum()
            if norm > 0:
                values = values / norm
                
                # Plot with stepped lines
                plt.step(edges, np.append(values, values[-1]), 
                        where='post', 
                        label=process,
                        color=group['color'],
                        linewidth=2)

    # Plot data
    data_group = process_groups['Data']
    for file_path in data_group['files']:
        try:
            with uproot.open(file_path) as f:
                hist = f['outputTree'][hist_name]
                values, edges = hist.to_numpy()
                plt.step(edges, np.append(values, values[-1]), 
                         where='post', 
                         label='Data',
                         color=data_group['color'],
                         linewidth=2,
                         linestyle='--')
        except:
            continue

    # Plot formatting
    plt.title(hist_name, fontsize=16, pad=20)
    plt.xlabel(hist_name, fontsize=14)
    plt.ylabel("Normalized Units", fontsize=14)
    #plt.yscale('log')
    plt.grid(True, which='both', alpha=0.4)
    plt.legend(loc='upper right', fontsize=12)
    
    # Adjust y-axis limits
    plt.ylim(top=0.2, bottom=1e-5)  # Adjust based on your distributions
    
    # Save plot
    plt.tight_layout()
    plt.savefig(f"{hist_name}_shape_comparison.png", dpi=300)
    plt.close()

print("All shape comparison plots generated!")
