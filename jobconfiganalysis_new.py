
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
        'year': 2022,

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
        'nrootfiles': 1000,
        # 'nrootfiles': 50,

        ###### Make a copy of the analyzer in the same directory as the analyzed root files ######
        'copyInstance': False,
        }

# input directory where your input root tree resides
# output directory where your output should go
outDir='/eos/uscms/store/user/vsinha/processed_ntuple'

nanoaod_inputdir_outputdir_pairs = [

    # TTBAR SAMPLE
    ['/TTtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/TTbar_SemiLept.root', outDir+'/TTbar_SemiLept.out'],
    ['/TTto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/TTbar_Dilept.root', outDir+'/TTbar_Dilept.out'],
    ['/TTto4Q_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/TTbar_Hadronic.root', outDir+'/TTbar_Hadronic.out'],

    # Rare backgrounds
    ['/WW_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/WW.root', outDir+'/WW.out'],
    ['/WZ_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/WZ.root', outDir+'/WZ.out'],
    ['/ZZ_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/ZZ.root', outDir+'/ZZ.out'],

    # Drell-Yan
    ['/DYto2L-2Jets_MLL-10to50_TuneCP5_13p6TeV_amcatnloFXFX-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/DYjetsM10to50.root', outDir+'/DYjetsM10to50.out'],
    ['/DYJetsToLL_M-50_TuneCP5_13p6TeV-madgraphMLM-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/DYJetsM50.root', outDir+'/DYJetsM50.out'],

    # W+jets
    ['/WtoLNu-2Jets_0J_TuneCP5_13p6TeV_amcatnloFXFX-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v3/NANOAODSIM', outDir+'/W0Jets.root', outDir+'/W0Jets.out'],
    ['/WtoLNu-2Jets_1J_TuneCP5_13p6TeV_amcatnloFXFX-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/W1Jets.root', outDir+'/W1Jets.out'],
    ['/WtoLNu-2Jets_2J_TuneCP5_13p6TeV_amcatnloFXFX-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/W2Jets.root', outDir+'/W2Jets.out'],

    # QCD MuEnriched
    ['/QCD_PT-15to20_MuEnrichedPt5_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt15to20_Mu.root', outDir+'/QCD_Pt15to20_Mu.out'],
    ['/QCD_PT-20to30_MuEnrichedPt5_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt20to30_Mu.root', outDir+'/QCD_Pt20to30_Mu.out'],
    ['/QCD_PT-30to50_MuEnrichedPt5_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt30to50_Mu.root', outDir+'/QCD_Pt30to50_Mu.out'],
    ['/QCD_PT-50to80_MuEnrichedPt5_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt50to80_Mu.root', outDir+'/QCD_Pt50to80_Mu.out'],
    ['/QCD_PT-80to120_MuEnrichedPt5_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt80to120_Mu.root', outDir+'/QCD_Pt80to120_Mu.out'],
    ['/QCD_PT-120to170_MuEnrichedPt5_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt120to170_Mu.root', outDir+'/QCD_Pt120to170_Mu.out'],
    ['/QCD_PT-170to300_MuEnrichedPt5_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt170to300_Mu.root', outDir+'/QCD_Pt170to300_Mu.out'],
    ['/QCD_PT-300to470_MuEnrichedPt5_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt300to470_Mu.root', outDir+'/QCD_Pt300to470_Mu.out'],
    ['/QCD_PT-470to600_MuEnrichedPt5_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt470to600_Mu.root', outDir+'/QCD_Pt470to600_Mu.out'],
    ['/QCD_PT-600to800_MuEnrichedPt5_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt600to800_Mu.root', outDir+'/QCD_Pt600to800_Mu.out'],
    ['/QCD_PT-800to1000_MuEnrichedPt5_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt800to1000_Mu.root', outDir+'/QCD_Pt800to1000_Mu.out'],
    ['/QCD_PT-1000_MuEnrichedPt5_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt1000toInf_Mu.root', outDir+'/QCD_Pt1000toInf_Mu.out'],

    # QCD EleEnriched
    ['/QCD_PT-10to30_EMEnriched_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt10to30_Ele.root', outDir+'/QCD_Pt10to30_Ele.out'],
    ['/QCD_PT-30to50_EMEnriched_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt30to50_Ele.root', outDir+'/QCD_Pt30to50_Ele.out'],
    ['/QCD_PT-50to80_EMEnriched_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt50to80_Ele.root', outDir+'/QCD_Pt50to80_Ele.out'],
    ['/QCD_PT-80to120_EMEnriched_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt80to120_Ele.root', outDir+'/QCD_Pt80to120_Ele.out'],
    ['/QCD_PT-120to170_EMEnriched_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt120to170_Ele.root', outDir+'/QCD_Pt120to170_Ele.out'],
    ['/QCD_PT-170to300_EMEnriched_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt170to300_Ele.root', outDir+'/QCD_Pt170to300_Ele.out'],
    ['/QCD_PT-300toInf_EMEnriched_TuneCP5_13p6TeV_pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/QCD_Pt300toInf_Ele.root', outDir+'/QCD_Pt300toInf_Ele.out'],

    # Signal samples
    ['/TbarBQ_t-channel_4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/TbarBQ_t_channel.root', outDir+'/TbarBQ_t_channel.out'],
    ['/TBbarQ_t-channel_4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/TBbarQ_t_channel.root', outDir+'/TBbarQ_t_channel.out'],

    # s-channel background
    ['/TbarBtoLminusNuB-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/TbarBtoLminusNuB_s_channel.root', outDir+'/TbarBtoLminusNuB_s_channel.out'],
    ['/TBbartoLplusNuBbar-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/TBbartoLplusNuBbar_s_channel.root', outDir+'/TBbartoLplusNuBbar_s_channel.out'],

    # STW samples
    ['/TbarWplusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/TbarWplusto2L2Nu.root', outDir+'/TbarWplusto2L2Nu.out'],
    ['/TWminusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/TWminusto2L2Nu.root', outDir+'/TWminusto2L2Nu.out'],
    ['/TbarWplusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5_ext1-v2/NANOAODSIM', outDir+'/TbarWplusto2L2Nu_ext.root', outDir+'/TbarWplusto2L2Nu_ext.out'],
    ['/TWminusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5_ext1-v2/NANOAODSIM', outDir+'/TWminusto2L2Nu_ext.root', outDir+'/TWminusto2L2Nu_ext.out'],

    ['/TbarWplusto4Q_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/TbarWplusto4Q.root', outDir+'/TbarWplusto4Q.out'],
    ['/TWminusto4Q_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/TWminusto4Q.root', outDir+'/TWminusto4Q.out'],
    ['/TbarWplusto4Q_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5_ext1-v2/NANOAODSIM', outDir+'/TbarWplusto4Q_ext.root', outDir+'/TbarWplusto4Q_ext.out'],
    ['/TWminusto4Q_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5_ext1-v2/NANOAODSIM', outDir+'/TWminusto4Q_ext.root', outDir+'/TWminusto4Q_ext.out'],

    ['/TbarWplustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/TbarWplustoLNu2Q.root', outDir+'/TbarWplustoLNu2Q.out'],
    ['/TWminustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2/NANOAODSIM', outDir+'/TWminustoLNu2Q.root', outDir+'/TWminustoLNu2Q.out'],
    ['/TbarWplustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5_ext1-v2/NANOAODSIM', outDir+'/TbarWplustoLNu2Q_ext.root', outDir+'/TbarWplustoLNu2Q_ext.out'],
    ['/TWminustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5_ext1-v2/NANOAODSIM', outDir+'/TWminustoLNu2Q_ext.root', outDir+'/TWminustoLNu2Q_ext.out'],

    # Data samples
    ['/SingleMuon/Run2022C-16Dec2023-v1/NANOAOD', outDir+'/Data_SingleMu_c.root', outDir+'/Data_SingleMu_c.out'],
    ['/Muon/Run2022C-16Dec2023-v1/NANOAOD', outDir+'/Data_Mu_c.root', outDir+'/Data_Mu_c.out'],
    ['/Muon/Run2022D-16Dec2023-v1/NANOAOD', outDir+'/Data_Mu_d.root', outDir+'/Data_Mu_d.out'],
    ['/Muon/Run2022E-16Dec2023-v1/NANOAOD', outDir+'/Data_Mu_e.root', outDir+'/Data_Mu_e.out'],
    ['/Muon/Run2022F-16Dec2023-v1/NANOAOD', outDir+'/Data_Mu_f.root', outDir+'/Data_Mu_f.out'],
    ['/Muon/Run2022G-16Dec2023-v1/NANOAOD', outDir+'/Data_Mu_g.root', outDir+'/Data_Mu_g.out'],

    ['/EGamma/Run2022C-16Dec2023-v1/NANOAOD', outDir+'/Data_Ele_c.root', outDir+'/Data_Ele_c.out'],
    ['/EGamma/Run2022D-16Dec2023-v1/NANOAOD', outDir+'/Data_Ele_d.root', outDir+'/Data_Ele_d.out'],
    ['/EGamma/Run2022E-16Dec2023-v1/NANOAOD', outDir+'/Data_Ele_e.root', outDir+'/Data_Ele_e.out'],
    ['/EGamma/Run2022F-16Dec2023-v1/NANOAOD', outDir+'/Data_Ele_f.root', outDir+'/Data_Ele_f.out'],
    ['/EGamma/Run2022G-16Dec2023-v1/NANOAOD', outDir+'/Data_Ele_g.root', outDir+'/Data_Ele_g.out'],
]

