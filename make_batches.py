#!/usr/bin/env python3
import os
import math
import subprocess

# Separate batch sizes for MC and Data
MC_BATCH_SIZE =5 
DATA_BATCH_SIZE = 395

REDIRECTOR = "root://cmsxrootd.fnal.gov"
EOS_REDIRECTOR = "root://cmseos.fnal.gov"
EOS_DATA_PATH = "/eos/uscms/store/user/vsinha/skimmed_data/"

def das_files(dataset):
    """Query DAS for files in a dataset"""
    q = f'file dataset={dataset}'
    cmd = f'dasgoclient -query "{q}"'
    out = subprocess.check_output(cmd, shell=True, text=True)
    files = [line.strip() for line in out.splitlines() if line.strip()]
    return files

def eos_files(eos_path):
    """List files in EOS directory"""
    try:
        cmd = f'eos root://cmseos.fnal.gov ls {eos_path}'
        out = subprocess.check_output(cmd, shell=True, text=True)
        files = [line.strip() for line in out.splitlines() if line.strip() and line.strip().endswith('.root')]
        return files
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] Failed to list EOS directory: {e}")
        return []

def sanitize(name):
    """Sanitize dataset name for use in filenames"""
    return name.strip("/").replace("/", "_")

def ensure_dir(d):
    """Create directory if it doesn't exist"""
    if d and not os.path.exists(d):
        os.makedirs(d, exist_ok=True)

def main():
    in_list = "sample_list_2022.txt"
    out_list = "sample_list_split.txt"
    batch_dir = "batches"

    # Create batches directory
    ensure_dir(batch_dir)

    if not os.path.isfile(in_list):
        raise SystemExit(f"Missing {in_list}")

    with open(in_list) as f:
        lines = [l.strip() for l in f if l.strip() and not l.strip().startswith("#")]

    out_lines = []
    batch_counter_global = 0

    # First, handle DAS datasets from input file (ALL MONTE CARLO)
    print("\n" + "="*60)
    print("PROCESSING MONTE CARLO DATASETS FROM DAS")
    print("="*60)
    
    for line in lines:
        parts = line.split()

        if len(parts) < 4:
            print(f"Skip malformed line: {line}")
            continue

        # Accept both 4-column (data) and 5-column (MC) lines
        if len(parts) == 4:
            dataset, outroot, outlog, xsec = parts[0], parts[1], parts[2], parts[3]
            genweight_sum = "1"
        else:
            dataset, outroot, outlog, xsec, genweight_sum = parts

        ds_tag = sanitize(dataset)
        
        print(f"\n[INFO] Querying DAS for {dataset} ...")
        print(f"       Batch size: {MC_BATCH_SIZE}")
        
        file_paths = das_files(dataset)
        if not file_paths:
            print(f"[WARN] No files for {dataset}")
            continue

        urls = [f"{REDIRECTOR}//{p}" for p in file_paths]

        nbatches = math.ceil(len(urls) / MC_BATCH_SIZE)
        print(f"[INFO] {len(urls)} files -> {nbatches} batches of {MC_BATCH_SIZE}")

        outroot_base, outroot_ext = os.path.splitext(outroot)
        outlog_base, outlog_ext = os.path.splitext(outlog)

        for bi in range(nbatches):
            batch_counter_global += 1
            batch_id = f"{batch_counter_global:04d}"

            start, end = bi * MC_BATCH_SIZE, (bi + 1) * MC_BATCH_SIZE
            chunk = urls[start:end]

            # Create batch file INSIDE batches/
            batch_file_name = f"batch_{ds_tag}_{batch_id}.txt"
            batch_file_path = os.path.join(batch_dir, batch_file_name)

            with open(batch_file_path, "w") as bf:
                bf.write("\n".join(chunk) + "\n")

            outroot_i = f"{outroot_base}_b{batch_id}{outroot_ext}"
            outlog_i  = f"{outlog_base}_b{batch_id}{outlog_ext}"

            # Write relative path in sample_list_split
            out_lines.append(f"{batch_file_path} {outroot_i} {outlog_i} {xsec} {genweight_sum}")

    # Now handle EOS skimmed data files
    print("\n" + "="*60)
    print("PROCESSING EOS SKIMMED DATA FILES")
    print("="*60)
    
    print(f"\n[INFO] Listing files in {EOS_DATA_PATH} ...")
    eos_file_list = eos_files(EOS_DATA_PATH)
    
    if eos_file_list:
        print(f"[INFO] Found {len(eos_file_list)} files in EOS")
        
        # Create full URLs for EOS files
        eos_urls = [f"{EOS_REDIRECTOR}//{EOS_DATA_PATH}{fname}" for fname in eos_file_list]
        
        # Create batches for EOS data
        nbatches_eos = math.ceil(len(eos_urls) / DATA_BATCH_SIZE)
        print(f"[INFO] Creating {nbatches_eos} batches of {DATA_BATCH_SIZE} for EOS data")
        
        for bi in range(nbatches_eos):
            batch_counter_global += 1
            batch_id = f"{batch_counter_global:04d}"
            
            start, end = bi * DATA_BATCH_SIZE, (bi + 1) * DATA_BATCH_SIZE
            chunk = eos_urls[start:end]
            
            # Create batch file for EOS data
            batch_file_name = f"batch_EOS_SkimmedData_{batch_id}.txt"
            batch_file_path = os.path.join(batch_dir, batch_file_name)
            
            with open(batch_file_path, "w") as bf:
                bf.write("\n".join(chunk) + "\n")
            
            # Output naming for EOS data batches
            outroot_i = f"output_EOS_SkimmedData_b{batch_id}.root"
            outlog_i = f"log_EOS_SkimmedData_b{batch_id}.out"
            xsec = "1.0"  # Data has cross-section = 1
            genweight_sum = "1"  # Data has genweight_sum = 1
            
            out_lines.append(f"{batch_file_path} {outroot_i} {outlog_i} {xsec} {genweight_sum}")
    else:
        print(f"[WARN] No files found in EOS path: {EOS_DATA_PATH}")

    # Write the master split file list
    with open(out_list, "w") as f:
        f.write("\n".join(out_lines) + "\n")

    print("\n" + "="*60)
    print(f"[DONE] Wrote {len(out_lines)} batch lines to {out_list}")
    print("="*60)
    print("\nExample first lines:")
    for example in out_lines[:3]:
        print("   ", example)
    
    print("\nBatch size configuration:")
    print(f"   Monte Carlo: {MC_BATCH_SIZE} files per batch")
    print(f"   Data: {DATA_BATCH_SIZE} files per batch")

if __name__ == "__main__":
    main()
