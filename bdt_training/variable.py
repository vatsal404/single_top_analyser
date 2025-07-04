import ROOT
import os

ROOT.ROOT.EnableImplicitMT()

LUMINOSITY = 41480.0
BINNING = (32, 100, 400)


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
    "data.root"
]

CROSS_SECTIONS = {
    "PROCESSED_ST_tchannel_antitop_5f": 80.95,
    "PROCESSED_ST_tchannel_top_5f": 136.02,
    "PROCESSED_TTbar-channel_top_UL17": 831.76,
    "PROCESSED_WJetsToLNu_0J_UL17": 49670.0,
    "PROCESSED_WJetsToLNu_1J_UL17": 8264.0,
    "PROCESSED_WJetsToLNu_2J_UL17": 3226.0,
    "PROCESSED_ST_tW_top_5f": 35.85,
    "PROCESSED_ST_tW_antitop_5f": 35.85,
    "PROCESSED_DYJetsToLL_M-50_UL17": 4895.0,
    "PROCESSED_DYJetsToLL_M-10to50_UL17": 18610.0
}

SUM_GEN_WEIGHTS = {
    "PROCESSED_ST_tchannel_antitop_5f": 4462868882.059891,
    "PROCESSED_ST_tchannel_top_5f": 13808000645.662537,
    "PROCESSED_TTbar-channel_top_UL17": 165610016.0,
    "PROCESSED_WJetsToLNu_0J_UL17": 9552834599026.219,
    "PROCESSED_WJetsToLNu_1J_UL17": 8064660889430.6875,
    "PROCESSED_WJetsToLNu_2J_UL17": 2350902331638.5547,
    "PROCESSED_ST_tW_top_5f": 183187615.677055,
    "PROCESSED_ST_tW_antitop_5f": 184446306.893539,
    "PROCESSED_DYJetsToLL_M-50_UL17": 98921180.0,
    "PROCESSED_DYJetsToLL_M-10to50_UL17": 68480179.0
}

categories = {
    "ttbar": [],
    "wjets": [],
    "dyjets": [],
    "signal_top": [],
    "signal_antitop": [],
    "stw": []
}

for filepath in INPUT_FILES:
    filename = os.path.basename(filepath).replace(".root", "")
    if "TTbar" in filename:
        categories["ttbar"].append((filename, filepath))
    elif "WJetsToLNu" in filename:
        categories["wjets"].append((filename, filepath))
    elif "DYJets" in filename:
        categories["dyjets"].append((filename, filepath))
    elif "tchannel_top" in filename:
        categories["signal_top"].append((filename, filepath))
    elif "tchannel_antitop" in filename:
        categories["signal_antitop"].append((filename, filepath))
    elif "tW" in filename:
        categories["stw"].append((filename, filepath))

output_file = ROOT.TFile("output_histograms.root", "RECREATE")

def make_mc_hist(category_name, file_list, region):
    hist = ROOT.TH1D(category_name + "_" + region, category_name, *BINNING)
    for fname, path in file_list:
        xsec = CROSS_SECTIONS[fname]
        sumw = SUM_GEN_WEIGHTS[fname]
        scale = LUMINOSITY * xsec / sumw
        df = ROOT.RDataFrame("outputTree", path)
        df_sel = df.Filter(f"region_2j1t && {region}")\
                   .Define("weight", f"evWeight * genWeight * {scale}")
        hist_tmp = df_sel.Histo1D(("tmp", "", *BINNING), "top_mass", "weight")
        hist.Add(hist_tmp.GetValue())
    return hist

# Make MC histograms in both regions
hist_mc = {}
for cat in categories:
    hist_mc[cat] = make_mc_hist(cat, categories[cat], "electronChannel")
    hist_mc[cat].Write()

# QCD region MC sum
hist_mc_qcd = ROOT.TH1D("mc_qcd", "Sum MC QCD", *BINNING)
for cat in categories:
    h_tmp = make_mc_hist(cat, categories[cat], "QCDelectronChannel")
    hist_mc_qcd.Add(h_tmp)
hist_mc_qcd.Write()

# Data histograms
df_data = ROOT.RDataFrame("outputTree", "data.root")
hist_data = df_data.Filter("region_2j1t && electronChannel")\
                   .Histo1D(("data", "ElectronChannel Data", *BINNING), "top_mass")
hist_data.Write()

hist_data_qcd = df_data.Filter("region_2j1t && QCDelectronChannel")\
                       .Histo1D(("data_qcd", "QCD Data", *BINNING), "top_mass")
hist_data_qcd.Write()

# Multijet = data_qcd - sum_MC_qcd, with non-negative bins
hist_multijet = hist_data_qcd.GetValue().Clone("multijet")
hist_multijet.SetTitle("Multijet")
hist_multijet.Add(hist_mc_qcd, -1)

for i in range(1, hist_multijet.GetNbinsX() + 1):
    if hist_multijet.GetBinContent(i) < 0:
        hist_multijet.SetBinContent(i, 0)

hist_multijet.Write()

output_file.Close()
print("Histograms saved to output_histograms.root")

