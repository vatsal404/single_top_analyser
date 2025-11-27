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
//	c1.Add("root://cmsxrootd.fnal.gov///store/mc/Run3Summer23NanoAODv12/TTtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2023_realistic_v14-v2/2520000/04fb6a6e-b9c5-4df1-99fa-c804ef776cba.root"); //ttbar
    c1.Add("root://cmsxrootd.fnal.gov///store/mc/Run3Summer23NanoAODv12/TbarWplusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2023_realistic_v15-v4/50000/063916a6-99cf-4832-a945-c2ddfaee53dd.root");//tw
//c1.Add("root://xrootd-cms.infn.it//store/mc/Run3Summer23NanoAODv12/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2023_realistic_v14-v3/2810000/0019dbdc-5ef5-4cb5-85e5-7655c7002d47.root");//DY
        //c1.Add("root://cmsxrootd.fnal.gov///store/data/Run2023C/EGamma0/NANOAOD/24Jan2024_v4-v1/2820000/023f229d-676e-4ac2-a022-da17fd2dbb0e.root"); // data

	BaseAnalyser nanoaodrdf(&c1, "test_tw.root",2,2);
	nanoaodrdf.setParams(2022, "UL", -1);
	nanoaodrdf.setHLT();

	string goodjsonfname = "data/golden_json_2023.json";
	string pileupfname = "data/LUM/2023/puWeights.json";
	string pileuptag = "Collisions2023_366403_369802_eraBC_GoldenJson";
	string btvfname = "data/BTV/2023/btagging.json";
	string btvtype = "deepJet_shape";
//	string fname_btagEff = "data/BTV/2017_UL/BtaggingEfficiency.root";Summer22_22Sep2023_RunCD_V2_DATA_L1L2L3Res_AK4PFPuppi
//	string hname_btagEff_bcflav = "h_btagEff_bcflav";
//	string hname_btagEff_lflav = "h_btagEff_lflav";
	string jercfname = "data/JERC/2023/jet_jerc.json";
	string jerctag = "Summer23Prompt23_V2_DATA_L1L2L3Res_AK4PFPuppi";
    string jettagMC = "Summer23Prompt23_V2_MC_L1L2L3Res_AK4PFPuppi";
	string jercunctag = "Summer23Prompt23_V2_MC_Total_AK4PFPuppi";
	string muon_roch_fname = "data/MUON/2023/muon_scalesmearing.json";
	string muon_fname = "data/MUON/2023/muon_Z.json.gz";
	string muonHLTtype = "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight";
//	string muonRECOtype = "NUM_TrackerMuons_DEN_genTracks";
	string muonIDtype = "NUM_TightID_DEN_TrackerMuons";
	string muonISOtype = "NUM_TightPFIso_DEN_TightID";
	string electron_fname = "data/EGM/2023/electron.json";
	string electronHlt_fname = "data/EGM/2023/electronHlt.json";
    string electronHlt_type="HLT_SF_Ele30_TightID";
    string electron_reco_type1 = "Reco20to75";
	string electron_reco_type2 =  "RecoAbove75";
	string electron_id_type = "Tight";
        string jet_veto_f_name="data/JERC/2023/jetvetomaps.json";
        string jet_veto_tag = "Summer23Prompt23_RunC_V1";
        string electron_SSF = "data/EGM/2023/electronSS_EtDependent.json";
        string metpt_fname = "data/JERC/2023/met_xyCorrections_2023_2023.json";
	nanoaodrdf.setupCorrections(goodjsonfname, pileupfname, pileuptag, btvfname, btvtype, muon_roch_fname, muon_fname, muonHLTtype, /*muonRECOtype*/ muonIDtype, muonISOtype, electron_fname,electronHlt_fname,electronHlt_type, electron_reco_type1,electron_reco_type2, electron_id_type, jercfname, jerctag,jettagMC, jercunctag,jet_veto_f_name,jet_veto_tag,electron_SSF,metpt_fname );

	nanoaodrdf.setupObjects();
	nanoaodrdf.setupAnalysis();
	nanoaodrdf.run(false, "outputTree");

	return EXIT_SUCCESS;
}
