"""
compare_systemetics.py
----------------------
For each process directory under the year-stamped systemetics path, produces
diagnostic plots for THREE kinds of uncertainties:

  1. FILE-BASED systematics   – separate ROOT files for each up/down variation
  2. SF-BRANCH systematics    – up/down branches already inside the nominal file
                                (btag, ele, muon, hlt, puWeight)
  3. THEORETICAL systematics  – array branches (LHEPdfWeight, PSWeight, …)
                                normalised by their companion sumw branches
  4. TW-SCHEME systematics    – compare each TW/ scheme variation against a
                                merged nominal built on-the-fly by summing
                                TWminusto2L2Nu_nominal + TbarWplusto2L2Nu_nominal

For each pair a 2×2 figure is saved:
  Row 1: raw (unweighted)   |  ratio to nominal
  Row 2: centrally weighted |  ratio to nominal

Usage:
    python3 compare_systemetics.py --year 2022
    python3 compare_systemetics.py --year 2023
    python3 compare_systemetics.py --year 2022 --process TTbar_Dilept
    python3 compare_systemetics.py --year 2022 --tw-only
"""

import argparse
import sys
import os
import glob
import gc
import re
import numpy as np
import uproot
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# ─── constants that do NOT depend on year ─────────────────────────────────────

TREE_NAME  = "outputTree"
VARIABLE   = "leading_lepton_pt"
BIN_RANGE  = (0, 300, 30)
XLABEL     = r"Leading Lepton $p_T$ [GeV]"
CHUNK      = 200_000
OUTPUT_DIR = "raw_vs_weighted_plots_22"

REGIONS = {
    "region_1j1t": "1j1b",
    "region_2j1t": "2j1b",
    "region_2j2t": "2j2b",
}

# Central weight branches (the baseline product used for "weighted" histograms)
CENTRAL_WEIGHT_BRANCHES = [
    "no_puWeight", "muon_SF_central", "puWeight",
    "ele_SF_central", "btag_SF_bcflav_central", "btag_SF_lflav_central", "hlt_sf",
]

# ── SF-branch systematics ──────────────────────────────────────────────────────
SF_BRANCH_SYSTEMATICS = [
    ("btag_bcflav_corr",
     "btag_SF_bcflav_central",
     "btag_SF_bcflav_up_correlated",
     "btag_SF_bcflav_down_correlated"),
    ("btag_bcflav_uncorr",
     "btag_SF_bcflav_central",
     "btag_SF_bcflav_up_uncorrelated",
     "btag_SF_bcflav_down_uncorrelated"),
    ("btag_lflav_corr",
     "btag_SF_lflav_central",
     "btag_SF_lflav_up_correlated",
     "btag_SF_lflav_down_correlated"),
    ("btag_lflav_uncorr",
     "btag_SF_lflav_central",
     "btag_SF_lflav_up_uncorrelated",
     "btag_SF_lflav_down_uncorrelated"),
    ("ele_SF",
     "ele_SF_central",
     "ele_SF_up",
     "ele_SF_down"),
    ("muon_SF_syst",
     "muon_SF_central",
     "muon_SF_up",
     "muon_SF_down"),
    ("hlt_sf",
     "hlt_sf",
     "hlt_sf_up",
     "hlt_sf_down"),
    ("puWeight",
     "puWeight",
     "puWeight_up",
     "puWeight_down"),
]

# ── topPtWeight systematic (TTbar_Dilept only) ─────────────────────────────────
TOP_PT_SF_SYSTEMATIC = ("topPtWeight", None, "topPtWeight", None)
TOP_PT_WEIGHT_BRANCH = "topPtWeight"

# ── Theoretical array-branch systematics ──────────────────────────────────────
THEORY_SYSTEMATICS = [
    ("LHEPdfWeight",    "LHEPdfWeight",   "LHEPdfSumw",   None),
    ("PSWeight_ISR_up",   "PSWeight", "PSSumw",  0),
    ("PSWeight_ISR_down", "PSWeight", "PSSumw",  1),
    ("PSWeight_FSR_up",   "PSWeight", "PSSumw",  2),
    ("PSWeight_FSR_down", "PSWeight", "PSSumw",  3),
    ("LHEScaleWeight", "LHEScaleWeight", "LHEScaleSumw", None),
]

# ── TW scheme systematics ──────────────────────────────────────────────────────
# The nominal is built on-the-fly by summing histograms from the two
# component directories (TWminusto2L2Nu and TbarWplusto2L2Nu).
# Each entry in TW_SCHEME_VARIATIONS is a (label, filename_stem) pair;
# the file is looked up as:
#   <base_dir>/TW/<filename_stem>_nominal.root
TW_MINUS_DIR   = "TWminusto2L2Nu"
TBAR_WPLUS_DIR = "TbarWplusto2L2Nu"

# Nominal file stems inside those directories
TW_MINUS_STEM   = "TWminusto2L2Nu_nominal"
TBAR_WPLUS_STEM = "TbarWplusto2L2Nu_nominal"

# The six scheme variations in TW/
TW_SCHEME_VARIATIONS = [
    ("DR1",      "TWto2L2Nu-DR1_nominal"),
    ("DR2",      "TWto2L2Nu-DR2_nominal"),
    ("DS-FS",    "TWto2L2Nu-DS-FS_nominal"),
    ("DS-IS",    "TWto2L2Nu-DS-IS_nominal"),
    ("DS-FS-BW", "TWto2L2Nu-DS-FS-BW_nominal"),
    ("DS-IS-BW", "TWto2L2Nu-DS-IS-BW_nominal"),
]

# ── TTbar pattern ──────────────────────────────────────────────────────────────
TTBAR_DILEPT_PATTERN = re.compile(r"(?i)ttbar.*dilept|dilept.*ttbar")

BINS    = np.linspace(BIN_RANGE[0], BIN_RANGE[1], BIN_RANGE[2] + 1)
CENTERS = 0.5 * (BINS[:-1] + BINS[1:])
WIDTHS  = BINS[1:] - BINS[:-1]
NB      = len(BINS) - 1


# ─── argument parsing ─────────────────────────────────────────────────────────

def parse_args():
    parser = argparse.ArgumentParser(
        description="Compare systematic variations (raw vs weighted).")
    parser.add_argument(
        "--year", required=True,
        help="Data-taking year, e.g. 2022 or 2023")
    parser.add_argument(
        "--process", default=None,
        help="Optional: run only this process sub-directory")
    parser.add_argument(
        "--tw-only", action="store_true",
        help="Run only the TW-scheme comparison, skip all other processes")
    return parser.parse_args()


def build_paths(year):
    base_dir = f"/eos/uscms/store/user/vsinha/Result_{year}/systemetics/"
    up_suffixes   = ("Up",   f"Up_{year}",   "up",   f"up_{year}")
    down_suffixes = ("Down", f"Down_{year}", "dn",   f"dn_{year}",
                     "down", f"down_{year}")
    return base_dir, up_suffixes, down_suffixes


# ─── helper: syst-label from file path ────────────────────────────────────────

def _syst_label(path, process, up_suf, down_suf):
    stem = os.path.splitext(os.path.basename(path))[0]
    if stem == process or stem == f"{process}_nominal":
        return "nominal"
    label = stem[len(process):].lstrip("_") or "nominal"
    return label


def _base_and_direction(label, up_suf, down_suf):
    for suf in up_suf:
        if label.endswith(suf):
            return label[:len(label) - len(suf)].rstrip("_"), "up"
    for suf in down_suf:
        if label.endswith(suf):
            return label[:len(label) - len(suf)].rstrip("_"), "down"
    return label, None


def _pair_systematics(all_labels, up_suf, down_suf):
    pairs = {}
    for sl in sorted(all_labels):
        base, direction = _base_and_direction(sl, up_suf, down_suf)
        if direction is None:
            pairs[sl] = {"up": sl, "down": None, "one_sided": True}
        else:
            if base not in pairs:
                pairs[base] = {"up": None, "down": None, "one_sided": False}
            pairs[base][direction] = sl
    return pairs


# ─── histogram builders ───────────────────────────────────────────────────────

def read_hist(filepath, region_branch, weighted=True, top_pt=False,
              replace_sf=None):
    h = np.zeros(NB, dtype=np.float64)
    try:
        with uproot.open(filepath) as f:
            keys = [k.split(";")[0] for k in f.keys()]
            if TREE_NAME not in keys:
                return h
            tree  = f[TREE_NAME]
            tkeys = set(tree.keys())
            if VARIABLE not in tkeys or region_branch not in tkeys:
                return h

            if weighted:
                w_branches = list(CENTRAL_WEIGHT_BRANCHES)
                if replace_sf is not None:
                    central_b, replacement_b = replace_sf
                    w_branches = [
                        replacement_b if b == central_b else b
                        for b in w_branches
                    ]
                if top_pt:
                    w_branches.append(TOP_PT_WEIGHT_BRANCH)
                avail_w = [b for b in w_branches if b in tkeys]
            else:
                avail_w = []

            read_cols = list({VARIABLE, region_branch} | set(avail_w))

            for chunk in tree.iterate(read_cols, step_size=CHUNK, library="np"):
                mask = np.asarray(chunk[region_branch], dtype=bool)
                if not mask.any():
                    del chunk; continue

                vals = np.asarray(chunk[VARIABLE], dtype=np.float64)[mask]

                if avail_w:
                    w = np.ones(mask.sum(), dtype=np.float64)
                    for wb in avail_w:
                        arr = np.asarray(chunk[wb], dtype=np.float64)[mask]
                        bad = ~np.isfinite(arr)
                        if bad.any():
                            arr = arr.copy(); arr[bad] = 1.0
                        w *= arr
                else:
                    w = None

                hi, _ = np.histogram(vals, bins=BINS, weights=w)
                h += hi
                del chunk; gc.collect()
    except Exception as e:
        print(f"  ERROR reading {os.path.basename(filepath)}: {e}")
    return h


def read_hist_toppt(filepath, region_branch):
    h_up   = read_hist(filepath, region_branch, weighted=True,  top_pt=True)
    h_down = read_hist(filepath, region_branch, weighted=True,  top_pt=False)
    return h_up, h_down


def read_hist_summed(filepaths, region_branch, weighted=True):
    """
    Sum histograms from multiple ROOT files into one array.
    Used to merge TWminusto2L2Nu + TbarWplusto2L2Nu into the tW nominal.
    """
    h_total = np.zeros(NB, dtype=np.float64)
    for fp in filepaths:
        if fp is None or not os.path.isfile(fp):
            print(f"  WARNING: file not found, skipping contribution: {fp}")
            continue
        h_total += read_hist(fp, region_branch, weighted=weighted, top_pt=False)
    return h_total


def read_theory_hist(filepath, region_branch, theory_branch, sumw_branch,
                     index, top_pt=False):
    single = (index is not None)
    h_list = None

    try:
        with uproot.open(filepath) as f:
            keys = [k.split(";")[0] for k in f.keys()]
            if TREE_NAME not in keys:
                return (np.zeros(NB) if single else [])
            tree  = f[TREE_NAME]
            tkeys = set(tree.keys())

            if VARIABLE not in tkeys or region_branch not in tkeys:
                return (np.zeros(NB) if single else [])
            if theory_branch not in tkeys:
                return (np.zeros(NB) if single else [])

            avail_cw = [b for b in CENTRAL_WEIGHT_BRANCHES if b in tkeys]
            if top_pt and TOP_PT_WEIGHT_BRANCH in tkeys:
                avail_cw = avail_cw + [TOP_PT_WEIGHT_BRANCH]

            read_cols = list({VARIABLE, region_branch, theory_branch} | set(avail_cw))
            if sumw_branch and sumw_branch in tkeys:
                read_cols.append(sumw_branch)

            for chunk in tree.iterate(read_cols, step_size=CHUNK, library="np"):
                mask = np.asarray(chunk[region_branch], dtype=bool)
                if not mask.any():
                    del chunk; continue

                vals = np.asarray(chunk[VARIABLE], dtype=np.float64)[mask]

                w_central = np.ones(mask.sum(), dtype=np.float64)
                for wb in avail_cw:
                    arr = np.asarray(chunk[wb], dtype=np.float64)[mask]
                    bad = ~np.isfinite(arr)
                    if bad.any():
                        arr = arr.copy(); arr[bad] = 1.0
                    w_central *= arr

                raw_th = chunk[theory_branch]
                try:
                    th_arr = np.asarray(raw_th, dtype=np.float64)
                except (ValueError, TypeError):
                    th_arr = np.array([np.asarray(row, dtype=np.float64)
                                       for row in raw_th])

                th_arr = th_arr[mask]
                if th_arr.ndim == 1:
                    th_arr = th_arr[:, np.newaxis]

                n_wts = th_arr.shape[1]

                sumw_arr = None
                if sumw_branch and sumw_branch in tkeys:
                    raw_sw = chunk[sumw_branch]
                    try:
                        sw = np.asarray(raw_sw, dtype=np.float64)
                    except (ValueError, TypeError):
                        sw = np.array([np.asarray(r, dtype=np.float64)
                                       for r in raw_sw])
                    sw = sw[mask]
                    if sw.ndim == 1 and sw.shape[0] == mask.sum():
                        sumw_arr = sw[:, np.newaxis]
                    else:
                        sumw_arr = sw

                if h_list is None:
                    if single:
                        h_list = np.zeros(NB, dtype=np.float64)
                    else:
                        h_list = [np.zeros(NB, dtype=np.float64)
                                  for _ in range(n_wts)]

                indices_to_fill = [index] if single else range(n_wts)
                for i in indices_to_fill:
                    if i >= n_wts:
                        continue
                    wt = th_arr[:, i].copy()
                    bad = ~np.isfinite(wt)
                    if bad.any():
                        wt[bad] = 1.0

                    if sumw_arr is not None:
                        if sumw_arr.ndim == 2:
                            sw_i = sumw_arr[:, min(i, sumw_arr.shape[1]-1)]
                        elif hasattr(sumw_arr, '__len__'):
                            sw_i = sumw_arr[min(i, len(sumw_arr)-1)]
                        else:
                            sw_i = float(sumw_arr)
                        sw_safe = np.where(np.abs(sw_i) > 0, sw_i, 1.0)
                        wt = wt / sw_safe

                    w_final = w_central * wt
                    hi, _ = np.histogram(vals, bins=BINS, weights=w_final)

                    if single:
                        h_list += hi
                    else:
                        h_list[i] += hi

                del chunk; gc.collect()

    except Exception as e:
        print(f"  ERROR (theory) reading {os.path.basename(filepath)}: {e}")

    if h_list is None:
        return np.zeros(NB) if single else []
    return h_list


# ─── plotting ─────────────────────────────────────────────────────────────────

def _print_ratio(tag, h_var, nom_int, label):
    if h_var is not None and nom_int > 0:
        r = h_var.sum() / nom_int
        delta = 100 * (h_var.sum() - nom_int) / nom_int
        print(f"    {tag:22s}  {label:10s} = {r:.4f}  ({delta:+.2f}%)")


def make_pair_plot(
    base,
    up_label, down_label,
    nom_raw, up_raw, down_raw,
    nom_wt,  up_wt,  down_wt,
    region_label, outpath,
    one_sided=False,
):
    fig, axes = plt.subplots(2, 2, figsize=(13, 8),
                             gridspec_kw={"width_ratios": [3, 1.5]})
    title_sfx = "  [one-sided]" if one_sided else ""
    fig.suptitle(f"{base}{title_sfx}  [{region_label}]",
                 fontsize=13, fontweight="bold")

    for row, (nom, h_up, h_down, tag) in enumerate([
        (nom_raw, up_raw, down_raw, "Raw (no weights)"),
        (nom_wt,  up_wt,  down_wt,  "Weighted"),
    ]):
        ax_m = axes[row, 0]
        ax_r = axes[row, 1]

        ax_m.step(np.append(BINS[:-1], BINS[-1]),
                  np.append(nom, nom[-1]),
                  where="post", color="black", linewidth=2, label="Nominal")

        if h_up is not None:
            lbl = up_label if up_label else "Up"
            ax_m.step(np.append(BINS[:-1], BINS[-1]),
                      np.append(h_up, h_up[-1]),
                      where="post", color="red", linewidth=1.5,
                      linestyle="-", label=f"Up ({lbl})")
        if h_down is not None:
            lbl = down_label if down_label else "Down"
            ax_m.step(np.append(BINS[:-1], BINS[-1]),
                      np.append(h_down, h_down[-1]),
                      where="post", color="blue", linewidth=1.5,
                      linestyle="--", label=f"Down ({lbl})")

        ax_m.set_ylabel("Events / bin", fontsize=10)
        ax_m.set_xlim(BINS[0], BINS[-1])
        ax_m.set_ylim(bottom=0)
        ax_m.legend(fontsize=8)
        ax_m.set_title(tag, fontsize=10)
        ax_m.set_xlabel(XLABEL, fontsize=9)

        with np.errstate(divide="ignore", invalid="ignore"):
            if h_up is not None:
                r_up = np.where(nom > 0, h_up / nom, np.nan)
                ax_r.step(np.append(BINS[:-1], BINS[-1]),
                          np.append(r_up, r_up[-1]),
                          where="post", color="red", linewidth=1.5, label="Up/Nom")
            if h_down is not None:
                r_dn = np.where(nom > 0, h_down / nom, np.nan)
                ax_r.step(np.append(BINS[:-1], BINS[-1]),
                          np.append(r_dn, r_dn[-1]),
                          where="post", color="blue", linewidth=1.5,
                          linestyle="--", label="Down/Nom")

        ax_r.axhline(1.0, color="black", linewidth=0.8, linestyle=":")
        ax_r.set_ylim(0.5, 1.5)
        ax_r.yaxis.set_major_locator(ticker.MultipleLocator(0.1))
        ax_r.set_ylabel("Syst / Nom", fontsize=9)
        ax_r.set_xlabel(XLABEL, fontsize=9)
        ax_r.set_xlim(BINS[0], BINS[-1])
        ax_r.legend(fontsize=8)
        ax_r.set_title(f"{tag}  ratio", fontsize=10)

        nom_int = nom.sum()
        _print_ratio(tag, h_up,   nom_int, "Up/Nom  ")
        _print_ratio(tag, h_down, nom_int, "Down/Nom")

    fig.tight_layout()
    fig.savefig(outpath, bbox_inches="tight", dpi=130)
    plt.close(fig)


def make_theory_envelope_plot(base, nom_wt, hist_list, region_label, outpath):
    if not hist_list:
        return
    arr = np.vstack(hist_list)
    h_max = arr.max(axis=0)
    h_min = arr.min(axis=0)
    rms   = np.sqrt(np.mean((arr - nom_wt[np.newaxis, :]) ** 2, axis=0))

    fig, (ax_m, ax_r) = plt.subplots(1, 2, figsize=(13, 5),
                                      gridspec_kw={"width_ratios": [3, 1.5]})
    fig.suptitle(f"{base}  [{region_label}]  — theory envelope",
                 fontsize=12, fontweight="bold")

    x = np.append(BINS[:-1], BINS[-1])

    ax_m.step(x, np.append(nom_wt, nom_wt[-1]),
              where="post", color="black", linewidth=2, label="Nominal")
    ax_m.fill_between(CENTERS, h_min, h_max,
                      step="mid", alpha=0.35, color="orange", label="Envelope")
    ax_m.fill_between(CENTERS, nom_wt - rms, nom_wt + rms,
                      step="mid", alpha=0.30, color="green", label="RMS band")
    ax_m.set_ylabel("Events / bin", fontsize=10)
    ax_m.set_xlim(BINS[0], BINS[-1]); ax_m.set_ylim(bottom=0)
    ax_m.legend(fontsize=8); ax_m.set_xlabel(XLABEL, fontsize=9)

    with np.errstate(divide="ignore", invalid="ignore"):
        r_max = np.where(nom_wt > 0, h_max / nom_wt, np.nan)
        r_min = np.where(nom_wt > 0, h_min / nom_wt, np.nan)
    ax_r.fill_between(CENTERS, r_min, r_max,
                      step="mid", alpha=0.35, color="orange", label="Envelope")
    ax_r.axhline(1.0, color="black", linewidth=0.8, linestyle=":")
    ax_r.set_ylim(0.5, 1.5)
    ax_r.yaxis.set_major_locator(ticker.MultipleLocator(0.1))
    ax_r.set_ylabel("Var / Nom", fontsize=9)
    ax_r.set_xlabel(XLABEL, fontsize=9)
    ax_r.set_xlim(BINS[0], BINS[-1]); ax_r.legend(fontsize=8)

    fig.tight_layout()
    fig.savefig(outpath, bbox_inches="tight", dpi=130)
    plt.close(fig)


def make_theory_per_index_plots(base, nom_wt, hist_list, region_label, out_dir):
    if not hist_list:
        return

    n_indices = len(hist_list)
    print(f"      → writing {n_indices} per-index plots for {base} in {out_dir}")

    for idx, h_var in enumerate(hist_list):
        label   = f"{base}_idx{idx:03d}"
        outpath = os.path.join(out_dir, f"{label}.pdf")

        fig, axes = plt.subplots(1, 2, figsize=(13, 4),
                                 gridspec_kw={"width_ratios": [3, 1.5]})
        fig.suptitle(f"{base}  index {idx}  [{region_label}]",
                     fontsize=12, fontweight="bold")

        ax_m, ax_r = axes

        ax_m.step(np.append(BINS[:-1], BINS[-1]),
                  np.append(nom_wt, nom_wt[-1]),
                  where="post", color="black", linewidth=2, label="Nominal")
        ax_m.step(np.append(BINS[:-1], BINS[-1]),
                  np.append(h_var, h_var[-1]),
                  where="post", color="red", linewidth=1.5,
                  linestyle="-", label=f"Index {idx}")
        ax_m.set_ylabel("Events / bin", fontsize=10)
        ax_m.set_xlim(BINS[0], BINS[-1])
        ax_m.set_ylim(bottom=0)
        ax_m.legend(fontsize=8)
        ax_m.set_title("Weighted", fontsize=10)
        ax_m.set_xlabel(XLABEL, fontsize=9)

        with np.errstate(divide="ignore", invalid="ignore"):
            ratio = np.where(nom_wt > 0, h_var / nom_wt, np.nan)
        ax_r.step(np.append(BINS[:-1], BINS[-1]),
                  np.append(ratio, ratio[-1]),
                  where="post", color="red", linewidth=1.5, label="Var/Nom")
        ax_r.axhline(1.0, color="black", linewidth=0.8, linestyle=":")
        ax_r.set_ylim(0.5, 1.5)
        ax_r.yaxis.set_major_locator(ticker.MultipleLocator(0.1))
        ax_r.set_ylabel("Var / Nom", fontsize=9)
        ax_r.set_xlabel(XLABEL, fontsize=9)
        ax_r.set_xlim(BINS[0], BINS[-1])
        ax_r.legend(fontsize=8)
        ax_r.set_title("Ratio", fontsize=10)

        if nom_wt.sum() > 0:
            r_val  = h_var.sum() / nom_wt.sum()
            delta  = 100 * (h_var.sum() - nom_wt.sum()) / nom_wt.sum()
            print(f"      idx {idx:3d}  Var/Nom = {r_val:.4f}  ({delta:+.2f}%)")

        fig.tight_layout()
        fig.savefig(outpath, bbox_inches="tight", dpi=130)
        plt.close(fig)


def make_tw_overlay_plot(
    nom_raw, nom_wt,
    var_hists_raw, var_hists_wt,
    var_labels,
    region_label, outpath,
):
    """
    2×2 overlay: all six TW scheme variations on the same axes,
    plus ratio panels showing each scheme / merged-nominal.

    Row 0: Raw (unweighted)   | ratio to merged nominal
    Row 1: Centrally weighted | ratio to merged nominal
    """
    # Colour palette — one per scheme variation
    COLORS = ["tab:blue", "tab:orange", "tab:green",
              "tab:red",  "tab:purple", "tab:brown"]

    fig, axes = plt.subplots(2, 2, figsize=(14, 9),
                             gridspec_kw={"width_ratios": [3, 1.5]})
    fig.suptitle(f"tW scheme comparison  [{region_label}]",
                 fontsize=13, fontweight="bold")

    x = np.append(BINS[:-1], BINS[-1])

    for row, (nom, var_hists, tag) in enumerate([
        (nom_raw, var_hists_raw, "Raw (no weights)"),
        (nom_wt,  var_hists_wt,  "Weighted"),
    ]):
        ax_m = axes[row, 0]
        ax_r = axes[row, 1]

        # Merged nominal reference
        ax_m.step(x, np.append(nom, nom[-1]),
                  where="post", color="black", linewidth=2.0,
                  label="Nominal (TWminus + TbarWplus)")

        nom_int = nom.sum()

        for (label, h_var), color in zip(var_hists.items(), COLORS):
            ax_m.step(x, np.append(h_var, h_var[-1]),
                      where="post", color=color, linewidth=1.4,
                      linestyle="-", label=label)

            with np.errstate(divide="ignore", invalid="ignore"):
                ratio = np.where(nom > 0, h_var / nom, np.nan)
            ax_r.step(x, np.append(ratio, ratio[-1]),
                      where="post", color=color, linewidth=1.4,
                      linestyle="-", label=label)

            if nom_int > 0:
                delta = 100 * (h_var.sum() - nom_int) / nom_int
                print(f"    {tag:22s}  {label:12s}  "
                      f"Var/Nom = {h_var.sum()/nom_int:.4f}  ({delta:+.2f}%)")

        ax_m.set_ylabel("Events / bin", fontsize=10)
        ax_m.set_xlim(BINS[0], BINS[-1])
        ax_m.set_ylim(bottom=0)
        ax_m.legend(fontsize=7, ncol=2)
        ax_m.set_title(tag, fontsize=10)
        ax_m.set_xlabel(XLABEL, fontsize=9)

        ax_r.axhline(1.0, color="black", linewidth=0.8, linestyle=":")
        ax_r.set_ylim(0.5, 1.5)
        ax_r.yaxis.set_major_locator(ticker.MultipleLocator(0.1))
        ax_r.set_ylabel("Scheme / Nom", fontsize=9)
        ax_r.set_xlabel(XLABEL, fontsize=9)
        ax_r.set_xlim(BINS[0], BINS[-1])
        ax_r.legend(fontsize=7, ncol=2)
        ax_r.set_title(f"{tag}  ratio", fontsize=10)

    fig.tight_layout()
    fig.savefig(outpath, bbox_inches="tight", dpi=130)
    plt.close(fig)


# ─── TW-scheme processing ─────────────────────────────────────────────────────

def process_tw_schemes(base_dir):
    """
    Build the merged tW nominal on-the-fly (sum of TWminus + TbarWplus),
    then compare each of the six TW/ scheme files against it.

    Produces:
      <OUTPUT_DIR>/TW_schemes/<region>/tW_scheme_comparison.pdf
        – overlay of all six schemes on one canvas (2×2: raw/weighted × abs/ratio)
      <OUTPUT_DIR>/TW_schemes/<region>/<scheme_label>.pdf
        – individual 2×2 pair-plot for each scheme vs merged nominal
    """
    print("\n" + "="*70)
    print("  TW scheme systematics")
    print("="*70)

    # Locate the two nominal component files
    tw_minus_path = os.path.join(
        base_dir, TW_MINUS_DIR, f"{TW_MINUS_STEM}.root")
    tbar_wplus_path = os.path.join(
        base_dir, TBAR_WPLUS_DIR, f"{TBAR_WPLUS_STEM}.root")

    missing = []
    for p in (tw_minus_path, tbar_wplus_path):
        if not os.path.isfile(p):
            missing.append(p)
    if missing:
        print("  ERROR: cannot find nominal component file(s):")
        for m in missing:
            print(f"    {m}")
        print("  Skipping TW scheme systematics.")
        return

    print(f"  Nominal components:")
    print(f"    tW-  : {tw_minus_path}")
    print(f"    tbar W+: {tbar_wplus_path}")

    # Locate the six TW/ scheme files
    tw_dir = os.path.join(base_dir, "TW")
    scheme_paths = {}
    for label, stem in TW_SCHEME_VARIATIONS:
        fp = os.path.join(tw_dir, f"{stem}.root")
        if os.path.isfile(fp):
            scheme_paths[label] = fp
            print(f"  Found scheme {label:12s}: {fp}")
        else:
            print(f"  WARNING: scheme file not found, skipping {label}: {fp}")

    if not scheme_paths:
        print("  No TW scheme files found. Skipping.")
        return

    nominal_components = [tw_minus_path, tbar_wplus_path]

    for rkey, rlabel in REGIONS.items():
        out_dir = os.path.join(OUTPUT_DIR, "TW_schemes", rlabel)
        os.makedirs(out_dir, exist_ok=True)

        print(f"\n  Region: {rlabel}")

        # ── Build merged nominal (histogram sum, NOT hadd) ────────────────
        nom_raw = read_hist_summed(nominal_components, rkey, weighted=False)
        nom_wt  = read_hist_summed(nominal_components, rkey, weighted=True)

        if nom_raw.sum() == 0 and nom_wt.sum() == 0:
            print(f"    WARNING: merged nominal is empty for region {rlabel}, skipping.")
            continue

        # ── Read histograms for each scheme variation ─────────────────────
        var_hists_raw = {}
        var_hists_wt  = {}
        for label, fp in scheme_paths.items():
            var_hists_raw[label] = read_hist(fp, rkey, weighted=False)
            var_hists_wt[label]  = read_hist(fp, rkey, weighted=True)

        # ── Overlay plot: all schemes on one canvas ───────────────────────
        overlay_path = os.path.join(out_dir, "tW_scheme_comparison.pdf")
        print(f"    Writing overlay: {overlay_path}")
        make_tw_overlay_plot(
            nom_raw, nom_wt,
            var_hists_raw, var_hists_wt,
            list(scheme_paths.keys()),
            rlabel, overlay_path,
        )

        # ── Individual pair-plot for each scheme ──────────────────────────
        for label in scheme_paths:
            h_raw = var_hists_raw[label]
            h_wt  = var_hists_wt[label]
            safe  = label.replace("/", "_").replace(" ", "_")
            indiv_path = os.path.join(out_dir, f"{safe}.pdf")
            print(f"    Writing individual: {indiv_path}")
            make_pair_plot(
                f"tW  {label}",
                label, None,            # only "up" (the scheme); no "down"
                nom_raw, h_raw, None,   # raw
                nom_wt,  h_wt,  None,   # weighted
                rlabel, indiv_path,
                one_sided=True,         # each scheme is one-sided vs nominal
            )


# ─── per-process logic ────────────────────────────────────────────────────────

def process_one_dir(proc_dir, up_suf, down_suf):
    process    = os.path.basename(proc_dir)
    is_ttbar   = bool(TTBAR_DILEPT_PATTERN.search(process))
    root_files = sorted(glob.glob(os.path.join(proc_dir, "*.root")))
    if not root_files:
        print(f"  No ROOT files in {proc_dir}, skipping.")
        return

    nom_path   = None
    syst_files = {}
    for fp in root_files:
        sl = _syst_label(fp, process, up_suf, down_suf)
        if sl == "nominal":
            nom_path = fp
        else:
            syst_files[sl] = fp

    if nom_path is None:
        print(f"  No nominal file in {proc_dir}, skipping.")
        return

    pairs = _pair_systematics(set(syst_files.keys()), up_suf, down_suf)
    n_one_sided = sum(1 for v in pairs.values() if v["one_sided"])
    print(f"  {process:35s}  "
          f"nominal + {len(syst_files)} systs -> "
          f"{len(pairs)} nuisances  "
          f"({n_one_sided} one-sided)")

    for rkey, rlabel in REGIONS.items():

        out_file_dir   = os.path.join(OUTPUT_DIR, process, rlabel, "file_systs")
        out_sf_dir     = os.path.join(OUTPUT_DIR, process, rlabel, "sf_systs")
        out_theory_dir = os.path.join(OUTPUT_DIR, process, rlabel, "theory_systs")
        for d in (out_file_dir, out_sf_dir, out_theory_dir):
            os.makedirs(d, exist_ok=True)

        nom_raw = read_hist(nom_path, rkey, weighted=False, top_pt=False)
        nom_wt  = read_hist(nom_path, rkey, weighted=True,  top_pt=is_ttbar)

        # ═══════════════════════════════════════════════════════════════════
        # 1. FILE-BASED systematics
        # ═══════════════════════════════════════════════════════════════════
        for base, sides in sorted(pairs.items()):
            up_sl   = sides["up"]
            down_sl = sides["down"]
            one_sd  = sides["one_sided"]

            up_raw = down_raw = up_wt = down_wt = None
            if up_sl:
                up_raw = read_hist(syst_files[up_sl], rkey, weighted=False)
                up_wt  = read_hist(syst_files[up_sl], rkey, weighted=True,
                                   top_pt=is_ttbar)
            if down_sl:
                down_raw = read_hist(syst_files[down_sl], rkey, weighted=False)
                down_wt  = read_hist(syst_files[down_sl], rkey, weighted=True,
                                     top_pt=is_ttbar)

            safe    = base.replace("/", "_").replace(" ", "_")
            outpath = os.path.join(out_file_dir, f"{safe}.pdf")
            print(f"    [file] {base}  ({rlabel})")
            make_pair_plot(
                base, up_sl, down_sl,
                nom_raw, up_raw, down_raw,
                nom_wt,  up_wt,  down_wt,
                rlabel, outpath, one_sided=one_sd,
            )

        # ═══════════════════════════════════════════════════════════════════
        # 2. SF-BRANCH systematics
        # ═══════════════════════════════════════════════════════════════════
        for (sf_base, central_b, up_b, down_b) in SF_BRANCH_SYSTEMATICS:
            print(f"    [sf  ] {sf_base}  ({rlabel})")

            up_wt = down_wt = None
            if up_b:
                up_wt = read_hist(
                    nom_path, rkey, weighted=True, top_pt=is_ttbar,
                    replace_sf=(central_b, up_b))
            if down_b:
                down_wt = read_hist(
                    nom_path, rkey, weighted=True, top_pt=is_ttbar,
                    replace_sf=(central_b, down_b))

            safe    = sf_base.replace("/", "_")
            outpath = os.path.join(out_sf_dir, f"{safe}.pdf")
            make_pair_plot(
                sf_base, up_b, down_b,
                nom_raw, nom_raw, nom_raw,
                nom_wt,  up_wt,  down_wt,
                rlabel, outpath,
            )

        # ── topPtWeight (TTbar_Dilept only) ──────────────────────────────
        if is_ttbar:
            print(f"    [sf  ] topPtWeight  ({rlabel})")
            h_toppt_with, h_toppt_without = read_hist_toppt(nom_path, rkey)
            outpath = os.path.join(out_sf_dir, "topPtWeight.pdf")
            make_pair_plot(
                "topPtWeight",
                "with topPtWeight", "without topPtWeight",
                nom_raw, nom_raw, nom_raw,
                nom_wt,  h_toppt_with, h_toppt_without,
                rlabel, outpath, one_sided=False,
            )

        # ═══════════════════════════════════════════════════════════════════
        # 3. THEORETICAL systematics
        # ═══════════════════════════════════════════════════════════════════
        for (th_name, th_branch, sumw_branch, th_index) in THEORY_SYSTEMATICS:
            print(f"    [th  ] {th_name}  ({rlabel})")
            safe    = th_name.replace("/", "_")
            outpath = os.path.join(out_theory_dir, f"{safe}.pdf")

            if th_index is None:
                hist_list = read_theory_hist(
                    nom_path, rkey, th_branch, sumw_branch,
                    index=None, top_pt=is_ttbar)
                make_theory_envelope_plot(
                    th_name, nom_wt, hist_list, rlabel, outpath)
                per_idx_dir = os.path.join(out_theory_dir, f"{safe}_per_index")
                os.makedirs(per_idx_dir, exist_ok=True)
                make_theory_per_index_plots(
                    th_name, nom_wt, hist_list, rlabel, per_idx_dir)
            else:
                h_var  = read_theory_hist(
                    nom_path, rkey, th_branch, sumw_branch,
                    index=th_index, top_pt=is_ttbar)
                is_up   = "up"   in th_name.lower()
                is_down = "down" in th_name.lower()
                up_h   = h_var if (is_up   or (not is_up and not is_down)) else None
                down_h = h_var if  is_down else None
                make_pair_plot(
                    th_name, th_name if up_h is not None else None,
                    th_name if down_h is not None else None,
                    nom_raw, up_h if up_h is not None else None,
                    down_h if down_h is not None else None,
                    nom_wt,  up_h if up_h is not None else None,
                    down_h if down_h is not None else None,
                    rlabel, outpath,
                )


# ─── main ─────────────────────────────────────────────────────────────────────

def main():
    args = parse_args()
    year = args.year

    base_dir, up_suf, down_suf = build_paths(year)

    if not os.path.isdir(base_dir):
        print(f"ERROR: directory does not exist: {base_dir}")
        sys.exit(1)

    print(f"Year         : {year}")
    print(f"Input dir    : {base_dir}")
    print(f"Up suffixes  : {up_suf}")
    print(f"Down suffixes: {down_suf}")

    # ── TW scheme systematics (always run unless --process targets something else)
    if not args.process or args.tw_only:
        process_tw_schemes(base_dir)

    # ── Standard per-process systematics ─────────────────────────────────────
    if not args.tw_only:
        if args.process:
            proc_dirs = [os.path.join(base_dir, args.process)]
        else:
            proc_dirs = sorted([
                d for d in glob.glob(os.path.join(base_dir, "*"))
                if os.path.isdir(d)
                and os.path.basename(d) != "Data"
                and os.path.basename(d) != "logs"
                and not os.path.basename(d).startswith("temp")
                # Skip the TW component dirs — they are handled above
                and os.path.basename(d) not in ("TW", TW_MINUS_DIR, TBAR_WPLUS_DIR)
            ])

        if not proc_dirs:
            print(f"No process directories found under '{base_dir}'")
        else:
            print(f"\nFound {len(proc_dirs)} process directories.\n")
            for proc_dir in proc_dirs:
                process_one_dir(proc_dir, up_suf, down_suf)

    print(f"\nAll done. Plots saved under '{OUTPUT_DIR}/'")


if __name__ == "__main__":
    main()
