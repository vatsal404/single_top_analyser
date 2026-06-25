import uproot
import numpy as np
import matplotlib.pyplot as plt
import os

# === User Config ===
directory = "merged/"  # Path where all ROOT files are stored

# Base output directory
base_output_dir = "plots"

# Human-readable axis labels for each variable
variable_labels = {
    "goodmuons_leading_pt": "Leading Muon $p_T$ [GeV]",
    "goodmuons_leading_eta": "Leading Muon $\\eta$",
    "goodmuons_leading_phi": "Leading Muon $\\phi$",
    "goodElectrons_leading_pt": "Leading Electron $p_T$ [GeV]",
    "goodElectrons_leading_eta": "Leading Electron $\\eta$",
    "goodElectrons_leading_phi": "Leading Electron $\\phi$",
    "leading_lepton_pt": "Leading Lepton $p_T$ [GeV]",
    "subleading_lepton_pt": "Subleading Lepton $p_T$ [GeV]",
    "Electron_pt_corr": "Corrected Electron $p_T$ [GeV]",
    "Muon_pt_corr": "Corrected Muon $p_T$ [GeV]",
    "PuppiMET_pt_corr": "Corrected PuppiMET $p_T$ [GeV]",
    "PuppiMET_phi_corr": "Corrected PuppiMET $\\phi$",
    "leptons_invariant_mass": "Dilepton Invariant Mass [GeV]",
    "dilepton_invariant_mass": "Dilepton Invariant Mass [GeV]",
    "dilepton_jet_pt": "Dilepton + Jet $p_T$ [GeV]",
    "dilepton_jet_mass": "Dilepton + Jet Mass [GeV]",
    "dilepton_del_phi": "$\\Delta\\phi$ (Dilepton)",
    "Selected_jet_leading_pt": "Leading Jet $p_T$ [GeV]",
    "Selected_jet_subleading_pt": "Subleading Jet $p_T$ [GeV]",
    "Selected_jet_leading_phi": "Leading Jet $\\phi$",
    "Selected_jet_leading_eta": "Leading Jet $\\eta$",
    "Selected_jet_leading_mass": "Leading Jet Mass [GeV]",
    "Selected_loosejet_leadingpt": "Leading Loose Jet $p_T$ [GeV]",
    "leading_lepton_jet_pt": "Leading Lepton + Jet $p_T$ [GeV]",
    "delR_dilepton_jet": "$\\Delta R$ (Dilepton, Jet)",
    "delR_ele_muon": "$\\Delta R$ (Electron, Muon)",
    "delR_leadinglepton_jet": "$\\Delta R$ (Leading Lepton, Jet)",
    "sphericity": "Sphericity",
    "aplanery": "Aplanarity",
    "centrality": "Centrality",
}

# Variables to plot with custom binning: (variable_name, (start, end, num_bins))
variables_to_plot = [

    # Global event variables
    ("HT", (0, 1000, 40)),
    ("ST", (0, 1500, 40)),
    ("MT2", (0, 300, 30)),
    ("centrality", (0, 1, 20)),
    ("sphericity", (0, 1, 20)),
    ("aplanery", (0, 0.5, 20)),

    # Lepton kinematics
    ("goodmuons_leading_pt", (0, 200, 25)),
    ("goodmuons_leading_eta", (-2.5, 2.5, 20)),
    ("goodmuons_leading_phi", (-3.2, 3.2, 20)),
    ("goodmuons_leading_mass", (0, 1, 20)),

    ("goodElectrons_leading_pt", (0, 200, 25)),
    ("goodElectrons_leading_eta", (-2.5, 2.5, 20)),
    ("goodElectrons_leading_phi", (-3.2, 3.2, 20)),
    ("goodElectrons_leading_mass", (0, 1, 20)),

    ("leading_lepton_pt", (0, 300, 30)),
    ("subleading_lepton_pt", (0, 200, 20)),
    ("lepton_pt_asymmetry", (0, 1, 20)),

    # Corrected lepton pT
    ("Electron_pt_corr", (0, 200, 30)),
    ("Muon_pt_corr", (0, 200, 30)),


    # MET
    ("PuppiMET_pt_corr", (0, 300, 30)),
    ("PuppiMET_phi_corr", (-3.2, 3.2, 25)),

    # Dilepton system
    ("leptons_invariant_mass", (0, 300, 30)),
    ("dilepton_invariant_mass", (0, 300, 30)),
    ("mbl_min", (0, 300, 30)),
    ("mbl_max", (0, 500, 40)),

    ("dilepton_jet_pt", (0, 400, 25)),
    ("dilepton_jet_mass", (0, 500, 25)),
    ("dilepton_del_phi", (0, 3.2, 20)),

    # Jet kinematics
    ("Selected_jet_leading_pt", (0, 400, 30)),
    ("Selected_jet_leading_eta", (-2.5, 2.5, 20)),
    ("Selected_jet_leading_phi", (-3.2, 3.2, 20)),
    ("Selected_jet_leading_mass", (0, 100, 25)),

    ("Selected_jet_subleading_pt", (0, 300, 25)),
    ("Selected_jet_subleading_eta", (-2.5, 2.5, 20)),
    ("Selected_jet_subleading_phi", (-3.2, 3.2, 20)),
    ("Selected_jet_subleading_mass", (0, 100, 25)),

    # Loose jets
    ("Selected_loosejet_leadingpt", (0, 100, 25)),

    # b-jet kinematics
    ("Selected_bjet_leading_pt", (0, 400, 30)),
    ("Selected_bjet_leading_eta", (-2.5, 2.5, 20)),
    ("Selected_bjet_leading_phi", (-3.2, 3.2, 20)),
    ("Selected_bjet_leading_mass", (0, 100, 25)),

    # Lepton-jet observables
    ("leading_lepton_jet_pt", (0, 400, 25)),

    # Angular separations (DeltaR)
    ("delR_dilepton_jet", (0, 6, 20)),
    ("delR_ele_muon", (0, 6, 20)),
    ("delR_leadinglepton_jet", (0, 6, 20)),
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
    "drell_yan": [
    "DYto2E-2Jets_MLL-10to50.root",
    "DYto2E-2Jets_MLL-50.root",
    "DYto2Mu-2Jets_MLL-10to50.root",
    "DYto2Mu-2Jets_MLL-50.root",
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
    "ttbar": [
        "TTbar_Dilept.root",
    ],
    "ttbar_semileptonic": [
        "TTbar_SemiLept.root"
    ],
    "signal": [
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
        "WtoLNu.root",
        "WtoENu-2Jets.root",
        "WtoMuNu-2Jets.root"
    ],
}

# Colors for different processes
colors = {
    "signal": "gold",
    "ttbar": "red",
    "drell_yan": "blue",
    "ttg_ttv": "gray",
    "ttbar_semileptonic": "brown",
    "vv": "purple",
    "wjets": "green",
}

# Display names for legend (matplotlib LaTeX formatting)
display_names = {
    "signal": r"single $tW$",
    "ttbar": r"$t\bar{t}$ dileptonic",
    "drell_yan": r"Drell-Yan",
    "ttg_ttv": r"$t\bar{t}+\gamma$/V",
    "ttbar_semileptonic": r"$t\bar{t}$ semileptonic",
    "vv": r"Diboson",
    "wjets": r"W+jets",
}

# Data file
data_file = os.path.join(directory, "data.root")


def get_axis_label(variable):
    """Return a human-readable axis label for a given variable name."""
    return variable_labels.get(variable, variable.replace("_", " ").title())


def load_histogram_data(file_groups, directory, variable, region_flag, channel_flag, bins):
    """
    Load histogram data for a specific region and channel
    """
    hist_data = {}
    
    print(f"    Loading MC data for {region_flag} in {channel_flag}")
    
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
                    
                    required_branches = [variable, region_flag, channel_flag, "evWeight_hlt"]
                    missing_branches = [b for b in required_branches if b not in available_branches]
                    
                    if missing_branches:
                        continue
                    
                    arr = tree.arrays(required_branches, library="np")
                    
                    mask = (arr[region_flag].astype(bool)) & (arr[channel_flag].astype(bool))
                    
                    n_selected = np.sum(mask)
                    
                    if n_selected == 0:
                        continue
                    
                    filtered_values = arr[variable][mask]
                    filtered_weights = arr["evWeight_hlt"][mask]
                    
                    valid_mask = np.isfinite(filtered_values)
                    filtered_values = filtered_values[valid_mask]
                    filtered_weights = filtered_weights[valid_mask]
                    
                    if len(filtered_values) > 0:
                        values.append(filtered_values)
                        weights.append(filtered_weights)
                    
            except Exception as e:
                continue
        
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
    Load histogram data from data file for a specific region and channel.
    Uses chunking to avoid memory issues with large files.
    """
    if not os.path.exists(data_file):
        return np.zeros(len(bins) - 1)
    
    try:
        with uproot.open(data_file) as file:
            tree = file["outputTree"]
            available_branches = tree.keys()
            
            required_branches = [variable, region_flag, channel_flag]
            missing_branches = [b for b in required_branches if b not in available_branches]
            
            if missing_branches:
                return np.zeros(len(bins) - 1)
            
            num_entries = tree.num_entries
            chunk_size = 1_000_000
            data_hist = np.zeros(len(bins) - 1)
            
            for start in range(0, num_entries, chunk_size):
                stop = min(start + chunk_size, num_entries)
                
                arr = tree.arrays(required_branches, library="np", 
                                entry_start=start, entry_stop=stop)
                
                mask = (arr[region_flag].astype(bool)) & (arr[channel_flag].astype(bool))
                
                n_selected = np.sum(mask)
                
                if n_selected > 0:
                    filtered_values = arr[variable][mask]
                    
                    valid_mask = np.isfinite(filtered_values)
                    filtered_values = filtered_values[valid_mask]
                    
                    if len(filtered_values) > 0:
                        chunk_hist, _ = np.histogram(filtered_values, bins=bins)
                        data_hist += chunk_hist
            
            return data_hist
            
    except Exception as e:
        return np.zeros(len(bins) - 1)


def plot_stacked_histogram(hist_data, data_hist, bins, variable, region_name, channel, output_path):
    """
    Create and save stacked histogram with data overlay and ratio plot.
    """
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10), sharex=True,
                                   gridspec_kw={'height_ratios': [3, 1]})
    
    bin_centers = 0.5 * (bins[:-1] + bins[1:])
    bin_widths = np.diff(bins)
    
    # --- Stack Plot ---
    bottom = np.zeros(len(bins) - 1)
    for group_name in file_groups.keys():
        if group_name in hist_data and np.sum(hist_data[group_name]) > 0:
            ax1.bar(bin_centers, hist_data[group_name], width=bin_widths, bottom=bottom,
                    color=colors[group_name], label=display_names[group_name], 
                    align='center', alpha=0.8)
            bottom += hist_data[group_name]
    
    # Overlay data as points — bigger markers
    data_errors = np.sqrt(data_hist)
    data_errors[data_hist == 0] = 0

    ax1.errorbar(bin_centers, data_hist, yerr=data_errors, 
                 fmt='o', color='black', label='Data',
                 markersize=9,       # bigger dots
                 capsize=4,
                 linewidth=2.0)
    
    ax1.set_ylabel("Events", fontsize=18, fontweight='bold')
    # Title: region + channel only, no variable name
    ax1.set_title(f"{region_name}  |  {channel}", fontsize=17, fontweight='bold', pad=15)
    ax1.legend(loc='best', fontsize=16, frameon=True, shadow=True)  # bigger legend
    ax1.grid(True, alpha=0.3, linestyle='--')
    ax1.tick_params(axis='both', labelsize=14)   # bigger tick numbers
    
    max_val = max(np.max(bottom), np.max(data_hist))
    if max_val > 0:
        ax1.set_ylim(bottom=0, top=max_val * 1.4)
    
    # --- Ratio Plot ---
    mc_total = np.sum([hist_data[group] for group in file_groups if group in hist_data], axis=0)
    
    ratio = np.zeros_like(data_hist, dtype=float)
    ratio_err = np.zeros_like(data_hist, dtype=float)
    
    valid = mc_total > 0
    ratio[valid] = data_hist[valid] / mc_total[valid]
    ratio_err[valid] = np.sqrt(data_hist[valid]) / mc_total[valid]
    
    ax2.errorbar(bin_centers, ratio, yerr=ratio_err,
                 fmt='o', color='black',
                 markersize=9,       # bigger dots
                 capsize=4,
                 linewidth=2.0)
    ax2.axhline(1.0, color='red', linestyle='--', linewidth=2)
    ax2.fill_between(bins, 0.9, 1.1, alpha=0.2, color='gray')
    ax2.set_ylabel("Data / MC", fontsize=18, fontweight='bold')
    ax2.set_xlabel(get_axis_label(variable), fontsize=18, fontweight='bold')  # human-readable label
    ax2.grid(True, alpha=0.3, linestyle='--')
    ax2.set_ylim(0, 2)
    ax2.tick_params(axis='both', labelsize=14)   # bigger tick numbers
    
    plt.tight_layout()
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
    
    for variable, binning in variables_to_plot:
        print(f"\n{'='*80}")
        print(f"VARIABLE: {variable}  →  {get_axis_label(variable)}")
        print(f"Binning: {binning[0]} to {binning[1]} with {binning[2]} bins")
        print(f"{'='*80}")
        
        bins = np.linspace(binning[0], binning[1], binning[2] + 1)
        
        for region_flag, region_dir in regions.items():
            print(f"\n  Region: {region_flag} ({region_dir})")
            
            output_dir = os.path.join(base_output_dir, region_dir)
            os.makedirs(output_dir, exist_ok=True)
            
            for channel_name, channel_flag in channels.items():
                print(f"    Channel: {channel_name}")
                
                hist_data = load_histogram_data(file_groups, directory, variable, 
                                               region_flag, channel_flag, bins)
                
                data_hist = load_data_histogram(data_file, variable, 
                                               region_flag, channel_flag, bins)
                
                total_mc = sum([np.sum(hist_data[g]) for g in file_groups if g in hist_data])
                total_data = np.sum(data_hist)
                
                print(f"      MC events: {total_mc:.1f} | Data events: {total_data:.0f}")
                
                if total_mc > 0 or total_data > 0:
                    safe_channel = channel_name.replace(" ", "_")
                    output_path = os.path.join(output_dir, f"{variable}_{safe_channel}.png")
                    
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
