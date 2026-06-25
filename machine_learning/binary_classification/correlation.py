import ROOT
import os
import numpy as np
import pandas as pd

# ==============================
# ROOT setup (LPC safe)
# ==============================
ROOT.EnableImplicitMT()
ROOT.gROOT.SetBatch(True)
ROOT.gErrorIgnoreLevel = ROOT.kWarning

# ==============================
# Inputs
# ==============================
input_dir = "../../merged"
tree_name = "outputTree"

bdt_vars = [
    "leading_lepton_pt",
    "dilepton_del_phi",
    "leptons_invariant_mass",
    "dilepton_jet_pt",
    "Selected_loosejet_leadingpt",
    "leading_lepton_jet_pt",
    "dilepton_jet_mass",
    "sphericity",
    "aplanery",
    "delR_leadinglepton_jet",
    "delR_ele_muon",
    "subleading_lepton_pt",
    "HT",
    "ST",
    "MT2",
    "centrality",
    "lepton_pt_asymmetry",
    "PuppiMET_pt_corr",
    "PuppiMET_phi_corr",
    "dilepton_invariant_mass",
    "mbl_min",
    "mbl_max",
    "max_delR",
    "min_delR",
    "jet_pt_assymmetry",
    "jet_pt_ratio",
    "dijet_mass"
]

regions = {
    "region_1j1t": "region_1j1t == 1",
    "region_2j1t": "region_2j1t == 1",
}

channel_cut = "eu_channel == 1"

# ------------------------------
# Files
# ------------------------------
signal_files = [
    "TWminusto2L2Nu.root",
    "TbarWplusto2L2Nu.root",
]

background_files = [
    "TTbar_Dilept.root",
    "TTbar_SemiLept.root",
]

data_files = [
    "data.root",
]

signal_paths     = [os.path.join(input_dir, f) for f in signal_files]
background_paths = [os.path.join(input_dir, f) for f in background_files]
data_paths       = [os.path.join(input_dir, f) for f in data_files]

os.makedirs("correlations", exist_ok=True)

# ==============================
# Helpers
# ==============================
def get_rdf(files):
    return ROOT.RDataFrame(tree_name, files)


def make_correlation(rdf, label, region_name):
    """
    Compute and save a well-formatted correlation matrix for a given RDF.
    - X-axis labels rotated 90 degrees
    - Numbers clearly visible inside bins
    - Larger canvas with generous margins
    """

    nevt = rdf.Count().GetValue()
    print(f"[{label}] Events after selection: {nevt}")

    if nevt == 0:
        print(f"[SKIP] No events for {label} in {region_name}")
        return

    # ----------------------------------
    # Remove zero-variance / bad vars
    # ----------------------------------
    good_vars = []
    for v in bdt_vars:
        try:
            vmin = rdf.Min(v).GetValue()
            vmax = rdf.Max(v).GetValue()
            if vmin == vmax or np.isnan(vmin) or np.isnan(vmax):
                print(f"[{label} DROP] {v:35s} (zero variance or NaN)")
            else:
                good_vars.append(v)
        except Exception as e:
            print(f"[{label} DROP] {v:35s} (exception: {e})")

    if len(good_vars) < 2:
        print(f"[SKIP] Not enough valid variables for {label} in {region_name}")
        return

    print(f"[{label}] Using {len(good_vars)} variables:")
    for v in good_vars:
        print(f"  - {v}")

    # ----------------------------------
    # Convert to numpy
    # ----------------------------------
    data = rdf.AsNumpy(good_vars)
    df   = pd.DataFrame(data)
    df   = df.replace([np.inf, -np.inf], np.nan).dropna()

    if len(df) < 2:
        print(f"[SKIP] Too few clean events for {label} in {region_name}")
        return

    # ----------------------------------
    # Pearson correlation
    # ----------------------------------
    corr = df.corr(method="pearson")

    # Save CSV
    csv_out = f"correlations/corr_{region_name}_{label}.csv"
    corr.to_csv(csv_out)
    print(f"[{label}] Saved CSV: {csv_out}")

    # ----------------------------------
    # Plotting
    # ----------------------------------
    n = len(good_vars)

    h_corr = ROOT.TH2D(
        f"h_corr_{region_name}_{label}",
        f"Correlation matrix  ({region_name}, {label})",
        n, 0, n,
        n, 0, n,
    )

    for i, vx in enumerate(good_vars):
        for j, vy in enumerate(good_vars):
            h_corr.SetBinContent(i + 1, j + 1, corr.loc[vx, vy])

    # ---- Axis labels ----
    for i, v in enumerate(good_vars):
        h_corr.GetXaxis().SetBinLabel(i + 1, v)
        h_corr.GetYaxis().SetBinLabel(i + 1, v)

    # X-axis: rotate 90 degrees so long names don't overlap
    h_corr.GetXaxis().SetLabelSize(0.030)   # slightly smaller font
    h_corr.GetXaxis().LabelsOption("v")     # "v" = vertical (90-degree rotation)
    h_corr.GetXaxis().SetLabelOffset(0.005)

    # Y-axis: keep horizontal, slightly larger
    h_corr.GetYaxis().SetLabelSize(0.033)
    h_corr.GetYaxis().SetLabelOffset(0.005)

    # Z-axis (color bar)
    h_corr.GetZaxis().SetLabelSize(0.028)
    h_corr.GetZaxis().SetTitleSize(0.030)

    h_corr.SetMinimum(-1.0)
    h_corr.SetMaximum(+1.0)

    # Title
    h_corr.GetXaxis().SetTitle("")
    h_corr.GetYaxis().SetTitle("")
    h_corr.SetTitleSize(0.040)

    # ---- Global style ----
    ROOT.gStyle.SetOptStat(0)
    ROOT.gStyle.SetPaintTextFormat(".2f")   # 2 decimal places in boxes

    # Use a diverging palette: blue (−1) → white (0) → red (+1)
    ROOT.gStyle.SetPalette(ROOT.kCool)      # fallback; overridden below
    n_cont = 255
    ROOT.gStyle.SetNumberContours(n_cont)

    # Custom blue-white-red palette
    stops  = np.array([0.00, 0.50, 1.00])
    red    = np.array([0.00, 1.00, 1.00])
    green  = np.array([0.00, 1.00, 0.00])
    blue   = np.array([1.00, 1.00, 0.00])
    ROOT.TColor.CreateGradientColorTable(
        len(stops),
        stops, red, green, blue,
        n_cont,
    )
    ROOT.gStyle.SetNumberContours(n_cont)

    # ---- Canvas ----
    # Make canvas big enough: ~60 px per variable (minimum 1400)
    px_per_var = 65
    base       = max(1600, n * px_per_var + 500)   # extra room for margins
    c = ROOT.TCanvas(f"c_{region_name}_{label}", "", base, base)
    c.SetBatch(True)

    # Generous margins to accommodate rotated x-labels and y-labels
    # Bottom margin is larger because rotated labels hang downward
    c.SetLeftMargin(0.26)
    c.SetBottomMargin(0.26)
    c.SetRightMargin(0.14)
    c.SetTopMargin(0.07)

    # ---- Draw ----
    # TEXT option prints numbers; COLZ adds the colour palette
    # TextSize controls the in-bin numbers
    h_corr.SetMarkerSize(1.0)     # scale the TEXT size inside bins

    # For many variables the default marker size makes numbers overlap;
    # reduce if n > 15
    if n > 15:
        h_corr.SetMarkerSize(0.70)
    if n > 18:
        h_corr.SetMarkerSize(0.55)

    h_corr.Draw("COLZ TEXT")

    # Force axis label redraw after Draw() so LabelsOption takes effect
    c.Update()

    # ---- Save ----
    png_out = f"correlations/corr_{region_name}_{label}.png"
    c.SaveAs(png_out)
    print(f"[{label}] Saved PNG: {png_out}")

    del c, h_corr


# ==============================
# Main logic
# ==============================
for region_name, region_cut in regions.items():

    print("\n" + "=" * 60)
    print(f"Processing {region_name}")
    print("=" * 60)

    # ---- Signal ----
    rdf_signal = (
        get_rdf(signal_paths)
        .Filter(channel_cut)
        .Filter(region_cut)
    )
    make_correlation(rdf_signal, "Signal", region_name)

    # ---- Background ----
    rdf_background = (
        get_rdf(background_paths)
        .Filter(channel_cut)
        .Filter(region_cut)
    )
    make_correlation(rdf_background, "Background", region_name)

    # ---- Data ----
    rdf_data = (
        get_rdf(data_paths)
        .Filter(channel_cut)
        .Filter(region_cut)
    )
    make_correlation(rdf_data, "Data", region_name)

print("\nAll done.")
