
config = {

        # tree name of input file(s)
        'intreename': "Events",

        # tree name of output file(s) it cannot be the same as the input tree name or it'll crash
        'outtreename': "outputTree",

        #data year (2016,2017,2018)
        'year': "2022",

        # is ReReco or Ultra Legacy
        'runtype': 'PreEE',

        'datatype': -1, # 0=MC ; 1=DATA ; -1=Auto


        #for correction

        # good json file
        # 'goodjson': 'data/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt',
        'goodjson' : 'data/Cert_Collisions2022_355100_362760_Golden.json',

        # pileup weight for MC
        'pileupfname': 'data/LUM/2022_preEE/puWeights.json',

        'pileuptag': 'Collisions2022_355100_357900_eraBCD_GoldenJson',

        # json filename for BTV correction
        'btvfname': 'data/BTV/2022_preEE/btagging.json',

        # BTV correction type
        'btvtype': 'deepJet_shape',
        'fname_btagEff': 'data/BTV/2023/BtaggingEfficiency.root',
        'hname_btagEff_bcflav':'hist_btagEff_bcflav',
        'hname_btagEff_lflav': 'hist_btagEff_lflav',

        # json file name for JERC
        'jercfname': 'data/JERC/2022_preEE/jet_jerc.json',

        # conbined correction type for jets
        'jerctag': 'Summer22_22Sep2023_RunCD_V2_DATA_L1L2L3Res_AK4PFPuppi', 

        # jet uncertainty 
        'jercunctag': 'Summer22_22Sep2023_V2_MC_Total_AK4PFPuppi',
        'jettagMC' :'Summer22_22Sep2023_V2_MC_L1L2L3Res_AK4PFPuppi',
        'muon_roch_fname': 'data/MUON/2022_preEE/muon_scalesmearing.json',
        'muon_fname': 'data/MUON/2022_preEE/muon_Z.json.gz',
        'muonHLTtype': 'NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight',
      #  'muonRECOtype': 'NUM_TrackerMuons_DEN_genTracks',
        'muonIDtype': 'NUM_TightID_DEN_TrackerMuons',
        'muonISOtype': 'NUM_TightPFIso_DEN_TightID',
        'electron_fname': 'data/EGM/2022_preEE/electron.json.gz',
        'Hlt_fname':'data/trigger_scale_factors.root',
        'electron_reco_type1': 'Reco20to75',
        'electron_reco_type2' :  'RecoAbove75',
        'electron_id_type': 'Tight',
        'jet_veto_f_name':'data/JERC/2022_preEE/jetvetomaps.json.gz',
        'jet_veto_tag' : 'Summer22_23Sep2023_RunCD_V1',
        'electron_SSF' :'data/EGM/2022_preEE/electronSS_EtDependent.json.gz',
        'metpt_fname' : 'data/JERC/2022_preEE/met_xyCorrections_2022_2022.json',
        'JER_tag' : 'Summer22_22Sep2023_JRV1_MC_ScaleFactor_AK4PFPuppi'
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
        'nrootfiles': 1000,
        # 'nrootfiles': 50,

        ###### Make a copy of the analyzer in the same directory as the analyzed root files ######
        'copyInstance': False,
        }

