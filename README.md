With this package, we can do the following to NanoAOD files
- Apply good JSON
- Calculate b-tag weights for MC
- Apply Jet/MET corrections
- skim events

For this, we use the data frame concept to simplify the code, but
it requires to learn new way of using ROOT.
In a data frame concept, the operations on data should be defined
in terms of functions, which means that result of applying
a function should produce outputs and no internal states are allowed.
In ROOT, RDataFrame allows us to borrow the concept for analysis.

This package code implements a hadronic channel analysis using NanoAOD format.
However, it can be adapted for any flat ROOT trees.

Separate branches exist for 2017 samples and 2018 samples.

I. Introduction

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


II. Code

- The code consists of a main class NanoAODAnalyzerrdframe. 
There is one .h header file and one .cpp source file for it.
The class has several methods:
    - Object definitions (selectElectrons, selectMuons, ...)
    - Additional derived variables definition (defineMoreVars)
    - Histogram definitions (bookHists)
    - Selections (defineCuts)
    - Plus some utility methods/functions (gen4vec, readjson, removeOverlaps, helper_1DHistCreator, createHists)

- Users should modify: object definitions, define additional variables, histogram definitions, selections.

- To use the class, you need to pass a pointer to TChain object, output root file name and a JSON file.
  If there is "genWeight" branch in the ROOT Tree then the input is assumed to be MC, otherwise data.
  Look at nanoaoddataframe.cpp to find how to use within C++ 

- Compiling
  Need ROOT 6.17 or later (available in /master-data/common_tools/ROOT617 in our local  master server) then,

  > make

  this will compile and create a libnanoaodrdframe.so shared library that can be loaded in a ROOT session or macro:
  gSystem->Load("libnanoadrdframe.so");

  or within pyROOT (look in processnanoaod.py).

III. Running over large dataset

processnanoaod.py script can automatically run over all ROOT files in an input directory.
Take a look at skimevents.sh script to see how it's done.

Usage: processnanoaod.py [options] inputDir outputDir

Options:
  -h, --help            show this help message and exit
  -J JSON, --json=JSON  Select events using this JSON file, meaningful only
                        for data
  --split=SPLIT         How many jobs to split into
  --skipold             Skip existing root files
  --recursive           Process files in the subdirectories recursively
  --allinone            Process all files and output a single root file. You
                        must make sure MC and Data are not mixed together.
  --saveallbranches     Option to  save all branches. Default is to save only select branches
  --globaltag           Global tag information, which is used for JetMET corrections.
                        If omitted, then no corrections are applied.

  
By default, it will go into subdirectories recursively and process ROOT files. 
It will make the output directory to have the same  the directory structure as the input directory.
It will create one output file per one input file, so there is one-to-one correspondence.
--split option will allow multithreading you to use multiple CPUs to process the files. (e.g. --split=5)
--skipold option will let you skip files that were analyzed, by matching files that have _analyzed in the file names.
--recursive mode is on by default. If you don't want that, --recursive=False.
--allinone option will let you get a single output root file for all the files in the inputDir. In this mode, --skipold doesn't work

