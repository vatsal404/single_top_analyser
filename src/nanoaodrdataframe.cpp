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
//	c1.Add("root://xrootd-cms.infn.it//store/mc/RunIISummer20UL17NanoAODv9/WJetsToLNu_2J_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mc2017_realistic_v9-v2/2810000/42C8CAA1-8910-1749-8F1D-2DF492ED1A89.root"); //data
//	c1.Add("root://xrootd-cms.infn.it//store/mc/RunIISummer20UL17NanoAODv9/WJetsToLNu_0J_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mc2017_realistic_v9-v2/2810000/8EDF6CCF-B922-2B40-8889-B0CD442F88CC.root")c1.Add("root://xrootd-cms.infn.it//store/mc/RunIISummer20UL17NanoAODv9/TT_TuneCH3_13TeV-powheg-herwig7/NANOAODSIM/106X_mc2017_realistic_v9-v1/2520000/02DB06FB-1711-CB4D-A03E-1877AEEC3095.root"); // MC
//	c1.Add("root://xrootd-cms.infn.it//store/mc/Run3Summer22NanoAODv12/TbarWplusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2022_realistic_v5-v2/2530000/10821e1d-9f4a-49a9-96dd-6fdce1f1bbbb.root"); // MC
        c1.Add("root://xrootd-cms.infn.it//store/data/Run2022E/Muon/NANOAOD/16Dec2023-v1/2550000/18440a48-3d57-404c-a665-7db5f633927d.root"); // tw

	BaseAnalyser nanoaodrdf(&c1, "test.root",2);
	nanoaodrdf.setParams(2022, "UL", -1);
	// nanoaodrdf.setHLT();

	string goodjsonfname = "data/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt";
	string pileupfname = "data/LUM/2017_UL/puWeights.json";
	string pileuptag = "Collisions17_UltraLegacy_goldenJSON";
	string btvfname = "data/BTV/2017_UL/btagging.json";
	string btvtype = "deepJet_shape";
//	string fname_btagEff = "data/BTV/2017_UL/BtaggingEfficiency.root";
//	string hname_btagEff_bcflav = "h_btagEff_bcflav";
//	string hname_btagEff_lflav = "h_btagEff_lflav";
	//string electron_fname = "data/ELECTRON/2018_UL/electron_Z.json";
	//string electrontype = "UL-Electron-ID-SF";
	string jercfname = "data/JERC/UL17_jerc.json";
	string jerctag = "Summer19UL17_V5_MC_L1L2L3Res_AK4PFchs";
	string jercunctag = "Summer19UL17_V5_MC_Total_AK4PFchs";
	string muon_roch_fname = "data/MUO/2017_UL/RoccoR2017UL.txt";
	string muon_fname = "data/MUO/2017_UL/muon_Z.json.gz";
	string muonHLTtype = "NUM_IsoMu27_DEN_CutBasedIdTight_and_PFIsoTight";
	string muonRECOtype = "NUM_TrackerMuons_DEN_genTracks";
	string muonIDtype = "NUM_MediumID_DEN_TrackerMuons";
	string muonISOtype = "NUM_TightRelIso_DEN_MediumID";
	string electron_fname = "data/EGM/2017_UL/electron.json.gz";
	string electron_reco_type = "RecoAbove20";
	string electron_id_type = "Tight";



	nanoaodrdf.setupCorrections(goodjsonfname, pileupfname, pileuptag, btvfname, btvtype, muon_roch_fname, muon_fname, muonHLTtype, muonRECOtype, muonIDtype, muonISOtype, electron_fname, electron_reco_type, electron_id_type, jercfname, jerctag, jercunctag);

	nanoaodrdf.setupObjects();
	nanoaodrdf.setupAnalysis();
	nanoaodrdf.run(false, "outputTree");

	return EXIT_SUCCESS;
}
