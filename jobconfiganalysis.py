
"""
File contains job options 
"""


# options for Nanoaodrdframe
config = {
        
        # tree name of input file(s)
        'intreename': "Events",

        # tree name of output file(s) it cannot be the same as the input tree name or it'll crash
        'outtreename': "outputTree",

        #data year (2016,2017,2018)
        'year': 2017,

        # is ReReco or Ultra Legacy
        'runtype': 'UL',

        'datatype': -1, # 0=MC ; 1=DATA ; -1=Auto


        #for correction
        
        # good json file
        # 'goodjson': 'data/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt',
        'goodjson' : 'data/Legacy_RunII/Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON.txt',

        # pileup weight for MC
        'pileupfname': 'data/LUM/2017_UL/puWeights.json',

        'pileuptag': 'Collisions17_UltraLegacy_goldenJSON',

        # json filename for BTV correction
        'btvfname': 'data/BTV/2017_UL/btagging.json',

        # BTV correction type
        'btvtype': 'deepJet_shape',

        # json file name for JERC
        'jercfname': 'data/JERC/UL17_jerc.json',

        # conbined correction type for jets
        'jerctag': 'Summer19UL17_V5_MC_L1L2L3Res_AK4PFchs', 

        # jet uncertainty 
        'jercunctag': 'Summer19UL17_V5_MC_Total_AK4PFchs', 
        'muon_roch_fname': 'data/MUO/2017_UL/RoccoR2017UL.txt',
        'muon_fname': 'data/MUO/2017_UL/muon_Z.json.gz',
        'muonHLTtype': 'NUM_IsoMu27_DEN_CutBasedIdTight_and_PFIsoTight',
        'muonRECOtype': 'NUM_TrackerMuons_DEN_genTracks',
        'muonIDtype': 'NUM_MediumID_DEN_TrackerMuons',
        'muonISOtype': 'NUM_TightRelIso_DEN_MediumID',
        'electron_fname': 'data/EGM/2017_UL/electron.json.gz',
        'electron_reco_type': 'RecoAbove20',
        'electron_id_type': 'Tight'
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
        'nrootfiles': 50,
        # 'nrootfiles': 50,

        ###### Make a copy of the analyzer in the same directory as the analyzed root files ######
        'copyInstance': False,
        }

# input directory where your input root tree resides
# output directory where your output should go
outDir='/srv/CMSSW_12_3_4/src/fly'
# dump of stderr/stdout to file

nanoaod_inputdir_outputdir_pairs = [


#         ['/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8/RunIISummer20UL17NanoAODv9-20UL17JMENano_106X_mc2017_realistic_v9-v1/NANOAODSIM',outDir + 'process_ST_t-channel_top_4f.root',outDir + '/process_ST_t-channel_top_4f.txt'],


 #        ['/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM',outDir + 'process_ST_t-channel_antitop_4f.root',outDir + '/process_ST_t-channel_antitop_4f.txt'],


         #['/ST_tW_antitop_5f_inclusiveDecays_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM','/srv/test.root','outputtest.out']

         ['/ST_tW_antitop_5f_inclusiveDecays_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM', '/srv/PROC_ST_tW_top_5f.root', '/PROC_ST_t-channel_top_UL17.txt' ], #197
         ['/ST_tW_top_5f_inclusiveDecays_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM', outDir +  '/PROC_ST_tW_antitop_5f.root', outDir +  '/PROC_ST_t-channel_antitop_UL17.out'], #60

        ['/TT_TuneCH3_13TeV-powheg-herwig7/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM', outDir + '/PROC_ST_tW-channel_top_UL17.root', outDir + '/PROC_ST_tW-channel_top_UL17.out'], #43
        # [mc17 + '/ST_tW_antitop_5f_inclusiveDecays_TuneCP5_13TeV-powheg-pythia8', outDir + '/PROC_ST_tW-channel_antitop_UL17.root', outDir + '/PROC_ST_tW-channel_antitop_UL17.out'], #48
        ['/ST_s-channel_4f_leptonDecays_TuneCP5_13TeV-amcatnlo-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM', outDir + '/PROC_ST_s-channel_UL17.root', outDir + '/PROC_ST_s-channel_UL17.out'], #16

        ['/DYJetsToLL_M-10to50_TuneCP5_13TeV-madgraphMLM-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM', outDir + '/PROC_DYJetsToLL_M-10to50_UL17.root', outDir + '/PROC_DYJetsToLL_M-10to50_UL17.out'], #77
        ['/DYJetsToLL_M-50_TuneCP5_13TeV-madgraphMLM-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM', outDir + '/PROC_DYJetsToLL_M-50_UL17.root', outDir + '/PROC_DYJetsToLL_M-50_UL17.out'], #75

        # [mc17 + '/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8', outDir + '/PROC_TTTo2L2Nu_UL17.root', outDir + '/PROC_TTTo2L2Nu_UL17.out'], #99
        # [mc17 + '/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8', outDir + '/PROC_TTToSemiLeptonic_UL17.root', outDir + '/PROC_TTToSemiLeptonic_UL17.out'], #297
        # [mc17 + '/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8', outDir + '/PROC_TTWJetsToLNu_UL17.root', outDir + '/PROC_TTWJetsToLNu_UL17.out'], #9
        # [mc17 + '/TTWJetsToQQ_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8', outDir + '/PROC_TTWJetsToQQ_UL17.root', outDir + '/PROC_TTWJetsToQQ_UL17.out'], #10
        # [mc17 + '/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8', outDir + '/PROC_TTZToLLNuNu_UL17.root', outDir + '/PROC_TTZToLLNuNu_UL17.out'], #19
        # [mc17 + '/TTZToQQ_TuneCP5_13TeV-amcatnlo-pythia8', outDir + '/PROC_TTZToQQ_UL17.root', outDir + '/PROC_TTZToQQ_UL17.out'], #35
     
        ['/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM', outDir + '/PROC_WJetsToLNu_UL17.root', outDir + '/PROC_WJetsToLNu_UL17.out'], #81

         #['/WW_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM', outDir + '/PROC_WW_UL17.root', outDir + '/PROC_WW_UL17.out'], #16
        # [mc17 + '/WZ_TuneCP5_13TeV-pythia8', outDir + '/PROC_WZ_UL17.root', outDir + '/PROC_WZ_UL17.out'], #20
        # [mc17 + '/ZZ_TuneCP5_13TeV-pythia8', outDir + '/PROC_ZZ_UL17.root', outDir + '/PROC_ZZ_UL17.out'], #2

         ['/QCD_Pt-15to20_EMEnriched_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM'      ,  outDir + '/PROC_QCD_Pt-15to20_EMEnriched.root'  ,  outDir + '/PROC_QCD_Pt-15to20_EMEnriched.out'], #25
         ['/QCD_Pt-20to30_EMEnriched_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM'      ,  outDir + '/PROC_QCD_Pt-20to30_EMEnriched.root'  ,  outDir + '/PROC_QCD_Pt-20to30_EMEnriched.out'], #72
         ['/QCD_Pt-30to50_EMEnriched_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM'      ,  outDir + '/PROC_QCD_Pt-30to50_EMEnriched.root'  ,  outDir + '/PROC_QCD_Pt-30to50_EMEnriched.out'], #7
         ['/QCD_Pt-50to80_EMEnriched_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM'      ,  outDir + '/PROC_QCD_Pt-50to80_EMEnriched.root'  ,  outDir + '/PROC_QCD_Pt-50to80_EMEnriched.out'], #8
         ['/QCD_Pt-80to120_EMEnriched_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM'     ,  outDir + '/PROC_QCD_Pt-80to120_EMEnriched.root' ,  outDir + '/PROC_QCD_Pt-80to120_EMEnriched.out'], #21
         ['/QCD_Pt-120to170_EMEnriched_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM'    ,  outDir + '/PROC_QCD_Pt-120to170_EMEnriched.root',  outDir + '/PROC_QCD_Pt-120to170_EMEnriched.out'], #37
         ['/QCD_Pt-170to300_EMEnriched_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM'    ,  outDir + '/PROC_QCD_Pt-70to300_EMEnriched.root' ,  outDir + '/PROC_QCD_Pt-70to300_EMEnriched.out'], #5
#         ['/QCD_Pt-300toinf_EMEnriched_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM'    ,  outDir + '/PROC_QCD_Pt-300toInf_EMEnriched.root',  outDir + '/PROC_QCD_Pt-300toInf_EMEnriched.out'], #5

         ['/QCD_Pt-15To20_MuEnrichedPt5_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM'   ,  outDir + '/PROC_QCD_Pt-15To20_MuEnrichedPt5.root'   ,  outDir + '/PROC_QCD_Pt-15To20_MuEnrichedPt5.out'], #18
         [ '/QCD_Pt-20To30_MuEnrichedPt5_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM'   ,  outDir + '/PROC_QCD_Pt-20To30_MuEnrichedPt5.root'   ,  outDir + '/PROC_QCD_Pt-20To30_MuEnrichedPt5.out'], #87
         [ '/QCD_Pt-30To50_MuEnrichedPt5_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM'   ,  outDir + '/PROC_QCD_Pt-30To50_MuEnrichedPt5.root'   ,  outDir + '/PROC_QCD_Pt-30To50_MuEnrichedPt5.out'], #110
         ['/QCD_Pt-50To80_MuEnrichedPt5_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM'   ,  outDir + '/PROC_QCD_Pt-50To80_MuEnrichedPt5.root'   ,  outDir + '/PROC_QCD_Pt-50To80_MuEnrichedPt5.out'], #46
         ['/QCD_Pt-80To120_MuEnrichedPt5_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM'  ,  outDir + '/PROC_QCD_Pt-80To120_MuEnrichedPt5.root'  ,  outDir + '/PROC_QCD_Pt-80To120_MuEnrichedPt5.out'], #60
         ['/QCD_Pt-120To170_MuEnrichedPt5_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM' ,  outDir + '/PROC_QCD_Pt-120To170_MuEnrichedPt5.root' ,  outDir + '/PROC_QCD_Pt-120To170_MuEnrichedPt5.out'], #78
         ['/QCD_Pt-170To300_MuEnrichedPt5_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM' ,  outDir + '/PROC_QCD_Pt-170To300_MuEnrichedPt5.root' ,  outDir + '/PROC_QCD_Pt-170To300_MuEnrichedPt5.out'], #94
         ['/QCD_Pt-300To470_MuEnrichedPt5_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM' ,  outDir + '/PROC_QCD_Pt-300To470_MuEnrichedPt5.root' ,  outDir + '/PROC_QCD_Pt-300To470_MuEnrichedPt5.out'], #68
         ['/QCD_Pt-470To600_MuEnrichedPt5_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM' ,  outDir + '/PROC_QCD_Pt-470To600_MuEnrichedPt5.root' ,  outDir + '/PROC_QCD_Pt-470To600_MuEnrichedPt5.out'], #42
         ['/QCD_Pt-600To800_MuEnrichedPt5_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM' ,  outDir + '/PROC_QCD_Pt-600To800_MuEnrichedPt5.root' ,  outDir + '/PROC_QCD_Pt-600To800_MuEnrichedPt5.out'], #58
         ['/QCD_Pt-800To1000_MuEnrichedPt5_TuneCP5_13TeV-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM',  outDir + '/PROC_QCD_Pt-800To1000_MuEnrichedPt5.root',  outDir + '/PROC_QCD_Pt-800To1000_MuEnrichedPt5.out'], #101
         ['/QCD_Pt-1000_MuEnrichedPt5_TuneCP5_13TeV-pythia8'     ,  outDir + '/PROC_QCD_Pt-1000_MuEnrichedPt5.root'     ,  outDir + '/PROC_QCD_Pt-1000_MuEnrichedPt5.out'], #39


          [ '/SingleElectron/Run2017B-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD' , outDir +'/Run2017B_UL_SingleElectron.root' ,  outDir + '/Run2017B_UL_SingleElectron.out'], #32
#          [ '/SingleElectron/Run2017C-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD' ,  outDir + '/Run2017C_UL_SingleElectron.root' ,  outDir + '/Run2017C_UL_SingleElectron.out'], #59
 #         [ '/SingleElectron/Run2017D-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD' ,  outDir + '/Run2017D_UL_SingleElectron.root' ,  outDir + '/Run2017D_UL_SingleElectron.out'], #37
  #        [ '/SingleElectron/Run2017E-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD' ,  outDir + '/Run2017E_UL_SingleElectron.root' ,  outDir + '/Run2017E_UL_SingleElectron.out'],
   #       [ '/SingleElectron/Run2017F-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD' ,  outDir + '/Run2017F_UL_SingleElectron.root' ,  outDir + '/Run2017F_UL_SingleElectron.out'], #66

#         ['/SingleMuon/Run2017B-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD' ,  outDir + '/Run2017B_UL_SingleMuon.root' ,  outDir + '/Run2017B_UL_SingleMuon.out'], #79
 #        ['/SingleMuon/Run2017C-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD' ,  outDir + '/Run2017C_UL_SingleMuon.root' ,  outDir + '/Run2017C_UL_SingleMuon.out'], #117
  #       ['/SingleMuon/Run2017D-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD' ,  outDir + '/Run2017D_UL_SingleMuon.root' ,  outDir + '/Run2017D_UL_SingleMuon.out'], #47
   #      ['/SingleMuon/Run2017E-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD' ,  outDir + '/Run2017E_UL_SingleMuon.root' ,  outDir + '/Run2017E_UL_SingleMuon.out'],
         ['/SingleMuon/Run2017F-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD' ,  outDir + '/Run2017F_UL_SingleMuon.root' ,  outDir + '/Run2017F_UL_SingleMuon.out'] #115'''
]
