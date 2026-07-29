"""
run_bdt_add_scores.py — score every input ROOT file with two BDTs (a 1j1t
region model and a 2j1t region model) and write the resulting discriminants
back into the file itself as new branches, in place.

Output layout: files stay exactly where they are; they just gain two new
branches:

    bdt_variable_1j1t   (discriminant from the 1j1t-region BDT)
    bdt_variable_2j1t   (discriminant from the 2j1t-region BDT)

Every event is scored by both models (each with its own input-variable
list and its own even/odd model pair, selected by event number parity,
exactly as before).

Set the MODEL_TYPE constant near the top of this file to "multiclass" or
"binary" depending on which kind of BDTs you're scoring with — it applies
to both the 1j1t and 2j1t models (this script assumes you're never mixing
the two types in a single run). "multiclass" keeps the class-0 output of
each model, matching how the class-0 score was used as "the" BDT
discriminant in the original era-split histogramming script. "binary"
keeps the single predicted probability/score of each model directly.

All pre-existing branches in the tree are preserved untouched. Era-splitting,
histogramming, and systematics can be done later directly from these
branches (using the existing datasetId / evWeight columns already in the
tree) — this script's only job is to add the two discriminant columns.

Because uproot cannot append columns to an existing TTree in place, each
file is streamed in chunks (bounded by --chunk_size entries), scored, and
appended to a new tree written to a temp file, which is then renamed over
the original (atomic on the same filesystem).

PERFORMANCE NOTES (see --workers / --compression below)
---------------------------------------------------------
Files are fully independent of one another, so this version scores several
files concurrently using a process pool (--workers, default: all cores).
Each worker process loads the 4 XGBoost boosters exactly once (not once per
file) and then works through whatever files the pool hands it. Each
booster's internal thread count is capped to cpu_count() / workers so that
N workers running at once don't oversubscribe the machine's cores.

The write-side compression algorithm was also changed from ROOT's default
(zlib) to LZ4 (--compression, default: lz4). LZ4 is dramatically faster to
compress with different tradeoff CPU vs size — for typical flat-ntuple
columns the size difference vs zlib is usually a few percent, but the CPU
time saved on write is large. Pass --compression zlib to restore the old
behavior if disk space on EOS is tight.
"""

import argparse, os, sys, glob, time, warnings
import numpy as np

# ── constants ─────────────────────────────────────────────────────────────────
# Set this to "multiclass" or "binary" depending on which kind of BDT models
# you're scoring with. Applies to BOTH the 1j1t and 2j1t models — this
# script assumes you're never mixing the two types in a single run.
#   "multiclass": keep the class-0 output of predict() (original behavior)
#   "binary":     keep the single predicted score from predict() directly
MODEL_TYPE = "binary"

COMBINED_DIR = "/eos/uscms/store/user/vsinha/combined"

BDT_DIR = ("/uscms/home/vsinha/nobackup/CMSSW_13_3_3/src/fly/"
           "machine_learning/")

# Multiclass and binary model files live side by side in BDT_DIR, named
# BDT_<multiclass|binary>_<region>_<even|odd>.json. Build the default
# paths from MODEL_TYPE so flipping that one switch above is enough —
# no need to also pass --bdt_* paths on the command line.
BDT_1J1T_EVEN = os.path.join(BDT_DIR, f"BDT_{MODEL_TYPE}_1j1t_even.json")
BDT_1J1T_ODD  = os.path.join(BDT_DIR, f"BDT_{MODEL_TYPE}_1j1t_odd.json")
BDT_2J1T_EVEN = os.path.join(BDT_DIR, f"BDT_{MODEL_TYPE}_2j1t_even.json")
BDT_2J1T_ODD  = os.path.join(BDT_DIR, f"BDT_{MODEL_TYPE}_2j1t_odd.json")

TREE_NAME    = "outputTree"
WEIGHT_VAR   = "evWeight"
EVENT_NUMBER = "event"
DATASET_ID   = "datasetId"

BRANCH_1J1T = "bdt_variable_1j1t"
BRANCH_2J1T = "bdt_variable_2j1t"

MODEL_TYPES = ("multiclass", "binary")

INPUT_VARS_1J1T = [
    "leading_lepton_pt", "dilepton_del_phi", "leptons_invariant_mass",
    "dilepton_jet_pt", "Selected_loosejet_leadingpt", "leading_lepton_jet_pt",
    "dilepton_jet_mass", "sphericity", "aplanery", "delR_leadinglepton_jet",
    "delR_ele_muon", "subleading_lepton_pt", "HT", "ST", "MT2",
    "centrality", "lepton_pt_asymmetry", "mbl_min", "max_delR", "min_delR",
    "datasetId",
]

INPUT_VARS_2J1T = [
    "leading_lepton_pt",
    "dilepton_del_phi",
    "leptons_invariant_mass",
    "dilepton_jet_pt",
    "Selected_loosejet_leadingpt",
    "leading_lepton_jet_pt",
    "dilepton_jet_mass",
    "sphericity",
    "aplanery",
    "delR_leadinglepton_jet",
    "delR_ele_muon",
    "subleading_lepton_pt",
    "HT",
    "ST",
    "MT2",
    "centrality",
    "lepton_pt_asymmetry",
    "mbl_min",
    "delR_dilepton_jet",
    "Selected_jet_subleading_pt",
    "max_delR",
    "min_delR",
    "jet_pt_assymmetry",
    "jet_pt_ratio",
]

# ── args ──────────────────────────────────────────────────────────────────────
def parse_args():
    p = argparse.ArgumentParser()
    p.add_argument("--combined_dir", default=COMBINED_DIR,
                   help="Base dir containing <process>/*.root input files")
    p.add_argument("--bdt_1j1t_even", default=BDT_1J1T_EVEN)
    p.add_argument("--bdt_1j1t_odd",  default=BDT_1J1T_ODD)
    p.add_argument("--bdt_2j1t_even", default=BDT_2J1T_EVEN)
    p.add_argument("--bdt_2j1t_odd",  default=BDT_2J1T_ODD)
    p.add_argument("--processes",    nargs="*", default=None,
                   help="Limit to specific process names (default: all)")
    p.add_argument("--force", action="store_true",
                   help="Re-score and overwrite files even if score "
                        "branches already exist")
    p.add_argument("--dry_run", action="store_true",
                   help="Score and report, but do not write any files")
    p.add_argument("--event_branch", default=EVENT_NUMBER,
                   help=f"Name of the per-event-number branch used for the "
                        f"even/odd BDT split (default: '{EVENT_NUMBER}'). "
                        f"A few common alternates are also tried "
                        f"automatically if this one isn't found.")
    p.add_argument("--chunk_size", type=int, default=200_000,
                   help="Max number of tree entries held in memory at once "
                        "while scoring/rewriting a file (default: 200000). "
                        "Lower this if you hit out-of-memory kills on large "
                        "files (many jagged branches, e.g. LHE weights).")
    p.add_argument("--workers", type=int, default=None,
                   help="Number of files to score concurrently in separate "
                        "worker processes (default: os.cpu_count()). Set "
                        "to 1 to reproduce the old fully-serial behavior. "
                        "Each worker loads all 4 boosters once and reuses "
                        "them for every file it's handed.")
    p.add_argument("--compression", choices=["lz4", "zlib", "zstd", "none"],
                   default="lz4",
                   help="Compression algorithm used when writing the "
                        "rescored tree (default: lz4, much faster to "
                        "write than ROOT's default zlib at a similar "
                        "size for typical flat ntuple columns). Use "
                        "'zlib' to reproduce the old behavior, or 'none' "
                        "for maximum write speed at the cost of disk "
                        "space on EOS.")
    return p.parse_args()

# ── BDT scoring ───────────────────────────────────────────────────────────────
def score_model(X, event_numbers, booster_even, booster_odd, input_vars,
                 model_type="multiclass"):
    """Score X with the odd/even model pair (selected by event-number
    parity) and return a flat float32 array of discriminant values.

    For model_type == "multiclass" the class-0 output of each model is
    kept (original behavior). For model_type == "binary" the single
    predicted probability/score for each event is kept directly.
    """
    import xgboost as xgb

    if model_type not in MODEL_TYPES:
        raise ValueError(f"model_type must be one of {MODEL_TYPES}, "
                          f"got {model_type!r}")

    n      = len(event_numbers)
    scores = np.zeros(n, dtype=np.float32)
    for mask, booster in [((event_numbers % 2) == 0, booster_odd),
                           ((event_numbers % 2) == 1, booster_even)]:
        idx = np.where(mask)[0]
        if len(idx) == 0:
            continue
        dm = xgb.DMatrix(X[idx], feature_names=input_vars)
        p  = booster.predict(dm)
        p  = np.asarray(p)

        if model_type == "binary":
            # Binary classification models (e.g. objective
            # "binary:logistic") return a flat (n,) array of scores —
            # keep it as-is.
            scores[idx] = p.reshape(-1)
        else:
            # Multiclass models return (n, n_classes) — keep class-0,
            # matching the original discriminant definition.
            if p.ndim == 1:
                p = p.reshape(-1, 1)
            scores[idx] = p[:, 0]
    return scores

# ── compression helper ────────────────────────────────────────────────────────
def _resolve_compression(name):
    import uproot
    if name == "lz4":
        return uproot.LZ4(4)
    if name == "zstd":
        return uproot.ZSTD(1)
    if name == "zlib":
        return uproot.ZLIB(4)  # ROOT/uproot's old default
    return None  # "none"

# ── process one input ROOT file ───────────────────────────────────────────────
def process_file(fpath,
                  booster_1j1t_even, booster_1j1t_odd,
                  booster_2j1t_even, booster_2j1t_odd,
                  model_type=MODEL_TYPE,
                  force=False, dry_run=False, event_branch=EVENT_NUMBER,
                  chunk_size=200_000, compression="lz4"):
    """
    Streams the tree in chunks of up to `chunk_size` entries, computes both
    BDT discriminants per chunk, and appends each scored chunk to a new
    tree written to a temp file, which is then renamed over the original
    in place.

    Returns (status, n_events, msg) where status is one of:
        "written", "skipped_existing", "skipped_empty", "error"
    msg is "" except for "error", where it holds a short diagnostic string
    (and, on unexpected exceptions, a full traceback) explaining what went
    wrong. Never swallow this — it's the only way to tell "the file was
    fine but skipped on purpose" apart from "the file failed and still has
    no branches".
    """
    import uproot

    event_candidates = list(dict.fromkeys(
        [event_branch, "event", "Event", "evt", "eventNumber", "EventNumber"]
    ))

    try:
        with uproot.open(f"{fpath}:{TREE_NAME}") as tree:
            available = set(tree.keys())

            if not force and BRANCH_1J1T in available and BRANCH_2J1T in available:
                return "skipped_existing", 0, ""

            total_entries = tree.num_entries
            if total_entries == 0:
                return "skipped_empty", 0, ""

            all_branches = list(tree.keys())

            # Split branches into "flat" (simple per-event scalars) and
            # "jagged" (variable-length, e.g. LHEScaleWeight). Flat
            # branches are read with library="np" because uproot's numpy
            # path correctly handles ROOT's big-endian on-disk storage;
            # some flat/counter branches come through with a raw
            # non-native dtype like '>u4' when read via library="ak",
            # which awkward's writer then refuses.
            flat_names, jagged_names = [], []
            for name in all_branches:
                interp = tree[name].interpretation
                if type(interp).__name__ == "AsDtype":
                    flat_names.append(name)
                else:
                    jagged_names.append(name)

            # Counter branches (e.g. "nLHEPdfWeight" for jagged branch
            # "LHEPdfWeight") must NOT be read/written explicitly: uproot
            # auto-generates them from the jagged array itself when
            # writing. Passing one explicitly under the same name
            # collides with the auto-generated one and the write fails
            # with "missing '<counter>'". So drop any branch that is a
            # counter for one of our jagged branches.
            counter_names = set()
            for name in jagged_names:
                cb = getattr(tree[name], "count_branch", None)
                if cb is not None:
                    counter_names.add(cb.name)
            flat_names = [n for n in flat_names if n not in counter_names]

            # Resolve the event-number branch name once, up front.
            found_event_name = next(
                (c for c in event_candidates if c in flat_names), None
            )
            if found_event_name is None:
                for cand in event_candidates:
                    if cand in available and cand not in flat_names:
                        try:
                            tree.arrays([cand], entry_start=0, entry_stop=1,
                                        library="np")
                            flat_names.append(cand)
                            found_event_name = cand
                            break
                        except Exception:
                            continue

            if found_event_name is None:
                sample = sorted(available)[:40]
                msg = (f"none of {event_candidates} found in this file. "
                       f"First branches present: {sample}"
                       f"{' ...' if len(available) > 40 else ''}")
                return "error", 0, msg

            event_branch = found_event_name

            if dry_run:
                return "written", total_entries, ""

            # Auto-shrink chunk_size for large and/or jagged-heavy files.
            # `chunk_size` (from --chunk_size) is a per-worker peak-memory
            # knob, but a fixed entry count doesn't account for how much
            # each entry actually weighs: a DY-inclusive file with LHE
            # scale/PDF weight arrays can be 10-50x heavier per event than
            # a small skim, even at the same entry count. Estimate
            # decompressed bytes/entry from (on-disk file size / entries),
            # inflate by a conservative factor for decompression +
            # jagged/awkward overhead, and cap the chunk at a fixed memory
            # budget. This makes the effective chunk size adapt per file,
            # so N workers running concurrently can't blow up the node's
            # memory just because several of them happened to draw large
            # files at the same time.
            TARGET_CHUNK_BYTES = 300 * 1024 ** 2      # ~300 MB/chunk/worker
            DECOMPRESSION_INFLATION = 10               # conservative
            note = ""
            try:
                file_size = os.path.getsize(fpath)
                avg_bytes_per_entry = file_size / total_entries
                est_chunk = int(TARGET_CHUNK_BYTES /
                                 max(1.0, avg_bytes_per_entry * DECOMPRESSION_INFLATION))
                eff_chunk_size = max(5_000, min(chunk_size, est_chunk))
            except Exception:
                eff_chunk_size = chunk_size
            if eff_chunk_size < chunk_size:
                note = (f"auto-reduced chunk_size {chunk_size} -> "
                        f"{eff_chunk_size} for this file "
                        f"({file_size / 1024**2:.0f} MB on disk, "
                        f"{total_entries} entries)")
            chunk_size = eff_chunk_size

            tmp_path = fpath + f".tmp.{os.getpid()}"
            n_chunks = (total_entries + chunk_size - 1) // chunk_size

            comp = _resolve_compression(compression)
            with uproot.recreate(tmp_path, compression=comp) as rf:
                for chunk_i, start in enumerate(range(0, total_entries, chunk_size)):
                    stop = min(start + chunk_size, total_entries)

                    flat_arrays = (
                        tree.arrays(flat_names, entry_start=start,
                                    entry_stop=stop, library="np")
                        if flat_names else {}
                    )
                    jagged_arrays = (
                        tree.arrays(jagged_names, entry_start=start,
                                    entry_stop=stop, library="ak")
                        if jagged_names else None
                    )

                    n_chunk = stop - start

                    def flat_column(name):
                        """Flat float32 column for a BDT input feature,
                        0-filled if missing/non-finite."""
                        if name in flat_arrays:
                            col = flat_arrays[name].astype(np.float32)
                            col = np.where(np.isfinite(col), col, 0.0)
                        else:
                            col = np.zeros(n_chunk, dtype=np.float32)
                        return col

                    event_numbers = flat_arrays[event_branch].astype(np.int64)

                    X_1j1t = np.column_stack(
                        [flat_column(v) for v in INPUT_VARS_1J1T]
                    ).astype(np.float32)
                    X_2j1t = np.column_stack(
                        [flat_column(v) for v in INPUT_VARS_2J1T]
                    ).astype(np.float32)

                    var_1j1t = score_model(
                        X_1j1t, event_numbers,
                        booster_1j1t_even, booster_1j1t_odd, INPUT_VARS_1J1T,
                        model_type=model_type)
                    var_2j1t = score_model(
                        X_2j1t, event_numbers,
                        booster_2j1t_even, booster_2j1t_odd, INPUT_VARS_2J1T,
                        model_type=model_type)

                    out_columns = dict(flat_arrays)
                    if jagged_names:
                        for name in jagged_names:
                            out_columns[name] = jagged_arrays[name]
                    out_columns[BRANCH_1J1T] = var_1j1t
                    out_columns[BRANCH_2J1T] = var_2j1t

                    if chunk_i == 0:
                        rf[TREE_NAME] = out_columns
                    else:
                        rf[TREE_NAME].extend(out_columns)

            os.rename(tmp_path, fpath)
            return "written", total_entries, note

    except Exception as ex:
        import traceback
        # Best-effort cleanup so a failed file doesn't leave a stray
        # .tmp.<pid> file sitting on EOS.
        try:
            if 'tmp_path' in locals() and os.path.exists(tmp_path):
                os.remove(tmp_path)
        except Exception:
            pass
        return "error", 0, traceback.format_exc()

# ── per-worker global state ────────────────────────────────────────────────────
# Each worker process in the pool loads the 4 boosters exactly once (in
# _worker_init, run when the process starts) and reuses them for every file
# it's subsequently handed via _worker_score_one. This avoids re-loading
# ~4 JSON model files from disk for every single input file, and lets
# xgboost's internal thread pool be sized once per process.
_WORKER = {}

def _worker_init(bdt_1j1t_even, bdt_1j1t_odd, bdt_2j1t_even, bdt_2j1t_odd,
                  nthread):
    import xgboost as xgb
    warnings.filterwarnings("ignore", category=UserWarning)

    b1e = xgb.Booster(); b1e.load_model(bdt_1j1t_even)
    b1o = xgb.Booster(); b1o.load_model(bdt_1j1t_odd)
    b2e = xgb.Booster(); b2e.load_model(bdt_2j1t_even)
    b2o = xgb.Booster(); b2o.load_model(bdt_2j1t_odd)

    # Cap each booster's internal thread count so that `workers` processes
    # running at once don't collectively oversubscribe the machine's cores.
    for b in (b1e, b1o, b2e, b2o):
        b.set_param({"nthread": max(1, nthread)})

    _WORKER["boosters"] = (b1e, b1o, b2e, b2o)


def _worker_score_one(task):
    fpath, model_type, force, dry_run, event_branch, chunk_size, compression = task
    b1e, b1o, b2e, b2o = _WORKER["boosters"]
    try:
        status, n, msg = process_file(
            fpath, b1e, b1o, b2e, b2o,
            model_type=model_type, force=force, dry_run=dry_run,
            event_branch=event_branch, chunk_size=chunk_size,
            compression=compression,
        )
    except Exception:
        # Belt-and-suspenders: process_file already catches its own
        # exceptions, but if something outside that try block blows up
        # (e.g. a MemoryError raised between calls), still report it
        # instead of letting the worker die silently.
        import traceback
        return fpath, "error", 0, traceback.format_exc()
    return fpath, status, n, msg


# ── main ──────────────────────────────────────────────────────────────────────
def main():
    args         = parse_args()
    combined_dir = args.combined_dir

    warnings.filterwarnings("ignore", category=UserWarning)

    if args.processes:
        proc_dirs = [os.path.join(combined_dir, p) for p in args.processes]
    else:
        proc_dirs = sorted(
            d for d in glob.glob(os.path.join(combined_dir, "*"))
            if os.path.isdir(d)
        )

    if not proc_dirs:
        sys.exit(f"ERROR: No process directories found under {combined_dir}")

    # Build the full flat list of (proc, fpath) up front so we can farm it
    # out across worker processes regardless of which process directory
    # each file happens to belong to.
    all_files = []  # list of (proc, fpath)
    for proc_dir in proc_dirs:
        proc = os.path.basename(proc_dir)
        root_files = sorted(glob.glob(os.path.join(proc_dir, "*.root")))
        for fpath in root_files:
            all_files.append((proc, fpath))

    if not all_files:
        sys.exit(f"ERROR: No .root files found under any process directory "
                  f"in {combined_dir}")

    n_workers = args.workers or os.cpu_count() or 1
    n_workers = max(1, min(n_workers, len(all_files)))
    nthread_per_worker = max(1, (os.cpu_count() or n_workers) // n_workers)

    print(f"Model type: {MODEL_TYPE}", flush=True)
    print(f"Branches to add: '{BRANCH_1J1T}' (from 1j1t BDT, "
          f"{len(INPUT_VARS_1J1T)} input vars), "
          f"'{BRANCH_2J1T}' (from 2j1t BDT, {len(INPUT_VARS_2J1T)} input vars)",
          flush=True)
    print(f"Files will be updated IN PLACE under: {combined_dir}", flush=True)
    print(f"{len(all_files)} file(s) across {len(proc_dirs)} process "
          f"director{'y' if len(proc_dirs)==1 else 'ies'}, "
          f"{n_workers} worker process(es), "
          f"{nthread_per_worker} xgboost thread(s)/worker, "
          f"compression={args.compression}", flush=True)
    if args.dry_run:
        print("DRY RUN — no files will be modified.\n", flush=True)
    print()

    t0 = time.time()
    grand_written = grand_skip_existing = grand_skip_empty = grand_err = 0

    tasks = [
        (fpath, MODEL_TYPE, args.force, args.dry_run, args.event_branch,
         args.chunk_size, args.compression)
        for (_, fpath) in all_files
    ]

    if n_workers == 1:
        # Serial path (also useful for debugging): load boosters once in
        # this process and reuse the same worker function directly.
        _worker_init(args.bdt_1j1t_even, args.bdt_1j1t_odd,
                     args.bdt_2j1t_even, args.bdt_2j1t_odd,
                     nthread_per_worker)
        results = []
        for i, task in enumerate(tasks, 1):
            fpath, status, n, msg = _worker_score_one(task)
            results.append((fpath, status, n))
            print(f"  [{i}/{len(tasks)}] {os.path.basename(fpath)}: "
                  f"{status} ({n} events)", flush=True)
            if status == "error":
                print(f"    ERROR: {msg}", flush=True)
            elif status == "written" and msg:
                print(f"    note: {msg}", flush=True)
    else:
        import concurrent.futures as cf
        results = []
        broken_pool_warned = False
        with cf.ProcessPoolExecutor(
            max_workers=n_workers,
            initializer=_worker_init,
            initargs=(args.bdt_1j1t_even, args.bdt_1j1t_odd,
                      args.bdt_2j1t_even, args.bdt_2j1t_odd,
                      nthread_per_worker),
        ) as ex:
            futures = {ex.submit(_worker_score_one, task): task[0]
                       for task in tasks}
            done = 0
            for fut in cf.as_completed(futures):
                fpath = futures[fut]
                done += 1
                try:
                    fpath, status, n, msg = fut.result()
                except cf.process.BrokenProcessPool as e:
                    # A worker process died outright (almost always an
                    # OOM kill from the kernel, not a Python exception —
                    # that's why process_file's own try/except never
                    # saw it). This file (and possibly others still
                    # in-flight in the same pool) will show up here as
                    # errors; rerun the script afterwards with a lower
                    # --workers and/or --chunk_size and it will only
                    # touch files still missing the branches, since
                    # already-written files are skipped.
                    status, n = "error", 0
                    msg = (f"worker process died unexpectedly (likely "
                           f"OOM-killed by the kernel): {e}")
                    if not broken_pool_warned:
                        print("  *** A worker process was killed "
                              "(most likely out-of-memory). Remaining "
                              "in-flight files in this run will be "
                              "reported as errors below. Rerun with a "
                              "lower --workers or --chunk_size — files "
                              "that already have both branches are "
                              "skipped automatically, so this is safe "
                              "to re-run. ***", flush=True)
                        broken_pool_warned = True
                except Exception as e:
                    import traceback
                    status, n = "error", 0
                    msg = traceback.format_exc()
                print(f"  [{done}/{len(tasks)}] {os.path.basename(fpath)}: "
                      f"{status}{'' if status != 'written' else f' ({n} events)'}",
                      flush=True)
                if status == "error":
                    print(f"    ERROR: {msg}", flush=True)
                elif status == "written" and msg:
                    print(f"    note: {msg}", flush=True)
                results.append((fpath, status, n))

    for fpath, status, n in results:
        if status == "written":
            grand_written += 1
        elif status == "skipped_existing":
            grand_skip_existing += 1
        elif status == "skipped_empty":
            grand_skip_empty += 1
        else:
            grand_err += 1

    elapsed = time.time() - t0
    print(f"\n{'='*60}", flush=True)
    print(f"All done in {elapsed:.1f}s — "
          f"{grand_written} file(s) written, "
          f"{grand_skip_existing} skipped (already scored), "
          f"{grand_skip_empty} skipped (empty), "
          f"{grand_err} errors", flush=True)


if __name__ == "__main__":
    main()
