"""
File contains job options 
"""

# options for Nanoaodrdframe
config = {
        # tree name of input file(s)
        'intreename': "Events",
        # tree name of output file(s) it cannot be the same as the input tree name or it'll crash
        'outtreename': "outputTree2",
        #data year (2016,2017,2018)
        'year': 2018,
        # is ReReco or Ultra Legacy
        'runtype': 'UL',
        'datatype': -1,


        #for correction
        # good json file 
        'goodjson': 'data/Legacy_RunII/Cert_314472-325175_13TeV_Legacy2018_Collisions18_JSON.txt', #UL_2018
        #'goodjson': 'data/Legacy_RunII/Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON.txt', #UL_2017

        # pileup weight for MC 
        'pileupfname': 'data/LUM/2018_UL/puWeights.json',
        #'pileupfname': 'data/LUM/2017_UL/puWeights.json',

        'pileuptag': 'Collisions18_UltraLegacy_goldenJSON',
        #'pileuptag': 'Collisions17_UltraLegacy_goldenJSON',

        # json filename for BTV correction 
        'btvfname': 'data/BTV/2018_UL/btagging.json', 
        #'btvfname': 'data/BTV/2017_UL/btagging.json',
        # BTV correction type
        'btvtype': 'deepJet_shape',

        # json file name for JERC
        'jercfname': 'data/JERC/jetUL18_jerc.json',
        #'jercfname': 'data/JERC/UL17_jerc.json',

        # conbined correction type for jets
        'jerctag': 'Summer19UL18_V5_MC_L1L2L3Res_AK4PFchs', 
        #'jerctag': 'Summer19UL17_V5_MC_L1L2L3Res_AK4PFchs', 
        # jet uncertainty 
        'jercunctag': 'Summer19UL18_V5_MC_Total_AK4PFchs', 
        #'jercunctag': 'Summer19UL17_V5_MC_Total_AK4PFchs', 
        
        }

# processing options
procflags = {
        # how many jobs?
        'split': 1,
        # if False, one output file per input file, if True then one output file for everything
        'allinone': True, 
        # if True then skip existing analyzed files
        'skipold': True,
        # travel through the subdirectories and their subdirecties when processing.
        # becareful not to mix MC and real DATA in them.
        'recursive': True,
        # if False then only selected branches which is done in the .cpp file will be saved
        'saveallbranches': False,
        #How many input files?
        'nrootfiles': 2,
        }
nanoaod_inputdir_outputdir_pairs = [
        ['testinputdata/MC/2018/BG/','analyzed/testcorrection.root', 'stderrout.out' ],
        #for process one file:
        #['testinputdata/data/JetHT_2017C_DA05CA1A-3265-EE47-84F9-10CB09D22BDA.root','analyzed/testcorrection_data.root', 'stderrout.out' ],
        #for data
        #['/eos/lyoeos.in2p3.fr/grid/cms/store/data/Run2017B/SingleElectron/NANOAOD/UL2017_MiniAODv2_NanoAODv9-v1/70000/','analyzed/coorectiontest_data.root', 'stdertest.out' ]
]

