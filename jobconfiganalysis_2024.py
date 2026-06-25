

config = {

        # tree name of input file(s)
        'intreename': "Events",

        # tree name of output file(s) it cannot be the same as the input tree name or it'll crash
        'outtreename': "outputTree",

        #data year (2016,2017,2018)
        'year': "2024",

        # is ReReco or Ultra Legacy
        'runtype': 'none',

        'datatype': -1, # 0=MC ; 1=DATA ; -1=Auto


        #for correction

        # good json file
        # 'goodjson': 'data/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt',
        'goodjson' : 'data/golden_json_2024.json',

        # pileup weight for MC
        'pileupfname': 'data/LUM/2024/puWeights.json',

        'pileuptag': 'Collisions24_BCDEFGHI_goldenJSON',

        # json filename for BTV correction
        'btvfname': 'data/BTV/2024/btagging.json',

        # BTV correction type
        'btvtype': 'deepJet_shape',
        'fname_btagEff': 'data/BTV/2024/BtaggingEfficiency.root',
        'hname_btagEff_bcflav':'hist_btagEff_bcflav',
        'hname_btagEff_lflav': 'hist_btagEff_lflav',

        # json file name for JERC
        'jercfname': 'data/JERC/2024/jet_jerc.json',
        'jerctag' : 'Summer24Prompt24_V3_DATA_L1L2L3Res_AK4PFPuppi',
        # conbined correction type for jets
        'jercunctag': [
            "Summer24Prompt24_V3_MC_Regrouped_Total_AK4PFPuppi",
        ],
        # jet uncertainty 
        'jettagMC' :'Summer24Prompt24_V3_MC_L1L2L3Res_AK4PFPuppi',
        'muon_roch_fname': 'data/MUON/2024/muon_scalesmearing.json',
        'muon_fname': 'data/MUON/2024/muon_Z.json',
        'muonHLTtype': 'NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight',
      #  'muonRECOtype': 'NUM_TrackerMuons_DEN_genTracks',
        'muonIDtype': 'NUM_TightID_DEN_TrackerMuons',
        'muonISOtype': 'NUM_TightPFIso_DEN_TightID',
        'electron_fname': 'data/EGM/2024/electron.json',
        'Hlt_fname':'data/Hlt_scale_factor/2024/trigger_scale_factors.root',
        'electron_reco_type1': 'Reco20to75',
        'electron_reco_type2' :  'RecoAbove75',
        'electron_id_type': 'Tight',
        'jet_veto_f_name':'data/JERC/2024/jetvetomaps.json',
        'jet_veto_tag' : 'Summer24Prompt24_RunBCDEFGHI_V1',
        'electron_SSF' :'data/EGM/2024/electronSS_EtDependent.json',
        'metpt_fname' : 'data/JERC/2022_preEE/met_xyCorrections_2022_2022.json',
        'JER_tag' : 'Summer24Prompt24_JRV1_MC_ScaleFactor_AK4PFPuppi',
        'JER_tag_res' : 'Summer24Prompt24_JRV1_MC_PtResolution_AK4PFPuppi',
        }

# processing options
procflags = {
        ###### how many jobs? ######
        'split': 50, #only use 'Max' when nrootfiles = 'All'
        # 'split': 50,

        ###### if False, one output file per input file, if True then one output file for everything ######
#        'allinone': False,
        'allinone': True,

        ###### if True then skip existing analyzed files ######
        'skipold': True,

        ###### travel through the subdirectories and their subdirecties when processing. ######
        ###### becareful not to mix MC and real DATA in them. ######
        'recursive': True,

        ###### if False then only selected branches which is done in the .cpp file will be saved ######
        'saveallbranches': False,

        ###### How many input files? ######
        'nrootfiles': 10000,
        # 'nrootfiles': 50,

        ###### Make a copy of the analyzer in the same directory as the analyzed root files ######
        'copyInstance': False,
        }

