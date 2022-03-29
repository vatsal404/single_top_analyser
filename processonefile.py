#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Mon Sep 14 11:01:46 2018

@author: Suyong Choi (Department of Physics, Korea University suyong@korea.ac.kr)

This script applies nanoaod processing to one file
"""

import sys
import cppyy
import ROOT
ROOT.gInterpreter.Declare('#include "src/BaseAnalyser.h"')

if __name__=='__main__':
    from optparse import OptionParser
    parser = OptionParser(usage="%prog [options] inputDir outputDir")
    parser.add_option("-Y", "--year",          dest="year",                 type="string",             default="",         help="Select 2016, 2017, 2018 year of runs")
    parser.add_option("-R", "--runtype",      dest="runtype",                 type="string",             default="",         help="Select run type: UL or ReReco") # dest is Json or runtype?
    parser.add_option("-D",    "--isDATA",     dest="isDATA",                 type=int,                 default=-1,         help="Select DATA or MC")
    parser.add_option("--split",                 dest="split",                 type=int,                 default=1,             help="How many jobs to split into")
    parser.add_option("-S", "--syst",          dest="syst",                 type="string",             default="",         help="Systematic sources")
    parser.add_option("-J", "--json",          dest="json",                 type="string",             default="",         help="Select events using this JSON file, meaningful only for data")
    parser.add_option("--saveallbranches",     dest="saveallbranches",     action="store_true",     default=False,         help="Save all branches. False by default")
    parser.add_option("--globaltag",         dest="globaltag",             type="string",             default="",         help="Global tag to be used in JetMET corrections")

    (options, args) = parser.parse_args()
    if len(args) < 1:
        print("number of args")
        print(len(args))
        parser.print_help()
        sys.exit(1)
    infile = args[0]
    outfile = args[1]
    intreename = args[2]
    outtreename = args[3]

    print (infile, outfile,options.year,options.runtype, options.syst, options.json, options.globaltag,1,options.isDATA)

    # load compiled C++ library into ROOT/python
    cppyy.load_reflection_info("libnanoadrdframe.so")
    t = ROOT.TChain(intreename)
    t.Add(infile)
    print("Inside process one file..!!")

    aproc = ROOT.BaseAnalyser(t, outfile, options.year,options.runtype,options.syst, options.json, options.globaltag, options.isDATA)
    print("Called")
    aproc.setupAnalysis()
    aproc.run(options.saveallbranches, outtreename)

