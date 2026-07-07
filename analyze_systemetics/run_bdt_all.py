"""
run_bdt_add_scores.py — score every input ROOT file with two BDTs (a 1j1t
region model and a 2j1t region model) and write the resulting discriminants
back into the file itself as new branches, in place.

Output layout: files stay exactly where they are; they just gain two new
branches:

    bdt_variable_1j1t   (class-0 score of the 1j1t-region BDT)
    bdt_variable_2j1t   (class-0 score of the 2j1t-region BDT)

Every event is scored by both models (each with its own input-variable
list and its own even/odd model pair, selected by event number parity,
exactly as before). Only the class-0 output of each multiclass model is
kept, matching how the class-0 score was used as "the" BDT discriminant
in the original era-split histogramming script.

All pre-existing branches in the tree are preserved untouched. Era-splitting,
histogramming, and systematics can be done later directly from these
branches (using the existing datasetId / evWeight columns already in the
tree) — this script's only job is to add the two discriminant columns.

Because uproot cannot append columns to an existing TTree in place, each
file is streamed in chunks (bounded by --chunk_size entries), scored, and
appended to a new tree written to a temp file, which is then renamed over
the original (atomic on the same filesystem).
"""

import argparse, os, sys, glob, time, warnings
import numpy as np

# ── constants ─────────────────────────────────────────────────────────────────
COMBINED_DIR = "/eos/uscms/store/user/vsinha/combined"

BDT_DIR = ("/uscms/home/vsinha/nobackup/CMSSW_13_3_3/src/fly/"
           "machine_learning/muticlass_classification/bdt")

BDT_1J1T_EVEN = os.path.join(BDT_DIR, "BDT_multiclass_1j1t_even.json")
BDT_1J1T_ODD  = os.path.join(BDT_DIR, "BDT_multiclass_1j1t_odd.json")
BDT_2J1T_EVEN = os.path.join(BDT_DIR, "BDT_multiclass_2j1t_even.json")
BDT_2J1T_ODD  = os.path.join(BDT_DIR, "BDT_multiclass_2j1t_odd.json")

TREE_NAME    = "outputTree"
WEIGHT_VAR   = "evWeight"
EVENT_NUMBER = "event"
DATASET_ID   = "datasetId"

BRANCH_1J1T = "bdt_variable_1j1t"
BRANCH_2J1T = "bdt_variable_2j1t"

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
    return p.parse_args()

# ── BDT scoring ───────────────────────────────────────────────────────────────
def score_class0(X, event_numbers, booster_even, booster_odd, input_vars):
    """Score X with the odd/even model pair (selected by event-number
    parity) and return only the class-0 output as a flat float32 array."""
    import xgboost as xgb
    n      = len(event_numbers)
    class0 = np.zeros(n, dtype=np.float32)
    for mask, booster in [((event_numbers % 2) == 0, booster_odd),
                           ((event_numbers % 2) == 1, booster_even)]:
        idx = np.where(mask)[0]
        if len(idx) == 0:
            continue
        dm = xgb.DMatrix(X[idx], feature_names=input_vars)
        p  = booster.predict(dm)
        if p.ndim == 1:
            p = p.reshape(-1, 1)
        class0[idx] = p[:, 0]
    return class0

# ── process one input ROOT file ───────────────────────────────────────────────
def process_file(fpath,
                  booster_1j1t_even, booster_1j1t_odd,
                  booster_2j1t_even, booster_2j1t_odd,
                  force=False, dry_run=False, event_branch=EVENT_NUMBER,
                  chunk_size=200_000):
    """
    Streams the tree in chunks of up to `chunk_size` entries, computes both
    BDT discriminants per chunk, and appends each scored chunk to a new
    tree written to a temp file, which is then renamed over the original
    in place.

    Returns (status, n_events) where status is one of:
        "written", "skipped_existing", "skipped_empty", "error"
    """
    import uproot

    event_candidates = list(dict.fromkeys(
        [event_branch, "event", "Event", "evt", "eventNumber", "EventNumber"]
    ))

    try:
        with uproot.open(f"{fpath}:{TREE_NAME}") as tree:
            available = set(tree.keys())

            if not force and BRANCH_1J1T in available and BRANCH_2J1T in available:
                return "skipped_existing", 0

            total_entries = tree.num_entries
            if total_entries == 0:
                return "skipped_empty", 0

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
                print(f"    ERROR: none of {event_candidates} found in this "
                      f"file. First branches present: {sample}"
                      f"{' ...' if len(available) > 40 else ''}", flush=True)
                return "error", 0

            event_branch = found_event_name

            if dry_run:
                return "written", total_entries

            tmp_path = fpath + ".tmp"
            n_chunks = (total_entries + chunk_size - 1) // chunk_size

            with uproot.recreate(tmp_path) as rf:
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

                    var_1j1t = score_class0(
                        X_1j1t, event_numbers,
                        booster_1j1t_even, booster_1j1t_odd, INPUT_VARS_1J1T)
                    var_2j1t = score_class0(
                        X_2j1t, event_numbers,
                        booster_2j1t_even, booster_2j1t_odd, INPUT_VARS_2J1T)

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

                    if n_chunks > 1:
                        print(f"\n    chunk {chunk_i + 1}/{n_chunks} "
                              f"({stop}/{total_entries} entries)",
                              end="", flush=True)

            os.rename(tmp_path, fpath)
            return "written", total_entries

    except Exception as ex:
        print(f"    READ/WRITE ERROR: {ex}", flush=True)
        return "error", 0

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

    # Load all four boosters once, up front, and reuse them for every file
    # and every chunk.
    import xgboost as xgb
    booster_1j1t_even = xgb.Booster(); booster_1j1t_even.load_model(args.bdt_1j1t_even)
    booster_1j1t_odd  = xgb.Booster(); booster_1j1t_odd.load_model(args.bdt_1j1t_odd)
    booster_2j1t_even = xgb.Booster(); booster_2j1t_even.load_model(args.bdt_2j1t_even)
    booster_2j1t_odd  = xgb.Booster(); booster_2j1t_odd.load_model(args.bdt_2j1t_odd)

    print(f"Branches to add: '{BRANCH_1J1T}' (from 1j1t BDT, "
          f"{len(INPUT_VARS_1J1T)} input vars), "
          f"'{BRANCH_2J1T}' (from 2j1t BDT, {len(INPUT_VARS_2J1T)} input vars)",
          flush=True)
    print(f"Files will be updated IN PLACE under: {combined_dir}", flush=True)
    if args.dry_run:
        print("DRY RUN — no files will be modified.\n", flush=True)
    print()

    t0 = time.time()
    grand_written = grand_skip_existing = grand_skip_empty = grand_err = 0

    for proc_dir in proc_dirs:
        proc = os.path.basename(proc_dir)
        print(f"\n{'='*60}", flush=True)
        print(f"Process: {proc}", flush=True)

        root_files = sorted(glob.glob(os.path.join(proc_dir, "*.root")))
        if not root_files:
            print("  No .root files found, skipping.", flush=True)
            continue

        print(f"  {len(root_files)} ROOT file(s) to process", flush=True)

        for fpath in root_files:
            stem       = os.path.splitext(os.path.basename(fpath))[0]
            systematic = stem[len(proc) + 1:] if stem.startswith(proc + "_") else stem

            print(f"  {systematic} ...", end=" ", flush=True)

            try:
                status, n = process_file(
                    fpath,
                    booster_1j1t_even, booster_1j1t_odd,
                    booster_2j1t_even, booster_2j1t_odd,
                    force=args.force, dry_run=args.dry_run,
                    event_branch=args.event_branch,
                    chunk_size=args.chunk_size,
                )
            except Exception:
                import traceback
                print(f"ERROR\n{traceback.format_exc()}", flush=True)
                grand_err += 1
                continue

            if status == "written":
                print(f"✓ {n} events scored", flush=True)
                grand_written += 1
            elif status == "skipped_existing":
                print("skipped (score branches already present, use --force)",
                      flush=True)
                grand_skip_existing += 1
            elif status == "skipped_empty":
                print("skipped (0 events)", flush=True)
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
