#!/bin/bash

# Input arguments
batch_file=$1           # Batch file name with path (e.g., batches/batch_EGamma0_001.txt)
stderr_file="$(basename ${batch_file}).log"          # Log file name

# Configuration variables
eos_output_dir="/eos/uscms/store/user/vsinha/skimmed_data"

echo "=========================================="
echo "Running skim analysis with parameters:"
echo "Batch file: $batch_file"
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
    echo "Extracted contents:"
    ls -alh
else
    echo "Error: skim_package.tar.gz not found!"
    exit 1
fi

# Verify required files exist
if [ ! -f "./skim_data" ]; then
    echo "Error: skim_data executable not found!"
    exit 1
fi

# Extract dataset name from batch file to find the correct duplicate file
# Example: batches/batch_EGamma0_Run2023C-24Jan2024_v4-v1_NANOAOD_001.txt -> EGamma0
# or: batches/batch_MuonEG_Run2023C-22Sep2023_v4-v1_NANOAOD_0199.txt -> MuonEG
batch_basename=$(basename "$batch_file")
dataset_name=$(echo "$batch_basename" | sed 's/batch_//' | cut -d'_' -f1)
duplicate_file="duplicates_${dataset_name}.bin"

echo "=========================================="
echo "Dataset detection:"
echo "Batch file basename: $batch_basename"
echo "Detected dataset: $dataset_name"
echo "Looking for duplicate file: $duplicate_file"
echo "=========================================="

if [ -f "$duplicate_file" ]; then
    echo "Found duplicate file: $duplicate_file"
    ls -lh "$duplicate_file"
else
    echo "Warning: Duplicate file $duplicate_file not found"
    echo "Available duplicate files:"
    ls -lh duplicates_*.bin 2>/dev/null || echo "No duplicate files found"
    echo "Continuing without duplicate removal (job will still run)..."
fi

# Make executable runnable
chmod +x ./skim_data

echo "=========================================="
echo "Starting skim processing..."
echo "=========================================="

# Run the skimming job (the modified code auto-detects duplicate file)
./skim_data "$batch_file" > "$log_file" 2>&1
exit_code=$?

# Check if processing was successful
if [ $exit_code -ne 0 ]; then
    echo "Error: skim_data failed with exit code $exit_code"
    echo "Last 50 lines of log:"
    tail -n 50 "$log_file"
    exit $exit_code
fi

echo "=========================================="
echo "Skim processing completed successfully"
echo "=========================================="

# Find the output file (should be skimmed_*.root)
output_file=$(ls skimmed_*.root 2>/dev/null | head -n1)

if [ -z "$output_file" ]; then
    echo "Error: No output file found!"
    echo "Contents of working directory:"
    ls -lh
    exit 1
fi

echo "Output file: $output_file"
ls -lh "$output_file"

# Copy output to EOS if running in batch mode
if [ -n "${_CONDOR_SCRATCH_DIR}" ]; then
    echo "Copying output to EOS: ${eos_output_dir}/${output_file}"
    xrdcp -f "$output_file" "root://cmseos.fnal.gov/${eos_output_dir}/${output_file}"
    copy_status=$?
    
    if [ $copy_status -ne 0 ]; then
        echo "Error: Failed to copy output to EOS (exit code: $copy_status)"
        exit $copy_status
    fi
    
    echo "Successfully copied to EOS"
    
    # Also copy the log file
    echo "Copying log file to EOS..."
    xrdcp -f "$log_file" "root://cmseos.fnal.gov/${eos_output_dir}/logs/${stderr_file}"
fi

echo "=========================================="
echo "Job completed successfully!"
echo "=========================================="
exit 0
