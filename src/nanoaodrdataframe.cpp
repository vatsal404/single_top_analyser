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
//	c1.Add("root://cmsxrootd.fnal.gov///store/mc/Run3Summer22NanoAODv12/WtoLNu-2Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8/NANOAODSIM/130X_mcRun3_2022_realistic_v5-v2/30000/269ca882-375b-49dc-80ff-01dc2c227522.root"); //data
//	c1.Add("root://xrootd-cms.infn.it//store/mc/RunIISummer20UL17NanoAODv9/WJetsToLNu_0J_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mc2017_realistic_v9-v2/2810000/8EDF6CCF-B922-2B40-8889-B0CD442F88CC.root")
//
c1.Add("root://xrootd-cms.infn.it//store/mc/Run3Summer22NanoAODv12/TTtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2022_realistic_v5-v2/30000/670acfa8-2b1a-4ec5-932b-0512e54fd5f8.root");
	//c1.Add("root://cmsxrootd.fnal.gov//store/data/Run2022D/Muon/NANOAOD/16Dec2023-v1/50000/fa77d341-cad2-4902-a837-308655dbca47.root"); // MC
        //c1.Add("root://xrootd-cms.infn.it//store/data/Run2022D/Muon/NANOAOD/16Dec2023-v1/50000/a921644d-6ceb-4bc3-81c9-fddd5c7b1edb.root"); // tw

	BaseAnalyser nanoaodrdf(&c1, "test.root",2,2);
	nanoaodrdf.setParams(2022, "UL", -1);
	nanoaodrdf.setHLT();

	string goodjsonfname = "data/Cert_Collisions2022_355100_362760_Golden.json";
	string pileupfname = "data/LUM/2022_preEE/puWeights.json";
	string pileuptag = "Collisions2022_355100_357900_eraBCD_GoldenJson";
	string btvfname = "data/BTV/2022_preEE/btagging.json";
	string btvtype = "deepJet_shape";
//	string fname_btagEff = "data/BTV/2017_UL/BtaggingEfficiency.root";Summer22_22Sep2023_RunCD_V2_DATA_L1L2L3Res_AK4PFPuppi
//	string hname_btagEff_bcflav = "h_btagEff_bcflav";
//	string hname_btagEff_lflav = "h_btagEff_lflav";
	string jercfname = "data/JERC/2022_preEE/jet_jerc.json";
	string jerctag = "Summer22_22Sep2023_RunCD_V2_DATA_L1L2L3Res_AK4PFPuppi";
    string jettagMC = "Summer22_22Sep2023_V2_MC_L1L2L3Res_AK4PFPuppi";
	string jercunctag = "Summer22_22Sep2023_V2_MC_Total_AK4PFPuppi";
	string muon_roch_fname = "data/MUO/2017_UL/RoccoR2017UL.txt";
	string muon_fname = "data/MUON/2022_preEE/muon_Z.json.gz";
	string muonHLTtype = "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight";
//	string muonRECOtype = "NUM_TrackerMuons_DEN_genTracks";
	string muonIDtype = "NUM_TightID_DEN_TrackerMuons";
	string muonISOtype = "NUM_TightPFIso_DEN_TightID";
	string electron_fname = "data/EGM/2022_preEE/electron.json.gz";
	string electronHlt_fname = "data/EGM/2022_preEE/electronHlt.json.gz";
    string electronHlt_type="HLT_SF_Ele30_TightID";
    string electron_reco_type1 = "Reco20to75";
	string electron_reco_type2 =  "RecoAbove75";
	string electron_id_type = "wp90iso";
        string jet_veto_f_name="scalefactors/jetvetomaps.json";
        string jet_veto_tag = "Summer22_23Sep2023_RunCD_V1";
        string electron_SSF = "data/EGM/2022_preEE/electronSS.json";
	nanoaodrdf.setupCorrections(goodjsonfname, pileupfname, pileuptag, btvfname, btvtype,/* muon_roch_fname,*/ muon_fname, muonHLTtype, /*muonRECOtype*/ muonIDtype, muonISOtype, electron_fname,electronHlt_fname,electronHlt_type, electron_reco_type1,electron_reco_type2, electron_id_type, jercfname, jerctag,jettagMC, jercunctag,jet_veto_f_name,jet_veto_tag,electron_SSF);

	nanoaodrdf.setupObjects();
	nanoaodrdf.setupAnalysis();
	nanoaodrdf.run(false, "outputTree");

	return EXIT_SUCCESS;
}
