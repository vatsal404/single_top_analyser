import uproot
import numpy as np
import matplotlib.pyplot as plt
import os

# === User Config ===
directory = "merged/"  # Path where all ROOT files are stored

# Variable to plot (change this to any variable you want)
variable_to_plot = "goodmuons_leading_pt"

# Define custom binning: (start, end, num_bins)
binning = (0,200, 30)

# Define regions
regions = ["region_1j1t", "region_2j1t", "region_2j2t"]

# Define channels (these should be boolean branches in your ROOT files)
channels = {
    "Electron_muon": "eu_channel",
}

# Define file groups (MC samples)
file_groups = {
    "drell_yan": [
        "DYjetsM10to50.root",
        "DYjetsM50.root",
        "TTLL_MLL-50.root",
        "TTLL_MLL-4to50.root"
    ],
    "ttg_ttv": [
        "TTGJets_PTG-10to100.root",
        "TTGJets_PTG-100to200.root",
        "TTGJets_PTG-200.root",
        "TTLNu-1Jets.root",
        "TTZ-ZtoQQ-1Jets.root"
    ],
    "ttbar_dileptonic": [
        "TTbar_Dilept.root",
    ],
    "ttbar_semileptonic": [
        "TTbar_SemiLept.root"
    ],
    "single_tW": [
        "TWminusto2L2Nu.root",
        "TbarWplusto2L2Nu.root"
    ],
    "vv": [
        "WWto2L2Nu.root",
        "WZto2L2Q.root",
        "WZto3LNu.root",
        "ZZto2L2Q.root",
        "ZZto2LNu.root",
        "ZZto4L.root"
    ],
    "wjets": [
        "WtoLNu.root"
    ],
}

# Colors for different processes
colors = {
    "drell_yan": "orange",
    "ttg_ttv": "gray",
    "ttbar_dileptonic": "red",
    "ttbar_semileptonic":"green",
    "single_tW": "brown",
    "vv": "purple",
    "wjets": "blue",
}

# Data file
data_file = os.path.join(directory, "data.root")


def debug_file_contents(filepath, region_flag, channel_flag, variable):
    """
    Debug function to inspect what's actually in the ROOT file
    """
    print(f"\n{'='*70}")
    print(f"DEBUGGING: {os.path.basename(filepath)}")
    print(f"{'='*70}")
    
    try:
        with uproot.open(filepath) as file:
            tree = file["outputTree"]
            available_branches = tree.keys()
            
            # Check if branches exist
            print(f"\n1. Branch Existence Check:")
            print(f"   {region_flag} exists: {region_flag in available_branches}")
            print(f"   {channel_flag} exists: {channel_flag in available_branches}")
            print(f"   {variable} exists: {variable in available_branches}")
            
            # Read a small sample to check data types and values
            branches_to_check = [b for b in [region_flag, channel_flag, variable, "evWeight_hlt"] 
                                if b in available_branches]
            
            if not branches_to_check:
                print("\n   [ERROR] None of the required branches found!")
                return
            
            # Read first 100 events or all if less
            arr = tree.arrays(branches_to_check, library="np", entry_stop=100)
            
            print(f"\n2. Data Sample (first 100 events):")
            for branch in branches_to_check:
                if branch in arr:
                    values = arr[branch]
                    print(f"\n   {branch}:")
                    print(f"      Type: {values.dtype}")
                    print(f"      Unique values: {np.unique(values)}")
                    print(f"      First 5 values: {values[:5]}")
                    
                    # Check how many are "True" in different ways
                    if region_flag in arr and channel_flag in arr:
                        if branch == region_flag:
                            print(f"      == True count: {np.sum(values == True)}")
                            print(f"      == 1 count: {np.sum(values == 1)}")
                            print(f"      != 0 count: {np.sum(values != 0)}")
                        elif branch == channel_flag:
                            print(f"      == True count: {np.sum(values == True)}")
                            print(f"      == 1 count: {np.sum(values == 1)}")
                            print(f"      != 0 count: {np.sum(values != 0)}")
            
            # Check combined selection
            if region_flag in arr and channel_flag in arr:
                print(f"\n3. Combined Selection Check:")
                
                # Try different mask combinations
                mask1 = (arr[region_flag] == True) & (arr[channel_flag] == True)
                mask2 = (arr[region_flag] == 1) & (arr[channel_flag] == 1)
                mask3 = (arr[region_flag] != 0) & (arr[channel_flag] != 0)
                mask4 = (arr[region_flag].astype(bool)) & (arr[channel_flag].astype(bool))
                
                print(f"   (region == True) & (channel == True): {np.sum(mask1)} events")
                print(f"   (region == 1) & (channel == 1): {np.sum(mask2)} events")
                print(f"   (region != 0) & (channel != 0): {np.sum(mask3)} events")
                print(f"   bool(region) & bool(channel): {np.sum(mask4)} events")
                
    except Exception as e:
        print(f"\n[ERROR] Failed to debug file: {e}")
        import traceback
        traceback.print_exc()


def load_histogram_data(file_groups, directory, variable, region_flag, channel_flag, bins):
    """
    Load histogram data for a specific region and channel
    """
    hist_data = {}
    
    print(f"\n  Loading MC data for {region_flag} in {channel_flag}")
    
    # Loop through MC groups
    for group_name, files in file_groups.items():
        values = []
        weights = []
        
        for filename in files:
            filepath = os.path.join(directory, filename)
            if not os.path.exists(filepath):
                print(f"    [Skip] {filename} not found")
                continue
                
            try:
                with uproot.open(filepath) as file:
                    tree = file["outputTree"]
                    available_branches = tree.keys()
                    
                    # Check if all required branches exist
                    required_branches = [variable, region_flag, channel_flag, "evWeight_hlt"]
                    missing_branches = [b for b in required_branches if b not in available_branches]
                    
                    if missing_branches:
                        print(f"    [Skip] {filename}: missing branches {missing_branches}")
                        continue
                    
                    # Read data
                    arr = tree.arrays(required_branches, library="np")
                    
                    # Create combined mask - try multiple methods
                    # Method 1: Direct boolean comparison
                    mask = (arr[region_flag].astype(bool)) & (arr[channel_flag].astype(bool))
                    
                    n_selected = np.sum(mask)
                    
                    if n_selected == 0:
                        print(f"    [Skip] {filename}: 0 events pass selection")
                        continue
                    
                    print(f"    [OK] {filename}: {n_selected} events selected")
                    
                    # Filter data
                    filtered_values = arr[variable][mask]
                    filtered_weights = arr["evWeight_hlt"][mask]
                    
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
            print(f"  [{group_name}] Total weighted events: {np.sum(hist):.1f}")
        else:
            hist_data[group_name] = np.zeros(len(bins) - 1)
    
    return hist_data


def load_data_histogram(data_file, variable, region_flag, channel_flag, bins):
    """
    Load histogram data from data file for a specific region and channel
    Uses chunking to avoid memory issues with large files
    """
    print(f"  Loading data from {os.path.basename(data_file)}")
    
    if not os.path.exists(data_file):
        print(f"    [Warning] Data file not found")
        return np.zeros(len(bins) - 1)
    
    try:
        with uproot.open(data_file) as file:
            tree = file["outputTree"]
            available_branches = tree.keys()
            
            # Check if all required branches exist
            required_branches = [variable, region_flag, channel_flag]
            missing_branches = [b for b in required_branches if b not in available_branches]
            
            if missing_branches:
                print(f"    [Warning] Missing branches in data file: {missing_branches}")
                return np.zeros(len(bins) - 1)
            
            # Get total number of entries
            num_entries = tree.num_entries
            print(f"    [Info] Total entries in data file: {num_entries:,}")
            
            # Process in chunks to avoid memory issues
            chunk_size = 1_000_000  # Process 1M events at a time
            data_hist = np.zeros(len(bins) - 1)
            total_selected = 0
            
            for start in range(0, num_entries, chunk_size):
                stop = min(start + chunk_size, num_entries)
                
                # Read chunk
                arr = tree.arrays(required_branches, library="np", 
                                entry_start=start, entry_stop=stop)
                
                # Create combined mask using boolean conversion
                mask = (arr[region_flag].astype(bool)) & (arr[channel_flag].astype(bool))
                
                n_selected = np.sum(mask)
                total_selected += n_selected
                
                if n_selected > 0:
                    # Filter data
                    filtered_values = arr[variable][mask]
                    
                    # Remove NaN and Inf
                    valid_mask = np.isfinite(filtered_values)
                    filtered_values = filtered_values[valid_mask]
                    
                    if len(filtered_values) > 0:
                        # Add to histogram
                        chunk_hist, _ = np.histogram(filtered_values, bins=bins)
                        data_hist += chunk_hist
                
                # Progress update every 10M events
                if (stop % 10_000_000 == 0) or (stop == num_entries):
                    print(f"    [Progress] Processed {stop:,}/{num_entries:,} events "
                          f"({100*stop/num_entries:.1f}%), selected: {total_selected:,}")
            
            if total_selected == 0:
                print(f"    [Warning] No events selected from data")
                return np.zeros(len(bins) - 1)
            
            print(f"    [OK] {total_selected:,} total events selected from data")
            return data_hist
            
    except Exception as e:
        print(f"    [Error] Failed to read data file: {e}")
        import traceback
        traceback.print_exc()
        return np.zeros(len(bins) - 1)


def plot_stacked_histogram(hist_data, data_hist, bins, variable, region, channel, output_dir="plots"):
    """
    Create and save stacked histogram with data overlay and ratio plot
    """
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10), sharex=True,
                                   gridspec_kw={'height_ratios': [3, 1]})
    
    # --- Stack Plot ---
    bottom = np.zeros(len(bins) - 1)
    for group_name in file_groups.keys():
        if group_name in hist_data and np.sum(hist_data[group_name]) > 0:
            ax1.bar(bins[:-1], hist_data[group_name], width=np.diff(bins), bottom=bottom,
                    color=colors[group_name], label=group_name, align='edge', alpha=0.8)
            bottom += hist_data[group_name]
    
    # Overlay data as points
    bin_centers = 0.5 * (bins[:-1] + bins[1:])
    data_errors = np.sqrt(data_hist)
    data_errors[data_hist == 0] = 0  # No error for empty bins
    
    ax1.errorbar(bin_centers, data_hist, yerr=data_errors, 
                 fmt='o', color='black', label='Data', markersize=5, capsize=3)
    
    ax1.set_ylabel("Events", fontsize=12)
    ax1.set_title(f"{variable} - {region} - {channel}", fontsize=14, pad=10)
    ax1.legend(loc='best', fontsize=10)
    ax1.grid(True, alpha=0.3)
    
    # Set y-axis limits
    max_val = max(np.max(bottom), np.max(data_hist))
    if max_val > 0:
        ax1.set_ylim(bottom=0, top=max_val * 1.3)
    
    # --- Ratio Plot ---
    mc_total = np.sum([hist_data[group] for group in file_groups if group in hist_data], axis=0)
    
    # Calculate ratio and error
    ratio = np.zeros_like(data_hist, dtype=float)
    ratio_err = np.zeros_like(data_hist, dtype=float)
    
    valid = mc_total > 0
    ratio[valid] = data_hist[valid] / mc_total[valid]
    ratio_err[valid] = np.sqrt(data_hist[valid]) / mc_total[valid]
    
    ax2.errorbar(bin_centers, ratio, yerr=ratio_err, fmt='o', color='black', 
                 markersize=5, capsize=3)
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
    print(f"  [Saved] {output_name}\n")


# === Main Execution ===
if __name__ == "__main__":
    
    # First, run debugging on a sample file
    print("\n" + "="*80)
    print("RUNNING DIAGNOSTICS")
    print("="*80)
    
    # Debug data file
    if os.path.exists(data_file):
        debug_file_contents(data_file, regions[0], channels["Electron_muon"], variable_to_plot)
    
    # Debug first MC file
    first_mc_file = os.path.join(directory, file_groups["drell_yan"][0])
    if os.path.exists(first_mc_file):
        debug_file_contents(first_mc_file, regions[0], channels["Electron_muon"], variable_to_plot)
    
    input("\nPress Enter to continue with histogram generation...")
    
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
            total_mc = sum([np.sum(hist_data[g]) for g in file_groups if g in hist_data])
            total_data = np.sum(data_hist)
            
            print(f"\n  Summary:")
            print(f"    Total MC events: {total_mc:.1f}")
            print(f"    Total Data events: {total_data:.0f}")
            
            if total_mc > 0 or total_data > 0:
                # Create plot
                plot_stacked_histogram(hist_data, data_hist, bins, variable_to_plot, 
                                      region, channel_name)
                plot_count += 1
            else:
                print(f"  [Warning] No events found, skipping plot\n")
    
    print(f"\n{'='*80}")
    print(f"Generated {plot_count} plots successfully!")
    print(f"Plots saved in 'plots/' directory")
    print(f"{'='*80}")
