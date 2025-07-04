import ROOT

# Open the file
file = ROOT.TFile("output_histograms.root", "READ")

# Histogram categories and colors
categories = {
    "multijet": {"color": ROOT.kBlack - 2, "sf": 0.7689},
    "wjets": {"color": ROOT.kOrange - 1, "sf": 1.187},
    "ttbar": {"color": ROOT.kGreen, "sf": 1.0339},
    "dyjets": {"color": ROOT.kYellow, "sf": 1.187},
    "stw": {"color": ROOT.kCyan , "sf": 1.0339},
    "signal_top": {"color": ROOT.kRed, "sf": 0.97932},
    "signal_antitop": {"color": ROOT.kBlue - 1, "sf": 0.96528}
}

# Load histograms and apply scale factors
histograms = {}
for cat, props in categories.items():
    hist = file.Get(f"{cat}_electronChannel") if "signal" in cat else file.Get(f"{cat}_electronChannel") if cat != "multijet" else file.Get("multijet")
    hist.SetDirectory(0)
    hist.Scale(props["sf"])
    hist.SetFillColor(props["color"])
    histograms[cat] = hist

# Load data
hist_data = file.Get("data")
hist_data.SetDirectory(0)
hist_data.SetMarkerStyle(20)
hist_data.SetMarkerSize(1)
hist_data.SetLineColor(ROOT.kBlack)

# Stack MC histograms
stack = ROOT.THStack("stack", "")
order = ["multijet", "wjets", "dyjets", "stw", "ttbar", "signal_top", "signal_antitop"]
for cat in order:
    stack.Add(histograms[cat])

# Create canvas and pads
canvas = ROOT.TCanvas("canvas", "Stacked Plot", 800, 800)
canvas.Divide(1, 2)
pad1 = canvas.cd(1)
pad1.SetPad(0.0, 0.3, 1.0, 1.0)
pad1.SetBottomMargin(0.02)
pad1.SetLogy(False)
stack.Draw("HIST")
hist_data.Draw("E SAME")
stack.GetXaxis().SetLabelSize(0)
stack.GetYaxis().SetTitle("Events")
stack.GetYaxis().SetTitleSize(0.05)
stack.GetYaxis().SetTitleOffset(1.2)

# Make legend
legend = ROOT.TLegend(0.65, 0.5, 0.89, 0.89)
for cat in order:
    legend.AddEntry(histograms[cat], cat, "f")
legend.AddEntry(hist_data, "data", "lep")
legend.Draw()

# Create ratio plot
pad2 = canvas.cd(2)
pad2.SetPad(0.0, 0.0, 1.0, 0.3)
pad2.SetTopMargin(0.05)
pad2.SetBottomMargin(0.35)

# Sum of all MC
hist_sum_mc = histograms[order[0]].Clone("sum_mc")
for cat in order[1:]:
    hist_sum_mc.Add(histograms[cat])

ratio = hist_data.Clone("ratio")
ratio.Divide(hist_sum_mc)
ratio.SetLineColor(ROOT.kBlack)
ratio.SetMarkerStyle(20)
ratio.SetTitle("")

# Ratio axis formatting
ratio.GetYaxis().SetTitle("Data / MC")
ratio.GetYaxis().SetNdivisions(505)
ratio.GetYaxis().SetTitleSize(0.09)
ratio.GetYaxis().SetTitleOffset(0.5)
ratio.GetYaxis().SetLabelSize(0.08)
ratio.GetXaxis().SetTitle("W boson transverse mass (GeV)")
ratio.GetXaxis().SetTitleSize(0.1)
ratio.GetXaxis().SetLabelSize(0.08)

ratio.Draw("E")

# Draw unity line
line = ROOT.TLine(ratio.GetXaxis().GetXmin(), 1, ratio.GetXaxis().GetXmax(), 1)
line.SetLineColor(ROOT.kRed)
line.SetLineStyle(2)
line.Draw()

# Save
canvas.SaveAs("top_mass_stacked.pdf")
print("Saved plot as top_mass_stacked.pdf")

