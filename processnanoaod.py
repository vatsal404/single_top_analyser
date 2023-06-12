#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Sep 14 11:01:46 2018
Modified Feb 26 2022

@author: Suyong Choi (Department of Physics, Korea University suyong@korea.ac.kr)

This script applies nanoaod processing to all the files in the input
 directory and its subdirectory recursively, copying directory structure to the outputdirectory

The main class that steers the processing is Nanoaodprocessor. For processing a single ROOT file,
it calls function_calling_PostProcessor, which executes processonefile.py as a separate process.
(The reason this is done rather than doing everything inside Nanoaodprocessor is that the
memory usage grows with time when each file is processed inside Nanoaodprocessor, probably a problem
in memory allocation and clean up in Python)

"""
import os
import re
import subprocess
#import time
import sys

from multiprocessing import Process
import cppyy
import ROOT


def function_calling_PostProcessor(outdir, rootfileshere, jobconfmod):
    for afile in rootfileshere:
        rootfname = re.split('\/', afile)[-1]
        withoutext = re.split('\.root', rootfname)[0]
        outfname = outdir + '/' + withoutext + '_analyzed.root'
        subprocess.run(["./processonefile.py", afile, outfname, jobconfmod])
    pass


class Nanoaodprocessor:
    def __init__(self, indir, outdir, jobconfmod, procflags, config):
        self.outdir = outdir
        self.indir = indir
        self.jobconfmod = jobconfmod
        self.split = procflags['split']
        self.skipold = procflags['skipold']
        self.recursive = procflags['recursive']
        self.saveallbranches = procflags['saveallbranches']
        #set parameters
        self.nrootfiles =procflags['nrootfiles']
        self.year =config['year']
        self.runtype =config['runtype']
        self.datatype = config['datatype']
        print("year=", self.year)
        #print("data=", self.datatype)

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
            if re.match('.*\.root', fname) and os.path.isfile(fullname): # if it has .root file extension
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
            print("running1")
            if self.skipold: # if processed file exists, then skip
                oflist = os.listdir(outputdirectory)
                filteredoflist = []
                for fname in oflist :
                    fullname = os.path.join(outputdirectory, fname)
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
                print("running2")

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
                    p = Process(target=function_calling_PostProcessor, args=(outputdirectory, filesforjob, self.jobconfmod)) # positional arguments go into kwargs
                    p.start()
                    ap.append(p)
                for proc in ap:
                    proc.join()
            else: # no multiprocessing
                #function_calling_PostProcessor(outputdirectory, rootfileshere, self.json, self.isdata)
                aproc = None
                for afile in rootfileshere:
                    rootfname = re.split('\/', afile)[-1]
                    withoutext = re.split('\.root', rootfname)[0]
                    outfname = outputdirectory +'/'+ withoutext + '_analyzed.root'
                    subprocess.run(["./processonefile.py", afile, outfname, self.jobconfmod])

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

def Nanoaodprocessor_singledir(indir, outputroot, procflags, config):
    """Runs nanoaod analyzer over ROOT files in indir (but doesn't search recursively)
    and run outputs into a signel ROOT file.

    Arguments:
        outputroot {string} -- [description]
        indir {string} -- [description]
        json {string} -- [description]
    """

    if not re.match('.*\.root', outputroot):
        print("Output file should be a root file! Quitting")
        exit(-1)

    fullnamelist =[]
    rootfilestoprocess = []
    print("COLLECT root files in : \n"  + indir + "\n")
    if not procflags['recursive']:
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
            if counter<=procflags['nrootfiles'] and procflags['nrootfiles'] !=0 : #singledir fonksiyonu class icinde olmadigi icin proflags icinden rootfiles cagirdik
                rootfilestoprocess.append(fname)
            elif procflags['nrootfiles'] ==0 :
                rootfilestoprocess.append(fname)

    print("FILES to PROCESS")
    print(rootfilestoprocess)
    intreename = config['intreename']
    outtreename = config['outtreename']
    saveallbranches = procflags['saveallbranches']
    t = ROOT.TChain(intreename)
    for afile in rootfilestoprocess:
        t.Add(afile)
    nevents= t.GetEntries()
    print("-------------------------------------------------------------------")
    print("Total Number of Entries:")
    print(nevents)
    print("-------------------------------------------------------------------")

    #aproc = ROOT.FourtopAnalyzer(t, outputroot)
    aproc = ROOT.BaseAnalyser(t, outputroot)
    aproc.setParams(config['year'], config['runtype'],config['datatype']) 
    #
    #if your input root file already has good json, various corrections applied with
    #object clean up, you should skip the corrections step
    #
    skipcorrections = False
    if not skipcorrections:
        print("correction step is on play")

        aproc.setupCorrections(config['goodjson'], config['pileupfname'], config['pileuptag']\
            , config['btvfname'], config['btvtype'], config['muon_fname'], config['muontype'], config['jercfname'], config['jerctag'], config['jercunctag'])
    else:
        print("Skipping corrections step")
    #time.sleep(3)
    print("Starting setupanalysis")
    sys.stdout.flush() #to force printout in right order 
    aproc.setupObjects()
    aproc.setupAnalysis()
    aproc.run(saveallbranches, outtreename)

    
    pass

if __name__=='__main__':
    from importlib import import_module
    from argparse import ArgumentParser
    # inputDir and lower directories contain input NanoAOD files
    # outputDir is where the outputs will be created
    parser = ArgumentParser(usage="%prog inputDir outputDir jobconfmod")
    parser.add_argument("indir")
    parser.add_argument("outdir")
    parser.add_argument("jobconfmod")
    args = parser.parse_args()

    indir = args.indir
    outdir = args.outdir
    jobconfmod = args.jobconfmod

    # check whether input directory exists
    if not os.path.exists(indir):
        print ('Path '+indir+' does not exist. Stopping')
        exit(1)

    # load compiled C++ library into ROOT/python
    cppyy.load_reflection_info("libcorrectionlib.so")
    cppyy.load_reflection_info("libMathMore.so")
    cppyy.load_reflection_info("libnanoadrdframe.so")

    # read in configurations from job configuration python module
    mod = import_module(jobconfmod)
    procflags = getattr(mod, 'procflags')
    config = getattr(mod, 'config')

    if not procflags['allinone']:
        print("not allinone")
        n=Nanoaodprocessor(indir, outdir, jobconfmod, procflags, config)
        n.process()
    else:
        print("allinone")
        Nanoaodprocessor_singledir (indir, outdir, procflags, config) # although it says outdir, it should really be a output ROOT file name
