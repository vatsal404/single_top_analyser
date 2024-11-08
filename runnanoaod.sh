#!/bin/bash

# Exit on error
set -e

# Print commands as they are executed
set -x

echo "Starting job execution"
echo "Current working directory: $(pwd)"
ls -la

# Setup CMSSW environment
export VO_CMS_SW_DIR=/cvmfs/cms.cern.ch
source $VO_CMS_SW_DIR/cmsset_default.sh

# Change to working directory if specified by condor
if [ -n "$_CONDOR_JOB_IWD" ]; then
    cd $_CONDOR_JOB_IWD
    echo "Changed to Condor working directory: $(pwd)"
    ls -la
fi

# Setup CMSSW release
echo "Setting up CMSSW_12_3_4"
scramv1 project CMSSW CMSSW_12_3_4
cd CMSSW_12_3_4/src
eval `scramv1 runtime -sh`

# Move the fly directory to the correct location
mv ../../fly .

echo "Contents of fly directory:"
#ls -R fly/

# Make executable runnable
chmod +x fly/nanoaodrdataframe

# Change to fly directory and run
cd fly
./nanoaodrdataframe
# Exit with the status of the last command
exit $?
