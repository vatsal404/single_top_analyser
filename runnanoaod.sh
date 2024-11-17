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

# Store the initial condor directory
CONDOR_DIR=$(pwd)
echo "Condor directory: $CONDOR_DIR"

# Setup CMSSW release
echo "Setting up CMSSW_12_3_4"
scramv1 project CMSSW CMSSW_12_3_4
cd CMSSW_12_3_4/src
eval `scramv1 runtime -sh`

# Move the fly directory to the correct location
mv ../../fly .
echo "Contents of fly directory:"
ls -la fly/

# Make executable runnable
chmod +x fly/nanoaodrdataframe

# Change to fly directory
cd fly
echo "About to run nanoaodrdataframe from directory: $(pwd)"

# Run the executable
./nanoaodrdataframe

# Immediately after running, search for the file
echo "Searching for root file immediately after creation:"
find / -name "testout_h.root" 2>/dev/null || echo "File not found in filesystem"

echo "Current directory contents:"
ls -la

echo "Parent directory contents:"
ls -la ..

# Print environment variables that might affect file locations
echo "Relevant environment variables:"
env | grep -i path
env | grep -i dir
