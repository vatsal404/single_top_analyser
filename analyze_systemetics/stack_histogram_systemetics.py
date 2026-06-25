"""
plot_stack_with_systematics.py
------------------------------
Produces data/MC stack plots with a total uncertainty band for each analysis
region AND each variable in PLOT_VARIABLES.

Usage:
    python3 stack_with_systemetics.py --year 2022
    python3 stack_with_systemetics.py --year 2022 --no-syst
    python3 stack_with_systemetics.py --year 2022 --process TTbar_Dilept
    python3 stack_with_systemetics.py --year 2022 --variable HT
    python3 stack_with_systemetics.py --year 2022 --jobs 8

Key changes vs previous version
---------------------------------
1. LHEScaleWeight: only indices [1,3,5,7] are used (independent MUF/MUR
   variations; excludes the correlated extremes [0] and [8]).
2. Theory systematics (LHEScaleWeight, LHEPdfWeight, PSWeight) are applied
   ONLY to TTbar_Dilept and single-tW processes; every other process skips
   them entirely.
3. A yields table (events ± stat ± syst ± total) per process-group per region
   is printed to stdout and saved as yields_<year>.txt after all plots finish.
"""

import argparse, sys, os, glob, gc, re
import numpy as np
import uproot
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
from collections import defaultdict
from concurrent.futures import ProcessPoolExecutor, as_completed

# ── kinematics / binning ──────────────────────────────────────────────────────
TREE_NAME  = "outputTree"
CHUNK      = 500_000
OUTPUT_DIR = "stack_plots"

PLOT_VARIABLES = [
    ("HT",                        r"$H_T$ [GeV]",                      (0, 350, 25)),
    ("ST",                        r"$S_T$ [GeV]",                      (0, 800, 25)),
    ("MT2",                       r"$M_{T2}$ [GeV]",                   (0, 150, 25)),
    ("centrality",                r"Centrality",                        (0, 1,   20)),
    ("sphericity",                r"Sphericity",                        (0, 1,   20)),
    ("aplanery",                  r"Aplanarity",                        (0, 0.25,20)),
    ("goodmuons_leading_pt",      r"Leading Muon $p_T$ [GeV]",         (0, 200, 25)),
    ("goodmuons_leading_eta",     r"Leading Muon $\eta$",              (-2.5,2.5,20)),
    ("goodmuons_leading_phi",     r"Leading Muon $\phi$",              (-3.2,3.2,20)),
    ("goodElectrons_leading_pt",  r"Leading Electron $p_T$ [GeV]",     (0, 200, 25)),
    ("goodElectrons_leading_eta", r"Leading Electron $\eta$",          (-2.5,2.5,20)),
    ("goodElectrons_leading_phi", r"Leading Electron $\phi$",          (-3.2,3.2,20)),
    ("leading_lepton_pt",         r"Leading Lepton $p_T$ [GeV]",       (0, 300, 30)),
    ("subleading_lepton_pt",      r"Subleading Lepton $p_T$ [GeV]",    (0, 200, 20)),
    ("lepton_pt_asymmetry",       r"Lepton $p_T$ Asymmetry",           (0, 1,   20)),
    ("leptons_invariant_mass",    r"Leptons Invariant Mass [GeV]",      (0, 300, 30)),
    ("mbl_min",                   r"$M_{bl}^{\rm min}$ [GeV]",         (0, 250, 25)),
    ("mbl_max",                   r"$M_{bl}^{\rm max}$ [GeV]",         (0, 500, 30)),
    ("dilepton_jet_pt",           r"Dilepton+Jet $p_T$ [GeV]",         (0, 300, 25)),
    ("dilepton_jet_mass",         r"Dilepton+Jet Mass [GeV]",          (0, 500, 25)),
    ("dilepton_del_phi",          r"Dilepton $\Delta\phi$",            (0, 3.2, 20)),
    ("Selected_jet_leading_pt",   r"Leading Jet $p_T$ [GeV]",          (0, 300, 25)),
    ("Selected_jet_leading_eta",  r"Leading Jet $\eta$",               (-2.5,2.5,20)),
    ("Selected_jet_leading_phi",  r"Leading Jet $\phi$",               (-3.2,3.2,20)),
    ("Selected_jet_subleading_pt",r"Subleading Jet $p_T$ [GeV]",       (0, 300, 25)),
    ("Selected_jet_subleading_eta",r"Subleading Jet $\eta$",           (-2.5,2.5,20)),
    ("Selected_jet_subleading_phi",r"Subleading Jet $\phi$",           (-3.2,3.2,20)),
    ("Selected_loosejet_leadingpt",r"Leading Loose Jet $p_T$ [GeV]",   (0, 30,  20)),
    ("Selected_bjet_leading_pt",  r"Leading b-Jet $p_T$ [GeV]",        (0, 300, 25)),
    ("Selected_bjet_leading_eta", r"Leading b-Jet $\eta$",             (-2.5,2.5,20)),
    ("Selected_bjet_leading_phi", r"Leading b-Jet $\phi$",             (-3.2,3.2,20)),
    ("leading_lepton_jet_pt",     r"Leading Lepton+Jet $p_T$ [GeV]",   (0, 400, 25)),
    ("delR_dilepton_jet",         r"$\Delta R$(Dilepton, Jet)",         (0, 6,   20)),
    ("delR_ele_muon",             r"$\Delta R$(e, $\mu$)",             (0, 6,   20)),
    ("delR_leadinglepton_jet",    r"$\Delta R$(Leading Lepton, Jet)",   (0, 6,   20)),
]

REGIONS = {
    "region_1j1t": "1j1b",
    "region_2j1t": "2j1b",
    "region_2j2t": "2j2b",
}

# ── process → colour group mapping ───────────────────────────────────────────
FILE_GROUPS = {
    "drell_yan": [
        "DYto2E-2Jets_MLL-10to50", "DYto2E-2Jets_MLL-50",
        "DYto2Mu-2Jets_MLL-10to50", "DYto2Mu-2Jets_MLL-50",
        "DYjetsM10to50", "DYjetsM50",
        "TTLL_MLL-50", "TTLL_MLL-4to50",
    ],
    "ttg_ttv": [
        "TTGJets_PTG-10to100", "TTGJets_PTG-100to200", "TTGJets_PTG-200",
        "TTLNu-1Jets", "TTZ-ZtoQQ-1Jets",
    ],
    "ttbar_dileptonic":   ["TTbar_Dilept"],
    "ttbar_semileptonic": ["TTbar_SemiLept"],
    "single_tW":          ["TWminusto2L2Nu", "TbarWplusto2L2Nu"],
    "vv": [
        "WWto2L2Nu", "WZto2L2Q", "WZto3LNu",
        "ZZto2L2Q", "ZZto2LNu", "ZZto4L",
    ],
    "wjets": ["WtoLNu", "WtoENu-2Jets", "WtoMuNu-2Jets"],
}

PROC_TO_GROUP = {
    proc: grp
    for grp, procs in FILE_GROUPS.items()
    for proc in procs
}

GROUP_COLORS = {
    "drell_yan":          "purple",
    "ttg_ttv":            "gray",
    "ttbar_dileptonic":   "red",
    "ttbar_semileptonic": "green",
    "single_tW":          "yellow",
    "vv":                 "saddlebrown",
    "wjets":              "steelblue",
}
GROUP_LABELS = {
    "drell_yan":          "Drell-Yan",
    "ttg_ttv":            r"$t\bar{t}\gamma\,/\,t\bar{t}V$",
    "ttbar_dileptonic":   r"$t\bar{t}$ dileptonic",
    "ttbar_semileptonic": r"$t\bar{t}$ semileptonic",
    "single_tW":          "Single tW",
    "vv":                 "Diboson",
    "wjets":              "W+jets",
}
STACK_ORDER = [
    "wjets", "vv", "single_tW", "ttbar_semileptonic",
    "ttg_ttv", "drell_yan", "ttbar_dileptonic",
]

# ── weights ───────────────────────────────────────────────────────────────────
CENTRAL_WEIGHT_BRANCHES = [
    "no_puWeight", "muon_SF_central", "puWeight",
    "ele_SF_central", "btag_SF_bcflav_central", "btag_SF_lflav_central", "hlt_sf",
]
TOP_PT_WEIGHT_BRANCH = "topPtWeight"
TTBAR_DILEPT_PATTERN = re.compile(r"(?i)ttbar.*dilept|dilept.*ttbar")

SF_BRANCH_SYSTEMATICS = [
    ("btag_SF_bcflav_central", "btag_SF_bcflav_up_correlated",   "btag_SF_bcflav_down_correlated"),
    ("btag_SF_bcflav_central", "btag_SF_bcflav_up_uncorrelated", "btag_SF_bcflav_down_uncorrelated"),
    ("btag_SF_lflav_central",  "btag_SF_lflav_up_correlated",    "btag_SF_lflav_down_correlated"),
    ("btag_SF_lflav_central",  "btag_SF_lflav_up_uncorrelated",  "btag_SF_lflav_down_uncorrelated"),
    ("ele_SF_central",         "ele_SF_up",                      "ele_SF_down"),
    ("muon_SF_central",        "muon_SF_up",                     "muon_SF_down"),
    ("hlt_sf",                 "hlt_sf_up",                      "hlt_sf_down"),
    ("puWeight",               "puWeight_up",                    "puWeight_down"),
]

_ALL_SF_BRANCHES: list[str] = list(dict.fromkeys(
    b for tup in SF_BRANCH_SYSTEMATICS for b in tup
))

# ── Theory systematics ────────────────────────────────────────────────────────
# Each entry: (branch_name, sumw_branch, indices_or_None)
#   indices_or_None : list of int → only those LHE weight indices are used;
#                     None         → use all indices (e.g. PSWeight, LHEPdfWeight)
#
# FIX 1: LHEScaleWeight uses only indices [1,3,5,7]
#   [1] MUF=1.0 MUR=0.5  – only renorm scale down
#   [3] MUF=0.5 MUR=1.0  – only fact  scale down
#   [5] MUF=2.0 MUR=1.0  – only fact  scale up
#   [7] MUF=1.0 MUR=2.0  – only renorm scale up
#   Excluded: [0],[8] (both scales simultaneously halved/doubled – over-counts),
#             [4] (nominal = 1), [2],[6] (anti-correlated extremes).
THEORY_SYSTEMATICS = [
    ("LHEPdfWeight",   "LHEPdfSumw",   None),
    ("LHEScaleWeight", "LHEScaleSumw", [1, 3, 5, 7]),   # <── FIX 1
    ("PSWeight",       "PSSumw",       None),
]

# FIX 2: Theory systs are physically meaningful only for TTbar dileptonic
#         and single-tW processes.  All other processes skip them.
THEORY_PROC_PATTERNS = re.compile(
    r"(?i)ttbar.*dilept|dilept.*ttbar|TWminusto2L2Nu|TbarWplusto2L2Nu"
)

# ── TW DR/DS systematic variant names ────────────────────────────────────────
TW_PROC_NAMES = {"TWminusto2L2Nu", "TbarWplusto2L2Nu"}


# ── CLI ───────────────────────────────────────────────────────────────────────
def parse_args():
    p = argparse.ArgumentParser()
    p.add_argument("--year",    required=True)
    p.add_argument("--process", default=None)
    p.add_argument("--variable", default=None)
    p.add_argument("--no-syst", action="store_true")
    p.add_argument("--jobs", type=int, default=4)
    return p.parse_args()

def build_paths(year):
    base   = f"/eos/uscms/store/user/vsinha/Result_{year}/systemetics/"
    up_suf = ("Up", f"Up_{year}", "up", f"up_{year}")
    dn_suf = ("Down", f"Down_{year}", "dn", f"dn_{year}", "down", f"down_{year}")
    return base, up_suf, dn_suf

def make_bins(bin_range):
    bins = np.linspace(bin_range[0], bin_range[1], bin_range[2] + 1)
    return bins, len(bins) - 1


# ── syst file pairing ─────────────────────────────────────────────────────────
def _syst_label(path, process, up_suf, dn_suf):
    stem = os.path.splitext(os.path.basename(path))[0]
    if stem in (process, f"{process}_nominal"):
        return "nominal"
    return stem[len(process):].lstrip("_") or "nominal"

def _base_and_dir(label, up_suf, dn_suf):
    for s in up_suf:
        if label.endswith(s):
            return label[:len(label)-len(s)].rstrip("_"), "up"
    for s in dn_suf:
        if label.endswith(s):
            return label[:len(label)-len(s)].rstrip("_"), "down"
    return label, None

def _pair_systs(labels, up_suf, dn_suf):
    pairs = {}
    for sl in sorted(labels):
        base, d = _base_and_dir(sl, up_suf, dn_suf)
        if d is None:
            pairs.setdefault(sl, {"up": sl, "down": None})
        else:
            pairs.setdefault(base, {"up": None, "down": None})[d] = sl
    return pairs


# ── low-level readers ─────────────────────────────────────────────────────────
def _weighted_hist(filepath, region_branch, w_branches, variable, bins, nb):
    h = np.zeros(nb); h2 = np.zeros(nb)
    try:
        with uproot.open(filepath) as f:
            keys = [k.split(";")[0] for k in f.keys()]
            if TREE_NAME not in keys:
                return h, h2
            tree  = f[TREE_NAME]
            tkeys = set(tree.keys())
            if variable not in tkeys or region_branch not in tkeys:
                return h, h2
            avail = [b for b in w_branches if b in tkeys]
            cols  = list({variable, region_branch} | set(avail))
            for chunk in tree.iterate(cols, step_size=CHUNK, library="np"):
                mask = np.asarray(chunk[region_branch], dtype=bool)
                if not mask.any():
                    del chunk; continue
                vals = np.asarray(chunk[variable], dtype=np.float64)[mask]
                w = np.ones(mask.sum(), dtype=np.float64)
                for b in avail:
                    a = np.asarray(chunk[b], dtype=np.float64)[mask]
                    bad = ~np.isfinite(a)
                    if bad.any(): a = a.copy(); a[bad] = 1.0
                    w *= a
                hi, _  = np.histogram(vals, bins=bins, weights=w)
                hi2, _ = np.histogram(vals, bins=bins, weights=w*w)
                h += hi; h2 += hi2
                del chunk; gc.collect()
    except Exception as e:
        print(f"    ERROR {os.path.basename(filepath)}: {e}")
    return h, h2


def _sf_syst_single_pass(filepath, region_branch, base_cw, variable, bins, nb):
    results = {}
    stat_h2 = np.zeros(nb)
    try:
        with uproot.open(filepath) as f:
            keys = [k.split(";")[0] for k in f.keys()]
            if TREE_NAME not in keys:
                return results, stat_h2
            tree  = f[TREE_NAME]
            tkeys = set(tree.keys())
            if variable not in tkeys or region_branch not in tkeys:
                return results, stat_h2

            avail_cw  = [b for b in base_cw         if b in tkeys]
            avail_sf  = [b for b in _ALL_SF_BRANCHES if b in tkeys]
            cols = list({variable, region_branch} | set(avail_cw) | set(avail_sf))

            syst_plans = []
            for (cen, up, dn) in SF_BRANCH_SYSTEMATICS:
                for var_b in (up, dn):
                    if var_b in tkeys:
                        cw_var = [var_b if b == cen else b for b in avail_cw]
                        syst_plans.append((var_b, cw_var))

            results["central"] = np.zeros(nb)
            for (vk, _) in syst_plans:
                results.setdefault(vk, np.zeros(nb))

            for chunk in tree.iterate(cols, step_size=CHUNK, library="np"):
                mask = np.asarray(chunk[region_branch], dtype=bool)
                if not mask.any():
                    del chunk; continue
                vals = np.asarray(chunk[variable], dtype=np.float64)[mask]

                branch_cache = {}
                for b in set(avail_cw) | set(avail_sf):
                    if b in chunk:
                        a = np.asarray(chunk[b], dtype=np.float64)[mask]
                        bad = ~np.isfinite(a)
                        if bad.any(): a = a.copy(); a[bad] = 1.0
                        branch_cache[b] = a

                w_cen = np.ones(mask.sum(), dtype=np.float64)
                for b in avail_cw:
                    if b in branch_cache:
                        w_cen = w_cen * branch_cache[b]
                hi, _  = np.histogram(vals, bins=bins, weights=w_cen)
                results["central"] += hi
                hi2, _ = np.histogram(vals, bins=bins, weights=w_cen * w_cen)
                stat_h2 += hi2

                for (vk, cw_var) in syst_plans:
                    w_v = np.ones(mask.sum(), dtype=np.float64)
                    for b in cw_var:
                        if b in branch_cache:
                            w_v = w_v * branch_cache[b]
                    hi, _ = np.histogram(vals, bins=bins, weights=w_v)
                    results[vk] += hi

                del chunk; gc.collect()
    except Exception as e:
        print(f"    ERROR (sf_syst) {os.path.basename(filepath)}: {e}")

    return results, stat_h2


def _theory_envelope(filepath, region_branch, cw_branches,
                     th_branch, sumw_branch, variable, bins, nb,
                     indices=None):
    """
    Build an envelope (max, min) across theory weight variations.

    Parameters
    ----------
    indices : list[int] or None
        If given, only this subset of LHE weight vector indices is used.
        None means use all available indices.
        For LHEScaleWeight pass [1, 3, 5, 7] to exclude correlated extremes.

    Returns (h_max, h_min, used) – used=False if branch absent.
    """
    hist_list = None
    try:
        with uproot.open(filepath) as f:
            keys = [k.split(";")[0] for k in f.keys()]
            if TREE_NAME not in keys:
                return None, None, False
            tree  = f[TREE_NAME]
            tkeys = set(tree.keys())
            if th_branch not in tkeys:
                return None, None, False
            avail = [b for b in cw_branches if b in tkeys]
            cols  = list({variable, region_branch, th_branch} | set(avail))
            if sumw_branch and sumw_branch in tkeys:
                cols.append(sumw_branch)

            for chunk in tree.iterate(cols, step_size=CHUNK, library="np"):
                mask = np.asarray(chunk[region_branch], dtype=bool)
                if not mask.any():
                    del chunk; continue
                vals = np.asarray(chunk[variable], dtype=np.float64)[mask]
                w_c = np.ones(mask.sum(), dtype=np.float64)
                for b in avail:
                    a = np.asarray(chunk[b], dtype=np.float64)[mask]
                    bad = ~np.isfinite(a)
                    if bad.any(): a = a.copy(); a[bad] = 1.0
                    w_c *= a

                raw = chunk[th_branch]
                try:
                    th = np.asarray(raw, dtype=np.float64)
                except Exception:
                    th = np.array([np.asarray(r, dtype=np.float64) for r in raw])
                th = th[mask]
                if th.ndim == 1:
                    th = th[:, np.newaxis]
                n_avail = th.shape[1]
                if n_avail == 0:
                    del chunk; gc.collect(); continue

                # ── FIX 1: restrict to requested indices ──────────────────
                if indices is not None:
                    idx_list = [i for i in indices if i < n_avail]
                    if not idx_list:
                        del chunk; gc.collect(); continue
                else:
                    idx_list = list(range(n_avail))

                if hist_list is None:
                    hist_list = [np.zeros(nb) for _ in idx_list]

                # Optional sumw normalization (ratio correction)
                sw = None
                if sumw_branch and sumw_branch in tkeys:
                    raw_sw = chunk[sumw_branch]
                    try:
                        sw = np.asarray(raw_sw, dtype=np.float64)[mask]
                    except Exception:
                        sw = np.array([np.asarray(r) for r in raw_sw])[mask]
                    if sw.ndim == 1:
                        sw = sw[:, np.newaxis]

                for slot, i in enumerate(idx_list):
                    wt = th[:, i].copy()
                    wt[~np.isfinite(wt)] = 1.0
                    if sw is not None and sw.shape[1] > i:
                        si = sw[:, i]
                        # LHEScaleWeight[4] is nominal; ratio-correct via sumw
                        nom_col = min(4, sw.shape[1] - 1)
                        sw_nom  = sw[:, nom_col]
                        correction = np.where(
                            (np.abs(si) > 0) & np.isfinite(si) &
                            (np.abs(sw_nom) > 0) & np.isfinite(sw_nom),
                            sw_nom / si,
                            1.0
                        )
                        wt *= correction
                    hist_list[slot] += np.histogram(
                        vals, bins=bins, weights=w_c * wt
                    )[0]
                del chunk; gc.collect()

    except Exception as e:
        print(f"    ERROR (theory) {os.path.basename(filepath)}: {e}")

    if not hist_list:
        return None, None, False
    arr = np.vstack(hist_list)
    return arr.max(axis=0), arr.min(axis=0), True


def _data_hist(filepath, region_branch, variable, bins, nb):
    h = np.zeros(nb)
    try:
        with uproot.open(filepath) as f:
            keys = [k.split(";")[0] for k in f.keys()]
            if TREE_NAME not in keys: return h
            tree  = f[TREE_NAME]
            tkeys = set(tree.keys())
            if variable not in tkeys or region_branch not in tkeys: return h
            for chunk in tree.iterate([variable, region_branch],
                                      step_size=CHUNK, library="np"):
                mask = np.asarray(chunk[region_branch], dtype=bool)
                if not mask.any(): del chunk; continue
                vals = np.asarray(chunk[variable], dtype=np.float64)[mask]
                hi, _ = np.histogram(vals, bins=bins)
                h += hi
                del chunk; gc.collect()
    except Exception as e:
        print(f"    ERROR data {os.path.basename(filepath)}: {e}")
    return h


# ── TW DR/DS envelope ─────────────────────────────────────────────────────────
def _tw_drds_envelope(tw_dir, region_branch, cw_branches, variable, bins, nb):
    if not os.path.isdir(tw_dir):
        return None, None
    root_files = sorted(glob.glob(os.path.join(tw_dir, "*.root")))
    if not root_files:
        return None, None
    tag_hists = {}
    prefix = "TWto2L2Nu-"; suffix = "_nominal"
    for fp in root_files:
        stem = os.path.splitext(os.path.basename(fp))[0]
        tag  = stem
        if tag.startswith(prefix):  tag = tag[len(prefix):]
        if tag.endswith(suffix):    tag = tag[:-len(suffix)]
        h, _ = _weighted_hist(fp, region_branch, cw_branches, variable, bins, nb)
        tag_hists[tag] = h
        print(f"      TW variant [{tag}] → sum={h.sum():.2f}")
    if not tag_hists:
        return None, None
    arr = np.vstack(list(tag_hists.values()))
    return arr.max(axis=0), arr.min(axis=0)


# ── add in quadrature helper ──────────────────────────────────────────────────
def _add_quad(syst_up2, syst_dn2, h_nom, h_up, h_dn):
    up_shift = np.zeros_like(h_nom)
    dn_shift = np.zeros_like(h_nom)
    if h_up is not None:
        d = h_up - h_nom
        up_shift = np.maximum(up_shift, np.where(d > 0,  d, 0.0))
        dn_shift = np.maximum(dn_shift, np.where(d < 0, -d, 0.0))
    if h_dn is not None:
        d = h_dn - h_nom
        up_shift = np.maximum(up_shift, np.where(d > 0,  d, 0.0))
        dn_shift = np.maximum(dn_shift, np.where(d < 0, -d, 0.0))
    syst_up2 += up_shift**2
    syst_dn2 += dn_shift**2


# ── per-process histogram + uncertainty builder ───────────────────────────────
def build_process_hists(proc_dir, region_branch, up_suf, dn_suf,
                        variable, bins, nb, include_syst=True,
                        tw_dir=None):
    process  = os.path.basename(proc_dir)
    is_ttbar = bool(TTBAR_DILEPT_PATTERN.search(process))
    is_tw    = process in TW_PROC_NAMES

    # FIX 2: theory systematics only for TTbar dileptonic and tW
    apply_theory = bool(THEORY_PROC_PATTERNS.search(process))

    root_files = sorted(glob.glob(os.path.join(proc_dir, "*.root")))
    nom_path, syst_files = None, {}
    for fp in root_files:
        sl = _syst_label(fp, process, up_suf, dn_suf)
        if sl == "nominal":
            nom_path = fp
        else:
            syst_files[sl] = fp

    zeros = tuple(np.zeros(nb) for _ in range(4))
    if nom_path is None:
        print(f"    [no nominal] {process}")
        return zeros

    cw = list(CENTRAL_WEIGHT_BRANCHES)
    if is_ttbar:
        cw = cw + [TOP_PT_WEIGHT_BRANCH]

    if not include_syst:
        nom_wt, stat_h2 = _weighted_hist(nom_path, region_branch, cw,
                                         variable, bins, nb)
        return nom_wt, stat_h2, np.zeros(nb), np.zeros(nb)

    # single pass: nominal + all SF systematics
    sf_results, stat_h2 = _sf_syst_single_pass(nom_path, region_branch, cw,
                                                variable, bins, nb)
    nom_wt = sf_results.get("central", np.zeros(nb))

    sup2 = np.zeros(nb)
    sdn2 = np.zeros(nb)

    for (central_b, up_b, dn_b) in SF_BRANCH_SYSTEMATICS:
        _add_quad(sup2, sdn2, nom_wt, sf_results.get(up_b), sf_results.get(dn_b))

    # file-based systematics (JEC etc.)
    pairs = _pair_systs(set(syst_files.keys()), up_suf, dn_suf)
    for base, sides in pairs.items():
        h_up = h_dn = None
        if sides["up"]:
            h_up, _ = _weighted_hist(syst_files[sides["up"]], region_branch, cw,
                                     variable, bins, nb)
        if sides["down"]:
            h_dn, _ = _weighted_hist(syst_files[sides["down"]], region_branch, cw,
                                     variable, bins, nb)
        _add_quad(sup2, sdn2, nom_wt, h_up, h_dn)

    # topPtWeight systematic (TTbar only)
    if is_ttbar:
        cw_no = list(CENTRAL_WEIGHT_BRANCHES)
        h_without, _ = _weighted_hist(nom_path, region_branch, cw_no,
                                      variable, bins, nb)
        _add_quad(sup2, sdn2, nom_wt, nom_wt, h_without)

    # ── FIX 2: theory systs only for TTbar dileptonic and tW ─────────────────
    if apply_theory:
        theory_used = []
        for (th_branch, sumw_branch, idx_filter) in THEORY_SYSTEMATICS:
            h_max, h_min, used = _theory_envelope(
                nom_path, region_branch, CENTRAL_WEIGHT_BRANCHES,
                th_branch, sumw_branch, variable, bins, nb,
                indices=idx_filter,          # <── FIX 1 passed through
            )
            if used:
                theory_used.append(
                    f"{th_branch}[{idx_filter}]" if idx_filter else th_branch
                )
                _add_quad(sup2, sdn2, nom_wt, h_max, h_min)
        if theory_used:
            print(f"      [{process}] Theory systs used: {', '.join(theory_used)}")
        else:
            print(f"      [{process}] Theory systs: branches not found in file")
    else:
        print(f"      [{process}] Theory systs: SKIPPED (not TTbar dilept / tW)")

    # ── TW DR/DS envelope ────────────────────────────────────────────────────
    if is_tw and tw_dir is not None:
        h_max, h_min = _tw_drds_envelope(tw_dir, region_branch,
                                         CENTRAL_WEIGHT_BRANCHES,
                                         variable, bins, nb)
        if h_max is not None:
            print(f"      [{process}] DR/DS envelope applied from TW/")
            _add_quad(sup2, sdn2, nom_wt, h_max, h_min)
        else:
            print(f"      [{process}] TW/ dir found but no variants read")

    return nom_wt, stat_h2, sup2, sdn2


# ── worker function for parallel execution ────────────────────────────────────
def _worker(args_tuple):
    proc_dir, region_branch, up_suf, dn_suf, variable, bins, nb, include_syst, tw_dir = args_tuple
    process = os.path.basename(proc_dir)
    grp     = PROC_TO_GROUP.get(process)
    if grp is None:
        return None, process, grp
    result = build_process_hists(proc_dir, region_branch, up_suf, dn_suf,
                                 variable, bins, nb,
                                 include_syst=include_syst,
                                 tw_dir=tw_dir)
    return result, process, grp


# ── plotting ──────────────────────────────────────────────────────────────────
def make_stack_plot(region_label, year, include_syst,
                    group_hists, group_stat2, group_sup2, group_sdn2,
                    data_hist, bins, xlabel, outpath):

    nb = len(bins) - 1
    centers = 0.5 * (bins[:-1] + bins[1:])
    total_mc = np.zeros(nb)
    total_s2 = np.zeros(nb)
    total_u2 = np.zeros(nb)
    total_d2 = np.zeros(nb)

    bars = []
    bottom = np.zeros(nb)
    for grp in STACK_ORDER:
        h = group_hists.get(grp, np.zeros(nb))
        if h.sum() == 0:
            continue
        bars.append((bottom.copy(), h, GROUP_LABELS[grp], GROUP_COLORS[grp]))
        bottom   += h
        total_mc += h
        total_s2 += group_stat2.get(grp, np.zeros(nb))
        total_u2 += group_sup2.get(grp,  np.zeros(nb))
        total_d2 += group_sdn2.get(grp,  np.zeros(nb))

    err_up = np.sqrt(total_s2 + total_u2)
    err_dn = np.sqrt(total_s2 + total_d2)

    fig = plt.figure(figsize=(9, 8))
    gs  = fig.add_gridspec(2, 1, height_ratios=[3, 1], hspace=0.04)
    ax  = fig.add_subplot(gs[0])
    axr = fig.add_subplot(gs[1], sharex=ax)

    for bot, h, lbl, col in bars:
        top = bot + h
        ax.stairs(top, bins, baseline=bot, fill=True, color=col,
                  alpha=0.88, label=lbl, linewidth=0)

    band_label = "Stat ⊕ Syst" if include_syst else "Stat"
    ax.stairs(total_mc + err_up, bins, baseline=total_mc - err_dn,
              fill=True, color="black", alpha=0.30, hatch="////",
              linewidth=0, label=band_label)

    nonzero_data = data_hist > 0
    ax.errorbar(
        centers[nonzero_data], data_hist[nonzero_data],
        yerr=np.sqrt(data_hist[nonzero_data]),
        fmt="ko", markersize=5, linewidth=1.3,
        label="Data", zorder=10,
    )

    ax.set_ylabel("Events / bin", fontsize=12)
    ax.set_xlim(bins[0], bins[-1])
    ymax = max(total_mc.max(), data_hist.max()) * 1.45
    ax.set_ylim(0, ymax if ymax > 0 else 1)
    ax.legend(fontsize=8, ncol=2, loc="upper right",
              framealpha=0.8, edgecolor="none")
    ax.set_title(f"{year}   Region: {region_label}",
                 fontsize=11, loc="left", fontweight="bold")
    plt.setp(ax.get_xticklabels(), visible=False)

    with np.errstate(divide="ignore", invalid="ignore"):
        ratio     = np.where(total_mc > 0, data_hist  / total_mc, np.nan)
        ratio_err = np.where(total_mc > 0, np.sqrt(data_hist) / total_mc, np.nan)
        b_up = np.where(total_mc > 0, 1.0 + err_up / total_mc, np.nan)
        b_dn = np.where(total_mc > 0, 1.0 - err_dn / total_mc, np.nan)

    axr.stairs(b_up, bins, baseline=b_dn, fill=True, color="black",
               alpha=0.30, hatch="////", linewidth=0)
    axr.errorbar(
        centers[nonzero_data], ratio[nonzero_data],
        yerr=ratio_err[nonzero_data],
        fmt="ko", markersize=4, linewidth=1.0, zorder=10,
    )
    axr.axhline(1.0, color="black", linewidth=0.9, linestyle=":")
    axr.set_ylim(0.5, 1.5)
    axr.yaxis.set_major_locator(ticker.MultipleLocator(0.25))
    axr.yaxis.set_minor_locator(ticker.MultipleLocator(0.125))
    axr.set_ylabel("Data / MC", fontsize=11)
    axr.set_xlabel(xlabel, fontsize=11)
    axr.set_xlim(bins[0], bins[-1])

    fig.tight_layout()
    fig.savefig(outpath, bbox_inches="tight", dpi=150)
    plt.close(fig)
    print(f"  → saved {outpath}")


# ── yields table ──────────────────────────────────────────────────────────────
def print_yields_table(yields_store, year, outpath):
    """
    yields_store : dict  region_label → dict  group → (N, stat_up, stat_dn, syst_up, syst_dn)
    Prints and saves a formatted yields table.
    """
    col_w   = 14
    grp_w   = 24
    regions = list(yields_store.keys())

    header_line = f"{'Process':<{grp_w}}" + "".join(
        f"{'Region '+r:^{col_w*3}}" for r in regions
    )
    sub_header = " " * grp_w + "".join(
        f"{'Yield':>{col_w}}{'±stat':>{col_w}}{'±syst':>{col_w}}"
        for _ in regions
    )
    sep = "─" * (grp_w + col_w * 3 * len(regions))

    lines = []
    lines.append(f"\nYields Table — {year}  (LHEScaleWeight indices [1,3,5,7], TTbar+tW only)\n")
    lines.append(sep)
    lines.append(header_line)
    lines.append(sub_header)
    lines.append(sep)

    totals = {r: {"N": 0.0, "s2": 0.0, "u2": 0.0, "d2": 0.0} for r in regions}

    for grp in STACK_ORDER:
        lbl  = GROUP_LABELS[grp]
        # strip LaTeX for plain-text table
        lbl_plain = re.sub(r"\$[^$]*\$", lambda m: m.group(0).replace("\\", ""), lbl)
        row  = f"{lbl_plain:<{grp_w}}"
        for r in regions:
            entry = yields_store[r].get(grp)
            if entry is None:
                row += f"{'—':>{col_w}}{'':>{col_w}}{'':>{col_w}}"
                continue
            N, s2, u2, d2 = entry
            stat = np.sqrt(s2)
            syst = 0.5 * (np.sqrt(u2) + np.sqrt(d2))   # symmetrised syst
            row += f"{N:>{col_w}.1f}{stat:>{col_w}.1f}{syst:>{col_w}.1f}"
            totals[r]["N"]  += N
            totals[r]["s2"] += s2
            totals[r]["u2"] += u2
            totals[r]["d2"] += d2
        lines.append(row)

    lines.append(sep)
    tot_row = f"{'Total MC':<{grp_w}}"
    for r in regions:
        t = totals[r]
        N    = t["N"]
        stat = np.sqrt(t["s2"])
        syst = 0.5 * (np.sqrt(t["u2"]) + np.sqrt(t["d2"]))
        tot_row += f"{N:>{col_w}.1f}{stat:>{col_w}.1f}{syst:>{col_w}.1f}"
    lines.append(tot_row)
    lines.append(sep)

    # also add a combined (stat⊕syst) row
    comb_row = f"{'Total (stat⊕syst)':<{grp_w}}"
    for r in regions:
        t = totals[r]
        N     = t["N"]
        total = 0.5 * (np.sqrt(t["s2"] + t["u2"]) + np.sqrt(t["s2"] + t["d2"]))
        comb_row += f"{N:>{col_w}.1f}{'±'+f'{total:.1f}':>{col_w}}{'':>{col_w}}"
    lines.append(comb_row)
    lines.append(sep + "\n")

    text = "\n".join(lines)
    print(text)
    with open(outpath, "w") as fh:
        fh.write(text)
    print(f"Yields table saved to {outpath}")


# ── main ──────────────────────────────────────────────────────────────────────
def main():
    args     = parse_args()
    year     = args.year
    base_dir, up_suf, dn_suf = build_paths(year)

    if not os.path.isdir(base_dir):
        print(f"ERROR: {base_dir} does not exist"); sys.exit(1)

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    plot_vars = PLOT_VARIABLES
    if args.variable:
        known = {v[0] for v in PLOT_VARIABLES}
        if args.variable not in known:
            print(f"ERROR: --variable '{args.variable}' not found in PLOT_VARIABLES.")
            print(f"  Known variables: {', '.join(v[0] for v in PLOT_VARIABLES)}")
            sys.exit(1)
        plot_vars = [v for v in PLOT_VARIABLES if v[0] == args.variable]
        print(f"Single-variable mode: plotting only '{args.variable}'")

    all_proc_dirs = sorted([
        d for d in glob.glob(os.path.join(base_dir, "*"))
        if os.path.isdir(d)
        and os.path.basename(d) not in {"Data", "logs", "TW"}
        and not os.path.basename(d).startswith("temp")
    ])

    if args.process:
        all_proc_dirs = [d for d in all_proc_dirs
                         if os.path.basename(d) == args.process]

    if not all_proc_dirs:
        print("No process directories found."); sys.exit(1)

    data_path = os.path.join(base_dir, "Data", "Data_nominal.root")
    if not os.path.isfile(data_path):
        print(f"WARNING: data file not found at {data_path}")
        data_path = None

    tw_dir = os.path.join(base_dir, "TW")
    if os.path.isdir(tw_dir):
        tw_files = glob.glob(os.path.join(tw_dir, "*.root"))
        print(f"TW dir found: {tw_dir}  ({len(tw_files)} files)")
        tw_tags = []
        for fp in sorted(tw_files):
            stem = os.path.splitext(os.path.basename(fp))[0]
            tag  = stem.removeprefix("TWto2L2Nu-").removesuffix("_nominal")
            tw_tags.append(tag)
        print(f"  DR/DS variants (envelope): {', '.join(tw_tags)}")
    else:
        tw_dir = None
        print("TW dir not found – DR/DS systematic skipped")

    print(f"\nYear       : {year}")
    print(f"Base dir   : {base_dir}")
    print(f"Processes  : {len(all_proc_dirs)}")
    print(f"Variables  : {len(plot_vars)}")
    print(f"Systematics: {'OFF (stat only)' if args.no_syst else 'ON'}")
    print(f"Workers    : {args.jobs}")
    print(f"LHEScaleWeight indices : [1,3,5,7]  (TTbar dilept + tW only)\n")

    # Store yields: region → group → (N, stat2, sup2, sdn2) accumulated over variables
    # We only store yields from the FIRST variable loop (integrated counts are
    # variable-independent; summing over bins gives the total yield).
    yields_store   = {rlabel: {} for rlabel in REGIONS.values()}
    yields_filled  = False

    # ── outer loop: variables ────────────────────────────────────────────────
    for variable, xlabel, bin_range in plot_vars:
        bins, nb = make_bins(bin_range)

        print(f"\n{'═'*55}")
        print(f"Variable: {variable}  bins={bin_range}")
        print(f"{'═'*55}")

        for rkey, rlabel in REGIONS.items():
            print(f"\n  {'─'*51}")
            print(f"  Region: {rlabel}  ({rkey})")
            print(f"  {'─'*51}")

            group_hists = defaultdict(lambda: np.zeros(nb))
            group_stat2 = defaultdict(lambda: np.zeros(nb))
            group_sup2  = defaultdict(lambda: np.zeros(nb))
            group_sdn2  = defaultdict(lambda: np.zeros(nb))

            worker_args = [
                (proc_dir, rkey, up_suf, dn_suf,
                 variable, bins, nb, not args.no_syst, tw_dir)
                for proc_dir in all_proc_dirs
            ]

            with ProcessPoolExecutor(max_workers=args.jobs) as pool:
                futures = {pool.submit(_worker, wa): wa[0] for wa in worker_args}
                for fut in as_completed(futures):
                    result, process, grp = fut.result()
                    if grp is None:
                        print(f"  [unmapped] {process}")
                        continue
                    nom, s2, u2, d2 = result
                    print(f"    {process:40s} → {grp}")
                    group_hists[grp] += nom
                    group_stat2[grp] += s2
                    group_sup2[grp]  += u2
                    group_sdn2[grp]  += d2

            # ── Accumulate yields on first variable only ──────────────────
            if not yields_filled:
                for grp in STACK_ORDER:
                    N    = group_hists[grp].sum()
                    s2   = group_stat2[grp].sum()
                    u2   = group_sup2[grp].sum()
                    d2   = group_sdn2[grp].sum()
                    if N > 0 or s2 > 0:
                        yields_store[rlabel][grp] = (N, s2, u2, d2)

            data_h = (_data_hist(data_path, rkey, variable, bins, nb)
                      if data_path else np.zeros(nb))

            region_dir = os.path.join(OUTPUT_DIR, rlabel)
            os.makedirs(region_dir, exist_ok=True)
            outpath = os.path.join(region_dir,
                                   f"stack_{variable}_{rlabel}_{year}.pdf")
            make_stack_plot(
                rlabel, year, not args.no_syst,
                dict(group_hists), dict(group_stat2),
                dict(group_sup2),  dict(group_sdn2),
                data_h, bins, xlabel, outpath,
            )

        # Mark yields as filled after first variable's worth of regions
        yields_filled = True

    # ── Print + save yields table ────────────────────────────────────────────
    yields_path = os.path.join(OUTPUT_DIR, f"yields_{year}.txt")
    print_yields_table(yields_store, year, yields_path)

    print(f"\nDone. Plots in '{OUTPUT_DIR}/'")


if __name__ == "__main__":
    main()
