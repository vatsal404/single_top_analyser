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
        #'goodjson': 'data/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt',
        # pileup weight for MC
        #'pileupfname': 'data/LUM/2016postVFP_UL/puWeights.json',
        #'pileupfname': 'data/LUM/2017_UL/puWeights.json',
        #'pileuptag': 'Collisions16_UltraLegacy_goldenJSON',
        #'pileuptag': 'Collisions17_UltraLegacy_goldenJSON',
        # json filename for BTV correction
        #'btvfname': 'data/BTV/2016postVFP_UL/btagging.json',
        #'btvfname': 'data/BTV/2017_UL/btagging.json',
        # BTV correction type
        #'btvtype': 'deepJet_shape',
        # json file name for JERC
        #'jercfname': 'data/JERC/UL16postVFP_jerc.json',
        #'jercfname': 'data/JERC/UL17_jerc.json',
        # conbined correction type for jets
        #'jerctag': 'Summer19UL16_V7_MC_L1L2L3Res_AK4PFchs', 
        #'jerctag': 'Summer19UL17_V5_MC_L1L2L3Res_AK4PFchs', 
        # jet uncertainty 
        #'jercunctag': 'Summer19UL16_V7_MC_Total_AK4PFchs', 
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
        'nrootfiles': 1,
        }
nanoaod_inputdir_outputdir_pairs = [
        ['testinputdata/MC/2018/Signal/','analyzed/testsignal.root', 'stderrout.out' ],
        #['/eos/lyoeos.in2p3.fr/grid/cms/store/data/Run2017B/SingleElectron/NANOAOD/UL2017_MiniAODv2_NanoAODv9-v1/70000/AA0215EC-9E53-5D4C-A655-D54C667A38C1.root','analyzed/Chris_data.root', 'stdertest.out' ]
]

