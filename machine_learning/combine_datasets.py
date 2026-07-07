#!/usr/bin/env python3
"""
Combines ROOT files across datasets (Result_2022, Result_2022EE, Result_2023, Result_2023BPix, ...).
For each file in each systemetics subdirectory:
  1. Copies outputTree and adds a new 'datasetId' branch (0/1/2/3/...)
  2. hadds the tagged copies into combined/<subdir>/<filename>.root

Usage:
  python3 combine_systematics.py /path/to/working/dir

To add a new dataset (e.g. Result_2024) in the future, just append it to DATASETS below.
The datasetId will be assigned automatically based on order (0-indexed).
"""

import os
import sys
import glob
import subprocess
import tempfile
import shutil
import argparse

try:
    import ROOT
    ROOT.gROOT.SetBatch(True)
except ImportError:
    print("ERROR: PyROOT not available. Please run inside a CMSSW environment.")
    sys.exit(1)

# ── Add new datasets here as needed ─────────────────────────────────────────
#   datasetId is assigned by position: 0=Result_2022, 1=Result_2022EE, etc.
#   To add Result_2024, simply append "Result_2024" to this list.
DATASETS = [
    "Result_2022",       # datasetId = 0
    "Result_2022EE",     # datasetId = 1
    "Result_2023",       # datasetId = 2
    "Result_2023BPix",   # datasetId = 3
    # "Result_2024",     # datasetId = 4  ← uncomment when ready
]
# ─────────────────────────────────────────────────────────────────────────────

SYST_SUBDIR   = "systemetics"
TREE_NAME     = "outputTree"

def parse_args():
    parser = argparse.ArgumentParser(
        description="Combine ROOT systematics files across datasets with a datasetId branch."
    )
    parser.add_argument(
        "base_dir",
        help="Path to the directory containing Result_2022, Result_2022EE, etc."
    )
    parser.add_argument(
        "--output", "-o",
        default=None,
        help="Output directory for combined files (default: <base_dir>/combined)"
    )
    return parser.parse_args()

args      = parse_args()
BASE_DIR  = os.path.abspath(args.base_dir)
COMBINED_DIR = os.path.abspath(args.output) if args.output else os.path.join(BASE_DIR, "combined")
DATASET_IDS  = {d: i for i, d in enumerate(DATASETS)}
TEMP_DIR     = tempfile.mkdtemp(prefix="combine_tmp_")

print(f"Base directory : {BASE_DIR}")
print(f"Output directory: {COMBINED_DIR}")
print(f"Datasets ({len(DATASETS)}):")
for ds, did in DATASET_IDS.items():
    path_exists = "✓" if os.path.isdir(os.path.join(BASE_DIR, ds)) else "✗ NOT FOUND"
    print(f"  [{did}] {ds}  {path_exists}")
print()


def add_dataset_id_branch(input_path, output_path, dataset_id):
    """Copy outputTree from input_path to output_path, adding datasetId branch."""
    fin = ROOT.TFile.Open(input_path, "READ")
    if not fin or fin.IsZombie():
        print(f"  [WARN] Cannot open {input_path} — skipping")
        return False

    tree_in = fin.Get(TREE_NAME)
    if not tree_in:
        print(f"  [WARN] Tree '{TREE_NAME}' not found in {input_path} — skipping")
        fin.Close()
        return False

    fout = ROOT.TFile.Open(output_path, "RECREATE")
    tree_out = tree_in.CloneTree(0)   # clone structure, no entries yet

    # Add new branch
    import array
    did = array.array('i', [dataset_id])
    branch = tree_out.Branch("datasetId", did, "datasetId/I")

    n = tree_in.GetEntries()
    for i in range(n):
        tree_in.GetEntry(i)
        did[0] = dataset_id
        tree_out.Fill()

    fout.Write("", ROOT.TObject.kOverwrite)
    fout.Close()
    fin.Close()
    return True


def is_eos_path(path):
    """Check if a path is on EOS (requires xrdcp instead of direct write)."""
    return "/eos/" in path or path.startswith("root://")


def hadd_and_copy(final_out, input_paths):
    """
    Run hadd. If output is on EOS, hadd into a local temp file first,
    then xrdcp it to the final EOS destination.
    """
    if is_eos_path(final_out):
        local_tmp = os.path.join(TEMP_DIR, "hadd_out_" + os.path.basename(final_out))
        cmd = ["hadd", "-f", local_tmp] + input_paths
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"  [ERROR] hadd failed:\n{result.stderr}")
            return False
        # xrdcp to EOS
        xrd_cmd = ["xrdcp", "-f", local_tmp, final_out]
        xrd = subprocess.run(xrd_cmd, capture_output=True, text=True)
        os.remove(local_tmp)
        if xrd.returncode != 0:
            print(f"  [ERROR] xrdcp failed:\n{xrd.stderr}")
            return False
    else:
        cmd = ["hadd", "-f", final_out] + input_paths
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"  [ERROR] hadd failed:\n{result.stderr}")
            return False
    return True


# Build a set of year/era suffixes from dataset names for smart warning suppression
# e.g. "Result_2022" → "2022", used to detect year-specific filenames like _2022.root
DATASET_YEAR_TAGS = set()
for ds in DATASETS:
    # Extract the part after "Result_" (e.g. "2022", "2022EE", "2023BPix")
    tag = ds.replace("Result_", "")
    DATASET_YEAR_TAGS.add(tag)


def is_year_specific(fname, present_in_ds):
    """
    Returns True if fname contains a year/era tag that matches exactly one
    dataset — i.e. it is expected to only exist in that one dataset.
    E.g. DYjetsM50_JEC_..._2022EE.root → only expected in Result_2022EE.
    """
    # Check which tags appear in the filename
    matched_tags = [tag for tag in DATASET_YEAR_TAGS if fname.endswith(f"_{tag}.root")]
    if len(matched_tags) == 1:
        expected_ds = f"Result_{matched_tags[0]}"
        return present_in_ds == expected_ds
    return False


def main():
    # Collect all subdirectories across ALL datasets (not just first one)
    all_subdirs = set()
    for ds in DATASETS:
        syst_path = os.path.join(BASE_DIR, ds, SYST_SUBDIR)
        if not os.path.isdir(syst_path):
            print(f"[WARN] Systematics path not found, skipping: {syst_path}")
            continue
        for d in os.listdir(syst_path):
            if os.path.isdir(os.path.join(syst_path, d)):
                all_subdirs.add(d)

    if not all_subdirs:
        print("ERROR: No subdirectories found in any dataset.")
        sys.exit(1)

    subdirs = sorted(all_subdirs)
    print(f"Found {len(subdirs)} subdirectories across all datasets\n")

    # Create output dir — skip os.makedirs for EOS paths (xrdcp creates dirs)
    if not is_eos_path(COMBINED_DIR):
        os.makedirs(COMBINED_DIR, exist_ok=True)

    total_files = 0
    skipped     = 0

    for subdir in subdirs:
        out_subdir = os.path.join(COMBINED_DIR, subdir)
        if not is_eos_path(out_subdir):
            os.makedirs(out_subdir, exist_ok=True)

        # Collect all unique filenames across all datasets for this subdir
        # Track which dataset each file came from
        file_to_datasets = {}   # fname → list of datasets that have it
        for ds in DATASETS:
            pattern = os.path.join(BASE_DIR, ds, SYST_SUBDIR, subdir, "*.root")
            for f in glob.glob(pattern):
                fname = os.path.basename(f)
                file_to_datasets.setdefault(fname, []).append(ds)

        if not file_to_datasets:
            print(f"[{subdir}] No ROOT files found — skipping subdir")
            continue

        print(f"[{subdir}] Processing {len(file_to_datasets)} file(s)...")

        for fname in sorted(file_to_datasets):
            present_in = file_to_datasets[fname]
            tagged_paths = []

            for ds in DATASETS:
                src = os.path.join(BASE_DIR, ds, SYST_SUBDIR, subdir, fname)
                if not os.path.isfile(src):
                    # Suppress warning if file is year-specific and legitimately
                    # only belongs to another dataset
                    if not any(is_year_specific(fname, p) for p in present_in):
                        print(f"  [WARN] Missing in {ds}: {fname}")
                    continue

                tagged_out = os.path.join(
                    TEMP_DIR, f"{ds}__{subdir}__{fname}"
                )
                did = DATASET_IDS[ds]
                print(f"  Adding datasetId={did} → {ds}/{subdir}/{fname}")
                ok = add_dataset_id_branch(src, tagged_out, did)
                if ok:
                    tagged_paths.append(tagged_out)

            if not tagged_paths:
                print(f"  [SKIP] No valid inputs for {fname}")
                skipped += 1
                continue

            final_out = os.path.join(out_subdir, fname)
            print(f"  hadding {len(tagged_paths)} file(s) → combined/{subdir}/{fname}")
            ok = hadd_and_copy(final_out, tagged_paths)
            if ok:
                total_files += 1
            else:
                skipped += 1

            # Clean up tagged temp files for this fname
            for p in tagged_paths:
                if os.path.exists(p):
                    os.remove(p)

        print()

    # Clean up temp dir
    shutil.rmtree(TEMP_DIR, ignore_errors=True)

    print("=" * 60)
    print(f"Done. Combined files created : {total_files}")
    print(f"       Skipped / failed       : {skipped}")
    print(f"Output directory             : {COMBINED_DIR}")


if __name__ == "__main__":
    main()
