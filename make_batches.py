#!/usr/bin/env python3 
import os 
import math 
import subprocess 
import shlex 

BATCH_SIZE = 25 
REDIRECTOR = "root://cmsxrootd.fnal.gov" 

def das_files(dataset): 
    # Returns list of file *paths* like /store/data/... 
    # Requires dasgoclient on your submit node environment 
    q = f'file dataset={dataset}' 
    cmd = f'dasgoclient -query "{q}"' 
    out = subprocess.check_output(cmd, shell=True, text=True) 
    files = [line.strip() for line in out.splitlines() if line.strip()] 
    return files 

def sanitize(name): 
    # Make a safe prefix for filenames 
    return name.strip("/").replace("/", "_") 

def ensure_dir(d):
    if d and not os.path.exists(d):
        os.makedirs(d, exist_ok=True)

def main():
    in_list = "sample_list.txt"
    out_list = "sample_list_split.txt"

    if not os.path.isfile(in_list):
        raise SystemExit(f"Missing {in_list}")

    with open(in_list) as f:
        lines = [l.strip() for l in f if l.strip() and not l.strip().startswith("#")]

    out_lines = []
    batch_counter_global = 0

    for line in lines:
        # Expect: <dataset> <outroot> <outlog> <xsec> <genweight_sum>
        parts = line.split()
        if len(parts) < 4:
            print(f"Skip malformed line: {line}")
            continue
        
        # Handle both 4-column (old format) and 5-column (new format with genweight)
        if len(parts) == 4:
            dataset, outroot, outlog, xsec = parts[0], parts[1], parts[2], parts[3]
            genweight_sum = "1"  # Default for data or missing genweight
        else:
            dataset, outroot, outlog, xsec, genweight_sum = parts[0], parts[1], parts[2], parts[3], parts[4]
        
        ds_tag = sanitize(dataset)

        print(f"[INFO] Querying DAS for {dataset} ...")
        file_paths = das_files(dataset)
        if not file_paths:
            print(f"[WARN] No files for {dataset}")
            continue

        # Turn into XRootD URLs
        urls = [f"{REDIRECTOR}//{p}" for p in file_paths]

        nbatches = math.ceil(len(urls) / BATCH_SIZE)
        print(f"[INFO] {len(urls)} files -> {nbatches} batches of {BATCH_SIZE}")

        # Make an output-friendly base names
        outroot_base, outroot_ext = os.path.splitext(outroot)
        outlog_base, outlog_ext = os.path.splitext(outlog)

        for bi in range(nbatches):
            batch_counter_global += 1
            start, end = bi * BATCH_SIZE, (bi + 1) * BATCH_SIZE
            chunk = urls[start:end]

            batch_id = f"{batch_counter_global:04d}"
            batch_file = f"batch_{ds_tag}_{batch_id}.txt"

            with open(batch_file, "w") as bf:
                bf.write("\n".join(chunk) + "\n")

            # Unique outputs per batch
            outroot_i = f"{outroot_base}_b{batch_id}{outroot_ext}"
            outlog_i  = f"{outlog_base}_b{batch_id}{outlog_ext}"

            # Write one line per batch to the new sample list, including genweight_sum
            out_lines.append(f"{batch_file} {outroot_i} {outlog_i} {xsec} {genweight_sum}")

    with open(out_list, "w") as f:
        f.write("\n".join(out_lines) + "\n")

    print(f"\n[DONE] Wrote {len(out_lines)} batch lines to {out_list}")
    print("       Example first lines:")
    for example in out_lines[:3]:
        print("       ", example)

if __name__ == "__main__":
    main()
