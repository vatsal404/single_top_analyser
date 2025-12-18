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
    std::string year = "2023"; // default
    if (argc > 1) {
        year = argv[1];  // Directly assign string
        if (year != "2022" && year != "2023") {
            cerr << "Error: Year must be 2022 or 2023" << endl;
            cerr << "Usage: " << argv[0] << " [year]" << endl;
            return EXIT_FAILURE;
        }
    }

    cout << "Running analysis for year: " << year << endl;

    TChain c1("Events");

    string outputFile;
    string era;
    if (year == 2022) {
        // 2022 configuration
        //      c1.Add("root://cmsxrootd.fnal.gov///store/mc/Run3Summer22NanoAODv12/WtoLNu-2Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8/NANOAODSIM/130X_mcRun3_2022_realistic_v5-v2/30000/269ca882-375b-49dc-80ff-01dc2c227522.root"); //data
        //      c1.Add("root://xrootd-cms.infn.it//store/mc/RunIISummer20UL17NanoAODv9/WJetsToLNu_0J_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mc2017_realistic_v9-v2/2810000/8EDF6CCF-B922-2B40-8889-B0CD442F88CC.root")
        //
       // c1.Add("root://xrootd-cms.infn.it//store/mc/Run3Summer22NanoAODv12/TTtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2022_realistic_v5-v2/30000/670acfa8-2b1a-4ec5-932b-0512e54fd5f8.root");
              c1.Add("root://cmsxrootd.fnal.gov//store/data/Run2022D/Muon/NANOAOD/16Dec2023-v1/50000/fa77d341-cad2-4902-a837-308655dbca47.root"); // MC
        //c1.Add("root://xrootd-cms.infn.it//store/data/Run2022D/Muon/NANOAOD/16Dec2023-v1/50000/a921644d-6ceb-4bc3-81c9-fddd5c7b1edb.root"); // tw
        outputFile = "test_2022.root";
        era = "PreEE";  // or whatever era you need
    } else {
        // 2023 configuration
        //c1.Add("root://cmsxrootd.fnal.gov///store/mc/Run3Summer23NanoAODv12/TTtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2023_realistic_v14-v2/2520000/04fb6a6e-b9c5-4df1-99fa-c804ef776cba.root"); //ttbar
        c1.Add("root://cmsxrootd.fnal.gov///store/mc/Run3Summer23NanoAODv12/TbarWplusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2023_realistic_v15-v4/50000/063916a6-99cf-4832-a945-c2ddfaee53dd.root");//tw
        //c1.Add("root://xrootd-cms.infn.it//store/mc/Run3Summer23NanoAODv12/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2023_realistic_v14-v3/2810000/0019dbdc-5ef5-4cb5-85e5-7655c7002d47.root");//DY

        //  c1.Add("root://cmsxrootd.fnal.gov///store/data/Run2023C/MuonEG/NANOAOD/22Sep2023_v4-v1/30000/0874994b-9d31-4c1f-bdbc-d0073f7c7c4a.root");
        outputFile = "test_2023.root";
        era = "PreEE";
    }

    BaseAnalyser nanoaodrdf(&c1, outputFile, 10, 10);
    nanoaodrdf.setParams(year, era, -1);
    nanoaodrdf.setHLT();

    // Configuration based on year
    string goodjsonfname, pileupfname, pileuptag, btvfname, btvtype;
    string fname_btagEff, hname_btagEff_bcflav, hname_btagEff_lflav;
    string jercfname, jerctag, jettagMC, jercunctag;
    string muon_roch_fname, muon_fname, muonHLTtype, muonIDtype, muonISOtype;
    string electron_fname, electronHlt_fname, electronHlt_type;
    string electron_reco_type1, electron_reco_type2, electron_id_type;
    string jet_veto_f_name, jet_veto_tag, electron_SSF, metpt_fname, JER_tag;
    
    if (year == 2022) {
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
//       muonRECOtype = "NUM_TrackerMuons_DEN_genTracks";
         muonIDtype = "NUM_TightID_DEN_TrackerMuons";
         muonISOtype = "NUM_TightPFIso_DEN_TightID";
         electron_fname = "data/EGM/2022_preEE/electron.json.gz";
         electronHlt_fname = "data/EGM/2022_preEE/electronHlt.json.gz";
        electronHlt_type="HLT_SF_Ele30_TightID";
        electron_reco_type1 = "Reco20to75";
         electron_reco_type2 =  "RecoAbove75";
         electron_id_type = "Tight";
         jet_veto_f_name="data/JERC/2022_preEE/jetvetomaps.json.gz";
         jet_veto_tag = "Summer22_23Sep2023_RunCD_V1";
         electron_SSF = "data/EGM/2022_preEE/electronSS_EtDependent.json.gz";
         metpt_fname = "data/JERC/2022_preEE/met_xyCorrections_2022_2022.json";
         JER_tag = "Summer22_22Sep2023_JRV1_MC_ScaleFactor_AK4PFPuppi";

    } else {
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
        electronHlt_fname = "data/EGM/2023/electronHlt.json";
        electronHlt_type = "HLT_SF_Ele30_TightID";
        electron_reco_type1 = "Reco20to75";
        electron_reco_type2 = "RecoAbove75";
        electron_id_type = "Tight";
        jet_veto_f_name = "data/JERC/2023/jetvetomaps.json";
        jet_veto_tag = "Summer23Prompt23_RunC_V1";
        electron_SSF = "data/EGM/2023/electronSS_EtDependent.json";
        metpt_fname = "data/JERC/2023/met_xyCorrections_2023_2023.json";
        JER_tag = "Summer23Prompt23_RunCv1234_JRV1_MC_ScaleFactor_AK4PFPuppi";
    }
    
    nanoaodrdf.setupCorrections(goodjsonfname, pileupfname, pileuptag, btvfname, btvtype,
                                fname_btagEff, hname_btagEff_bcflav, hname_btagEff_lflav,
                                muon_roch_fname, muon_fname, muonHLTtype, muonIDtype, muonISOtype,
                                electron_fname, electronHlt_fname, electronHlt_type,
                                electron_reco_type1, electron_reco_type2, electron_id_type,
                                jercfname, jerctag, jettagMC, jercunctag, jet_veto_f_name,
                                jet_veto_tag, electron_SSF, metpt_fname, JER_tag);
    
    nanoaodrdf.setupObjects();
    nanoaodrdf.setupAnalysis();
    nanoaodrdf.run(false, "outputTree");
    
    return EXIT_SUCCESS;
}
