#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Modified version of the NanoAOD processor with added XRootD support and extended corrections
Original author: Suyong Choi (Department of Physics, Korea University suyong@korea.ac.kr)
"""
import os
import re
import subprocess
import sys
from multiprocessing import Process
import cppyy
import ROOT

def get_root_file_paths(indir, xrootd_prefix="root://cmsxrootd.fnal.gov/"):
    """
    Function to retrieve ROOT file paths using dasgoclient.
    """
    dataset_query = f"file dataset={indir}"
    command = f"dasgoclient --query='{dataset_query}'"
    try:
        output = subprocess.check_output(command, shell=True, text=True)
    except subprocess.CalledProcessError as e:
        print(f"Error running dasgoclient: {e.output}")
        return []
    
    files = output.strip().split('\n')
    root_files = [xrootd_prefix + f for f in files]
    return root_files

def is_valid_das_path(indir):
    """
    Check if the indir is a valid DAS path by querying dasgoclient.
    """
    command = f"dasgoclient --query='dataset={indir}'"
    try:
        output = subprocess.check_output(command, shell=True, text=True)
        return bool(output.strip())
    except subprocess.CalledProcessError:
        return False

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
        self.nrootfiles = procflags['nrootfiles']
        self.year = config['year']
        self.runtype = config['runtype']
        self.datatype = config['datatype']
        self.skipcorrections = procflags.get('skipcorrections', False)  # Added skipcorrections flag
        print("year=", self.year)

        # Check if input is a DAS path or local directory
        self.is_das_path = is_valid_das_path(self.indir)
        if not self.is_das_path and not os.path.exists(self.indir):
            print(f'Path {indir} is neither a valid DAS path nor an existing local directory')
            exit(1)

    def process(self):
        self._processROOTfiles(self.indir, self.outdir)
        pass

    def _processROOTfiles(self, inputdirectory, outputdirectory):
        if not os.path.exists(outputdirectory):
            os.makedirs(outputdirectory)

        rootfileshere = []
        if self.is_das_path:
            # Handle remote files using XRootD
            rootfileshere = get_root_file_paths(inputdirectory)
            if self.nrootfiles > 0:
                rootfileshere = rootfileshere[:self.nrootfiles]
        else:
            # Original local file handling
            flist = os.listdir(inputdirectory)
            counter = 0
            for fname in flist:
                fullname = os.path.join(inputdirectory, fname)
                if re.match('.*\.root', fname) and os.path.isfile(fullname):
                    counter += 1
                    if counter <= self.nrootfiles and self.nrootfiles != 0:
                        rootfileshere.append(fullname)
                    elif self.nrootfiles == 0:
                        rootfileshere.append(fullname)

        print(f"Files found in {'DAS dataset' if self.is_das_path else 'directory'} {inputdirectory}")
        print(rootfileshere)

        if len(rootfileshere) > 0:
            if self.skipold:
                oflist = os.listdir(outputdirectory)
                filteredoflist = []
                for fname in oflist:
                    fullname = os.path.join(outputdirectory, fname)
                    if re.match('.*\.root', fname) and os.path.isfile(fullname):
                        withoutext = re.split("\.root", fname)[0]
                        wihoutskimtext = re.split("\_analyzed", withoutext)[0]
                        filteredoflist.append(wihoutskimtext)

                filterediflist = []
                for ifname in rootfileshere:
                    rootfname = re.split('\/', ifname)[-1]
                    withoutext = re.split('\.root', rootfname)[0]
                    if withoutext not in filteredoflist:
                        print(f'{withoutext} not yet in output dir')
                        filterediflist.append(ifname)
                    else:
                        print(f'{withoutext} in output dir')

                rootfileshere = filterediflist

            if self.split > 1:
                njobs = min(self.split, len(rootfileshere))
                nfileperjob = len(rootfileshere) / njobs

                ap = []
                for i in range(njobs):
                    start_idx = int(i * nfileperjob)
                    end_idx = int((i + 1) * nfileperjob) if i < njobs - 1 else None
                    filesforjob = rootfileshere[start_idx:end_idx]
                    p = Process(target=function_calling_PostProcessor, 
                              args=(outputdirectory, filesforjob, self.jobconfmod))
                    p.start()
                    ap.append(p)
                for proc in ap:
                    proc.join()
            else:
                for afile in rootfileshere:
                    rootfname = re.split('\/', afile)[-1]
                    withoutext = re.split('\.root', rootfname)[0]
                    outfname = outputdirectory + '/' + withoutext + '_analyzed.root'
                    subprocess.run(["./processonefile.py", afile, outfname, self.jobconfmod])

def Nanoaodprocessor_singledir(indir, outputroot, procflags, config):
    """
    Runs nanoaod analyzer over ROOT files in indir and outputs into a single ROOT file.
    Now supports both local and remote files via XRootD.
    """
    if not re.match('.*\.root', outputroot):
        print("Output file should be a root file! Quitting")
        exit(-1)

    rootfilestoprocess = []
    is_das_path = is_valid_das_path(indir)

    if is_das_path:
        print(f"COLLECT root files from DAS dataset:\n{indir}\n")
        rootfilestoprocess = get_root_file_paths(indir)
    else:
        print(f"COLLECT root files in:\n{indir}\n")
        fullnamelist = []
        if not procflags['recursive']:
            flist = os.listdir(indir)
            for fname in flist:
                fullname = os.path.join(indir, fname)
                fullnamelist.append(fullname)
        else:
            for root, dirs, flist in os.walk(indir):
                for fname in flist:
                    fullname = os.path.join(root, fname)
                    fullnamelist.append(fullname)

        counter = 0
        for fname in fullnamelist:
            if re.match('.*\.root', fname) and os.path.isfile(fname):
                counter += 1
                if counter <= procflags['nrootfiles'] and procflags['nrootfiles'] != 0:
                    rootfilestoprocess.append(fname)
                elif procflags['nrootfiles'] == 0:
                    rootfilestoprocess.append(fname)

    if procflags['nrootfiles'] > 0:
        rootfilestoprocess = rootfilestoprocess[:procflags['nrootfiles']]

    print("FILES to PROCESS")
    print(rootfilestoprocess)

    intreename = config['intreename']
    outtreename = config['outtreename']
    saveallbranches = procflags['saveallbranches']

    t = ROOT.TChain(intreename)
    for afile in rootfilestoprocess:
        t.Add(afile)
    nevents = t.GetEntries()
    print("-------------------------------------------------------------------")
    print("Total Number of Entries:", nevents)
    print("-------------------------------------------------------------------")

#    aproc = ROOT.BaseAnalyser(t, outputroot)
 #   aproc.setParams(config['year'], config['runtype'], config['datatype'])
    aproc = ROOT.BaseAnalyser(t, outputroot)

    try:
        aproc.setParams(config['year'], config['runtype'], config['datatype'])
    except Exception as e:
        print(f"Error calling setParams(): {e}")
        print(f"config['year']: {config['year']}")
        print(f"config['runtype']: {config['runtype']}")
        print(f"config['datatype']: {config['datatype']}")
        raise

    # Handle corrections with expanded configuration
    skipcorrections = procflags.get('skipcorrections', False)
    if not skipcorrections:
        print("Applying corrections...")
        aproc.setupCorrections(
            config['goodjson'],
            config['pileupfname'],
            config['pileuptag'],
            config['btvfname'],
            config['btvtype'],
            config['muon_roch_fname'],
            config['muon_fname'],
            config['muonHLTtype'],
            config['muonRECOtype'],
            config['muonIDtype'],
            config['muonISOtype'],
            config['electron_fname'],
            config['electron_reco_type'],
            config['electron_id_type'],
            config['jercfname'],
            config['jerctag'],
            config['jercunctag']
        )
    else:
        print("Skipping corrections step")

    print("Starting setupanalysis")
    sys.stdout.flush()
    aproc.setupObjects()
    aproc.setupAnalysis()
    aproc.run(saveallbranches, outtreename)

if __name__ == '__main__':
    from importlib import import_module
    from argparse import ArgumentParser

    parser = ArgumentParser(usage="%(prog)s inputDir outputDir jobconfmod")
    parser.add_argument("indir")
    parser.add_argument("outdir")
    parser.add_argument("jobconfmod")
    args = parser.parse_args()

    # Load compiled C++ libraries
    cppyy.load_reflection_info("libcorrectionlib.so")
    cppyy.load_reflection_info("libMathMore.so")
    cppyy.load_reflection_info("libnanoadrdframe.so")

    # Read configurations
    mod = import_module(args.jobconfmod)
    procflags = getattr(mod, 'procflags')
    config = getattr(mod, 'config')

    if not procflags['allinone']:
        print("not allinone")
        n = Nanoaodprocessor(args.indir, args.outdir, args.jobconfmod, procflags, config)
        n.process()
    else:
        print("allinone")
        Nanoaodprocessor_singledir(args.indir, args.outdir, procflags, config)
