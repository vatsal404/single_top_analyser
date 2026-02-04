// src/nanoaodrdataframe.cpp (modified)
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include "NanoAODAnalyzerrdframe.h"
#include "BaseAnalyser.h"
#include "TChain.h"

using namespace std;
using namespace ROOT;


int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string year = "2023BPix"; // default
    if (argc > 1) {
        year = argv[1];  // Directly assign string
        if (year != "2022" && year != "2023" && year != "2022EE" && year != "2023BPix") {
            cerr << "Error: Year must be 2022, 2022EE, 2023, or 2023BPix" << endl;
            cerr << "Usage: " << argv[0] << " [year]" << endl;
            return EXIT_FAILURE;
        }
    }

    cout << "Running analysis for year: " << year << endl;

    TChain c1("Events");

    string outputFile;
    string era;
    
    if (year == "2022") {
        // 2022 configuration
        //c1.Add("root://cmsxrootd.fnal.gov//store/mc/Run3Summer22NanoAODv12/TTtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2022_realistic_v5-v2/30000/670acfa8-2b1a-4ec5-932b-0512e54fd5f8.root");
       c1.Add("root://cmsxrootd.fnal.gov//store/data/Run2022D/Muon/NANOAOD/16Dec2023-v1/50000/fa77d341-cad2-4902-a837-308655dbca47.root");
        outputFile = "test_2022.root";
        era = "PreEE";
    } else if (year == "2022EE") {
        // 2022 PostEE configuration
        c1.Add("root://cmsxrootd.fnal.gov//store/mc/Run3Summer22EENanoAODv12/TTLL_MLL-4to50_TuneCP5_13p6TeV_amcatnlo-pythia8/NANOAODSIM/130X_mcRun3_2022_realistic_postEE_v6-v2/2520000/716d2d2f-6ac3-4ac2-aa93-ad9034e8a9fd.root");
       // c1.Add("root://cmsxrootd.fnal.gov//store/data/Run2022F/MuonEG/NANOAOD/22Sep2023-v1/2520000/11f0ddf5-660e-4066-b4a1-ad5ec991baa1.root");
    //    c1.Add("root://cmsxrootd.fnal.gov//store/mc/Run3Summer22EENanoAODv12/TWminusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2022_realistic_postEE_v6-v2/2520000/41fc870a-c510-41a9-aa22-afa245d77acd.root");
        outputFile = "test_2022EE.root";
        era = "PostEE";
    } else if (year == "2023") {
        // 2023 configuration
        c1.Add("root://cmsxrootd.fnal.gov///store/mc/Run3Summer23NanoAODv12/TbarWplusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2023_realistic_v15-v4/50000/063916a6-99cf-4832-a945-c2ddfaee53dd.root");
        //c1.Add("root://cmsxrootd.fnal.gov///store/data/Run2023C/EGamma1/NANOAOD/24Jan2024_v1-v1/2820000/f89c06db-6003-4234-a6b3-a7383a7248b0.root");
        outputFile = "test_2023.root";
        era = "PreBPix";
    } else {
        // 2023 BPix configuration
       // c1.Add("root://cmsxrootd.fnal.gov///store/data/Run2023D/MuonEG/NANOAOD/22Sep2023_v2-v1/2540000/2b1baeec-bc24-4a11-b7ae-220dd5987884.root");
       c1.Add("root://cmsxrootd.fnal.gov///store/mc/Run3Summer23BPixNanoAODv12/TTto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2023_realistic_postBPix_v2-v3/2550000/1fc49961-22ba-4b79-86d7-e85128f21146.root");
        outputFile = "test_2023BPix.root";
        era = "PostBPix";
    }

    BaseAnalyser nanoaodrdf(&c1, outputFile, 10, 10);
    nanoaodrdf.setParams(year, era, -1);
    nanoaodrdf.setHLT();

    // Configuration based on year
    string goodjsonfname, pileupfname, pileuptag, btvfname, btvtype;
    string fname_btagEff, hname_btagEff_bcflav, hname_btagEff_lflav;
    string jercfname, jerctag, jettagMC, jercunctag;
    string muon_roch_fname, muon_fname, muonHLTtype, muonIDtype, muonISOtype;
    string electron_fname, Hlt_fname ;
    string electron_reco_type1, electron_reco_type2, electron_id_type;
    string jet_veto_f_name, jet_veto_tag, electron_SSF, metpt_fname, JER_tag;
    
    if (year == "2022") {
        goodjsonfname = "data/Cert_Collisions2022_355100_362760_Golden.json";
        pileupfname = "data/LUM/2022_preEE/puWeights.json";
        pileuptag = "Collisions2022_355100_357900_eraBCD_GoldenJson";
        btvfname = "data/BTV/2022_preEE/btagging.json";
        btvtype = "deepJet_shape";
        fname_btagEff = "data/BTV/2022_preEE/BtaggingEfficiency.root";
        hname_btagEff_bcflav = "hist_btagEff_bcflav";
        hname_btagEff_lflav = "hist_btagEff_lflav";
        jercfname = "data/JERC/2022_preEE/jet_jerc.json";
        jerctag = "Summer22_22Sep2023_RunCD_V2_DATA_L1L2L3Res_AK4PFPuppi";
        jettagMC = "Summer22_22Sep2023_V2_MC_L1L2L3Res_AK4PFPuppi";
        jercunctag = "Summer22_22Sep2023_V2_MC_Total_AK4PFPuppi";
        muon_roch_fname = "data/MUON/2022_preEE/muon_scalesmearing.json";
        muon_fname = "data/MUON/2022_preEE/muon_Z.json.gz";
        muonHLTtype = "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight";
        muonIDtype = "NUM_TightID_DEN_TrackerMuons";
        muonISOtype = "NUM_TightPFIso_DEN_TightID";
        electron_fname = "data/EGM/2022_preEE/electron.json.gz";
        Hlt_fname = "data/trigger_scale_factors.root";
        electron_reco_type1 = "Reco20to75";
        electron_reco_type2 = "RecoAbove75";
        electron_id_type = "Tight";
        jet_veto_f_name = "data/JERC/2022_preEE/jetvetomaps.json.gz";
        jet_veto_tag = "Summer22_23Sep2023_RunCD_V1";
        electron_SSF = "data/EGM/2022_preEE/electronSS_EtDependent.json.gz";
        metpt_fname = "data/JERC/2022_preEE/met_xyCorrections_2022_2022.json";
        JER_tag = "Summer22_22Sep2023_JRV1_MC_ScaleFactor_AK4PFPuppi";
    } else if (year == "2022EE") {
        goodjsonfname = "data/Cert_Collisions2022_355100_362760_Golden.json";
        pileupfname = "data/LUM/2022_postEE/puWeights.json";
        pileuptag = "Collisions2022_359022_362760_eraEFG_GoldenJson";
        btvfname = "data/BTV/2022_postEE/btagging.json";
        btvtype = "deepJet_shape";
        fname_btagEff = "data/BTV/2022_postEE/BtaggingEfficiency.root";
        hname_btagEff_bcflav = "hist_btagEff_bcflav";
        hname_btagEff_lflav = "hist_btagEff_lflav";
        jercfname = "data/JERC/2022_postEE/jet_jerc.json";
        jerctag = "Summer22EE_22Sep2023_RunE_V2_DATA_L1L2L3Res_AK4PFPuppi";
        jettagMC = "Summer22EE_22Sep2023_V2_MC_L1L2L3Res_AK4PFPuppi";
        jercunctag = "Summer22EE_22Sep2023_V2_MC_Total_AK4PFPuppi";
        muon_roch_fname = "data/MUON/2022_postEE/muon_scalesmearing.json.gz";
        muon_fname = "data/MUON/2022_postEE/muon_Z.json.gz";
        muonHLTtype = "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight";
        muonIDtype = "NUM_TightID_DEN_TrackerMuons";
        muonISOtype = "NUM_TightPFIso_DEN_TightID";
        electron_fname = "data/EGM/2022_postEE/electron.json.gz";
        Hlt_fname = "data/trigger_scale_factors.root";
        electron_reco_type1 = "Reco20to75";
        electron_reco_type2 = "RecoAbove75";
        electron_id_type = "Tight";
        jet_veto_f_name = "data/JERC/2022_postEE/jetvetomaps.json.gz";
        jet_veto_tag = "Summer22EE_23Sep2023_RunEFG_V1";
        electron_SSF = "data/EGM/2022_postEE/electronSS_EtDependent.json.gz";
        metpt_fname = "data/JERC/2022_postEE/met_xyCorrections_2022_2022EE.json.gz";
        JER_tag = "Summer22EE_22Sep2023_JRV1_MC_ScaleFactor_AK4PFPuppi";
    } else if (year == "2023") {
        goodjsonfname = "data/golden_json_2023.json";
        pileupfname = "data/LUM/2023/puWeights.json";
        pileuptag = "Collisions2023_366403_369802_eraBC_GoldenJson";
        btvfname = "data/BTV/2023/btagging.json";
        btvtype = "deepJet_shape";
        fname_btagEff = "data/BTV/2023/BtaggingEfficiency.root";
        hname_btagEff_bcflav = "hist_btagEff_bcflav";
        hname_btagEff_lflav = "hist_btagEff_lflav";
        jercfname = "data/JERC/2023/jet_jerc.json";
        jerctag = "Summer23Prompt23_V2_DATA_L1L2L3Res_AK4PFPuppi";
        jettagMC = "Summer23Prompt23_V2_MC_L1L2L3Res_AK4PFPuppi";
        jercunctag = "Summer23Prompt23_V2_MC_Total_AK4PFPuppi";
        muon_roch_fname = "data/MUON/2023/muon_scalesmearing.json";
        muon_fname = "data/MUON/2023/muon_Z.json.gz";
        muonHLTtype = "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight";
        muonIDtype = "NUM_TightID_DEN_TrackerMuons";
        muonISOtype = "NUM_TightPFIso_DEN_TightID";
        electron_fname = "data/EGM/2023/electron.json";
        Hlt_fname = "data/trigger_scale_factors.root";
        electron_reco_type1 = "Reco20to75";
        electron_reco_type2 = "RecoAbove75";
        electron_id_type = "Tight";
        jet_veto_f_name = "data/JERC/2023/jetvetomaps.json";
        jet_veto_tag = "Summer23Prompt23_RunC_V1";
        electron_SSF = "data/EGM/2023/electronSS_EtDependent.json";
        metpt_fname = "data/JERC/2023/met_xyCorrections_2023_2023.json";
        JER_tag = "Summer23Prompt23_RunCv1234_JRV1_MC_ScaleFactor_AK4PFPuppi";
    } else {  // 2023BPix
        goodjsonfname = "data/golden_json_2023.json";
        pileupfname = "data/LUM/2023_post_BPIX/puWeights.json";
        pileuptag = "Collisions2023_369803_370790_eraD_GoldenJson";
        btvfname = "data/BTV/2023_post_BPIX/btagging.json";
        btvtype = "deepJet_shape";
        fname_btagEff = "data/BTV/2023/BtaggingEfficiency.root";
        hname_btagEff_bcflav = "hist_btagEff_bcflav";
        hname_btagEff_lflav = "hist_btagEff_lflav";
        jercfname = "data/JERC/2023_post_BPIX/jet_jerc.json";
        jerctag = "Summer23BPixPrompt23_V3_DATA_L1L2L3Res_AK4PFPuppi";
        jettagMC = "Summer23BPixPrompt23_V3_MC_L1L2L3Res_AK4PFPuppi";
        jercunctag = "Summer23BPixPrompt23_V3_MC_Total_AK4PFPuppi";
        muon_roch_fname = "data/MUON/2023_post_BPIX/muon_scalesmearing.json";
        muon_fname = "data/MUON/2023_post_BPIX/muon_Z.json.gz";
        muonHLTtype = "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight";
        muonIDtype = "NUM_TightID_DEN_TrackerMuons";
        muonISOtype = "NUM_TightPFIso_DEN_TightID";
        electron_fname = "data/EGM/2023_post_BPIX/electron.json";
        Hlt_fname = "data/trigger_scale_factors.root";
        electron_reco_type1 = "Reco20to75";
        electron_reco_type2 = "RecoAbove75";
        electron_id_type = "Tight";
        jet_veto_f_name = "data/JERC/2023_post_BPIX/jetvetomaps.json";
        jet_veto_tag = "Summer23BPixPrompt23_RunD_V1";
        electron_SSF = "data/EGM/2023_post_BPIX/electronSS_EtDependent.json";
        metpt_fname = "data/JERC/2023_post_BPIX/met_xyCorrections_2023_2023BPix.json.gz";
        JER_tag = "Summer23BPixPrompt23_RunD_JRV1_MC_ScaleFactor_AK4PFPuppi";
    }
    
    nanoaodrdf.setupCorrections(goodjsonfname, pileupfname, pileuptag, btvfname, btvtype,
                                fname_btagEff, hname_btagEff_bcflav, hname_btagEff_lflav,
                                muon_roch_fname, muon_fname, muonHLTtype, muonIDtype, muonISOtype,
                                electron_fname, Hlt_fname, 
                                electron_reco_type1, electron_reco_type2, electron_id_type,
                                jercfname, jerctag, jettagMC, jercunctag, jet_veto_f_name,
                                jet_veto_tag, electron_SSF, metpt_fname, JER_tag);
    
    nanoaodrdf.setupObjects();
    nanoaodrdf.setupAnalysis();
    nanoaodrdf.run(false, "outputTree");
    
    return EXIT_SUCCESS;
}
