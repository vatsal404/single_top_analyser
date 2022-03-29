#!/bin/python
# -*- coding: utf-8 -*-
"""
Created on Mon Sep 14 11:01:46 2018

@author: Suyong Choi (Department of Physics, Korea University suyong@korea.ac.kr)

This script applies nanoaod processing to all the files in the input
 directory and its subdirectory recursively, copying directory structure to the outputdirectory

"""
import sys
import os
import re
import string
import subprocess


from importlib import import_module
from multiprocessing import Process
import cppyy
import ROOT

def batchScript(cmd):

   script = """#!/bin/bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64
export LD_PRELOAD=/usr/lib64/libXrdPosixPreload.so
export XROOTD_VMP=cms-se.sdfarm.kr:1094:/xrootd=/xrd

# Your nanoaodrdframe location
#cd /cms/ldap_home/tjkim/work/run2/LQ/nanoaodrdframe
cd /cms/ldap_home/ljw1015/work/nanoaodrdframe

%s
""" % cmd

   return script

def condorScript(scriptFileName, outputdirectory, withoutext):

   script = """executable = %s/Log/%s
universe  = vanilla

JobBatchName = CMS_$(cluster)
should_transfer_files   = YES
when_to_transfer_output = ON_EXIT
transfer_input_files    = %s/Log/%s
getenv     = True

Accounting_Group        = group_cms

output = %s/Log/skim_%s.out
error = %s/Log/skim_%s.error
log = %s/Log/skim_%s.log

queue
""" % (outputdirectory, scriptFileName, outputdirectory, scriptFileName, outputdirectory, withoutext, outputdirectory, withoutext, outputdirectory, withoutext)

   return script

def function_calling_PostProcessor(outdir, rootfileshere, outtreename, intreeename, year, syst, json, saveallbranches, globaltag):
	for afile in rootfileshere:
		t = ROOT.TChain(intreeename)
		t.Add(afile)
		rootfname = re.split('\/', afile)[-1]
		withoutext = re.split('\/',afile)[-2] + "_" + re.split('\.root', rootfname)[0]
		outfname = outdir + '/' + withoutext + '_analyzed.root'
		if saveallbranches:
			subprocess.call(["./skimonefile.py", "--year=%s"%year, "--syst=%s"%syst, "--json=%s"%json, "--saveallbranches", "--globaltag=%s"%globaltag, afile, outfname, intreeename, outtreename])
		else:
			subprocess.call(["./skimonefile.py", "--year=%s"%year, "--syst=%s"%syst, "--json=%s"%json, "--globaltag=%s"%globaltag, afile, outfname, intreeename, outtreename])
	pass


class Nanoaodprocessor:
	def __init__(self, outdir, indir, outtree, intree, year, syst, json, split, batch, skipold, recursive, flat, saveallbranches, globaltag):
		self.outdir = outdir
		self.indir = indir
		self.outtreename = outtree
		self.intreeename = intree
		self.year = year
		self.syst = syst
		self.json = json
		self.split = split
		self.batch = batch
		self.skipold = skipold
		self.recursive = recursive
		self.flat = flat
		if self.flat:
			self.recursive = False
		self.saveallbranches = saveallbranches
		self.globaltag = globaltag
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
		flist=[]
		if self.flat:
			print("Flat splitted jobs")
			for root, dirs, files in os.walk(inputdirectory):
				for onefile in files:
					fullname = os.path.join(root, onefile)
					flist.append(fullname)
		else:
			print("Recursive")
			flist = os.listdir(inputdirectory) 
		print(flist)
		rootfileshere = []
		subdirs = []
		outsubdirs = []
		# create output directory if it doesn't already exist
		if not os.path.exists(outputdirectory):
			os.makedirs(outputdirectory)
		#if not os.path.exists(outputdirectory):
		#    os.mkdir(outputdirectory)
		# loop through the list
		# pick root files but not those that match  _analyzed.root 
		for fname in flist:
			fullname = os.path.join(inputdirectory, fname)
			if re.match('.*\.root', fname) and not re.match('.*_analyzed\.root', fname) and os.path.isfile(fullname): # if it has .root file extension
				rootfileshere.append(fullname) 
			elif os.path.isdir(fullname):  # if it's a directory name
				subdirs.append(fullname)
				outsubdirs.append(outputdirectory+'/'+fname)
		
		print("files found in directory "+inputdirectory)
		print(rootfileshere)
		
		# run
		if len(rootfileshere)>0:
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
					p = Process(target=function_calling_PostProcessor, args=(outputdirectory, filesforjob, self.outtreename, self.intreeename, self.year, self.syst, self.json, self.saveallbranches, self.globaltag)) # positional arguments go into kwargs
					p.start()
					ap.append(p)
				for proc in ap:
					proc.join()
			else: # no multiprocessing
				#function_calling_PostProcessor(outputdirectory, rootfileshere, self.json, self.isdata)
				aproc = None
				for afile in rootfileshere:
					afile = afile.replace("/xrootd","root://cms-xrdr.private.lo:2094//xrd") # Set xrootd URL (2094 for internal, 1094 for public)
					rootfname = re.split('\/', afile)[-1]
					withoutext = re.split('\.root', rootfname)[0]
					outfname = outputdirectory +'/'+ withoutext + '_analyzed.root'

					if not self.batch :  
						subprocess.call(["./skimonefile.py", "--year=%s"%self.year, "--syst=%s"%self.syst, "--json=%s"%self.json, "--globaltag=%s"%self.globaltag, afile, outfname, self.intreeename, self.outtreename])
					else: #if batch, submit jobs in condor@kisti
						cmd = "./skimonefile.py"+ " --json="+self.json + " --globaltag="+self.globaltag+" "+afile+" "+outfname+" "+self.intreeename+" "+self.outtreename

						logDir = outputdirectory+'/Log'
						if not os.path.exists(logDir):
					   		os.mkdir(logDir)

						scriptFileName = 'sub_'+withoutext+'.sh'
						scriptFile = open(scriptFileName,'w')
						scriptFile.write( batchScript(cmd))
						scriptFile.close()
						os.system('chmod +x %s' % scriptFileName)
						os.system('mv %s %s/Log' % (scriptFileName, outputdirectory) )

						jobFileName = 'job_'+withoutext+'.sub'
						jobFile = open(jobFileName,'w')
						jobFile.write( condorScript(scriptFileName, outputdirectory, withoutext))
						jobFile.close()
						os.system('mv %s %s/Log' % (jobFileName, outputdirectory) )

						submit = 'condor_submit '+outputdirectory+'/Log/'+jobFileName
						os.system(submit)

		# if there are subdirectories recursively call
		if self.recursive:
			for indir, outdir in zip(subdirs, outsubdirs):
				self._processROOTfiles(indir, outdir)
	
def Nanoaodprocessor_singledir(outputroot, indir, outtree, intree, year, syst, json, split, recursive, saveallbranches, globaltag):
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

	for  fname in fullnamelist:            
		if re.match('.*\.root', fname) and os.path.isfile(fname): # if it has .root file extension
			rootfilestoprocess.append(fname)
	print("files to process")
	print(rootfilestoprocess)
	t = ROOT.TChain(intree)
	for afile in rootfilestoprocess:
		t.Add(afile)
	aproc = ROOT.SkimEvents(t, outputroot, year, syst, json, globaltag, split)
	aproc.setupAnalysis()
	aproc.run(saveallbranches, outtree)
	pass

if __name__=='__main__':
	
	from optparse import OptionParser
	# inputDir and lower directories contain input NanoAOD files
	# outputDir is where the outputs will be created
	parser = OptionParser(usage="%prog [options] inputDir outputDir")
	parser.add_option("-Y", "--year",  dest="year", type="string", default="", help="Select 2016, 2017, or 2018 for runs")
	parser.add_option("-S", "--syst",  dest="syst", type="string", default="", help="Systematic sources")
	parser.add_option("-J", "--json",  dest="json", type="string", default="", help="Select events using this JSON file, meaningful only for data")
	parser.add_option("--split", dest="split", type=int, default=1, help="How many jobs to split into")
	parser.add_option("--batch", dest="batch", action="store_true", default=False, help="Submit jobs @KISTI")
	parser.add_option("--skipold", dest="skipold", action="store_true", default=False, help="Skip existing root files")
	parser.add_option("--recursive", dest="recursive", action="store_true", default=True, help="Process files in the subdirectories recursively")
	parser.add_option("-F","--flat", dest="flat", action="store_true", default=False, help="Process files in the subdirectories recursively and store not recursively")
	parser.add_option("--allinone", dest="allinone", action="store_true", default=False, help="Process all files and output a single root file. You must make sure MC and Data are not mixed together.")
	parser.add_option("--saveallbranches", dest="saveallbranches", action="store_true", default=False, help="Save all branches. False by default")
	parser.add_option("--globaltag", dest="globaltag", type="string", default="", help="Global tag to be used in JetMET corrections")
	(options, args) = parser.parse_args()

	if len(args) < 1 :
		parser.print_help()
		sys.exit(1)

	indir = args[0]
	outdir = args[1]

	# load compiled C++ library into ROOT/python
	cppyy.load_reflection_info("libnanoadrdframe.so")

	intree = "Events"
	outtree = "outputTree"

	if not options.allinone:
		n=Nanoaodprocessor(outdir, indir, outtree, intree, options.year, options.syst, options.json, options.split, options.batch, options.skipold, options.recursive, options.flat, options.saveallbranches, options.globaltag)
		n.process()
	else:
		Nanoaodprocessor_singledir(outdir, indir, outtree, intree, options.year, options.syst, options.json, options.split, options.recursive, options.saveallbranches, options.globaltag) # although it says outdir, it should really be a output ROOT file name
