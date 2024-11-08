#!/bin/bash

# Setup CMSSW environment
export VO_CMS_SW_DIR=/cvmfs/cms.cern.ch
source $VO_CMS_SW_DIR/cmsset_default.sh

# Set architecture for CMSSW_12_3_4
export SCRAM_ARCH=el8_amd64_gcc10
CMSSW_VERSION=CMSSW_12_3_4

# Check if CMSSW_BASE is set
if [ -z "$CMSSW_BASE" ]; then
    echo "ERROR: CMSSW_BASE is not set. Setting up CMSSW environment..."
    
    # Check if CMSSW version exists
    if [ ! -d "/cvmfs/cms.cern.ch/$SCRAM_ARCH/cms/cmssw/$CMSSW_VERSION" ]; then
        echo "ERROR: CMSSW version $CMSSW_VERSION not found!"
        echo "Available versions in $SCRAM_ARCH:"
        ls /cvmfs/cms.cern.ch/$SCRAM_ARCH/cms/cmssw/
        exit 1
    fi
    
    # Create and setup CMSSW environment
    scramv1 project CMSSW $CMSSW_VERSION
    cd $CMSSW_VERSION/src
    eval `scramv1 runtime -sh`
fi

echo "Creating CMSSW tarball from $CMSSW_BASE..."
cd $CMSSW_BASE/..
tar --exclude-vcs -zcf $JOB_DIR/cmssw.tar.gz $(basename $CMSSW_BASE)
echo "Created cmssw.tar.gz in $JOB_DIR"

# Verify tarball
if [ ! -f "$JOB_DIR/cmssw.tar.gz" ]; then
    echo "ERROR: Failed to create CMSSW tarball!"
    exit 1
fi
