#!/bin/bash

# Input arguments
batch_file=$1           # Batch file name (e.g., batch_EGamma0_001.txt)
duplicate_file=$2       # Duplicate events file name
stderr_file=$3          # Log file name

# Configuration variables
eos_output_dir="/eos/uscms/store/user/vsinha/skimmed_data"

echo "=========================================="
echo "Running skim analysis with parameters:"
echo "Batch file: $batch_file"
echo "Duplicate file: $duplicate_file"
echo "Log file: $stderr_file"
echo "=========================================="

# Determine execution environment
if [ -z "${_CONDOR_SCRATCH_DIR}" ] ; then
    start_time=$(date +%s)
    echo "Running Interactively"
    log_file="./${stderr_file}"
    work_dir=$(pwd)
else
    echo "Running in Batch (HTCondor)"
    cd ${_CONDOR_SCRATCH_DIR}
    work_dir=${_CONDOR_SCRATCH_DIR}
    echo "Condor Scratch Directory: ${work_dir}"

    # Setup CMSSW environment
    source /cvmfs/cms.cern.ch/cmsset_default.sh
    export SCRAM_ARCH=el8_amd64_gcc10

    # Use an existing CMSSW release if available
    if [ ! -d "CMSSW_12_3_4" ]; then
        eval `scramv1 project CMSSW CMSSW_12_3_4`
    fi

    cd CMSSW_12_3_4/src
    eval `scramv1 runtime -sh`
    cd ${work_dir}

    echo "CMSSW environment setup done."
    log_file="${work_dir}/${stderr_file}"
fi

echo "Working directory contents:"
ls -alh

# Extract tarball with executable and input files
if [ -f "skim_package.tar.gz" ]; then
    echo "Extracting skim_package.tar.gz..."
    tar -xzf skim_package.tar.gz
else
    echo "Error: skim_package.tar.gz not found!"
    exit 1
fi

# Verify required files exist
if [ ! -f "./skim_data" ]; then
    echo "Error: skim_data executable not found!"
    exit 1
fi

if [ ! -f "$batch_file" ]; then
    echo "Error: Batch file $batch_file not found!"
    exit 1
fi

if [ ! -f "$duplicate_file" ]; then
    echo "Warning: Duplicate file $duplicate_file not found, continuing without it..."
    duplicate_file=""
fi

# Make executable runnable
chmod +x ./skim_data

echo "=========================================="
echo "Starting skim processing..."
echo "=========================================="

# Run the skimming job
if [ -n "$duplicate_file" ]; then
    ./skim_data $batch_file $duplicate_file > $log_file 2>&1
    exit_code=$?
else
    ./skim_data $batch_file > $log_file 2>&1
    exit_code=$?
fi

# Check if processing was successful
if [ $exit_code -ne 0 ]; then
    echo "Error: Skim processing failed with exit code $exit_code"
    cat $log_file
    exit 1
fi

echo "=========================================="
echo "Skim processing completed successfully"
echo "=========================================="

# Display log file
cat $log_file

# Find output file
output_file=$(ls skimmed_*.root 2>/dev/null | head -n 1)

if [ -z "$output_file" ]; then
    echo "Error: No output file (skimmed_*.root) found!"
    exit 1
fi

echo "Output file created: $output_file"
ls -lh $output_file

# Copy output to EOS (only in Condor environment)
if [ -n "${_CONDOR_SCRATCH_DIR}" ]; then
    echo "=========================================="
    echo "Copying output to EOS..."
    echo "=========================================="
    
    # Ensure EOS output directory exists
    eos root://cmseos.fnal.gov mkdir -p $eos_output_dir
    
    # Copy output file to EOS
    xrdcp -f $output_file root://cmseos.fnal.gov/${eos_output_dir}/${output_file}
    xrdcp_exit=$?
    
    if [ $xrdcp_exit -eq 0 ]; then
        echo "Successfully copied $output_file to ${eos_output_dir}"
    else
        echo "Error: Failed to copy output file to EOS (exit code: $xrdcp_exit)"
        exit 1
    fi
    
    # Also copy log file to EOS
    xrdcp -f $log_file root://cmseos.fnal.gov/${eos_output_dir}/${stderr_file}
    if [ $? -eq 0 ]; then
        echo "Successfully copied log file to ${eos_output_dir}"
    fi
fi

echo "=========================================="
echo "Job completed successfully!"
echo "=========================================="

exit 0
