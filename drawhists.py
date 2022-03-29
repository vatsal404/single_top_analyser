import stackhists
import CMS_lumi
import sys
import argparse
import tdrstyle

tdrstyle.setTDRStyle()
# Subplot ( Ratio plot : option -R, --ratio (default) / significance plot :  option -S, --significance )
parser = argparse.ArgumentParser()
parser.add_argument('-R', '--ratio', dest='ratio', action='store_true', default=False)
parser.add_argument('-S', '--significance', dest='significance', action='store_true', default=False)
parser.add_argument('-L', '--logstyle', dest='logstyle', action='store_true', default=False)
parser.add_argument('-Y', '--year', dest='year', type=str, default="")
args = parser.parse_args()
logstyle = args.logstyle
ratio = args.ratio
significance = args.significance
year = args.year
sys = "_norm"
# Lumi ratio dictionary for integrated Run2
rlumi={"16pre":1.,"16post":1.,"17":1.,"18":1.}
runs=[]
if year=="16pre":
	CMS_lumi.lumi_13TeV = "19.5 fb^{-1}"
	s = stackhists.Stackhists(19.5)
	runs=["16pre"]
if year=="16post":
	CMS_lumi.lumi_13TeV = "16.8 fb^{-1}"
	s = stackhists.Stackhists(16.8)
	runs=["16post"]
elif year=="17":
	CMS_lumi.lumi_13TeV = "41.5 fb^{-1}"
	s = stackhists.Stackhists(41.48)
	runs=["17"]
elif year=="18":
	CMS_lumi.lumi_13TeV = "59.8 fb^{-1}"
	s = stackhists.Stackhists(59.83)
	runs=["18"]
elif year=="run2":
	CMS_lumi.lumi_13TeV = "138 fb^{-1}"
	s = stackhists.Stackhists(137.65)
	rlumi["16pre"] = 19.5/137.65
	rlumi["16post"] = 16.8/137.65
	rlumi["17"] = 41.48/137.65
	rlumi["18"] = 59.83/137.65
	runs=["16pre","16post","17","18"]

year=str(year)
s.setupStyle(alpha=1)
CMS_lumi.extraText = ""
#CMS_lumi.extraText = "Simulation"

# DATA
if year=="run2":
	s.addChannel("Run2"+sys+".root", "data", 999, isMC=False)
else:
	s.addChannel(year+"/Run"+year+sys+".root", "data", 999, isMC=False)
for run in runs:
	# LFV ( must be added before other MC )
	s.addChannel(run+"/ST_LFV_TCMuTau_Vector_"+run+sys+".root", "LFV STc", 10, isMC=True, xsec=36.8, counterhistogramroot=run+"/ST_LFV_TCMuTau_Vector_"+run+sys+".root")
	s.addChannel(run+"/ST_LFV_TUMuTau_Vector_"+run+sys+".root", "LFV STu", 11, isMC=True, xsec=393, counterhistogramroot=run+"/ST_LFV_TUMuTau_Vector_"+run+sys+".root")
	s.addChannel(run+"/TT_LFV_TToCMuTau_Vector_"+run+sys+".root", "LFV TTc", 12, isMC=True, xsec=21.5, counterhistogramroot=run+"/TT_LFV_TToCMuTau_Vector_"+run+sys+".root")
	s.addChannel(run+"/TT_LFV_TToUMuTau_Vector_"+run+sys+".root", "LFV TTu", 13, isMC=True, xsec=21.5, counterhistogramroot=run+"/TT_LFV_TToUMuTau_Vector_"+run+sys+".root")
	# MC ( reference twiki : https://twiki.cern.ch/twiki/bin/viewauth/CMS/SummaryTable1G25ns )
	# TTbar
	s.addChannel(run+"/TTTo2L2Nu_"+run+sys+".root", "TT-di", 0, isMC=True, xsec=rlumi[run]*88290.0, counterhistogramroot=run+"/TTTo2L2Nu_"+run+sys+".root")
	s.addChannel(run+"/TTToSemiLeptonic_"+run+sys+".root", "TT-semi", 1, isMC=True, xsec=rlumi[run]*365340.0, counterhistogramroot=run+"/TTToSemiLeptonic_"+run+sys+".root")
	s.addChannel(run+"/TTToHadronic_"+run+sys+".root", "TT-had", 2, isMC=True, xsec=rlumi[run]*377960.0, counterhistogramroot=run+"/TTToHadronic_"+run+sys+".root")

	#s.addChannel(run+"/WJetsToLNu_"+run+sys+".root", "W+jets", 3, isMC=True, xsec=rlumi[run]*61526700, counterhistogramroot=run+"/WJetsToLNu_"+run+sys+".root")

	# W+Jets
	if run=="16pre":
		s.addChannel(run+"/WJetsToLNu_inclHT100_"+run+sys+".root", "W+jets", 3, isMC=True, xsec=rlumi[run]*61526700*0.9647, counterhistogramroot=run+"/WJetsToLNu_inclHT100_"+run+sys+".root")
	elif run=="16post":
		s.addChannel(run+"/WJetsToLNu_inclHT100_"+run+sys+".root", "W+jets", 3, isMC=True, xsec=rlumi[run]*61526700*0.9647, counterhistogramroot=run+"/WJetsToLNu_inclHT100_"+run+sys+".root")
	elif run=="17":
		s.addChannel(run+"/WJetsToLNu_inclHT100_"+run+sys+".root", "W+jets", 3, isMC=True, xsec=rlumi[run]*61526700*0.9645, counterhistogramroot=run+"/WJetsToLNu_inclHT100_"+run+sys+".root")
	elif run=="18":
		s.addChannel(run+"/WJetsToLNu_inclHT100_"+run+sys+".root", "W+jets", 3, isMC=True, xsec=rlumi[run]*61526700*0.9645, counterhistogramroot=run+"/WJetsToLNu_inclHT100_"+run+sys+".root")
	s.addChannel(run+"/WJetsToLNu_HT-100To200_"+run+sys+".root", "W+jets", 3, isMC=True, xsec=rlumi[run]*1345000.0*0.993, counterhistogramroot=run+"/WJetsToLNu_HT-100To200_"+run+sys+".root")
	s.addChannel(run+"/WJetsToLNu_HT-200To400_"+run+sys+".root", "W+jets", 3, isMC=True, xsec=rlumi[run]*359700.0*1.002, counterhistogramroot=run+"/WJetsToLNu_HT-200To400_"+run+sys+".root")
	s.addChannel(run+"/WJetsToLNu_HT-400To600_"+run+sys+".root", "W+jets", 3, isMC=True, xsec=rlumi[run]*48910.0*1.009, counterhistogramroot=run+"/WJetsToLNu_HT-400To600_"+run+sys+".root")
	s.addChannel(run+"/WJetsToLNu_HT-600To800_"+run+sys+".root", "W+jets", 3, isMC=True, xsec=rlumi[run]*12050.0*1.120, counterhistogramroot=run+"/WJetsToLNu_HT-600To800_"+run+sys+".root")
	s.addChannel(run+"/WJetsToLNu_HT-800To1200_"+run+sys+".root", "W+jets", 3, isMC=True, xsec=rlumi[run]*5501.0*1.202, counterhistogramroot=run+"/WJetsToLNu_HT-800To1200_"+run+sys+".root")
	s.addChannel(run+"/WJetsToLNu_HT-1200To2500_"+run+sys+".root", "W+jets", 3, isMC=True, xsec=rlumi[run]*1329.0*1.332, counterhistogramroot=run+"/WJetsToLNu_HT-1200To2500_"+run+sys+".root")
	s.addChannel(run+"/WJetsToLNu_HT-2500ToInf_"+run+sys+".root", "W+jets", 3, isMC=True, xsec=rlumi[run]*32.16*4.200, counterhistogramroot=run+"/WJetsToLNu_HT-2500ToInf_"+run+sys+".root")

	# DY
	s.addChannel(run+"/DYJetsToLL_M-10to50_"+run+sys+".root", "Z+jets", 4, isMC=True, xsec=rlumi[run]*18610000.0, counterhistogramroot=run+"/DYJetsToLL_M-10to50_"+run+sys+".root")
	#s.addChannel(run+"/DYJetsToLL_M-50_"+run+sys+".root", "Z+jets", 4, isMC=True, xsec=rlumi[run]*6077220.0, counterhistogramroot=run+"/DYJetsToLL_M-50-amcatnloFXFX_"+run+sys+".root")
	s.addChannel(run+"/DYJetsToLL_M-50_madgraph_"+run+sys+".root", "Z+jets", 4, isMC=True, xsec=rlumi[run]*6077220.0, counterhistogramroot=run+"/DYJetsToLL_M-50_madgraph_"+run+sys+".root")

	# ST
	s.addChannel(run+"/ST_t-channel_top_"+run+sys+".root", "ST", 5, isMC=True, xsec=rlumi[run]*136020.0, counterhistogramroot=run+"/ST_t-channel_top_"+run+sys+".root")
	s.addChannel(run+"/ST_t-channel_antitop_"+run+sys+".root", "ST", 5, isMC=True, xsec=rlumi[run]*80950.0, counterhistogramroot=run+"/ST_t-channel_antitop_"+run+sys+".root")
	s.addChannel(run+"/ST_tW_top_"+run+sys+".root", "ST", 5, isMC=True, xsec=rlumi[run]*35850.0, counterhistogramroot=run+"/ST_tW_top_"+run+sys+".root")
	s.addChannel(run+"/ST_tW_antitop_"+run+sys+".root", "ST", 5, isMC=True, xsec=rlumi[run]*35850.0, counterhistogramroot=run+"/ST_tW_antitop_"+run+sys+".root")

	# VV
	s.addChannel(run+"/WW_"+run+sys+".root", "VV", 6, isMC=True, xsec=rlumi[run]*118700.0, counterhistogramroot=run+"/WW_"+run+sys+".root")
	s.addChannel(run+"/WZ_"+run+sys+".root", "VV", 6, isMC=True, xsec=rlumi[run]*47130.0, counterhistogramroot=run+"/WZ_"+run+sys+".root")
	s.addChannel(run+"/ZZ_"+run+sys+".root", "VV", 6, isMC=True, xsec=rlumi[run]*16523.0, counterhistogramroot=run+"/ZZ_"+run+sys+".root")

	# TTX
	s.addChannel(run+"/TTWJetsToLNu_"+run+sys+".root", "TTX", 7, isMC=True, xsec=rlumi[run]*204.3, counterhistogramroot=run+"/TTWJetsToLNu_"+run+sys+".root")
	s.addChannel(run+"/TTWJetsToQQ_"+run+sys+".root", "TTX", 7, isMC=True, xsec=rlumi[run]*406.2, counterhistogramroot=run+"/TTWJetsToQQ_"+run+sys+".root")
	s.addChannel(run+"/TTZToLLNuNu_"+run+sys+".root", "TTX", 7, isMC=True, xsec=rlumi[run]*252.9, counterhistogramroot=run+"/TTZToLLNuNu_"+run+sys+".root")
	s.addChannel(run+"/TTZToQQ_"+run+sys+".root", "TTX", 7, isMC=True, xsec=rlumi[run]*529.7, counterhistogramroot=run+"/TTZToQQ_"+run+sys+".root")

	# QCD
#    s.addChannel(run+"/QCD_Pt-15to20_MuEnrichedPt5_"+run+sys+".root", "QCD", 8, isMC=True, xsec=rlumi[run]*1273190000000*0.003, counterhistogramroot=run+"/QCD_Pt-15to20_MuEnrichedPt5_"+run+sys+".root")
#    s.addChannel(run+"/QCD_Pt-20to30_MuEnrichedPt5_"+run+sys+".root", "QCD", 8, isMC=True, xsec=rlumi[run]*558528000000*0.0053, counterhistogramroot=run+"/QCD_Pt-20to30_MuEnrichedPt5_"+run+sys+".root")
#    s.addChannel(run+"/QCD_Pt-30to50_MuEnrichedPt5_"+run+sys+".root", "QCD", 8, isMC=True, xsec=rlumi[run]*139803000000*0.01182, counterhistogramroot=run+"/QCD_Pt-30to50_MuEnrichedPt5_"+run+sys+".root")
#    s.addChannel(run+"/QCD_Pt-50to80_MuEnrichedPt5_"+run+sys+".root", "QCD", 8, isMC=True, xsec=rlumi[run]*19222500000*0.02276, counterhistogramroot=run+"/QCD_Pt-50to80_MuEnrichedPt5_"+run+sys+".root")
#    s.addChannel(run+"/QCD_Pt-80to120_MuEnrichedPt5_"+run+sys+".root", "QCD", 8, isMC=True, xsec=rlumi[run]*2758420000*0.03844, counterhistogramroot=run+"/QCD_Pt-80to120_MuEnrichedPt5_"+run+sys+".root")
#    s.addChannel(run+"/QCD_Pt-120to170_MuEnrichedPt5_"+run+sys+".root", "QCD", 8, isMC=True, xsec=rlumi[run]*469797000*0.05362, counterhistogramroot=run+"/QCD_Pt-120to170_MuEnrichedPt5_"+run+sys+".root")
#    s.addChannel(run+"/QCD_Pt-170to300_MuEnrichedPt5_"+run+sys+".root", "QCD", 8, isMC=True, xsec=rlumi[run]*117989000*0.07335, counterhistogramroot=run+"/QCD_Pt-170to300_MuEnrichedPt5_"+run+sys+".root")
#    s.addChannel(run+"/QCD_Pt-300to470_MuEnrichedPt5_"+run+sys+".root", "QCD", 8, isMC=True, xsec=rlumi[run]*7820250*0.10196, counterhistogramroot=run+"/QCD_Pt-300to470_MuEnrichedPt5_"+run+sys+".root")
#    s.addChannel(run+"/QCD_Pt-470to600_MuEnrichedPt5_"+run+sys+".root", "QCD", 8, isMC=True, xsec=rlumi[run]*645528*0.12242, counterhistogramroot=run+"/QCD_Pt-470to600_MuEnrichedPt5_"+run+sys+".root")
#    s.addChannel(run+"/QCD_Pt-600to800_MuEnrichedPt5_"+run+sys+".root", "QCD", 8, isMC=True, xsec=rlumi[run]*187109*0.13412, counterhistogramroot=run+"/QCD_Pt-600to800_MuEnrichedPt5_"+run+sys+".root")
#    s.addChannel(run+"/QCD_Pt-800to1000_MuEnrichedPt5_"+run+sys+".root", "QCD", 8, isMC=True, xsec=rlumi[run]*32348.6*0.14552, counterhistogramroot=run+"/QCD_Pt-800to1000_MuEnrichedPt5_"+run+sys+".root")
#    s.addChannel(run+"/QCD_Pt-1000toInf_MuEnrichedPt5_"+run+sys+".root", "QCD", 8, isMC=True, xsec=rlumi[run]*10430.5*0.15544, counterhistogramroot=run+"/QCD_Pt-1000toInf_MuEnrichedPt5_"+run+sys+".root")

# Bin lists for rebin
metbins=[0,20,40,60,80,100,120,140,160,180,200,240,300,400]
mutauptbins=[0,40,80,120,160,200,240,280,320,400,800]
mutaumassbins=[0,40,80,120,160,200,240,280,320,400,480,600,800,1000]
mutaudrbins=[0,0.4,0.8,1.2,1.6,1.8,2.0,2.2,2.4,2.6,2.8,3.0,3.2,3.4,3.6,3.8,4.0]
etabins=[-2.7,-2.4,-2.1,-1.8,-1.5,-1.2,-0.9,-0.6,-0.3,0,0.3,0.6,0.9,1.2,1.5,1.8,2.1,2.4,2.7]
drbins=[0,0.4,0.8,1.2,1.6,2.0,2.2,2.4,2.6,2.8,3.0,3.2,3.4,3.6,4.0]
mutaumassbins=[0,120,160,200,240,280,320,360,400,440,480,520,600,800,1000]
jet1ptbins=[0,20,40,60,80,100,120,140,160,180,200,220,240,280,320,400,600]
jetptbins=[0,20,40,60,80,100,120,140,160,180,200,220,260,300,600]


# Histograms
s.addHistogram("hnevents_pglep_nocut", "Nocut counter", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hnevents_pglep_cut0", "Nocut counter", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hnevents_pglep_cut00", "Nocut counter", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hnevents_pglep_cut000", "Nocut counter", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hnevents_cut0000", "Nocut counter", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hnevents_cut00000", "Nocut counter", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

# Muon multiplicity
#s.addHistogram("h1nmuonpass_cut0", "Number of Muons", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("h1nmuonpass_cut00", "Number of Muons", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("h1nmuonpass_cut000", "Number of Muons", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("hnmuonpass_cut0000", "Number of Muons", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

# Tau multiplicity
s.addHistogram("h1ncleantaupass_cut0", "Number of Hadronic Tau", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("h1ncleantaupass_cut00", "Number of Hadronic Tau", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("h1ncleantaupass_cut000", "Number of Hadronic Tau", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("hncleantaupass_cut0000", "Number of Hadronic Tau", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

# Jet multiplicity
s.addHistogram("h1ncleanjetspass_cut0", "Number of Jets", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("h1ncleanjetspass_cut00", "Number of Jets", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("h1ncleanjetspass_cut000", "Number of Jets", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("hncleanjetspass_cut0000", "Number of Jets", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

# bjet multiplicity
s.addHistogram("h1ncleanbjetspass_cut000", "Number of b-tagged jets", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("hncleanbjetspass_cut0000", "Number of b-tagged jets", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

# Jet Histograms
# Leading Jet
s.addHistogram("h1jet1pt_cut000", "p_{T} of Leading Jet (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("h1jet1eta_cut000", "#eta of Leading Jet", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("h1jet1btag_cut000", "b-tag discr. of Leading Jet", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

s.addHistogram("hjet1pt_cut00000", "p_{T} of Leading Jet (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1, binlist=jet1ptbins)
s.addHistogram("hjet1eta_cut00000", "#eta of Leading Jet", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hjet1btag_cut00000", "b-tag discr. of Leading Jet", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

# Sub-leading Jet
s.addHistogram("h1jet2pt_cut000", "p_{T} of Sub-leading Jet (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("h1jet2eta_cut000", "#eta of Sub-leading Jet", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("h1jet2btag_cut000", "b-tag discr. of Sub-leading Jet", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

s.addHistogram("hjet2pt_cut00000", "p_{T} of Sub-leading Jet (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1, binlist=jetptbins)
s.addHistogram("hjet2eta_cut00000", "#eta of Sub-leading Jet", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hjet2btag_cut00000", "b-tag discr. of Sub-leading Jet", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

# Third Jet
s.addHistogram("h1jet3pt_cut000", "p_{T} of Third Jet (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("h1jet3eta_cut000", "#eta of Third Jet", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("h1jet3btag_cut000", "b-tag discr. of Third Jet", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

s.addHistogram("hjet3pt_cut00000", "p_{T} of Third Jet (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1, binlist=jetptbins)
s.addHistogram("hjet3eta_cut00000", "#eta of Third Jet", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hjet3btag_cut00000", "b-tag discr. of Third Jet", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

# bJet Histograms
s.addHistogram("hbjet1pt_cut00000", "p_{T} of b-tagged Jet (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1, binlist=jetptbins)
s.addHistogram("hbjet1eta_cut00000", "#eta of b-tagged Jet", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

# Muon Histograms
#s.addHistogram("h1muon1pt_cut0", "p_{T} of Muon (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1, binlist=mutauptbins)
#s.addHistogram("h1muon1pt_cut00", "p_{T} of Muon (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1, binlist=mutauptbins)
#s.addHistogram("h1muon1pt_cut000", "p_{T} of Muon (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1, binlist=mutauptbins)
#s.addHistogram("h1muon1eta_cut0", "#eta of Muon", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("h1muon1eta_cut00", "#eta of Muon", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("h1muon1eta_cut000", "#eta of Muon", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

s.addHistogram("hmuon1pt_cut00000", "p_{T} of Muon (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1, binlist=mutauptbins)
s.addHistogram("hmuon1eta_cut00000", "#eta of Muon", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hmuon1mass_cut00000", "Mass of Muon (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

# Tau Histograms
#s.addHistogram("h1tau1pt_cut00", "p_{T} of Hadronic Tau (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1, binlist=mutauptbins)
#s.addHistogram("h1tau1pt_cut000", "p_{T} of Hadronic Tau (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1, binlist=mutauptbins)

#s.addHistogram("h1tau1eta_cut00", "#eta of Hadronic Tau", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("h1tau1eta_cut000", "#eta of Hadronic Tau", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

s.addHistogram("htau1pt_cut00000", "p_{T} of Hadronic Tau (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1, binlist=mutauptbins)
s.addHistogram("htau1eta_cut00000", "#eta of Hadronic Tau", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("htau1mass_cut00000", "Mass of Hadronic Tau (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

s.addHistogram("hmutau_dEta_cut00000", "#it{#Delta#eta}_{#mu,#tau}", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hmutau_dPhi_cut00000", "#it{#Delta#phi}_{#mu,#tau}", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hmutau_dR_cut00000", "#DeltaR_{#mu,#tau}", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hmutau_mass_cut00000", "M_{#mu,#tau} (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1, binlist=mutaumassbins)

# MET
#s.addHistogram("h1metpt_cut0", "MET (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1) 
#s.addHistogram("h1metpt_cut00", "MET (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("h1metpt_cut000", "MET (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

#s.addHistogram("h1metphi_cut0", "MET #phi", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("h1metphi_cut00", "MET #phi", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("h1metphi_cut000", "MET #phi", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

s.addHistogram("hmetpt_cut00000", "MET (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1, binlist=metbins)
s.addHistogram("hmetphi_cut00000", "MET #phi", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

# SumET
#s.addHistogram("hsumet_cut0", "Sum E_{T} (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("hsumet_cut00", "Sum E_{T} (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("hsumet_cut000", "Sum E_{T} (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
#s.addHistogram("hsumet_cut0000", "Sum E_{T} (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

# Top mass reconstruction ( W hadronic )
s.addHistogram("hchi2_cut00000", "#chi^{2} score", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hchi2_SMTop_mass_cut00000", "SM Top mass from #chi^{2} (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hchi2_SMW_mass_cut00000", "SM W mass from #chi^{2} (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

s.addHistogram("hchi2_wqq_absdEta_cut00000", "#it{#left|#Delta#eta#right|}_{wqq} from #chi^{2}", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hchi2_wqq_absdPhi_cut00000", "#it{#left|#Delta#phi#right|}_{wqq} from #chi^{2}", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hchi2_wqq_dEta_cut00000", "#it{#Delta#eta}_{wqq} from #chi^{2}", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hchi2_wqq_dPhi_cut00000", "#it{#Delta#phi}_{wqq} from #chi^{2}", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hchi2_wqq_dR_cut00000", "#DeltaR_{wqq}", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hmutau_absdEta_cut00000", "#it{#left|#Delta#eta#right|}_{#mu,#tau} from #chi^{2}", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("hmutau_absdPhi_cut00000", "#it{#left|#Delta#phi#right|}_{#mu,#tau} from #chi^{2}", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("h1mutau_dEta_cut00", "#it{#Delta#eta}_{#mu,#tau}", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("h1mutau_dPhi_cut00", "#it{#Delta#phi}_{#mu,#tau}", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("h1mutau_dR_cut00", "#DeltaR_{#mu,#tau}", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)
s.addHistogram("h1mutau_mass_cut00", "M_{#mu,#tau} (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1, binlist=mutaumassbins)

subplot="R"
if ratio : subplot="R"
elif significance : subplot="S"

s.draw(subplot)
 
