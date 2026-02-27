import uproot
import numpy as np
import matplotlib.pyplot as plt
import os

# === User Config ===
directory = "merged/"  # Path where all ROOT files are stored

# Base output directory
base_output_dir = "plots"

# Variables to plot with custom binning: (variable_name, (start, end, num_bins))
variables_to_plot = [
    # Lepton kinematics
    ("goodmuons_leading_pt", (0, 200, 25)),
    ("goodmuons_leading_eta", (-2.5, 2.5, 20)),
    ("goodmuons_leading_phi", (-3.2, 3.2, 20)),
    ("goodElectrons_leading_pt", (0, 200, 25)),
    ("goodElectrons_leading_eta", (-2.5, 2.5, 20)),
    ("goodElectrons_leading_phi", (-3.2, 3.2, 20)),
    ("leading_lepton_pt", (0, 200, 20)),
    ("subleading_lepton_pt", (0, 200, 20)),
    
    # Corrected lepton pT
    ("Electron_pt_corr", (0, 200, 30)),
    ("Muon_pt_corr", (0, 200, 30)),
    
    # MET
    ("PuppiMET_pt_corr", (0, 200, 30)),
    ("PuppiMET_phi_corr", (-3.2, 3.2, 25)),
    
    # Dilepton system
    ("leptons_invariant_mass", (0, 200, 30)),
    ("dilepton_invariant_mass", (0, 200, 30)),
    ("dilepton_jet_pt", (0, 300, 20)),
    ("dilepton_jet_mass", (0, 400, 25)),
    ("dilepton_del_phi", (0, 3.2, 25)),
    
    # Jet kinematics
    ("Selected_jet_leading_pt", (0, 300, 20)),
    ("Selected_jet_subleading_pt", (0, 250, 25)),
    ("Selected_jet_leading_phi", (-3.2, 3.2, 20)),
    ("Selected_jet_leading_eta", (-2.5, 2.5, 20)),
    ("Selected_jet_leading_mass", (0, 50, 25)),
    ("Selected_loosejet_leadingpt", (0, 300, 30)),
    ("leading_lepton_jet_pt", (0, 300, 25)),
    
    
    # Angular separations (DeltaR)
    ("delR_dilepton_jet", (0, 6, 20)),
    ("delR_ele_muon", (0, 6, 20)),
    ("delR_leadinglepton_jet", (0, 6, 20)),
    
    # Event shape variables
    ("sphericity", (0, 1, 20)),
    ("aplanery", (0, 0.5, 20)),
    ("centrality", (0, 1, 20)),
    
    
    # Event identification (usually not plotted, but included for completeness)
    # ("run", (0, 400000, 100)),
    # ("event", (0, 1e10, 100)),
    # ("luminosityBlock", (0, 3000, 100)),
]

# Define regions with their directory names
regions = {
    "region_1j1t": "1j1b",
    "region_2j1t": "2j1b", 
    "region_2j2t": "2j2b"
}

# Define channels (these should be boolean branches in your ROOT files)
channels = {
    "Electron_muon": "eu_channel",
}

# Define file groups (MC samples)
file_groups = {
    # Signal (yellow)
    "signal": [
        "TTbar_SemiLept.root"
    ],
    # TTbar (red)
    "ttbar": [
        "TTbar_Dilept.root",
    ],
    # Drell-Yan (blue)
    "drell_yan": [
        "DYjetsM10to50.root",
        "DYjetsM50.root",
        "TTLL_MLL-50.root",
        "TTLL_MLL-4to50.root"
    ],
    # Other backgrounds
    "ttg_ttv": [
        "TTGJets_PTG-10to100.root",
        "TTGJets_PTG-100to200.root",
        "TTGJets_PTG-200.root",
        "TTLNu-1Jets.root",
        "TTZ-ZtoQQ-1Jets.root"
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

# Colors for different processes (as per user request)
colors = {
    "signal": "yellow",           # Signal - yellow
    "ttbar": "red",               # TTbar - red
    "drell_yan": "blue",          # Drell-Yan - blue
    "ttg_ttv": "gray",            # TTG/TTV - gray
    "single_tW": "brown",         # Single top - brown
    "vv": "purple",               # Diboson - purple
    "wjets": "green",             # W+jets - green
}

# Display names for legend
display_names = {
    "signal": "ttbar semileptonic",
    "ttbar": "ttbar dileptonic",
    "drell_yan": "Drell-Yan",
    "ttg_ttv": "ttbar+gamma/V",
    "single_tW": "Single top (tW)",
    "vv": "Diboson",
    "wjets": "W+jets",
}

# Data file
data_file = os.path.join(directory, "data.root")


def load_histogram_data(file_groups, directory, variable, region_flag, channel_flag, bins):
    """
    Load histogram data for a specific region and channel
    """
    hist_data = {}
    
    print(f"    Loading MC data for {region_flag} in {channel_flag}")
    
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
                    required_branches = [variable, region_flag, channel_flag, "evWeight_hlt"]
                    missing_branches = [b for b in required_branches if b not in available_branches]
                    
                    if missing_branches:
                        continue
                    
                    # Read data
                    arr = tree.arrays(required_branches, library="np")
                    
                    # Create combined mask
                    mask = (arr[region_flag].astype(bool)) & (arr[channel_flag].astype(bool))
                    
                    n_selected = np.sum(mask)
                    
                    if n_selected == 0:
                        continue
                    
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
    Uses chunking to avoid memory issues with large files
    """
    if not os.path.exists(data_file):
        return np.zeros(len(bins) - 1)
    
    try:
        with uproot.open(data_file) as file:
            tree = file["outputTree"]
            available_branches = tree.keys()
            
            # Check if all required branches exist
            required_branches = [variable, region_flag, channel_flag]
            missing_branches = [b for b in required_branches if b not in available_branches]
            
            if missing_branches:
                return np.zeros(len(bins) - 1)
            
            # Get total number of entries
            num_entries = tree.num_entries
            
            # Process in chunks to avoid memory issues
            chunk_size = 1_000_000  # Process 1M events at a time
            data_hist = np.zeros(len(bins) - 1)
            
            for start in range(0, num_entries, chunk_size):
                stop = min(start + chunk_size, num_entries)
                
                # Read chunk
                arr = tree.arrays(required_branches, library="np", 
                                entry_start=start, entry_stop=stop)
                
                # Create combined mask
                mask = (arr[region_flag].astype(bool)) & (arr[channel_flag].astype(bool))
                
                n_selected = np.sum(mask)
                
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
            
            return data_hist
            
    except Exception as e:
        return np.zeros(len(bins) - 1)


def plot_stacked_histogram(hist_data, data_hist, bins, variable, region_name, channel, output_path):
    """
    Create and save stacked histogram with data overlay and ratio plot
    """
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10), sharex=True,
                                   gridspec_kw={'height_ratios': [3, 1]})
    
    # Calculate bin centers and widths
    bin_centers = 0.5 * (bins[:-1] + bins[1:])
    bin_widths = np.diff(bins)
    
    # --- Stack Plot ---
    # Stack in the order defined in file_groups
    bottom = np.zeros(len(bins) - 1)
    for group_name in file_groups.keys():
        if group_name in hist_data and np.sum(hist_data[group_name]) > 0:
            ax1.bar(bin_centers, hist_data[group_name], width=bin_widths, bottom=bottom,
                    color=colors[group_name], label=display_names[group_name], 
                    align='center', alpha=0.8)
            bottom += hist_data[group_name]
    
    # Overlay data as points
    data_errors = np.sqrt(data_hist)
    data_errors[data_hist == 0] = 0  # No error for empty bins
    
    ax1.errorbar(bin_centers, data_hist, yerr=data_errors, 
                 fmt='o', color='black', label='Data', markersize=6, capsize=3, linewidth=1.5)
    
    ax1.set_ylabel("Events", fontsize=14, fontweight='bold')
    ax1.set_title(f"{variable} - {region_name} - {channel}", fontsize=16, fontweight='bold', pad=15)
    ax1.legend(loc='best', fontsize=11, frameon=True, shadow=True)
    ax1.grid(True, alpha=0.3, linestyle='--')
    ax1.tick_params(labelsize=12)
    
    # Set y-axis limits
    max_val = max(np.max(bottom), np.max(data_hist))
    if max_val > 0:
        ax1.set_ylim(bottom=0, top=max_val * 1.4)
    
    # --- Ratio Plot ---
    mc_total = np.sum([hist_data[group] for group in file_groups if group in hist_data], axis=0)
    
    # Calculate ratio and error
    ratio = np.zeros_like(data_hist, dtype=float)
    ratio_err = np.zeros_like(data_hist, dtype=float)
    
    valid = mc_total > 0
    ratio[valid] = data_hist[valid] / mc_total[valid]
    ratio_err[valid] = np.sqrt(data_hist[valid]) / mc_total[valid]
    
    ax2.errorbar(bin_centers, ratio, yerr=ratio_err, fmt='o', color='black', 
                 markersize=6, capsize=3, linewidth=1.5)
    ax2.axhline(1.0, color='red', linestyle='--', linewidth=2)
    ax2.fill_between(bins, 0.9, 1.1, alpha=0.2, color='gray')
    ax2.set_ylabel("Data / MC", fontsize=14, fontweight='bold')
    ax2.set_xlabel(variable, fontsize=14, fontweight='bold')
    ax2.grid(True, alpha=0.3, linestyle='--')
    ax2.set_ylim(0, 2)
    ax2.tick_params(labelsize=12)
    
    plt.tight_layout()
    
    # Save figure
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f"      ✓ Saved: {output_path}")


# === Main Execution ===
if __name__ == "__main__":
    
    print("\n" + "="*80)
    print("STACKED HISTOGRAM GENERATOR")
    print("="*80)
    print(f"Processing {len(variables_to_plot)} variable(s) across {len(regions)} region(s)")
    print(f"Output directory: {base_output_dir}/")
    print("="*80 + "\n")
    
    total_plots = 0
    
    # Loop over all variables
    for variable, binning in variables_to_plot:
        print(f"\n{'='*80}")
        print(f"VARIABLE: {variable}")
        print(f"Binning: {binning[0]} to {binning[1]} with {binning[2]} bins")
        print(f"{'='*80}")
        
        # Create bins for this variable
        bins = np.linspace(binning[0], binning[1], binning[2] + 1)
        
        # Loop over all regions
        for region_flag, region_dir in regions.items():
            print(f"\n  Region: {region_flag} ({region_dir})")
            
            # Create output directory for this region
            output_dir = os.path.join(base_output_dir, region_dir)
            os.makedirs(output_dir, exist_ok=True)
            
            # Loop over all channels
            for channel_name, channel_flag in channels.items():
                print(f"    Channel: {channel_name}")
                
                # Load MC histogram data
                hist_data = load_histogram_data(file_groups, directory, variable, 
                                               region_flag, channel_flag, bins)
                
                # Load data histogram
                data_hist = load_data_histogram(data_file, variable, 
                                               region_flag, channel_flag, bins)
                
                # Check if we have any data
                total_mc = sum([np.sum(hist_data[g]) for g in file_groups if g in hist_data])
                total_data = np.sum(data_hist)
                
                print(f"      MC events: {total_mc:.1f} | Data events: {total_data:.0f}")
                
                if total_mc > 0 or total_data > 0:
                    # Create output filename
                    safe_channel = channel_name.replace(" ", "_")
                    output_path = os.path.join(output_dir, f"{variable}_{safe_channel}.png")
                    
                    # Create plot
                    plot_stacked_histogram(hist_data, data_hist, bins, variable, 
                                          region_dir, channel_name, output_path)
                    total_plots += 1
                else:
                    print(f"      ⚠ No events found, skipping plot")
    
    print(f"\n{'='*80}")
    print(f"✓ COMPLETE!")
    print(f"✓ Generated {total_plots} plots")
    print(f"✓ Plots organized in: {base_output_dir}/{{1j1b, 2j1b, 2j2b}}/")
    print(f"{'='*80}\n")
