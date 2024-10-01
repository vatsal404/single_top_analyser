//============================================================================
// Name        : nanoaodrdataframe.cpp
// Author      : Suyong Choi
// Version     :
// Copyright   : suyong@korea.ac.kr, Korea University, Department of Physics
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "NanoAODAnalyzerrdframe.h"
#include "BaseAnalyser.h"
//#include "SkimEvents.h"
#include "TChain.h"

using namespace std;
using namespace ROOT;

int main(void) {

	TChain c1("Events");
	//c1.Add("testinputdata/data/JetHT_2017C_DA05CA1A-3265-EE47-84F9-10CB09D22BDA.root"); //data
	//c1.Add("testinputdata/MC/2017/ttJets-2017MC-A258F579-5EC0-D840-95D7-4327595FC3DE.root"); // MC
	c1.Add("/eos/uscms/store/user/vsinha/375FDAF8-92F6-8046-9159-5040055F7708.root"); // MC
	BaseAnalyser nanoaodrdf(&c1, "testout.root");

	string goodjsonfname = "data/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt";
	string pileupfname = "data/LUM/2018_UL/puWeights.json";
	string pileuptag = "Collisions18_UltraLegacy_goldenJSON";
	string btvfname = "data/BTV/2018_UL/btagging.json";
	string btvtype = "deepJet_shape";
	string fname_btagEff = "data/BTV/2017_UL/BtaggingEfficiency.root";
	string hname_btagEff_bcflav = "h_btagEff_bcflav";
	string hname_btagEff_lflav = "h_btagEff_lflav";
	//string electron_fname = "data/ELECTRON/2018_UL/electron_Z.json";
	//string electrontype = "UL-Electron-ID-SF";
	string jercfname = "data/JERC/UL18_jerc.json";
	string jerctag = "Summer19UL18_V5_MC_L1L2L3Res_AK4PFchs";
	string jercunctag = "Summer19UL18_V5_MC_Total_AK4PFchs";
	string muon_roch_fname = "data/MUO/2017_UL/RoccoR2017UL.txt";
	string muon_fname = "data/MUO/2017_UL/muon_Z.json.gz";
	string muonHLTtype = "NUM_IsoMu27_DEN_CutBasedIdTight_and_PFIsoTight";
	string muonRECOtype = "NUM_TrackerMuons_DEN_genTracks";
	string muonIDtype = "NUM_MediumID_DEN_TrackerMuons";
	string muonISOtype = "NUM_TightRelIso_DEN_MediumID";
	string electron_fname = "data/EGM/2017_UL/electron.json.gz";
	string electron_reco_type = "RecoAbove20";
	string electron_id_type = "Tight";



	nanoaodrdf.setupCorrections(goodjsonfname, pileupfname, pileuptag, btvfname, btvtype, fname_btagEff, hname_btagEff_bcflav, hname_btagEff_lflav, muon_roch_fname, muon_fname, muonHLTtype, muonRECOtype, muonIDtype, muonISOtype, electron_fname, electron_reco_type, electron_id_type, jercfname, jerctag, jercunctag);

	nanoaodrdf.setupObjects();
	nanoaodrdf.setupAnalysis();
	nanoaodrdf.run(false, "outputTree");

	return EXIT_SUCCESS;
}
