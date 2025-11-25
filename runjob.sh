#!/bin/bash

# Input arguments
input_dir=$1
output_file=$2  # Now this will be the output file name (e.g., output.root)
stderr_file=$3  # Define the log file name
crossection=$4
sumgenWeight=$5
# Configuration variables
jobconfmod="jobconfiganalysis_2023"
eos_output_dir="/eos/uscms/store/user/vsinha/results"

echo "Running analysis with the following parameters:"
echo "Input directory: $input_dir"
echo "Output file: $output_file"

# Determine execution environment
if [ -z "${_CONDOR_SCRATCH_DIR}" ] ; then
    start_time=$(date +%s)
    echo "Running Interactively"
    log_file="./${stderr_file}"  # Local execution log file
else
    echo "Running in Batch (HTCondor)"
    cd ${_CONDOR_SCRATCH_DIR}
    echo "Condor Scratch Directory: ${_CONDOR_SCRATCH_DIR}"

    source /cvmfs/cms.cern.ch/cmsset_default.sh
    export SCRAM_ARCH=el8_amd64_gcc10

    # Use an existing CMSSW release if available
    if [ ! -d "CMSSW_12_3_4" ]; then
        eval `scramv1 project CMSSW CMSSW_12_3_4`
    fi

    cd CMSSW_12_3_4/src
    cmsenv
    eval `scramv1 runtime -sh`
    cd - ;

    echo "CMSSW environment setup done."
    log_file="${_CONDOR_SCRATCH_DIR}/${stderr_file}"
fi

ls -alh

# Ensure the output file directory exists

tar -xzf package.tar.gz
echo "input directory :${input_dir}"

echo "output file :${output_file}"
echo "log file :${stderr_file}"
echo "crossection :${crossection}"
echo "sum of genweight :${sumgenWeight}"
echo "config :${jobconfmod}"
# Run the analysis job
./processnanoaod.py $input_dir $output_file $jobconfmod $crossection $sumgenWeight> $log_file 2>&1 || { echo "Error: Processing failed"; exit 1; }

cat $log_file


echo "Job Completed."

