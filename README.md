# fly_project nanoaodrdframework 
Analysis of CMS experiment Nano AOD format using ROOT RDataFrame

The package shows how to analyze using the data frame concept.
In a data frame concept, the operations on data should be defined
in terms of functions, which means that result of applying
a function should produce outputs and no internal states are allowed.
In ROOT, RDataFrame allows us to borrow the concept for analysis.

This package code implements a hadronic channel analysis using NanoAOD format.
However, it can be adapted for any flat ROOT trees.

## I. Introduction

- The advantage of using RDataFrame is that you don't have to
worry about looping, filling of histograms at the right place,
writing out trees. In some sense you can concentrate more on
thinking about data analysis.

- Draw back is that you have to think of a new way of doing old things.
This could getting used to. Alos complicated algorithms may not
be so easy to implement with RDataFrame

- Purely data frame concept is not ideal for HEP data since
we have to treat real data and simulated data differently.
We must apply different operations depending on the input data,
   such as scale factors and corrections.
Therefore, some form of global state should be stored. 
In this package, the data frame concept is used together
with object oriented concept for this purpose.


## II. Code

- The code consists of a main class NanoAODAnalyzerrdframe. 
There is one.h header file and one .cpp source file for it.
The class has several methods:
    - Object definitions (selectElectrons, selectMuons, ...)
    - Additional derived variables definition (defineMoreVars)
    - Histogram definitions (bookHists)
    - Selections (defineCuts)
    - Plus some utility methods/functions (generate_4vec,calculateDR,reconstruction,..)

- Users should modify: object definitions, define additional variables, histogram definitions, selections.

## III. Clone Repository 

  * Setup CMSSW

  You must use CMSSW 12.3.X or higher since correctionlib 2 is available there.

      > cmsrel CMSSW_12_3_X
      > cd CMSSW_12_3_X/src

  * Clone with SSH :
      > git clone ssh://git@gitlab.cern.ch:7999/fly/fly.git

  * Clone with HTTPS:
      > git clone https://gitlab.cern.ch/fly/fly.git

  You must use ROOT 6.24. (ROOT 6.22 is the default for CMSSW 12.3)

      > source /cvmfs/cms.cern.ch/slc7_amd64_gcc900/lcg/root/6.24.07-db9b7135424812ed5b8723a7a77f4016/bin/thisroot.(c)sh

## IV. Compiling

        > make -j(24)

  this will compile and create a libnanoaodrdframe.so shared library that can be loaded in a ROOT session or macro:  gSystem->Load("libnanoadrdframe.so");


## V. Running over large dataset

      > ./submitanalysisjob.py jobconfiganalysis.py

  * "submitanalysisjob.py" script loop over the directories and submit jobs over the configuation python module (processnanoad.py)
      > os.system('./processnanoaod.py '+ indir + ' ' + outdir + ' ' + jobconfmod  + ' > ' + outfile + ' 2>&1 &')

  * "processnanoaod.py" script can automatically run over all ROOT files in an input directory and run over the subclass (BaseAnalyser.cpp)
      > aproc = ROOT.BaseAnalyser(t, outputroot)

  * "jobconfiganalysis.py" script consists of options for Nanoaodrdframe (config) + processing options (proflags) to enter your parameters and input/output directories.

        # options for Nanoaodrdframe
        
            config = {
            'intreename': "Events",       # tree name of input file(s)
            'outtreename': "outputTree2", # tree name of output file(s) it cannot be the same as thinput tree name or it'll crash
            'year': 2018,                 #data year (2016,2017,2018)
            'runtype': 'UL',              # is ReReco or Ultra Legacy
            'datatype': -1,               # -1: checking "gen" branch if it's exist run over MC. 1: Data0:MC
        }
        # processing options
        
          procflags = {
            'split': 1,                   # how many jobs?
            'allinone': True,             # if "False" one output file per input file, if True then onoutput file for everything
            'skipold': True,              # if "True" then skip existing analyzed files
            'recursive': True,            # travel through the subdirectories and their subdirecties wheprocessing.
                                      # be careful not to mix   MC and real DATA in them.
            'saveallbranches': False,     # if False then only selected branches which is done in thecpp file will be saved
            'nrootfiles': 1,              #How many input files?
        }

        # input directory where your input root tree resides && output directory wheere your output should go && dump of stderr/stdout to file

            nanoaod_inputdir_outputdir_pairs = [
                    ['testinputdata/MC/2018','analyzed/test_nano_v1.root', 'stderrout.out' ],
            ]

