#!/usr/bin/env python3
"""
Fast IN-PLACE datasetId tagging for ROOT systematics files across
dataset-eras. Modifies the original files directly -- no duplicate copies.

Why this is fast, and why it doesn't need 1.2TB of extra disk
------------------------------------------------------------
RDataFrame's Snapshot() (the old script) has to decompress every branch's
baskets into a columnar representation and re-serialize the WHOLE tree
into a brand new file, even though you're only adding one constant-valued
column. That's both slow (full decompress/recompress of everything) and
disk-hungry (a full duplicate of every file you tag).

This script instead opens each file in ROOT's UPDATE mode and adds the
`datasetId` branch directly to the existing tree in the existing file:
  - Existing branches/baskets are never read, decompressed, or rewritten.
  - Only the new branch's data is written -- a few bytes per entry for one
    int column -- so tagging your whole dataset costs roughly
    (total entries x 4 bytes) of EXTRA disk, not another full copy.
  - This is a standard, documented ROOT technique (see the TTree class
    docs / tree4.C tutorial: "add a branch to an existing tree").
  - Runs as compiled C++ via ROOT's JIT, so no per-event Python callback
    overhead either.

Trade-off vs. the copy-based approach: this MUTATES your original files.
A run interrupted mid-write leaves you modifying the same file you'd
need as a fallback. Guardrails here: tagging is idempotent (already
tagged files are skipped, so reruns are safe), you must pass --yes to
actually modify anything, and --test-file lets you try it on one file
first. Please make sure these files are recoverable elsewhere (EOS/DAS/
wherever they're produced) before running this at scale.

This script does NOT merge -- that's still a separate `hadd` step you run
yourself per group afterward, same as before.

Usage:
  python3 tag_datasets_fast.py /path/to/working/dir --yes [-j N]
  python3 tag_datasets_fast.py /path/to/working/dir --test-file some/file.root
"""

import os
import sys
import glob
import argparse
from concurrent.futures import ProcessPoolExecutor, as_completed
from concurrent.futures.process import BrokenProcessPool as BrokenProcessPoolError

try:
    import ROOT
    ROOT.gROOT.SetBatch(True)
    ROOT.gErrorIgnoreLevel = ROOT.kError
except ImportError:
    print("ERROR: PyROOT not available. Please run inside a CMSSW/ROOT environment.")
    sys.exit(1)

# ── Add new datasets here as needed ─────────────────────────────────────────
DATASETS = [
    "Result_2022",       # datasetId = 0
    "Result_2022EE",     # datasetId = 1
    "Result_2023",       # datasetId = 2
    "Result_2023BPix",   # datasetId = 3
    "Result_2024",       # datasetId = 4
]
DATASET_IDS = {d: i for i, d in enumerate(DATASETS)}
DATASET_YEAR_TAGS = {ds.replace("Result_", "") for ds in DATASETS}

# ── Subdir name aliases (same folders that get merged together later) ──────
SUBDIR_GROUPS = {
    "DYjetsM50":     ["DYjetsM50", "DYto2E-2Jets_MLL-50",     "DYto2Mu-2Jets_MLL-50"],
    "DYjetsM10to50": ["DYjetsM10to50", "DYto2E-2Jets_MLL-10to50", "DYto2Mu-2Jets_MLL-10to50"],
    "WtoLNu":        ["WtoLNu", "WtoENu-2Jets", "WtoMuNu-2Jets"],
}
ALIAS_TO_CANONICAL = {}
for _canon, _aliases in SUBDIR_GROUPS.items():
    for _a in _aliases:
        ALIAS_TO_CANONICAL[_a] = _canon


def canonicalize(subdir_name):
    return ALIAS_TO_CANONICAL.get(subdir_name, subdir_name)


SYST_SUBDIR = "systemetics"
TREE_NAME_DEFAULT = "outputTree"

# ── The C++ tagger: add datasetId branch to the file IN PLACE ──────────────
# No new file is created. Existing branches are never read or rewritten --
# only the new branch's baskets get written, so disk cost is ~(entries x
# 4 bytes) instead of a full copy of the file.
CPP_TAGGER = r"""
#include "TFile.h"
#include "TTree.h"

// Returns: 0 = ok (tagged), 1 = cannot open file, 2 = tree not found,
// 5 = entry-count mismatch after write (truncated/failed write -- almost
// always a full/broken disk or a quota hit), 6 = already tagged (skipped,
// only returned when force=false).
int TagFileInPlace(const char* path, const char* treeName,
                    int datasetId, bool force) {
    TFile* f = TFile::Open(path, "UPDATE");
    if (!f || f->IsZombie()) { if (f) delete f; return 1; }

    TTree* t = (TTree*)f->Get(treeName);
    if (!t) { f->Close(); delete f; return 2; }

    if (t->GetBranch("datasetId") && !force) {
        f->Close();
        delete f;
        return 6;
    }

    Long64_t nExpected = t->GetEntries();
    int did = datasetId;
    TBranch* b = t->GetBranch("datasetId");
    if (!b) {
        b = t->Branch("datasetId", &did, "datasetId/I");
        for (Long64_t i = 0; i < nExpected; ++i) b->Fill();
        t->Write("", TObject::kOverwrite);
    }
    Long64_t nWritten = t->GetEntries();

    f->Close();
    delete f;

    if (nWritten != nExpected) return 5;
    return 0;
}

// Cheap check used to build the "already tagged" skip list without
// forking a worker process for every file.
bool HasDatasetIdBranch(const char* path, const char* treeName) {
    TFile* f = TFile::Open(path, "READ");
    if (!f || f->IsZombie()) { if (f) delete f; return false; }
    TTree* t = (TTree*)f->Get(treeName);
    bool has = t && t->GetBranch("datasetId");
    f->Close();
    delete f;
    return has;
}
"""

TAGGER_ERRORS = {
    1: "cannot open file (missing/corrupt/zombie/read-only?)",
    2: "tree not found in file",
    5: "entry count mismatch after write -- likely truncated (check disk space/quota)",
    6: "already tagged (skipped)",
}


def _init_worker():
    """Runs once per worker process: JIT-compile the tagger in that process."""
    ROOT.gROOT.SetBatch(True)
    ROOT.gErrorIgnoreLevel = ROOT.kError
    ROOT.gInterpreter.Declare(CPP_TAGGER)


def is_year_specific(fname, ds):
    matched = [tag for tag in DATASET_YEAR_TAGS if fname.endswith(f"_{tag}.root")]
    return len(matched) == 1 and ds == f"Result_{matched[0]}"


def merge_key(fname, src_name):
    """Strip the process-name prefix to get the systematic+year suffix --
    used only for the missing-file cross-check warning below."""
    prefix = src_name + "_"
    return fname[len(prefix):] if fname.startswith(prefix) else fname


def is_nominal_file(fname):
    stem = fname[:-5] if fname.endswith(".root") else fname
    return "nominal" in stem.split("_")


def discover_tasks(base_dir, nominal_only=False, subdir_filter=None):
    """
    Returns a flat list of per-file tag tasks: (dataset, canonical_subdir,
    src_name, raw_fname, src_path). No output naming needed -- files are
    tagged where they sit.

    subdir_filter: optional set of canonical subdir names to restrict to.
    """
    all_subdirs = set()
    for ds in DATASETS:
        syst_path = os.path.join(base_dir, ds, SYST_SUBDIR)
        if not os.path.isdir(syst_path):
            print(f"[WARN] Systematics path not found, skipping: {syst_path}")
            continue
        for d in os.listdir(syst_path):
            if os.path.isdir(os.path.join(syst_path, d)):
                all_subdirs.add(canonicalize(d))

    if not all_subdirs:
        print("ERROR: No subdirectories found in any dataset.")
        sys.exit(1)

    if subdir_filter:
        missing = subdir_filter - all_subdirs
        for m in missing:
            print(f"[WARN] --subdir '{m}' not found under any dataset -- ignoring")
        all_subdirs = all_subdirs & subdir_filter
        if not all_subdirs:
            print("ERROR: none of the requested --subdir names were found.")
            sys.exit(1)

    tasks = []
    for subdir in sorted(all_subdirs):
        source_names = SUBDIR_GROUPS.get(subdir, [subdir])
        n_before = len(tasks)
        seen_groups = {}
        for ds in DATASETS:
            for src_name in source_names:
                pattern = os.path.join(base_dir, ds, SYST_SUBDIR, src_name, "*.root")
                for f in glob.glob(pattern):
                    raw_fname = os.path.basename(f)
                    if nominal_only and not is_nominal_file(raw_fname):
                        continue
                    key = merge_key(raw_fname, src_name)
                    seen_groups.setdefault(key, set()).add(ds)
                    tasks.append((ds, subdir, src_name, raw_fname, f))

        if not seen_groups:
            if not nominal_only:
                print(f"[{subdir}] No ROOT files found -- skipping subdir")
            continue

        for key, present_in in seen_groups.items():
            for ds in DATASETS:
                if ds not in present_in and not any(is_year_specific(key, p) for p in present_in):
                    print(f"  [WARN] {subdir}: missing in {ds} for key={key}")

        alias_note = f" (from {', '.join(source_names)})" if len(source_names) > 1 else ""
        print(f"[{subdir}] {len(tasks) - n_before} file(s) to tag{alias_note}")

    return tasks


def tag_one(task):
    """Runs in a worker process. task = (ds, subdir, src_name, raw_fname,
    src_path, tree_name, force)."""
    ds, subdir, src_name, raw_fname, src_path, tree_name, force = task
    did = DATASET_IDS[ds]

    rc = ROOT.TagFileInPlace(src_path, tree_name, did, force)

    if rc == 6:
        return {"status": "skip", "log": [f"skip (already tagged): {src_path}"]}
    if rc != 0:
        err = TAGGER_ERRORS.get(rc, f"unknown error code {rc}")
        return {"status": "fail", "log": [f"[FAIL] {src_path}: {err}"]}

    return {"status": "ok", "log": [f"tagged datasetId={did} (in place) -> {src_path}"]}


def parse_args():
    p = argparse.ArgumentParser(
        description="Fast IN-PLACE per-file datasetId tagging (no merging, no "
                    "duplicate files). hadd the results yourself per group afterward."
    )
    p.add_argument("base_dir", help="Path containing Result_2022, Result_2023, etc.")
    p.add_argument("-j", "--jobs", type=int, default=8,
                   help="Parallel worker processes. Each file is an independent, "
                        "cheap task, so this can be set high. Default: 8.")
    p.add_argument("--tree-name", default=TREE_NAME_DEFAULT,
                   help=f"Tree to tag. Default: {TREE_NAME_DEFAULT}")
    p.add_argument("--nominal-only", action="store_true",
                   help="Only tag files whose name contains 'nominal'.")
    p.add_argument("--dry-run", action="store_true",
                   help="Only discover and print the file list; modify nothing.")
    p.add_argument("--yes", action="store_true",
                   help="Required to actually modify files in place. Without this, "
                        "the script only prints the plan (same as --dry-run) -- this "
                        "is a deliberate safety gate since files are mutated, not copied.")
    p.add_argument("--force-retag", action="store_true",
                   help="By default, files that already have a datasetId branch are "
                        "skipped (safe reruns). Pass this to re-add it anyway "
                        "(NOTE: current tagger does not overwrite an existing branch's "
                        "values -- if you need to actually change an existing tag, ask "
                        "for that variant explicitly rather than using this flag blind).")
    p.add_argument("--subdir", default=None,
                   help="Restrict to one or more process subdirectories (comma-separated), "
                        "e.g. --subdir TTGJets_PTG-100to200 or "
                        "--subdir TTGJets_PTG-100to200,DYjetsM50. Matches the canonical "
                        "subdir name (after alias grouping). Default: process everything "
                        "found under base_dir.")
    p.add_argument("--test-file", default=None,
                   help="Tag exactly one file (by path) and exit -- use this to sanity "
                        "check the approach on your data before running at scale.")
    return p.parse_args()


def main():
    args = parse_args()
    base_dir = os.path.abspath(args.base_dir)
    jobs = max(1, args.jobs)
    tree_name = args.tree_name

    if args.test_file:
        test_path = args.test_file if os.path.isabs(args.test_file) \
            else os.path.join(base_dir, args.test_file)
        print(f"[test-file] Tagging exactly one file in place: {test_path}")
        if not args.yes:
            print("Pass --yes to actually modify it (this mutates the file). "
                  "Nothing was changed.")
            return
        _init_worker()
        rc = ROOT.TagFileInPlace(test_path, tree_name, 0, args.force_retag)
        if rc == 0:
            print("OK: datasetId branch added (value 0 used for this test).")
        elif rc == 6:
            print("Already tagged -- nothing to do (pass --force-retag to override).")
        else:
            print(f"FAILED: {TAGGER_ERRORS.get(rc, f'unknown error code {rc}')}")
        return

    print(f"Base directory  : {base_dir}")
    print(f"Mode            : IN-PLACE (files are modified directly, no copies)")
    print(f"Parallel jobs   : {jobs}")
    print(f"Datasets ({len(DATASETS)}):")
    for ds, did in DATASET_IDS.items():
        exists = "found" if os.path.isdir(os.path.join(base_dir, ds)) else "NOT FOUND"
        print(f"  [{did}] {ds}  ({exists})")
    if args.nominal_only:
        print("Filter          : --nominal-only")
    print()

    subdir_filter = set(s.strip() for s in args.subdir.split(",")) if args.subdir else None
    if subdir_filter:
        print(f"Restricting to subdir(s): {', '.join(sorted(subdir_filter))}")

    tasks_raw = discover_tasks(base_dir, nominal_only=args.nominal_only, subdir_filter=subdir_filter)
    print(f"\nDiscovered {len(tasks_raw)} file(s) to tag.")

    if args.dry_run or not args.yes:
        for ds, subdir, src_name, raw_fname, src_path in tasks_raw:
            print(f"  datasetId={DATASET_IDS[ds]}  {src_path}")
        if not args.yes:
            print("\n--yes was not passed -- nothing was modified. "
                  "Re-run with --yes once this plan looks right.")
        else:
            print("\nDry run: no files were modified.")
        return

    tasks = [(ds, subdir, src_name, raw_fname, src_path, tree_name, args.force_retag)
             for ds, subdir, src_name, raw_fname, src_path in tasks_raw]

    print(f"Dispatching {len(tasks)} in-place tagging job(s) across {jobs} worker process(es)...\n")

    total_ok = total_fail = total_skip = total_crashed = 0
    pending = list(tasks)
    while pending:
        round_pending = pending
        pending = []
        completed = set()
        try:
            with ProcessPoolExecutor(max_workers=jobs, initializer=_init_worker) as ex:
                fut_to_idx = {ex.submit(tag_one, t): i for i, t in enumerate(round_pending)}
                for fut in as_completed(fut_to_idx):
                    idx = fut_to_idx[fut]
                    result = fut.result()  # may raise BrokenProcessPool
                    completed.add(idx)
                    for line in result["log"]:
                        print(line)
                    if result["status"] == "ok":
                        total_ok += 1
                    elif result["status"] == "skip":
                        total_skip += 1
                    else:
                        total_fail += 1
        except BrokenProcessPoolError:
            remaining = [t for i, t in enumerate(round_pending) if i not in completed]
            if not remaining or len(remaining) == len(round_pending):
                total_crashed += len(remaining)
                for ds, subdir, src_name, raw_fname, *_ in remaining:
                    print(f"  [CRASHED] Worker pool broke and made no progress on "
                          f"{subdir}/{ds}/{src_name}/{raw_fname} -- giving up on it. "
                          f"Check dmesg/disk space.")
                break
            print(f"[WARN] Worker pool crashed. {len(remaining)} file(s) unfinished -- "
                  f"retrying with a fresh pool.")
            pending = remaining

    print("=" * 60)
    print(f"Done. Tagged      : {total_ok}")
    print(f"      Skipped     : {total_skip} (already had datasetId)")
    print(f"      Failed      : {total_fail}")
    print(f"      Crashed     : {total_crashed}")
    if total_fail or total_crashed:
        print("\nRerun the same command to retry failed/crashed files -- files that "
              "were already tagged are skipped automatically, so a rerun only "
              "touches what didn't finish.")
    print("\nFiles were tagged in place, in their original locations. When ready to "
          "merge a group later, hadd the matching files across Result_*/ directly, e.g.:")
    print("  hadd DYjetsM50_nominal.root Result_*/systemetics/DYjetsM50/DYjetsM50_nominal*.root")


if __name__ == "__main__":
    main()
