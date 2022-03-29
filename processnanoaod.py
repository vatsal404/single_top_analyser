#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Mon Sep 14 11:01:46 2018

@author: Suyong Choi (Department of Physics, Korea University suyong@korea.ac.kr)

This script applies nanoaod processing to all the files in the input
directory and its subdirectory recursively, copying directory structure to the outputdirectory

"""
from pydoc import isdata
import sys
import os
import re
import string
import subprocess
import time


from importlib import import_module
from multiprocessing import Process
import cppyy
import ROOT

# Enable multi-threading
#ROOT.ROOT.EnableImplicitMT()


ROOT.gInterpreter.Declare('#include "src/BaseAnalyser.h"') #header defined manually in python!!
def function_calling_PostProcessor(outdir, rootfileshere, outtreename, intreeename, year, runtype, syst, json, saveallbranches, globaltag, isDATA):
    print("Called post proc")
    for afile in rootfileshere:
        rootfname = re.split('\/', afile)[-1]
        withoutext = re.split('\.root', rootfname)[0]
        outfname = outdir + '/' + withoutext + '_analyzed.root'
        if saveallbranches:
            subprocess.call(["./processonefile.py","--year=%s"%year,"--runtype=%s"%runtype,"--syst=%s"%syst, "--json=%s"%json, "--saveallbranches", "--globaltag=%s"%globaltag, afile, outfname, intreeename, outtreename])
        else:
            subprocess.call(["./processonefile.py", "--year=%s"%year,"--runtype=%s"%runtype,"--syst=%s"%syst, "--json=%s"%json, "--globaltag=%s"%globaltag, afile, outfname, intreeename, outtreename])
    pass


class Nanoaodprocessor:
    def __init__(self, outdir, indir, outtree, intree, year,runtype, syst, json, split, skipold, recursive, saveallbranches, globaltag, nrootfiles, isDATA):
        self.outdir = outdir
        self.indir = indir
        self.outtreename = outtree
        self.intreeename = intree
        self.year = year
        self.runtype = runtype
        self.syst = syst
        self.json = json
        self.split = split
        self.skipold = skipold
        self.recursive = recursive
        self.saveallbranches = saveallbranches
        self.globaltag = globaltag
        self.nrootfiles = nrootfiles
        self.isDATA = isDATA


        # check whether input directory exists
        if not os.path.exists(self.indir):
            print ('Path '+indir+' does not exist')
            exit(1)
        pass

    def process(self):
        self._processROOTfiles(self.indir, self.outdir)
        pass


    def _processROOTfiles(self, inputdirectory, outputdirectory):
        # list currect directory
        flist = os.listdir(inputdirectory)
        print(flist)
        rootfileshere = []
        subdirs = []
        outsubdirs = []
        # create output directory if it doesn't already exist
        if not os.path.exists(outputdirectory):
            os.mkdir(outputdirectory)
        # loop through the list
        # pick root files but not those that match  _analyzed.root
        counter=0
        for fname in flist:
            fullname = os.path.join(inputdirectory, fname)
            if re.match('.*\.root', fname) and not re.match('.*_analyzed\.root', fname) and os.path.isfile(fullname): # if it has .root file extension
                counter+=1
                if counter<=self.nrootfiles and self.nrootfiles!=0 :
                    rootfileshere.append(fullname)
                elif self.nrootfiles ==0 :
                    rootfileshere.append(fullname)
            elif os.path.isdir(fullname):  # if it's a directory name
                subdirs.append(fullname)
                outsubdirs.append(outputdirectory+'/'+fname)

        print("files found in directory "+inputdirectory)
        print(rootfileshere)

        # run
        if len(rootfileshere)>0:
            #print("running1")
            if self.skipold: # if processed file exists, then skip
                oflist = os.listdir(outputdirectory)
                filteredoflist = []
                for fname in oflist :
                    fullname = os.path.join(outputdirectory, fname)
                    print(fullname)
                    if re.match('.*\.root', fname) and os.path.isfile(fullname): # if it has .root file extension:
                        withoutext = re.split("\.root", fname)[0]
                        wihoutskimtext = re.split("\_analyzed", withoutext)[0]
                        filteredoflist.append(wihoutskimtext)
                print('root files in output directory')
                print(filteredoflist)
                filterediflist = []
                for ifname in rootfileshere:
                    rootfname = re.split('\/', ifname)[-1]
                    withoutext = re.split('\.root', rootfname)[0]
                    if withoutext not in filteredoflist:
                        print(withoutext+' not yet in output dir')
                        filterediflist.append(ifname)
                    else:
                        print(withoutext+' in output dir')

                rootfileshere = filterediflist

            if self.split>1: # use multiprocessing
                njobs = self.split
                nfileperjob = len(rootfileshere) *1.0 / njobs
                #print("running2")

                # if number of files is less than the number of splits desired
                if len(rootfileshere) < self.split:
                    njobs = len(rootfileshere)
                    nfileperjob = 1

                print ("splitting files")

                ap = []
                for i in range(njobs):
                    if i<njobs-1:
                        filesforjob = rootfileshere[int(i*nfileperjob):int((i+1)*nfileperjob)]
                    else:
                        filesforjob = rootfileshere[int(i*nfileperjob):]
                    p = Process(target=function_calling_PostProcessor, args=(outputdirectory, filesforjob, self.outtreename, self.intreeename, self.year, self.runtype,self.syst, self.json, self.saveallbranches, self.globaltag,  self.isDATA)) # positional arguments go into kwargs
                    p.start()
                    ap.append(p)
                for proc in ap:
                    proc.join()
            else: # no multiprocessing
                #function_calling_PostProcessor(outputdirectory, rootfileshere, self.json, self.isdata,)
                aproc = None
                for afile in rootfileshere:
                    rootfname = re.split('\/', afile)[-1]
                    withoutext = re.split('\.root', rootfname)[0]
                    outfname = outputdirectory +'/'+ withoutext + '_analyzed.root'
                    print("running3")
                    print(self.year)
                    if self.saveallbranches:
                        subprocess.call(["./processonefile.py","--year=%s"%self.year,"--runtype=%s"%self.runtype, "--syst=%s"%self.syst,"--json=%s"%self.json, "--saveallbranches", "--globaltag=%s"%self.globaltag, afile, outfname, self.intreeename, self.outtreename])
                    else:
                        subprocess.call(["./processonefile.py","--year=%s"%self.year,"--runtype=%s"%self.runtype,"--syst=%s"%self.syst, "--json=%s"%self.json, "--globaltag=%s"%self.globaltag, afile, outfname, self.intreeename, self.outtreename])
                    print("running4")

                    #subprocess.call(["./processonefile.py", "--year=%s"%self.year,"--runtype=%s"%self.runtype,"--syst=%s"%self.syst, "--json=%s"%self.json, "--saveallbranches=%s"%self.saveallbranches, "--globaltag=%s"%self.globaltag, afile, outfname, self.intreeename, self.outtreename])

                    # the following works, but memory usage of this process grows with time.. Don't know how to solve it.
                    """
                    t = ROOT.TChain("Events")
                    t.Add(afile)
                    rootfname = re.split('\/', afile)[-1]
                    withoutext = re.split('\.root', rootfname)[0]
                    outfname = outputdirectory +'/'+ withoutext + '_analyzed.root'
                    if aproc is None:
                        aproc = ROOT.NanoAODAnalyzerrdframe(t, outfname, self.json)
                        aproc.setupAnalysis()
                    else:
                        aproc.setTree(t, outfname)
                    aproc.run()
                    t.Delete();
                    t = None
                    """
        # if there are subdirectories recursively call
        if self.recursive:
            for indir, outdir in zip(subdirs, outsubdirs):
                self._processROOTfiles(indir, outdir)

def Nanoaodprocessor_singledir(outputroot, indir, outtree, intree, year,runtype, syst, json, split, recursive, saveallbranches, globaltag, nrootfiles, isDATA):
    """Runs nanoaod analyzer over ROOT files in indir (but doesn't search recursively)
    and run outputs into a signel ROOT file.

    Arguments:
        outputroot {string} -- [description]
        indir {string} -- [description]
        json {string} -- [description]
    """
    #print("CHECK OUT ! isDATA == " )
    #print(isDATA)
    #print("INFO: ")
    #print("if isDATA == -1: DATA or MC selected according to the input file -genweight- branch")
    #print("if isDATA == 0: You SELECTED MC input files ")
    #print("if isDATA == 1: You SELECTED DATA input files")
    if not re.match('.*\.root', outputroot):
        print("Output file should be a root file! Quitting")
        exit(-1)

    fullnamelist =[]
    rootfilestoprocess = []
    print("collecting root files in "+indir)
    if not recursive:
        flist = os.listdir(indir)
        for fname in flist:
            fullname = os.path.join(indir, fname)
            fullnamelist.append(fullname)
    else: # os.walk lists directories recursively (here, we will not follow symlink)
        for root, dirs, flist in os.walk(indir):
            for fname in flist:
                fullname = os.path.join(root, fname)
                fullnamelist.append(fullname)
    counter=0
    for  fname in fullnamelist:
        if re.match('.*\.root', fname) and os.path.isfile(fname): # if it has .root file extension
            counter+=1
            if counter<=nrootfiles and nrootfiles !=0 :
                rootfilestoprocess.append(fname)
            elif nrootfiles ==0 :
                rootfilestoprocess.append(fname)

    print("files to process")
    print(rootfilestoprocess)
    t = ROOT.TChain(intree)
    for afile in rootfilestoprocess:
        t.Add(afile)
    nevents= t.GetEntries()
    print("-------------------------------------------------------------------")
    print("Total Number of Entries:")
    print(nevents)
    print("-------------------------------------------------------------------")

    aproc = ROOT.BaseAnalyser(t, outputroot, year, runtype, syst, json, globaltag, split,isDATA)
    aproc.setupAnalysis()
    aproc.run(saveallbranches, outtree)

    pass

if __name__=='__main__':

    from optparse import OptionParser
    # inputDir and lower directories contain input NanoAOD files
    # outputDir is where the outputs will be created
    parser = OptionParser(usage="%prog [options] inputDir outputDir")
    parser.add_option("-Y", "--year",              dest="year",                 type="string",             default="",         help="Select 2016, 2017, or 2018 runs")
    parser.add_option("-S",    "--syst",              dest="syst",                 type="string",             default="",         help="Systematic sources")
    parser.add_option("-R", "--runtype",          dest="runtype",             type="string",             default="",         help="Select run type : UL or ReReco")
    parser.add_option("-D",    "--isDATA",         dest="isDATA",                 type=int,                 default=-1,         help="Select DATA or MC")
    parser.add_option("-N",    "--nrootfiles",     dest="nrootfiles",             type=int,                 default=0,             help="Select Max number of input root files")
    parser.add_option("-J",    "--json",              dest="json",                 type="string",             default="",         help="Select events using this JSON file, meaningful only for data")
    parser.add_option("--split",                 dest="split",                 type=int,                 default=1,             help="How many jobs to split into")
    parser.add_option("--skipold",                 dest="skipold",             action="store_true",     default=False,        help="Skip existing root files")
    parser.add_option("--recursive",             dest="recursive",             action="store_true",     default=True,         help="Process files in the subdirectories recursively")
    parser.add_option("-A","--allinone",         dest="allinone",             action="store_true",    default=False,         help="Process all files and output a single root file. You must make sure MC and Data are not mixed together.")
    parser.add_option("--saveallbranches",         dest="saveallbranches",     action="store_true",     default=False,         help="Save all branches. False by default")
    parser.add_option("--globaltag",             dest="globaltag",             type="string",             default="",         help="Global tag to be used in JetMET corrections")
    (options, args) = parser.parse_args()

    if len(args) < 1 :
        parser.print_help()
        sys.exit(1)

    indir = args[0]
    outdir = args[1]

    # load compiled C++ library into ROOT/python
    cppyy.load_reflection_info("libnanoadrdframe.so")

    #intree = "outputTree"
    intree = "Events"
    outtree = "outputTree2"


    if not options.allinone:
        n=Nanoaodprocessor(outdir, indir, outtree, intree, options.year, options.runtype, options.syst, options.json, options.split, options.skipold, options.recursive, options.saveallbranches, options.globaltag,  options.nrootfiles, options.isDATA)
        n.process()
    else:
        Nanoaodprocessor_singledir(outdir, indir, outtree, intree, options.year, options.runtype, options.syst, options.json,  options.split, options.recursive, options.saveallbranches, options.globaltag, options.nrootfiles, options.isDATA) # although it says outdir, it should really be a output ROOT file name
