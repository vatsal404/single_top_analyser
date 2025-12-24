#!/bin/bash

# Input arguments
input_dir=$1
output_file=$2          # Expected output file name
stderr_file=$3          # Log file name
crossection=$4
sumgenWeight=$5

# Configuration variables
jobconfmod="jobconfiganalysis_2022EE"
eos_output_dir="/eos/uscms/store/user/vsinha/Hlt_scalefactor_22EE"

echo "Running analysis with the following parameters:"
echo "Input directory: $input_dir"
echo "Output file: $output_file"

# Determine execution environment
if [ -z "${_CONDOR_SCRATCH_DIR}" ] ; then
    start_time=$(date +%s)
    echo "Running Interactively"
    log_file="./${stderr_file}"
else
    echo "Running in Batch (HTCondor)"
    cd ${_CONDOR_SCRATCH_DIR}
    echo "Condor Scratch Directory: ${_CONDOR_SCRATCH_DIR}"

    source /cvmfs/cms.cern.ch/cmsset_default.sh
    export SCRAM_ARCH=el8_amd64_gcc10

    # Create CMSSW release if missing
    if [ ! -d "CMSSW_12_3_4" ]; then
        eval `scramv1 project CMSSW CMSSW_12_3_4`
    fi

    cd CMSSW_12_3_4/src
    cmsenv
    eval `scramv1 runtime -sh`
    cd -

    echo "CMSSW environment setup done."
    log_file="${_CONDOR_SCRATCH_DIR}/${stderr_file}"
fi

ls -alh

# Extract tarball
if [ -f "package.tar.gz" ]; then
    echo "Extracting package.tar.gz..."
    tar -xzf package.tar.gz
else
    echo "Error: package.tar.gz not found!"
    exit 1
fi

echo "Input directory: ${input_dir}"
echo "Output file: ${output_file}"
echo "Log file: ${stderr_file}"
echo "Cross section: ${crossection}"
echo "sum(genWeight): ${sumgenWeight}"
echo "Config: ${jobconfmod}"

# -------------------------------
# Run the actual processing
# -------------------------------
echo "Starting analysis..."
./processnanoaod.py "$input_dir" "$output_file" "$jobconfmod" "$crossection" "$sumgenWeight" \
    > "$log_file" 2>&1
exit_code=$?

# Check status
if [ $exit_code -ne 0 ]; then
    echo "Error: processnanoaod.py failed with exit code $exit_code"
    echo "Last 50 lines of log:"
    tail -n 50 "$log_file"
    exit $exit_code
fi

echo "=========================================="
echo "Processing completed successfully"
echo "=========================================="

# Look for output files - processnanoaod.py creates files with suffixes (_0.root, _1.root, etc.)
echo "Looking for output files..."
base_name="${output_file%.root}"
output_pattern="${base_name}*.root"

echo "Searching for files matching: $output_pattern"
ls -lh ${output_pattern} 2>/dev/null

# Check if any output files were created
if ! ls ${output_pattern} 1> /dev/null 2>&1; then
    echo "Error: No output files matching '${output_pattern}' found!"
    echo "Directory contents:"
    ls -lh
    exit 1
fi

echo "Found output files:"
ls -lh ${output_pattern}

# -------------------------------
# Copy output to EOS (Batch mode)
# -------------------------------
if [ -n "${_CONDOR_SCRATCH_DIR}" ]; then
    echo "=========================================="
    echo "Copying output files to EOS"
    echo "=========================================="
    
    # Create logs directory on EOS
    echo "Creating logs directory on EOS..."
    xrdfs root://cmseos.fnal.gov/ mkdir -p ${eos_output_dir}/logs 2>&1
    
    # Copy each output file to EOS
    for file in ${output_pattern}; do
        if [ -f "$file" ]; then
            echo "----------------------------------------"
            echo "Copying: $file"
            echo "To: ${eos_output_dir}/${file}"
            
            xrdcp -f "$file" "root://cmseos.fnal.gov/${eos_output_dir}/${file}"
            copy_status=$?
            
            if [ $copy_status -ne 0 ]; then
                echo "Error: Failed to copy $file to EOS (exit code: $copy_status)"
                exit $copy_status
            fi
            
            echo "Successfully copied $file to EOS"
        fi
    done
    
    # Copy log file
    echo "----------------------------------------"
    echo "Copying log file to EOS..."
    xrdcp -f "$log_file" "root://cmseos.fnal.gov/${eos_output_dir}/logs/${stderr_file}"
    log_status=$?
    
    if [ $log_status -ne 0 ]; then
        echo "Warning: Failed to copy log file (exit code: $log_status)"
    else
        echo "Successfully copied log file to EOS"
    fi
    
    echo "=========================================="
    echo "All files transferred to EOS successfully!"
    echo "=========================================="
else
    echo "=========================================="
    echo "Running interactively - skipping EOS transfer"
    echo "Output files are in: $(pwd)"
    echo "=========================================="
fi

echo "Job Completed."
exit 0
