import ROOT
import os
import math

ROOT.gROOT.SetBatch(True)
ROOT.EnableImplicitMT()

# --- Global style: remove stats box, use plain white background ---
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetOptTitle(0)

input_dir = "../merged"
tree_name = "outputTree"

bdt_vars = [
    "sphericity",
    "aplanery",
    "centrality",
    "leading_lepton_pt",
    "leptons_invariant_mass",
    "dilepton_jet_pt",
    "Selected_loosejet_leadingpt",
    "dilepton_del_phi",
    "Selected_jet_leading_pt",
    "leading_lepton_jet_pt",
    "dilepton_jet_mass",
    "subleading_lepton_pt",
    "delR_dilepton_jet",
    "Selected_jet_subleading_pt",
    "delR_ele_muon",
    "delR_leadinglepton_jet",
    "HT",
    "ST",
    "MT2",
    "lepton_pt_asymmetry",
    "mbl_min",
    "mbl_max"

]

regions = {
    "region_1j1t": "region_1j1t == 1",
    "region_2j1t": "region_2j1t == 1",
}

channel_cut = "eu_channel == 1"

signal_files = [
    "TWminusto2L2Nu.root",
    "TbarWplusto2L2Nu.root",
]
background_files = [
    "TTbar_Dilept.root",
    "TTbar_SemiLept.root",
]

signal_paths     = [os.path.join(input_dir, f) for f in signal_files]
background_paths = [os.path.join(input_dir, f) for f in background_files]

os.makedirs("shape_plots", exist_ok=True)


def get_rdf(files):
    return ROOT.RDataFrame(tree_name, files)


def smart_xmax(rdf, var, xmin, xmax, quantile=0.99):
    """
    Return a clipped upper x bound based on the given quantile so that
    long sparse tails (e.g. high-pt jets) don't dominate the x axis.
    Uses a fine temporary histogram to estimate the quantile.
    """
    nbins_tmp = 2000
    h_tmp = rdf.Histo1D(
        (f"htmp_{var}", "", nbins_tmp, xmin, xmax), var
    ).GetValue()

    total = h_tmp.Integral()
    if total <= 0:
        return xmax

    target = quantile * total
    cumulative = 0.0
    for i in range(1, nbins_tmp + 1):
        cumulative += h_tmp.GetBinContent(i)
        if cumulative >= target:
            return h_tmp.GetBinLowEdge(i + 1)   # right edge of this bin
    return xmax


# --- Get column list once ---
rdf_check = get_rdf(signal_paths)
available_columns = set(str(c) for c in rdf_check.GetColumnNames())

print("\nAvailable branches in tree:")
for col in sorted(available_columns):
    print("  ", col)
print("--------------------------------------------------\n")


for region_name, region_cut in regions.items():
    rdf_sig = get_rdf(signal_paths).Filter(channel_cut).Filter(region_cut)
    rdf_bkg = get_rdf(background_paths).Filter(channel_cut).Filter(region_cut)

    for var in bdt_vars:
        if var not in available_columns:
            print(f"[SKIP] Variable '{var}' not found in tree. Skipping.")
            continue

        try:
            # --- Raw min/max ---
            xmin_sig = rdf_sig.Min(var).GetValue()
            xmax_sig = rdf_sig.Max(var).GetValue()
            xmin_bkg = rdf_bkg.Min(var).GetValue()
            xmax_bkg = rdf_bkg.Max(var).GetValue()

            xmin = min(xmin_sig, xmin_bkg)
            xmax = max(xmax_sig, xmax_bkg)

            if xmin == xmax:
                xmin -= 1.0
                xmax += 1.0

            # --- Clip upper tail at 99th percentile (both samples) ---
            xmax_clip_sig = smart_xmax(rdf_sig, var, xmin, xmax, quantile=0.99)
            xmax_clip_bkg = smart_xmax(rdf_bkg, var, xmin, xmax, quantile=0.99)
            # Use the larger of the two so neither sample is cut off
            xmax_plot = max(xmax_clip_sig, xmax_clip_bkg)

            # Safety: never let the clipped max collapse too much
            if xmax_plot <= xmin:
                xmax_plot = xmax

            nbins = 30

            h_sig = rdf_sig.Histo1D(
                (f"h_sig_{var}", "", nbins, xmin, xmax_plot), var
            ).GetValue().Clone()
            h_bkg = rdf_bkg.Histo1D(
                (f"h_bkg_{var}", "", nbins, xmin, xmax_plot), var
            ).GetValue().Clone()

            # --- Unit normalisation ---
            if h_sig.Integral() > 0:
                h_sig.Scale(1.0 / h_sig.Integral())
            if h_bkg.Integral() > 0:
                h_bkg.Scale(1.0 / h_bkg.Integral())

            # -------------------------------------------------------
            # Styling — larger text throughout
            # -------------------------------------------------------
            title_size  = 0.055   # axis titles
            label_size  = 0.050   # tick numbers
            title_offset_x = 1.1
            title_offset_y = 1.1

            for h, color in [(h_sig, ROOT.kRed), (h_bkg, ROOT.kBlue)]:
                h.SetLineColor(color)
                h.SetLineWidth(2)
                h.SetTitle("")

                h.GetXaxis().SetTitle(var)
                h.GetXaxis().SetTitleSize(title_size)
                h.GetXaxis().SetLabelSize(label_size)
                h.GetXaxis().SetTitleOffset(title_offset_x)

                h.GetYaxis().SetTitle("Normalized events")
                h.GetYaxis().SetTitleSize(title_size)
                h.GetYaxis().SetLabelSize(label_size)
                h.GetYaxis().SetTitleOffset(title_offset_y)

            ymax = max(h_sig.GetMaximum(), h_bkg.GetMaximum())
            h_sig.SetMaximum(1.4 * ymax)
            h_sig.SetMinimum(0.0)

            # --- Canvas ---
            c = ROOT.TCanvas("c", "", 800, 600)
            c.SetBatch(True)
            c.SetLeftMargin(0.13)
            c.SetRightMargin(0.05)
            c.SetBottomMargin(0.13)
            c.SetTopMargin(0.06)

            h_sig.Draw("HIST")
            h_bkg.Draw("HIST SAME")

            # --- Legend (larger text) ---
            leg = ROOT.TLegend(0.65, 0.75, 0.90, 0.90)
            leg.SetBorderSize(0)
            leg.SetFillStyle(0)
            leg.SetTextSize(0.050)
            leg.AddEntry(h_sig, "tW",    "l")
            leg.AddEntry(h_bkg, "t#bar{t}", "l")
            leg.Draw()

            c.SaveAs(f"shape_plots/{var}_{region_name}.png")
            del c, h_sig, h_bkg

        except Exception as e:
            print(f"[ERROR] Failed for variable '{var}' in {region_name}: {e}")
            continue
