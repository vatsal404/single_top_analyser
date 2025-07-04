import uproot
import numpy as np
import matplotlib.pyplot as plt

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

LUMINOSITY = 41480.0  # in pb^-1

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

# Scale factors placeholder — set all to 1.0 for now
SCALE_FACTORS = {k: 1.0 for k in CROSS_SECTIONS}

variable_name = "Wboson_transversMass"
bins = 32
x_min = 0
x_max = 200
bin_edges = np.linspace(x_min, x_max, bins + 1)

def get_hist(file, category, selection):
    tree = uproot.open(f"{file}:outputTree")
    fname = file.split("/")[-1].replace(".root", "")

    if file == "data.root":
        array = tree.arrays([variable_name, "evWeight", "QCDelectronchannel", "region_2j1t"], library="np")
        mask = (array["QCDelectronchannel"]) & (array["region_2j1t"])
        values = array[variable_name][mask]
        weights = np.ones_like(values)
    else:
        array = tree.arrays([variable_name, "genWeight", "evWeight", "electronChannel", "region_2j1t"], library="np")
        mask = (array["electronChannel"]) & (array["region_2j1t"])
        values = array[variable_name][mask]
        weights = array["genWeight"][mask] * array["evWeight"][mask]
        scale = LUMINOSITY * CROSS_SECTIONS[fname] / SUM_GEN_WEIGHTS[fname] * SCALE_FACTORS[fname]
        weights *= scale

    hist, _ = np.histogram(values, bins=bin_edges, weights=weights)
    return hist

# Categorize files
categories = {
    "signal_top": ["PROCESSED_ST_tchannel_top_5f"],
    "signal_antitop": ["PROCESSED_ST_tchannel_antitop_5f"],
    "ttbar": ["PROCESSED_TTbar-channel_top_UL17", "PROCESSED_ST_tW_top_5f", "PROCESSED_ST_tW_antitop_5f"],
    "wjets": ["PROCESSED_WJetsToLNu_0J_UL17", "PROCESSED_WJetsToLNu_1J_UL17", "PROCESSED_WJetsToLNu_2J_UL17", 
              "PROCESSED_DYJetsToLL_M-50_UL17", "PROCESSED_DYJetsToLL_M-10to50_UL17"]
}

histograms = {cat: np.zeros(len(bin_edges) - 1) for cat in categories}

for file in INPUT_FILES:
    fname = file.split("/")[-1].replace(".root", "")
    for cat, members in categories.items():
        if fname in members:
            histograms[cat] += get_hist(file, cat, "electronChannel && region_2j1t")

# Get data histogram (for multijet estimate)
data_hist = get_hist("data.root", "data", "QCDelectronchannel && region_2j1t")

# Subtract all backgrounds from data to get multijet
bg_total = sum(histograms.values())
histograms["multijet"] = data_hist - bg_total
histograms["multijet"] = np.maximum(histograms["multijet"], 0)

# Plotting
plt.figure(figsize=(10, 8))

# Stack order (bottom to top)
stack_order = ["multijet", "wjets", "ttbar", "signal_antitop", "signal_top"]
colors = {
    "signal_top": "#ff6666",        # kRed-7
    "signal_antitop": "#6666ff",    # kBlue-7
    "ttbar": "#66cc66",             # kGreen-7
    "wjets": "#66ccff",             # kAzure-9
    "multijet": "#ffcc99"           # kOrange-3
}
edges = bin_edges
stack_data = [histograms[cat] for cat in stack_order]
stack_labels = stack_order

plt.hist([edges[:-1]] * len(stack_data), bins=edges, weights=stack_data,
         stacked=True, color=[colors[cat] for cat in stack_order],
         label=stack_labels, edgecolor='black')

plt.xlabel(variable_name)
plt.ylabel("Events")
plt.title(f"{variable_name} Distribution (2j1t Region)")
plt.legend()
plt.grid(True)

# Ratio plot
plt.figure(figsize=(10, 3))
mc_sum = sum(stack_data[:-1]) + 1e-6  # avoid zero division
ratio = data_hist / mc_sum
bin_centers = 0.5 * (bin_edges[1:] + bin_edges[:-1])
plt.errorbar(bin_centers, ratio, yerr=np.sqrt(data_hist) / mc_sum, fmt='o', color='black')
plt.axhline(1.0, color='red', linestyle='--')
plt.ylabel("Data/MC")
plt.xlabel(variable_name)
plt.grid(True)
plt.tight_layout()
plt.show()

