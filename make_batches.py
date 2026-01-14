#!/usr/bin/env python3
import os
import math
import subprocess

# Batch size for MC
MC_BATCH_SIZE = 5

REDIRECTOR = "root://cmsxrootd.fnal.gov"

def das_files(dataset):
    """Query DAS for files in a dataset"""
    q = f'file dataset={dataset}'
    cmd = f'dasgoclient -query "{q}"'
    out = subprocess.check_output(cmd, shell=True, text=True)
    files = [line.strip() for line in out.splitlines() if line.strip()]
    return files

def sanitize(name):
    """Sanitize dataset name for use in filenames"""
    return name.strip("/").replace("/", "_")

def ensure_dir(d):
    """Create directory if it doesn't exist"""
    if d and not os.path.exists(d):
        os.makedirs(d, exist_ok=True)

def main():
    in_list = "sample_list_hlt_2022.txt"
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

    # Process DAS datasets from input file
    print("\n" + "="*60)
    print("PROCESSING DATASETS FROM SAMPLE LIST")
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

    # Write the master split file list
    with open(out_list, "w") as f:
        f.write("\n".join(out_lines) + "\n")

    print("\n" + "="*60)
    print(f"[DONE] Wrote {len(out_lines)} batch lines to {out_list}")
    print("="*60)
    print("\nExample first lines:")
    for example in out_lines[:3]:
        print("   ", example)
    
    print(f"\nBatch size: {MC_BATCH_SIZE} files per batch")

if __name__ == "__main__":
    main()
