#!/bin/python
# -*- coding: utf-8 -*-
"""
Created on Mon Sep 14 11:01:46 2018

@author: Suyong Choi (Department of Physics, Korea University suyong@korea.ac.kr)

This script applies nanoaod processing to one file
"""

import sys
import cppyy
import ROOT

if __name__=='__main__':
    from optparse import OptionParser
    parser = OptionParser(usage="%prog [options] inputDir outputDir")
    parser.add_option("-Y", "--year",  dest="year", type="string", default="", help="Select 2016, 2017, or 2018 for years")
    parser.add_option("-S", "--syst",  dest="syst", type="string", default="", help="Systematic sources")
    parser.add_option("-J", "--json",  dest="json", type="string", default="", help="Select events using this JSON file, meaningful only for data")
    parser.add_option("--saveallbranches", dest="saveallbranches", action="store_true", default=False, help="Save all branches. False by default")
    parser.add_option("--globaltag", dest="globaltag", type="string", default="", help="Global tag to be used in JetMET corrections")
    (options, args) = parser.parse_args()

    if len(args) < 1:
        parser.print_help()
        sys.exit(1)
    infile = args[0]
    outfile = args[1]
    intreename = args[2]
    outtreename = args[3]


    # load compiled C++ library into ROOT/python
    cppyy.load_reflection_info("libnanoadrdframe.so")
    t = ROOT.TChain(intreename)
    t.Add(infile)
    aproc=None
    aproc = ROOT.SkimEvents(t, outfile, options.year, options.syst, options.json, options.globaltag, 1)
    aproc.setupAnalysis()
    aproc.run(options.saveallbranches, outtreename)
