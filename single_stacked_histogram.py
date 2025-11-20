import uproot
import numpy as np
import matplotlib.pyplot as plt
import os

# === User Config ===
directory = "merged/"  # Path where all ROOT files are stored

# Variable to plot (change this to any variable you want)
variable_to_plot ="Wboson_transversMass"

# Define custom binning: (start, end, num_bins)
# Adjust these based on your variable
binning = (0,200, 30)

# Define regions
regions = ["region_2j0t", "region_2j1t", "region_3j2t","region_4j1t","region_4j2t","region_2j2t","region_3j1b"]

# Define channels (these should be boolean branches in your ROOT files)
channels = {
    "Electron": "electronChannel",
    "Muon": "muonChannel", 
    "QCD Electron": "QCDelectronChannel",
    "QCD Muon": "QCDmuonChannel"
}

# Define file groups (MC samples)
file_groups = {
    "schannel": ["TbarBtoLminusNuB_s_channel.root", "TBbartoLplusNuBbar_s_channel.root"],
    "wjets": ["WtoLNu.root"],
    "single_tW": ["TbarWplusto2L2Nu.root", "TWminusto2L2Nu.root",
                     "TbarWplustoLNu2Q.root", "TWminustoLNu2Q.root"],
    "ttbar": ["TTbar_SemiLept.root", "TTbar_Dilept.root"],
    "t-channel": ["TbarBQ_t_channel.root", "TBbarQ_t_channel.root"],
    "drell_yan": ["DYjetsM10to50.root", "DYJetsM50.root"],
    "other": ["ZZto2L2Q.root", "ZZto2LNu.root", "ZZto4L.root", "WWto2L2Nu.root", "WZto3LNu.root",
              "WWW_4F.root", "WWZ_4F.root", "WZZ.root", "ZZZ.root",
              "TTGJets_PTG-10to100.root", "TTGJets_PTG-100to200.root", "TTGJets_PTG-200.root",
              "TTLNu-1Jets.root"],
}

# Colors for different processes
colors = {
    "ttbar": "red",
    "wjets": "blue",
    "t-channel": "green",
    "drell_yan": "orange",
    "other": "gray",
    "schannel": "purple",
    "single_tW": "brown"
}

# Data file
data_file = os.path.join(directory, "data.root")

def load_histogram_data(file_groups, directory, variable, region_flag, channel_flag, bins):
    """
    Load histogram data for a specific region and channel
    
    Returns:
    --------
    dict: histogram data for each process group
    """
    hist_data = {}
    
    print(f"\n  Loading data for {region_flag} in {channel_flag}")
    
    # Loop through MC groups
    for group_name, files in file_groups.items():
        values = []
        weights = []
        
        for filename in files:
            filepath = os.path.join(directory, filename)
            if not os.path.exists(filepath):
                continue
                
            try:
                with uproot.open(filepath) as file:
                    tree = file["outputTree"]
                    available_branches = tree.keys()
                    
                    # Check if all required branches exist
                    required_branches = [variable, region_flag, channel_flag, "Weight"]
                    if not all(b in available_branches for b in required_branches):
                        continue
                    
                    # Read data
                    arr = tree.arrays(required_branches, library="np")
                    
                    # Create combined mask for region AND channel
                    mask = (arr[region_flag] == True) & (arr[channel_flag] == True)
                    n_selected = np.sum(mask)
                    
                    if n_selected == 0:
                        continue
                    
                    # Filter data
                    filtered_values = arr[variable][mask]
                    filtered_weights = arr["Weight"][mask]
                    
                    # Remove NaN and Inf
                    valid_mask = np.isfinite(filtered_values)
                    filtered_values = filtered_values[valid_mask]
                    filtered_weights = filtered_weights[valid_mask]
                    
                    if len(filtered_values) > 0:
                        values.append(filtered_values)
                        weights.append(filtered_weights)
                    
            except Exception as e:
                print(f"    [Error] Failed to read {filename}: {e}")
                continue
        
        # Create histogram for this group
        if values:
            all_vals = np.concatenate(values)
            all_weights = np.concatenate(weights)
            hist, _ = np.histogram(all_vals, bins=bins, weights=all_weights)
            hist_data[group_name] = hist
        else:
            hist_data[group_name] = np.zeros(len(bins) - 1)
    
    return hist_data

def load_data_histogram(data_file, variable, region_flag, channel_flag, bins):
    """
    Load histogram data from data file for a specific region and channel
    """
    print(f"  Loading data from data.root")
    
    try:
        with uproot.open(data_file) as file:
            tree = file["outputTree"]
            available_branches = tree.keys()
            
            # Check if all required branches exist
            required_branches = [variable, region_flag, channel_flag]
            if not all(b in available_branches for b in required_branches):
                print(f"    [Warning] Missing branches in data file")
                return np.zeros(len(bins) - 1)
            
            # Read data
            arr = tree.arrays(required_branches, library="np")
            
            # Create combined mask for region AND channel
            mask = (arr[region_flag] == True) & (arr[channel_flag] == True)
            n_selected = np.sum(mask)
            
            if n_selected == 0:
                print(f"    [Warning] No events selected from data")
                return np.zeros(len(bins) - 1)
            
            # Filter data
            filtered_values = arr[variable][mask]
            
            # Remove NaN and Inf
            valid_mask = np.isfinite(filtered_values)
            filtered_values = filtered_values[valid_mask]
            
            if len(filtered_values) == 0:
                return np.zeros(len(bins) - 1)
            
            data_hist, _ = np.histogram(filtered_values, bins=bins)
            print(f"    [OK] Loaded {len(filtered_values)} events from data")
            return data_hist
            
    except Exception as e:
        print(f"    [Error] Failed to read data file: {e}")
        return np.zeros(len(bins) - 1)

def plot_stacked_histogram(hist_data, data_hist, bins, variable, region, channel, output_dir=""):
    """
    Create and save stacked histogram with data overlay and ratio plot
    """
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10), sharex=True,
                                   gridspec_kw={'height_ratios': [3, 1]})
    
    # --- Stack Plot ---
    bottom = np.zeros_like(bins[:-1])
    for group_name in file_groups.keys():
        ax1.bar(bins[:-1], hist_data[group_name], width=np.diff(bins), bottom=bottom,
                color=colors[group_name], label=group_name, align='edge', alpha=0.8)
        bottom += hist_data[group_name]
    
    # Overlay data as points
    bin_centers = 0.5 * (bins[:-1] + bins[1:])
    ax1.errorbar(bin_centers, data_hist, yerr=np.sqrt(data_hist), 
                 fmt='o', color='black', label='Data', markersize=4)
    
    ax1.set_ylabel("Events", fontsize=12)
    ax1.set_title(f"{variable} - {region} - {channel}", fontsize=14, pad=10)
    ax1.legend(loc='best', fontsize=10)
    ax1.grid(True, alpha=0.3)#    ax1.set_yscale('log')  # Log scale for better visibility
    ax1.set_ylim(bottom=0.1)  # Avoid log(0)
    
    # --- Ratio Plot ---
    mc_total = np.sum([hist_data[group] for group in file_groups], axis=0)
    ratio = np.divide(data_hist, mc_total, out=np.zeros_like(data_hist, dtype=float), 
                     where=mc_total > 0)
    ratio_err = np.divide(np.sqrt(data_hist), mc_total, 
                         out=np.zeros_like(data_hist, dtype=float), where=mc_total > 0)
    
    ax2.errorbar(bin_centers, ratio, yerr=ratio_err, fmt='o', color='black', markersize=4)
    ax2.axhline(1.0, color='red', linestyle='--', linewidth=2)
    ax2.set_ylabel("Data / MC", fontsize=12)
    ax2.set_xlabel(variable, fontsize=12)
    ax2.grid(True, alpha=0.3)
    ax2.set_ylim(0, 2)
    
    plt.tight_layout()
    
    # Save figure
    safe_channel = channel.replace(" ", "_")
    output_name = os.path.join(output_dir, f"stack_{variable}_{region}_{safe_channel}.png")
    plt.savefig(output_name, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f"    [Saved] {output_name}")

# === Main Execution ===
if __name__ == "__main__":
    print(f"\n{'='*80}")
    print(f"Generating stacked histograms for variable: {variable_to_plot}")
    print(f"Binning: {binning[0]} to {binning[1]} with {binning[2]} bins")
    print(f"{'='*80}")
    
    # Create bins
    bins = np.linspace(binning[0], binning[1], binning[2] + 1)
    
    # Loop over all combinations of regions and channels
    plot_count = 0
    for region in regions:
        for channel_name, channel_flag in channels.items():
            print(f"\n{'='*60}")
            print(f"Processing: {region} - {channel_name}")
            print(f"{'='*60}")
            
            # Load MC histogram data
            hist_data = load_histogram_data(file_groups, directory, variable_to_plot, 
                                           region, channel_flag, bins)
            
            # Load data histogram
            data_hist = load_data_histogram(data_file, variable_to_plot, 
                                           region, channel_flag, bins)
            
            # Check if we have any data
            total_mc = sum([np.sum(hist_data[g]) for g in file_groups])
            total_data = np.sum(data_hist)
            
            print(f"  Total MC events: {total_mc:.1f}")
            print(f"  Total Data events: {total_data:.0f}")
            
            if total_mc > 0 or total_data > 0:
                # Create plot
                plot_stacked_histogram(hist_data, data_hist, bins, variable_to_plot, 
                                      region, channel_name)
                plot_count += 1
            else:
                print(f"  [Warning] No events found, skipping plot")
    
    print(f"\n{'='*80}")
    print(f"Generated {plot_count} plots successfully!")
    print(f"{'='*80}")
