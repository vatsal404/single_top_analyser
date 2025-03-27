#!/bin/bash

# Input arguments
input_dir=$1
output_file=$2  # Now this will be the output file name (e.g., output.root)
stderr_file=$3  # Define the log file name

# Configuration variables
jobconfmod="jobconfiganalysis_new"
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
output_dir=$(dirname "Analyzed")
if [ ! -d "${output_dir}" ]; then
    echo "Output directory ${output_dir} does not exist, creating it..."
    mkdir -p "${output_dir}" || { echo "Failed to create directory ${output_dir}"; exit 1; }
else
    echo "Output directory ${output_dir} exists, proceeding with job..."
fi

local_output_path="${output_dir}/${output_file}"
echo "Final output file path: ${local_output_path}"


# Run the analysis job
./processnanoaod.py $input_dir $local_output_path $jobconfmod > $log_file 2>&1 || { echo "Error: Processing failed"; exit 1; }

cat $log_file

# Check if the output file exists before copying
if [ -f "${output_file}" ]; then
    echo "Copying file ${output_file} to EOS..."
    xrdcp -f $output_file ${eos_output_dir}$(basename $output_file)

    # Verify transfer
    xrdfs root://cmseos.fnal.gov/ stat ${eos_output_dir}$(basename $output_file)
    if [ $? -eq 0 ]; then
        echo "File successfully copied to EOS: ${eos_output_dir}$(basename $output_file)"
    else
        echo "Error: Failed to copy file to EOS"
        exit 1
    fi
else
    echo "Error: Local output file not found: $output_file"
    exit 1
fi

# Cleanup if running in Condor
if [ -n "${_CONDOR_SCRATCH_DIR}" ]; then
    echo "Cleaning up scratch directory..."
    rm -rf ${_CONDOR_SCRATCH_DIR}/*
    echo "Job Completed and scratch directory cleaned."
else
    echo "Running locally, no cleanup needed."
fi

echo "Job Completed."

