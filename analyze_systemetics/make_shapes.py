#!/usr/bin/env python3
"""
make_shapes_and_plots.py
==========================

Builds shapes.root: nominal histograms + WEIGHT-BASED systematics (SF
up/down branch swaps + theory weight envelopes) + FILE-BASED ("shape")
systematics (JEC/JER, lepton scale/smear, met_PU, ttbar/single-top modeling
variations that live in separate ROOT files).

Output: shapes.root with TDirectories 1j1b / 2j1b / 2j2b, each containing:
    <process>                      nominal
    <process>_<syst>Up/Down        for each systematic
    <process>_<syst>Up             for one-directional systematics
                                    (ttbar_CR1, ttbar_CR2, st_tune -- no
                                    mirrored Down is written for these)

WHAT COUNTS AS A "WEIGHT SYSTEMATIC" HERE
------------------------------------------
1. SF_BRANCH_SYSTEMATICS: swap one branch in the nominal weight product for
   an up/down branch (btag SFs, ele/muon SF, HLT SF, pileup).
2. THEORY_SYSTEMATICS: envelopes (bin-wise max/min) built from LHE weight
   vectors (PDF, alphaS, QCD scale, PS), each variation normalized by its
   relative sum-of-weights. Only built for the ttbar and single-tW processes
   (THEORY_SYST_PROCESSES) -- everything else gets no theory systematic at
   all. The two ttbar processes (dileptonic/semileptonic) share one nuisance
   parameter per theory uncertainty ("ttbar_pdf", "ttbar_qcd_scale", ...);
   the two single-tW processes share a separate one ("st_pdf",
   "st_qcd_scale", ...).

WHAT COUNTS AS A "FILE SYSTEMATIC" HERE
------------------------------------------
Every other ROOT file next to `<process>_nominal.root`, matched by filename
against SHAPE_SYST_PATTERNS (JEC/JER/lepton) or MODELING_TAG_PATTERNS
(Hdamp/MT/UE/CR1/CR2/TuneCH3), reweighted the same way as the nominal tree
and filled directly.

CONFIG YOU SHOULD DOUBLE-CHECK
--------------------------------
* REGIONS[*]["cut"]     -- uses region_1j1t / region_2j1t / region_2j2t flags.
* REGIONS[*]["binning_mode"] -- per-region binning strategy:
                           "equal_ttbar_yield" (1j1b/2j1b) splits bin edges so
                           each bin holds an equal ttbar-weighted yield;
                           "fixed_with_overflow" (2j2b) uses fixed-width bins
                           out to xmax, with the last bin's upper edge pushed
                           to catch every event above xmax as an overflow bin.
* ERA_BASE_DIRS          -- one top-level directory per era (2022/2022EE/
                           2023/2023BPix/2024). Each era's ROOT files live
                           under their own path now (rather than all eras
                           being mixed together and split apart via the
                           datasetId branch) -- fill in the real EOS paths.
"""

import os
import re
import glob
import gc
import resource
import numpy as np
import ROOT

ROOT.ROOT.EnableImplicitMT()

# PyROOT only closes the underlying TFile when its RDataFrame/TTree wrapper
# is actually garbage-collected, and across the full set of processes/eras
# we open several thousand short-lived files. Raise the process's open-file
# ceiling as a safety margin (on top of the explicit gc.collect() calls added
# below, which are the actual fix -- this just gives headroom if GC lags).
try:
    soft, hard = resource.getrlimit(resource.RLIMIT_NOFILE)
    resource.setrlimit(resource.RLIMIT_NOFILE, (hard, hard))
    print(f"Raised open-file limit: {soft} -> {hard}")
except Exception as e:
    print(f"WARNING: could not raise open-file limit ({e}); leaving at current ulimit")
ROOT.gROOT.SetBatch(True)

# --------------------------------------------------------------------------
# CONFIG
# --------------------------------------------------------------------------

# Each era's ROOT files now live in a SEPARATE top-level directory (rather
# than all being mixed together under one BASE_DIR and split apart event-by-
# event via the datasetId branch). Fill in the actual EOS path for every era
# you want to process -- directory layout under each path is unchanged from
# before:
#     <ERA_BASE_DIRS[era]>/<process>/<process>_nominal.root
#     <ERA_BASE_DIRS[era]>/<process>/<process>_<systTag>.root
# TODO: fill in the real paths below.
ERA_BASE_DIRS = {
    "2022":     "/eos/uscms/store/user/vsinha/Result_2022/systemetics/",
    "2022EE":   "/eos/uscms/store/user/vsinha/Result_2022EE/systemetics/",
    "2023":     "/eos/uscms/store/user/vsinha/Result_2023/systemetics/",
    "2023BPix": "/eos/uscms/store/user/vsinha/Result_2023BPix/systemetics/",
    "2024":     "/eos/uscms/store/user/vsinha/Result_2024/systemetics/",
}
OUT_FILE_TEMPLATE = "shapes_{era}.root"  # actual output files, one per era (see ERAS below)
TREE_NAME = "outputTree"

REGIONS = {
    "1j1b": dict(variable="bdt_variable_1j1t", cut="region_1j1t == 1",
                 nbins=15, xmin=0.0, xmax=1.0, binning_mode="equal_sb_yield"),
    "2j1b": dict(variable="bdt_variable_2j1t", cut="region_2j1t == 1",
                 nbins=15, xmin=0.0, xmax=1.0, binning_mode="equal_sb_yield"),
    "2j2b": dict(variable="Selected_jet_subleading_pt", cut="region_2j2t == 1",
                 nbins=15, xmin=0.0, xmax=180.0, binning_mode="fixed_with_overflow"),
}

# Raw process directory names (as they appear under BASE_DIR, matching the
# ttbar_dileptonic/ttbar_semileptonic entries in FILE_GROUPS below). Only
# used if you switch a region's binning_mode back to "equal_ttbar_yield"
# (see compute_equal_weight_edges / compute_all_edges below) -- currently
# unused since every region uses simple equal-width binning above.
TTBAR_BINNING_PROCESSES = ["TTbar_Dilept", "TTbar_SemiLept"]

# Upper edge substituted for a fixed-binning region's nominal xmax so that
# every event beyond xmax (not just ones inside [xmin, xmax]) is captured in
# the last bin instead of silently falling into ROOT's invisible overflow
# bin. Far larger than any realistic value of the plotted variable.
OVERFLOW_EDGE = 1.0e6

# combine does not accept histogram/TDirectory ("bin"/channel) names that
# start with a digit, so every region gets a "cat_" prefix wherever it is
# written out as a ROOT TDirectory name or used as a datacard "bin" (channel)
# name. Region keys everywhere else in the script (REGIONS, edges_by_region,
# accum, etc.) stay unprefixed -- translation to the channel name happens
# only at the point of writing/reading a TDirectory or a datacard column.
CHANNEL_PREFIX = "cat_"


def channel_name(region_name):
    return f"{CHANNEL_PREFIX}{region_name}"


SKIP_PROCESSES = {"TW"}
DATA_PROCESS_NAME = "Data"

# Processes to exclude from the equal-yield binning pool (e.g. real signal).
SIGNAL_PROCESSES_EXCLUDED_FROM_BINNING = set()

# datasetId branch mapping -- kept only to give the eras a canonical order
# (0..4) for printouts; no longer used to split events by era, since each
# era's events now live in their own directory (ERA_BASE_DIRS) rather than
# being mixed together and filtered via the datasetId branch.
DATASET_ID_YEAR_MAP = {0: "2022", 1: "2022EE", 2: "2023", 3: "2023BPix", 4: "2024"}

ERAS_TO_INCLUDE = {"2022","2022EE", "2023", "2023BPix", "2024"}


def _resolve_eras():
    """Build the final, ordered list of era names to process: every name in
    DATASET_ID_YEAR_MAP (canonical ordering) that (a) has an entry in
    ERA_BASE_DIRS and (b) passes ERAS_TO_INCLUDE (if that filter is set).
    Directory *existence* on disk is checked later, in build_shapes(), so a
    missing/not-yet-staged era directory only skips that one era instead of
    crashing here."""
    names = [name for _, name in sorted(DATASET_ID_YEAR_MAP.items())]
    resolved = [name for name in names if name in ERA_BASE_DIRS]
    missing_dirs_cfg = [name for name in names if name not in ERA_BASE_DIRS]
    if missing_dirs_cfg:
        print(f"NOTE: no ERA_BASE_DIRS entry for era(s) {missing_dirs_cfg} -- excluded")
    if ERAS_TO_INCLUDE:
        excluded = [name for name in resolved if name not in ERAS_TO_INCLUDE]
        if excluded:
            print(f"NOTE: ERAS_TO_INCLUDE is set -- excluding era(s): {excluded}")
        unknown = sorted(ERAS_TO_INCLUDE - set(names))
        if unknown:
            print(f"WARNING: ERAS_TO_INCLUDE contains unrecognized era name(s), ignored: {unknown}")
        resolved = [name for name in resolved if name in ERAS_TO_INCLUDE]
    if not resolved:
        raise ValueError("No eras left to process -- check ERA_BASE_DIRS / ERAS_TO_INCLUDE")
    return resolved


ERAS = _resolve_eras()  # e.g. ["2022", "2023", "2023BPix", "2024"]
print(f"Processing eras: {ERAS}")

CENTRAL_WEIGHT_BRANCHES = [
    "no_puWeight", "muon_SF_central", "puWeight",
    "ele_SF_central", "btag_SF_bcflav_central", "btag_SF_lflav_central", "hlt_sf",
]
TOP_PT_WEIGHT_BRANCH = "topPtWeight"
TTBAR_DILEPT_PATTERN = re.compile(r"(?i)ttbar.*dilept|dilept.*ttbar")

# combine refuses a histogram that integrates to exactly zero (e.g. no events
# from a given process passed a region's cut, or a systematic variation
# happens to shift every event out of the selection). Rather than silently
# writing (or dropping) an empty template, we give it a tiny placeholder
# yield in the first bin instead, so every template combine reads is
# technically non-empty.
EMPTY_HIST_FLOOR = 1e-6


def ensure_nonempty(hist):
    """If `hist` integrates to zero (or is otherwise empty), seed a tiny
    floor value into its first bin so combine doesn't choke on an empty
    template. Returns the same histogram for convenient chaining."""
    if hist.Integral() <= 0:
        hist.SetBinContent(1, EMPTY_HIST_FLOOR)
    return hist


# (central_branch, up_branch, down_branch, systematic_name)
SF_BRANCH_SYSTEMATICS = [
    ("btag_SF_bcflav_central", "btag_SF_bcflav_up_correlated",   "btag_SF_bcflav_down_correlated",   "btag_bc_corr"),
    ("btag_SF_bcflav_central", "btag_SF_bcflav_up_uncorrelated", "btag_SF_bcflav_down_uncorrelated", "btag_bc_uncorr"),
    ("btag_SF_lflav_central",  "btag_SF_lflav_up_correlated",    "btag_SF_lflav_down_correlated",    "btag_lf_corr"),
    ("btag_SF_lflav_central",  "btag_SF_lflav_up_uncorrelated",  "btag_SF_lflav_down_uncorrelated",  "btag_lf_uncorr"),
    ("ele_SF_central",         "ele_SF_up",                      "ele_SF_down",                      "eff_e"),
    ("muon_SF_central",        "muon_SF_up",                     "muon_SF_down",                     "eff_m"),
    ("hlt_sf",                 "hlt_sf_up",                      "hlt_sf_down",                      "eff_hlt"),
    ("puWeight",               "puWeight_up",                    "puWeight_down",                    "pileup"),
]

# (weight branch (vector), sumw branch/hist name, indices or None=all, systematic name)
THEORY_SYSTEMATICS = [
    dict(branch="LHEPdfWeight",   sumw="LHEPdfSumw",   indices=list(range(0, 100)), name="pdf"),
    dict(branch="LHEPdfWeight",   sumw="LHEPdfSumw",   indices=[100, 101],          name="alphaS"),
    dict(branch="LHEScaleWeight", sumw="LHEScaleSumw", indices=[1, 6],              name="qcd_scale_ur"),
    dict(branch="LHEScaleWeight", sumw="LHEScaleSumw", indices=[3, 4],              name="qcd_scale_uf"),
    dict(branch="PSWeight",       sumw="PSSumw",       indices=None,                name="ps"),
]

# Theory envelopes (pdf/alphaS/qcd_scale/ps) are only meaningful -- and only
# built -- for the ttbar and single-tW processes, not for every background.
# Everything else (DY, VV, W+jets, ttG/ttV, ...) never gets a theory
# systematic written, regardless of SYSTEMATIC_GROUP_SWITCHES["theory"].
#
# Within this set, the two ttbar processes (dileptonic/semileptonic) share
# ONE nuisance parameter per theory uncertainty, and the two single-tW
# processes share a separate, independent ONE nuisance parameter per theory
# uncertainty -- i.e. 2 groups x 4 theory uncertainties = 8 theory nuisances
# total (before era-decorrelation), not one per process. This is done by
# naming the stored systematic "<group>_<theory_name>" (e.g. "ttbar_pdf",
# "st_pdf") using modeling_prefix(proc) below to pick the group -- the same
# helper already used to name the file-based modeling variations (hdamp/UE/
# CR/tune), which follows the identical "ttbar"/"st" grouping convention.
THEORY_SYST_PROCESSES = {
    "TTbar_Dilept", "TTbar_SemiLept",           # -> group "ttbar"
    "TWminusto2L2Nu", "TbarWplusto2L2Nu",       # -> group "st"
}

# --------------------------------------------------------------------------
# DEBUG SWITCHES -- turn categories of systematics on/off, or isolate one
# specific systematic by name, without touching the rest of the script.
# --------------------------------------------------------------------------

# When True: build ONLY the nominal histogram for every process/region/era --
# no SF, theory, or file-based ("shape") systematics at all, regardless of
# SYSTEMATIC_GROUP_SWITCHES / ONLY_THESE_SYSTEMATICS below. No systematic
# weight columns get booked, no systematic files get globbed/opened. This is
# for quickly rerunning build_shapes() + run_plots() just to check binning
# and nominal yields/shapes without waiting on every variation.
NOMINAL_ONLY = False

SYSTEMATIC_GROUP_SWITCHES = dict(
    sf=True,             # btag/ele/muon/hlt/pileup SF up/down
    theory=True,         # pdf/alphaS/qcd_scale/ps envelopes
    jec_jer=True,        # JES (regrouped) + JER
    met_pu=True,         # met_PU up/down
    lepton_scale=True,   # muon/ele scale+smear
    modeling=False,       # ttbar/single-top: hdamp/mtop/UE/CR1/CR2/tune
)

# If non-empty, ONLY these exact systematic names are built (everything else
# skipped), regardless of SYSTEMATIC_GROUP_SWITCHES above. Handy for isolating
# one suspicious systematic, e.g. {"ttbar_CR1"} or {"pileup"}.
ONLY_THESE_SYSTEMATICS = set()

# Print, for every shape systematic, the ratio of its total (post-cut) yield
# to the nominal yield in that region. A modeling variation (hdamp/UE/CR/tune)
# that comes from a separately-generated sample can carry a different overall
# normalization than nominal if its sum-of-weights wasn't computed correctly
# upstream; this printout makes that immediately visible per systematic
# instead of only seeing it as a mysteriously huge total uncertainty band.
PRINT_YIELD_RATIOS = True
YIELD_RATIO_WARN_THRESHOLD = 1.5  # flag anything more than 50% off from nominal


def systematic_enabled(name, category):
    if ONLY_THESE_SYSTEMATICS:
        return name in ONLY_THESE_SYSTEMATICS
    return SYSTEMATIC_GROUP_SWITCHES.get(category, True)


# --------------------------------------------------------------------------
# CROSS-ERA CORRELATION FOR COMBINE
# --------------------------------------------------------------------------
# When the per-era datacards (2022, 2022EE, 2023, 2023BPix, 2024) get merged
# into one combined datacard with `combineCards.py`, combine treats any two
# nuisance parameters that share the EXACT SAME NAME across channels as one
# fully-correlated nuisance. Any nuisance name that differs between channels
# comes out fully independent (uncorrelated).
#
# Systematics whose underlying name already varies by era -- JEC/JER,
# met_PU, muon/electron scale+smear, which bake the year straight into the
# name via SHAPE_SYST_PATTERNS (e.g. "scale_j_FlavorQCD_2022EE") -- are
# already unique per era and are left completely untouched here.
#
# Everything else (SF weight systematics like pileup/eff_e/btag_*, theory
# envelopes like pdf/qcd_scale, and ttbar/single-top modeling variations
# like ttbar_hdamp) currently shares the exact same name across every era's
# file, so combineCards.py would treat them as fully correlated by default.
#
# List the systematic names below that you actually WANT correlated across
# eras (left unchanged). Every systematic NOT in this list gets "_<era>"
# appended to its name wherever it's written to output (e.g. "pileup" ->
# "pileup_2022EE"), so it naturally decorrelates once the per-era cards are
# combined -- no need to touch write_datacard() or the plotting code, since
# both just introspect whatever names end up in the shapes file.
CORRELATED_SYSTEMATICS = {"pileup","btag_bc_corr","btag_lf_corr","pdf","alphaS","qcd_scale_ur","qcd_scale_uf","ps"
                          


    # "pdf", "qcd_scale",  -- e.g. add names here for anything that should
    #                          stay correlated across eras
}


def apply_era_correlation(syst_name, era_name, already_has_year):
    """Return the systematic name as it should be stored for this era.

    - already_has_year=True (JEC/JER/met_PU/lepton scale -- year is already
      baked into syst_name by SHAPE_SYST_PATTERNS): returned unchanged.
    - syst_name in CORRELATED_SYSTEMATICS: returned unchanged (correlated
      across eras once combined).
    - otherwise: "<syst_name>_<era_name>" (uncorrelated across eras once
      combined).
    """
    if already_has_year:
        return syst_name
    if syst_name in CORRELATED_SYSTEMATICS:
        return syst_name
    return f"{syst_name}_{era_name}"


# These live in separate ROOT files (JEC/JER, lepton scale/smear, met_PU,
# and ttbar/single-top modeling variations like Hdamp/MT/UE/CR/TuneCH3).
# Each is read as a full nominal-style tree and reweighted with the same
# central weight product as the nominal sample.

def _year_suffixed(base, m):
    """Append '_<year>' to `base` only if this match actually captured an
    explicit year (some processes' shape-systematic files carry a
    '..._2022EE' style suffix; most don't and are just one multi-era file
    filtered by datasetId like the nominal file). Returns `base` unchanged
    when no year was present."""
    y = m.group('year')
    return f"{base}_{y}" if y else base


# The trailing "_<year>" is OPTIONAL in every pattern below: some processes'
# shape-systematic files encode the era in the filename, most don't (the era
# is picked out of the tree via the datasetId branch instead, same as the
# nominal file). When there's no year in the filename, `year_fn` returns
# None, so this systematic is built for every era's shapes file (same as
# nominal), and its final stored name goes through the same
# correlated/uncorrelated-by-era logic (apply_era_correlation) as the SF and
# theory weight systematics.
SHAPE_SYST_PATTERNS = [
    (re.compile(r'^JER_(?P<dir>Up|Down)(?:_(?P<year>\d{4}\w*))?$'),
     lambda m: _year_suffixed("res_j", m),
     lambda m: m.group('dir'), "jec_jer", lambda m: m.group('year')),
    (re.compile(r'^met_PU(?P<dir>Up|Down)(?:_(?P<year>\d{4}\w*))?$'),
     lambda m: _year_suffixed("met_PU", m),
     lambda m: m.group('dir'), "met_pu", lambda m: m.group('year')),
    (re.compile(r'^muon_reso(?P<dir>up|dn)(?:_(?P<year>\d{4}\w*))?$'),
     lambda m: _year_suffixed("res_m", m),
     lambda m: "Up" if m.group('dir') == "up" else "Down", "lepton_scale", lambda m: m.group('year')),
    (re.compile(r'^muon_scale(?P<dir>up|dn)(?:_(?P<year>\d{4}\w*))?$'),
     lambda m: _year_suffixed("scale_m", m),
     lambda m: "Up" if m.group('dir') == "up" else "Down", "lepton_scale", lambda m: m.group('year')),
    (re.compile(r'^EleSmear(?P<dir>Up|Down)(?:_(?P<year>\d{4}\w*))?$'),
     lambda m: _year_suffixed("scale_e", m),
     lambda m: m.group('dir'), "lepton_scale", lambda m: m.group('year')),
]
# Regrouped-JES sources ending in "_Year" (Absolute_Year, BBEC1_Year,
# EC2_Year, HF_Year, RelativeSample_Year) are the year-DEPENDENT half of
# that source by construction of the regrouped JES scheme, and must be
# decorrelated across eras. Everything else (Absolute, BBEC1, EC2, HF,
# FlavorQCD, RelativeBal, ...) is the correlated half and must keep the
# EXACT SAME stored name in every era's file/card so combineCards.py
# merges it into one correlated nuisance instead of one-per-era.
JEC_PATTERN = re.compile(r'^JEC_Regrouped_(?P<name>.+?)(?P<dir>Up|Down)(?:_(?P<year>\d{4}\w*))?$')


def _is_year_decorrelated_jec_source(source_name):
    return source_name.endswith("_Year")

def modeling_prefix(proc_name):
    if re.match(r'(?i)^t[bw]ar?w|^tw', proc_name):
        return "st"          # single top (TW / TbarWplus / TWminus, etc.)
    if re.match(r'(?i)^ttbar', proc_name):
        return "ttbar"
    return proc_name.lower()


# Hyphen before the number is optional, to cover both "Hdamp158" (ttbar) and
# "Hdamp-158" (single top) style filenames. These have no year in the
# filename -- we assume the tree itself is multi-era and gets split the same
# way as nominal, via the datasetId branch.
MODELING_TAG_PATTERNS = [
    (re.compile(r'^Hdamp-?158$'),  "hdamp", "Down"),
    (re.compile(r'^Hdamp-?418$'),  "hdamp", "Up"),
    (re.compile(r'^MT-?171p5$'),   "mtop",  "Down"),
    (re.compile(r'^MT-?173p5$'),   "mtop",  "Up"),
    (re.compile(r'^UE_Down$'),     "UE",    "Down"),
    (re.compile(r'^UE_Up$'),       "UE",    "Up"),
    # One-directional (no mirrored opposite side written):
    (re.compile(r'^CR1$'),         "CR1",   "Up"),
    (re.compile(r'^CR2$'),         "CR2",   "Up"),
    (re.compile(r'^TuneCH3$'),     "tune",  "Up"),
]


def discover_shape_systematics(proc_dir, proc_name):
    """Find every per-file systematic variation for this process (everything
    that isn't the nominal file), returning
    [(path, syst_name, direction, category, year_or_None, force_correlated), ...].
    year is None for systematics that aren't tied to one specific era
    (SF/theory weight systematics and the modeling variations) -- those get
    filtered by the datasetId branch instead, same as the nominal file.
    force_correlated is True only for the non-"_Year" regrouped-JES sources
    (see JEC_PATTERN / _is_year_decorrelated_jec_source above), which must
    keep an identical stored name across every era despite coming from a
    separate per-era file."""
    shape_systs = []
    for f in glob.glob(os.path.join(proc_dir, f"{proc_name}_*.root")):
        base = os.path.basename(f)[:-5]
        tag = base[len(proc_name):].lstrip("_")
        if tag in ("", "nominal"):
            continue
        matched = False

        for pattern, short_name, direction in MODELING_TAG_PATTERNS:
            if pattern.match(tag):
                shape_systs.append((f, f"{modeling_prefix(proc_name)}_{short_name}", direction, "modeling", None, False))
                matched = True
                break
        if matched:
            continue

        m = JEC_PATTERN.match(tag)
        if m:
            source = m.group('name')
            direction = m.group('dir')
            year = m.group('year')
            if _is_year_decorrelated_jec_source(source):
                name = _year_suffixed(f"scale_j_{source}", m)
                shape_systs.append((f, name, direction, "jec_jer", year, False))
            else:
                name = f"scale_j_{source}"
                shape_systs.append((f, name, direction, "jec_jer", year, True))
            continue

        for pattern, name_fn, dir_fn, category, year_fn in SHAPE_SYST_PATTERNS:
            m = pattern.match(tag)
            if m:
                shape_systs.append((f, name_fn(m), dir_fn(m), category, year_fn(m), False))
                matched = True
                break
        if not matched:
            print(f"  WARNING: could not parse systematic tag '{tag}' in {f} -- skipping")
    return shape_systs


def find_nominal_file(proc_dir, proc_name):
    for candidate in (f"{proc_name}_nominal.root", f"{proc_name}.root"):
        path = os.path.join(proc_dir, candidate)
        if os.path.exists(path):
            return path
    return None


def get_available_branches(path, tree_name=TREE_NAME):
    f = ROOT.TFile.Open(path)
    if not f or f.IsZombie():
        return set()
    t = f.Get(tree_name)
    if not t:
        f.Close()
        return set()
    names = {b.GetName() for b in t.GetListOfBranches()}
    f.Close()
    return names


def required_branches_for_cut(cut_expr):
    """Crude tokenizer: pull out identifiers that aren't immediately followed
    by '(' (so we don't pick up function calls), used to sanity-check that a
    cut expression's branches actually exist before handing it to RDataFrame."""
    return set(re.findall(r'\b[A-Za-z_][A-Za-z0-9_]*\b(?!\s*\()', cut_expr))


def nominal_weight_expr(proc_name, is_data, available_branches):
    if is_data:
        return "1.0"
    branches = [b for b in CENTRAL_WEIGHT_BRANCHES if b in available_branches]
    missing = [b for b in CENTRAL_WEIGHT_BRANCHES if b not in available_branches]
    if missing:
        print(f"  WARNING: {proc_name}: central weight branch(es) not found, "
              f"dropped from weight product: {missing}")
    expr = "*".join(branches) if branches else "1.0"
    if TTBAR_DILEPT_PATTERN.search(proc_name):
        if TOP_PT_WEIGHT_BRANCH in available_branches:
            expr = f"({expr})*{TOP_PT_WEIGHT_BRANCH}"
        else:
            print(f"  WARNING: {proc_name}: matched ttbar-dilept pattern but "
                  f"'{TOP_PT_WEIGHT_BRANCH}' branch not found -- skipping top-pt reweighting")
    return expr


def hist_model(name, edges):
    return ROOT.RDF.TH1DModel(name, name, len(edges) - 1, edges)


def make_hist(df, variable, weight_col, edges, name):
    return df.Histo1D(hist_model(name, edges), variable, weight_col)  # lazy


def get_sumw_array(path, hist_name, n_needed):
    f = ROOT.TFile.Open(path)
    if not f or f.IsZombie():
        return np.ones(n_needed)
    h = f.Get(hist_name)
    if not h:
        f.Close()
        return np.ones(n_needed)
    sumw = np.array([h.GetBinContent(i + 1) for i in range(h.GetNbinsX())], dtype=float)
    f.Close()
    if len(sumw) == 0 or sumw[0] == 0:
        return np.ones(n_needed)
    norm = np.ones(n_needed)
    for i in range(min(n_needed, len(sumw))):
        norm[i] = sumw[0] / sumw[i] if sumw[i] != 0 else 1.0
    return norm


def build_theory_envelope_multi(nominal_path, df0, wexpr, region_specs, syst_cfg, available_branches):
    """Build (h_up, h_down) envelopes for EVERY region in one event-loop pass.

    Important: the per-index normalization factor is applied via TH1::Scale()
    AFTER filling, not baked into the JIT'd weight expression as a literal.
    Embedding a distinct float literal per (process, file) forces Cling to
    JIT-compile a brand new, never-reused function every single time, which
    is what was driving unbounded memory growth over a long run. Now the
    weight expression only depends on (wexpr, branch, index), so it's
    typically shared/reused across most processes.
    """
    branch, sumw_name, indices, name = (syst_cfg["branch"], syst_cfg["sumw"],
                                         syst_cfg["indices"], syst_cfg["name"])
    if branch not in available_branches:
        print(f"  WARNING: branch '{branch}' not found -- skipping theory systematic '{name}'")
        return {}
    if indices is None:
        f = ROOT.TFile.Open(nominal_path)
        t = f.Get(TREE_NAME)
        t.GetEntry(0)
        try:
            n = len(getattr(t, branch))
        except Exception:
            n = 0
        f.Close()
        indices = list(range(n))
    if not indices:
        return {}

    norm = get_sumw_array(nominal_path, sumw_name, max(indices) + 1)

    # Build every (index, region) lazy node first -- nothing is triggered yet.
    lazy_by_region = {r: [] for r in region_specs}
    d = df0
    for i in indices:
        col = f"w_{name}_{i}"
        d = d.Define(col, f"({wexpr})*{branch}[{i}]")
        for region_name, spec in region_specs.items():
            dfilt = d.Filter(spec["cut"])
            lazy_by_region[region_name].append(
                dfilt.Histo1D(hist_model(f"tmp_{name}_{i}_{region_name}", spec["edges"]),
                               spec["variable"], col)
            )

    # Now trigger: the first .GetValue() call runs ONE event loop that fills
    # every lazy node created above (all indices, all regions) together.
    results = {}
    for region_name, lazy_list in lazy_by_region.items():
        triggered = []
        for pos, h in enumerate(lazy_list):
            hv = h.GetValue()
            hv.Scale(norm[indices[pos]])
            triggered.append(hv)
        h_up = triggered[0].Clone(f"{name}Up_{region_name}")
        h_down = triggered[0].Clone(f"{name}Down_{region_name}")
        nb = h_up.GetNbinsX()
        for b in range(1, nb + 1):
            vals = [h.GetBinContent(b) for h in triggered]
            h_up.SetBinContent(b, max(vals))
            h_down.SetBinContent(b, min(vals))
        results[region_name] = (h_up, h_down)
    return results


# --------------------------------------------------------------------------
# BINNING
# --------------------------------------------------------------------------

def compute_equal_weight_edges(era_process_pairs, region_cfg):
    """era_process_pairs: [(era_name, proc), ...] -- pooled across every era
    (and every process) that should contribute to this region's binning, each
    process looked up under ITS OWN era's ERA_BASE_DIRS directory."""
    all_vals, all_wts = [], []
    for era_name, proc in era_process_pairs:
        if proc == DATA_PROCESS_NAME or proc in SIGNAL_PROCESSES_EXCLUDED_FROM_BINNING:
            continue
        proc_dir = os.path.join(ERA_BASE_DIRS[era_name], proc)
        nominal = find_nominal_file(proc_dir, proc)
        if nominal is None:
            continue
        try:
            avail = get_available_branches(nominal)
            required = {region_cfg["variable"]} | required_branches_for_cut(region_cfg["cut"])
            missing_core = sorted(b for b in required if b not in avail)
            if missing_core:
                print(f"  WARNING: [{era_name}] {proc}: nominal file missing required branch(es) "
                      f"{missing_core} for binning; skipping")
                continue
            df = ROOT.RDataFrame(TREE_NAME, nominal)
            wexpr = nominal_weight_expr(proc, False, avail)
            df = df.Define("w_nom", wexpr).Filter(region_cfg["cut"])
            npy = df.AsNumpy([region_cfg["variable"], "w_nom"])
        except Exception as e:
            print(f"  WARNING: [{era_name}] {proc}: failed while computing binning contribution ({e}); skipping")
            continue
        finally:
            if "df" in locals():
                del df
            gc.collect()
        if len(npy[region_cfg["variable"]]) == 0:
            continue
        all_vals.append(np.asarray(npy[region_cfg["variable"]], dtype=float))
        all_wts.append(np.asarray(npy["w_nom"], dtype=float))

    if not all_vals:
        print("  WARNING: no events found for binning; falling back to fixed binning")
        return np.linspace(region_cfg["xmin"], region_cfg["xmax"], region_cfg["nbins"] + 1)

    vals = np.concatenate(all_vals)
    wts = np.concatenate(all_wts)
    order = np.argsort(vals)
    vals, wts = vals[order], wts[order]
    cum = np.cumsum(wts)
    total = cum[-1]

    nbins = region_cfg["nbins"]
    edges = [vals[0]]
    for k in range(1, nbins):
        target = total * k / nbins
        idx = min(np.searchsorted(cum, target), len(vals) - 1)
        edges.append(vals[idx])
    edges.append(vals[-1] * (1.0 + 1e-6) if vals[-1] != 0 else vals[-1] + 1e-6)

    edges = sorted(set(np.round(edges, 10)))
    if len(edges) < 3:
        print("  WARNING: equal-weight binning collapsed to <2 bins; falling back to fixed binning")
        return np.linspace(region_cfg["xmin"], region_cfg["xmax"], region_cfg["nbins"] + 1)
    return np.array(edges, dtype=float)


def compute_fixed_edges_with_overflow(region_cfg):
    """Fixed-width bins from xmin to xmax (region_cfg['nbins'] of them), but
    with the upper edge of the LAST bin pushed out to OVERFLOW_EDGE. This
    keeps the visible bin boundaries evenly spaced up to xmax while still
    catching every event above xmax in that final bin, rather than losing
    them to ROOT's normal (invisible, unfilled-in-the-template) overflow
    bin."""
    edges = np.linspace(region_cfg["xmin"], region_cfg["xmax"], region_cfg["nbins"] + 1)
    edges[-1] = OVERFLOW_EDGE
    return edges


# --------------------------------------------------------------------------
# MAIN
# --------------------------------------------------------------------------

def report_yield_ratio(proc, syst_label, region_name, h_nom, h_var, category=None):
    if not PRINT_YIELD_RATIOS:
        return
    nom_integral = h_nom.Integral()
    var_integral = h_var.Integral()
    if nom_integral == 0:
        return
    ratio = var_integral / nom_integral
    if abs(ratio - 1.0) > (YIELD_RATIO_WARN_THRESHOLD - 1.0):
        cat_str = f" [{category}]" if category else ""
        print(f"    YIELD RATIO FLAG: {proc}_{syst_label} ({region_name}){cat_str}: "
              f"{var_integral:.3g} / {nom_integral:.3g} = {ratio:.2f}x nominal")


def compute_all_edges(era_process_map):
    """era_process_map: {era_name: [proc, ...]} -- the processes actually
    found under each era's ERA_BASE_DIRS directory (see build_shapes()).
    Binning pools are built by flattening this across every included era, so
    the resulting edges are computed once from ALL eras' combined statistics
    and then reused identically for every era's output file."""
    edges_by_region = {}
    for region_name, region_cfg in REGIONS.items():
        print(f"\n=== Region {region_name} ({region_cfg['variable']}) binning ===")
        mode = region_cfg.get("binning_mode", "fixed_with_overflow")
        if mode == "equal_sb_yield":
            # Equal weighted-yield bins over EVERY signal+background process,
            # pooled across every era. compute_equal_weight_edges() already
            # skips DATA_PROCESS_NAME and anything in
            # SIGNAL_PROCESSES_EXCLUDED_FROM_BINNING (empty by default, so
            # signal is included in the split too).
            binning_pool = [(era, proc) for era, procs in era_process_map.items()
                             for proc in procs if proc != DATA_PROCESS_NAME]
            edges = compute_equal_weight_edges(binning_pool, region_cfg)
            print(f"  equal S+B-yield bin edges (pool={len(binning_pool)} era/process pairs): {edges}")
        elif mode == "equal_ttbar_yield":
            # legacy, kept for reference
            binning_pool = [(era, proc) for era, procs in era_process_map.items()
                             for proc in procs if proc in TTBAR_BINNING_PROCESSES]
            edges = compute_equal_weight_edges(binning_pool, region_cfg)
            print(f"  equal-ttbar-yield bin edges: {edges}")
        elif mode == "fixed_with_overflow":
            edges = compute_fixed_edges_with_overflow(region_cfg)
            print(f"  fixed bin edges (last bin catches overflow above "
                  f"{region_cfg['xmax']}): {edges}")
        else:
            raise ValueError(f"{region_name}: unknown binning_mode '{mode}'")
        edges_by_region[region_name] = edges
    return edges_by_region


def aggregate_by_group(region_procs):
    """region_procs: {proc_name: {key: TH1}} where key is 'nominal' or
    '<syst>Up'/'<syst>Down'. Returns {target_name: {key: TH1}} where
    target_name is the FILE_GROUPS group name (summed over its members, with
    any member missing a given systematic falling back to ITS OWN nominal for
    that key -- same convention used for the plotting uncertainty band), or
    the original process name for anything not listed in FILE_GROUPS. "Data"
    is passed straight through, ungrouped."""
    members_by_target = {}
    for proc in region_procs:
        if proc == DATA_PROCESS_NAME:
            continue
        target = PROC_TO_GROUP.get(proc, proc)
        members_by_target.setdefault(target, []).append(proc)

    result = {}
    if DATA_PROCESS_NAME in region_procs:
        result[DATA_PROCESS_NAME] = region_procs[DATA_PROCESS_NAME]

    for target, members in members_by_target.items():
        all_keys = set()
        for m in members:
            all_keys |= set(region_procs[m].keys())
        combined = {}
        for key in all_keys:
            h_sum = None
            for m in members:
                h = region_procs[m].get(key, region_procs[m].get("nominal"))
                if h is None:
                    continue
                if h_sum is None:
                    h_sum = h.Clone(f"{target}_{key}_tmp")
                else:
                    h_sum.Add(h)
            if h_sum is not None:
                combined[key] = ensure_nonempty(h_sum)
        result[target] = combined
    return result

def build_combined_shapes(era_names):
    """Sum every per-era shapes_<era>.root file into one shapes_combined.root,
    histogram-for-histogram (same TDirectory/key names in each), so the
    existing plot_region() can be reused unchanged on a single all-eras-
    summed file. This is safe because compute_all_edges() computes bin
    edges ONCE and reuses them identically across every era's output file
    -- so every era's histograms for a given region already share the same
    binning and can be added directly."""
    combined_path = "shapes_combined.root"
    outfile = ROOT.TFile.Open(combined_path, "RECREATE")
    for region_name in REGIONS:
        outfile.mkdir(channel_name(region_name))

    accum = {region_name: {} for region_name in REGIONS}  # hist_name -> TH1 (running sum)

    for era_name in era_names:
        path = f"shapes_{era_name}.root"
        infile = ROOT.TFile.Open(path)
        if not infile or infile.IsZombie():
            print(f"WARNING: could not open {path} for combining -- skipping this era")
            continue
        for region_name in REGIONS:
            region_dir = infile.Get(channel_name(region_name))
            if not region_dir:
                continue
            for key in region_dir.GetListOfKeys():
                name = key.GetName()
                h = region_dir.Get(name)
                if not h:
                    continue
                if name in accum[region_name]:
                    accum[region_name][name].Add(h)
                else:
                    hc = h.Clone(f"{name}_combined_tmp")
                    hc.SetDirectory(0)  # detach so infile.Close() doesn't invalidate it
                    accum[region_name][name] = hc
        infile.Close()

    for region_name in REGIONS:
        outfile.cd(channel_name(region_name))
        for name, h in accum[region_name].items():
            h.Write(name)

    outfile.Close()
    print(f"\n[combined] Wrote {combined_path} (summed over eras: {era_names})")
    return combined_path


def save_mc_only_stack(region_name, region_cfg, display_nominal_hists, mc_order,
                        band_graph, display_total_nominal, out_dir, era_name):
    """Same background stack + systematic uncertainty band as plot_region(),
    but with no data points and no ratio panel. Written alongside the
    data/MC comparison plot in the same era output directory, as
    'mcstack_<region>.pdf'. Uses CLONES of the display histograms so this
    never mutates anything the main data/MC canvas already drew (e.g. the
    x-axis label size that gets zeroed out when a ratio panel is present)."""
    stack_mc = ROOT.THStack(f"mcstack_{region_name}_{era_name}", "")
    legend_mc = ROOT.TLegend(0.71, 0.63, 0.89, 0.89)   # narrower + shorter box
    legend_mc.SetBorderSize(0)
    legend_mc.SetFillStyle(0)
    legend_mc.SetTextSize(0.022)                        # smaller text
    legend_mc.SetNColumns(1)                             # keep 1 col, or try 2 if it's tall

    for i, (target, label) in enumerate(mc_order):
        h = display_nominal_hists[target].Clone(
            f"{display_nominal_hists[target].GetName()}_mcclone")
        h.SetFillColor(GROUP_COLORS.get(target, FILL_COLORS[i % len(FILL_COLORS)]))
        h.SetLineColor(ROOT.kBlack)
        h.SetLineWidth(1)
        stack_mc.Add(h)
        legend_mc.AddEntry(h, label, "f")

    canvas_mc = ROOT.TCanvas(f"c_{region_name}_mc_{era_name}", f"{region_name}_mc", 700, 700)
    stack_mc.Draw("HIST")
    stack_mc.GetXaxis().SetTitle(region_cfg["variable"])
    stack_mc.GetYaxis().SetTitle("Events")
    stack_mc.SetMaximum(1.5 * display_total_nominal.GetMaximum())
    band_graph.Draw("2 SAME")
    legend_mc.AddEntry(band_graph, "Total syst. unc.", "f")
    legend_mc.Draw()

    out_path = os.path.join(out_dir, era_name, f"mcstack_{region_name}.pdf")
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    canvas_mc.SaveAs(out_path)
    print(f"  wrote {out_path}")
def build_shapes_for_era(era_name, base_dir, all_processes, edges_by_region):
    out_path = f"shapes_{era_name}.root"
    outfile = ROOT.TFile.Open(out_path, "RECREATE")
    for region_name in REGIONS:
        outfile.mkdir(channel_name(region_name))

    # accum[region_name][proc] = {"nominal": TH1, "<syst>Up": TH1, ...}
    accum = {region_name: {} for region_name in REGIONS}

    for proc in all_processes:
        print(f"\n--- [{era_name}] Process {proc} ---")
        is_data = (proc == DATA_PROCESS_NAME)
        proc_dir = os.path.join(base_dir, proc)
        nominal_path = find_nominal_file(proc_dir, proc)
        if nominal_path is None:
            print(f"  WARNING: no nominal file found for {proc} in {proc_dir}, skipping")
            continue

        avail = get_available_branches(nominal_path)
        wexpr = nominal_weight_expr(proc, is_data, avail)

        usable_regions = {}
        for region_name, region_cfg in REGIONS.items():
            # No datasetId/era cut here -- this era's directory already only
            # contains this era's events, so the region cut alone is enough.
            full_cut = region_cfg["cut"]
            required = {region_cfg["variable"]} | required_branches_for_cut(full_cut)
            missing_core = sorted(b for b in required if b not in avail)
            if missing_core:
                print(f"  WARNING: {proc}: nominal file missing required branch(es) "
                      f"{missing_core} for region {region_name}; skipping that region")
                continue
            usable_regions[region_name] = dict(region_cfg, cut=full_cut)
        if not usable_regions:
            print(f"  WARNING: {proc}: no usable regions for era {era_name}, skipping process entirely")
            continue

        try:
            df0 = ROOT.RDataFrame(TREE_NAME, nominal_path).Define("w_nom", wexpr)
            lazy_nom = {}
            for region_name, region_cfg in usable_regions.items():
                dfilt = df0.Filter(region_cfg["cut"])
                lazy_nom[region_name] = make_hist(dfilt, region_cfg["variable"], "w_nom",
                                                   edges_by_region[region_name], f"{proc}_nom_{era_name}")
        except Exception as e:
            print(f"  WARNING: {proc}: failed booking nominal histogram(s) ({e}); skipping process entirely")
            continue

        # ---- book SF weight systematics on the SAME df0, without triggering
        # anything yet. RDataFrame only reads/decompresses the tree once per
        # *trigger* (first .GetValue() call), and that one pass fills every
        # not-yet-triggered lazy node booked on this source -- nominal AND
        # every SF systematic below. Booking everything up front and
        # triggering once (right after this block) turns what used to be 1
        # (nominal) + up to 8 (one per SF systematic) = up to 9 full-tree
        # passes into a single pass. Data files naturally book zero SF
        # systematics here (the central-weight branches they need aren't
        # present), so this is a no-op overhead-wise for Data.
        sf_lazy = {}  # syst_name -> (lazy_up_by_region, lazy_dn_by_region)
        for central, up_b, down_b, syst_name in ([] if NOMINAL_ONLY else SF_BRANCH_SYSTEMATICS):
            if not systematic_enabled(syst_name, "sf"):
                continue
            if central not in avail:
                continue
            if up_b not in avail or down_b not in avail:
                print(f"  WARNING: {proc}: '{up_b}'/'{down_b}' not found -- skipping {syst_name}")
                continue
            try:
                up_expr = f"({wexpr})/({central})*({up_b})"
                down_expr = f"({wexpr})/({central})*({down_b})"
                # Column names are unique per systematic (rather than the old
                # shared "w_up"/"w_dn") since all of these now coexist on the
                # same df0 graph before anything is triggered.
                d_up0 = df0.Define(f"w_up_{syst_name}", up_expr)
                d_dn0 = df0.Define(f"w_dn_{syst_name}", down_expr)
                lazy_up, lazy_dn = {}, {}
                for region_name, region_cfg in usable_regions.items():
                    cut, variable = region_cfg["cut"], region_cfg["variable"]
                    lazy_up[region_name] = make_hist(d_up0.Filter(cut), variable, f"w_up_{syst_name}",
                                                      edges_by_region[region_name], f"{proc}_{syst_name}Up_{era_name}")
                    lazy_dn[region_name] = make_hist(d_dn0.Filter(cut), variable, f"w_dn_{syst_name}",
                                                      edges_by_region[region_name], f"{proc}_{syst_name}Down_{era_name}")
                sf_lazy[syst_name] = (lazy_up, lazy_dn)
            except Exception as e:
                print(f"  WARNING: {proc}: failed booking {syst_name} ({e}); skipping")

        # ---- trigger: the FIRST .GetValue() call below runs the single
        # combined event loop that fills the nominal histograms and every
        # booked SF systematic histogram together. ----
        try:
            nominal_hists = {}
            for region_name, h in lazy_nom.items():
                # Keep the RAW (unfloored) nominal as the yield-ratio baseline
                # below -- flooring it first would turn a legitimately empty
                # nominal (0 events passing the cut) into a ~1e-6 stand-in,
                # which then makes every systematic's ratio-to-nominal blow
                # up to absurd values instead of being skipped as
                # uninformative. The floor is only applied to the copy that
                # actually gets written to the output file.
                hv_raw = h.GetValue()
                nominal_hists[region_name] = hv_raw
                accum[region_name].setdefault(proc, {})["nominal"] = ensure_nonempty(hv_raw.Clone())
        except Exception as e:
            print(f"  WARNING: {proc}: failed computing nominal histogram(s) ({e}); skipping process entirely")
            del df0, sf_lazy
            gc.collect()
            continue

        n_sf_written, n_theory_written, n_shape_written = 0, 0, 0

        # ---- SF weight systematics: read back results from the same pass
        # triggered above -- no additional event loop here. ----
        for syst_name, (lazy_up, lazy_dn) in sf_lazy.items():
            try:
                out_syst_name = apply_era_correlation(syst_name, era_name, already_has_year=False)
                for region_name in lazy_up:
                    hu = lazy_up[region_name].GetValue()
                    hd = lazy_dn[region_name].GetValue()
                    # Report against the RAW nominal (see comment above) before
                    # flooring either side for storage. Use the original
                    # (non-era-suffixed) name for this printout -- it's just a
                    # human-readable label.
                    report_yield_ratio(proc, f"{syst_name}Up", region_name, nominal_hists[region_name], hu)
                    report_yield_ratio(proc, f"{syst_name}Down", region_name, nominal_hists[region_name], hd)
                    accum[region_name][proc][f"{out_syst_name}Up"] = ensure_nonempty(hu)
                    accum[region_name][proc][f"{out_syst_name}Down"] = ensure_nonempty(hd)
                n_sf_written += 1
            except Exception as e:
                print(f"  WARNING: {proc}: failed reading results for {syst_name} ({e}); skipping")

        if is_data:
            del df0, sf_lazy
            gc.collect()
            continue

        # ---- theory systematics ----
        # Only built for the ttbar and single-tW processes (THEORY_SYST_PROCESSES);
        # every other process is skipped entirely -- no envelope files are even
        # booked, let alone written, for DY/VV/W+jets/ttG/ttV/etc.
        region_specs = {
            r: dict(variable=usable_regions[r]["variable"], cut=usable_regions[r]["cut"],
                     edges=edges_by_region[r])
            for r in usable_regions
        }
        theory_group = modeling_prefix(proc)  # "ttbar" or "st" for the processes allowed below
        run_theory = (not NOMINAL_ONLY) and (proc in THEORY_SYST_PROCESSES)
        for syst_cfg in (THEORY_SYSTEMATICS if run_theory else []):
            if not systematic_enabled(syst_cfg["name"], "theory"):
                continue
            try:
                results = build_theory_envelope_multi(nominal_path, df0, wexpr, region_specs, syst_cfg, avail)
            except Exception as e:
                print(f"  WARNING: {proc}: failed building theory systematic {syst_cfg['name']} ({e}); skipping")
                continue
            if not results:
                continue
            # Shared name across the two members of a group (e.g. TTbar_Dilept
            # and TTbar_SemiLept both write "ttbar_pdf") so they land on ONE
            # nuisance parameter per theory uncertainty per group, not one per
            # process.
            theory_syst_name = f"{theory_group}_{syst_cfg['name']}"
            out_syst_name = apply_era_correlation(theory_syst_name, era_name, already_has_year=False)
            for region_name, (h_up, h_dn) in results.items():
                # Report against the RAW nominal before flooring either side.
                report_yield_ratio(proc, f"{theory_syst_name}Up", region_name, nominal_hists[region_name], h_up)
                report_yield_ratio(proc, f"{theory_syst_name}Down", region_name, nominal_hists[region_name], h_dn)
                accum[region_name][proc][f"{out_syst_name}Up"] = ensure_nonempty(h_up)
                accum[region_name][proc][f"{out_syst_name}Down"] = ensure_nonempty(h_dn)
            n_theory_written += 1

        # ---- file-based ("shape") systematics ----
        # Files carrying an explicit year (JEC/JER/lepton/met_PU) are skipped
        # entirely for eras they don't apply to -- no need to even open them.
        # NOMINAL_ONLY skips even globbing the process directory for these.
        shape_systs = [] if NOMINAL_ONLY else discover_shape_systematics(proc_dir, proc)
        for i_syst, (path, syst_name, direction, category, syst_year, force_correlated) in enumerate(shape_systs):
            if syst_year is not None and syst_year != era_name:
                continue
            if not systematic_enabled(syst_name, category):
                continue
            dsys0 = None
            try:
                sys_avail = get_available_branches(path)
                sys_wexpr = nominal_weight_expr(proc, False, sys_avail)
                dsys0 = ROOT.RDataFrame(TREE_NAME, path).Define("w_nom", sys_wexpr)
                lazy = {}
                for region_name, region_cfg in usable_regions.items():
                    variable, cut = region_cfg["variable"], region_cfg["cut"]
                    required = {variable} | required_branches_for_cut(cut)
                    missing_core = sorted(b for b in required if b not in sys_avail)
                    if missing_core:
                        print(f"  WARNING: {proc}: {syst_name}{direction} file missing required "
                              f"branch(es) {missing_core} for region {region_name} -- skipping that "
                              f"region ({os.path.basename(path)})")
                        continue
                    lazy[region_name] = make_hist(dsys0.Filter(cut), variable, "w_nom",
                                                   edges_by_region[region_name],
                                                   f"{proc}_{syst_name}{direction}_{era_name}")
                if not lazy:
                    continue
                # JEC/JER/met_PU/lepton-scale names already have the year
                # baked in (syst_year is not None) -- leave them alone.
                # Modeling variations (CR1/CR2/tune/hdamp/mtop/UE) have no
                # year in the name yet (syst_year is None), so they go
                # through the same correlated/uncorrelated logic as the SF
                # and theory systematics above.
                if force_correlated:
                    # Correlated regrouped-JES source (Absolute/BBEC1/EC2/HF/
                    # FlavorQCD/RelativeBal/...): store under the SAME name
                    # regardless of era, on purpose, even though it came from
                    # a separate per-era file.
                    out_syst_name = syst_name
                else:
                    out_syst_name = apply_era_correlation(syst_name, era_name, already_has_year=(syst_year is not None))
                for region_name, h in lazy.items():
                    hv = h.GetValue()
                    # Report against the RAW nominal before flooring for storage.
                    report_yield_ratio(proc, f"{syst_name}{direction}", region_name,
                                        nominal_hists[region_name], hv, category=category)
                    accum[region_name][proc][f"{out_syst_name}{direction}"] = ensure_nonempty(hv)
                n_shape_written += 1
            except Exception as e:
                print(f"  WARNING: {proc}: failed building shape systematic {syst_name}{direction} "
                      f"from {path} ({e}); skipping")
            finally:
                # PyROOT only closes the underlying TFile once this RDataFrame
                # (and every lazy node built on top of it) is garbage
                # collected. Drop the reference and force collection right
                # here instead of letting file handles pile up across the
                # ~100+ systematic files per process -- this is what was
                # causing "Too many open files".
                del dsys0
                if "lazy" in dir():
                    del lazy
                if (i_syst % 10) == 0:
                    gc.collect()



        print(f"  [{era_name}] wrote {proc}: nominal({sorted(usable_regions)}) + {n_sf_written} SF systs "
              f"+ {n_theory_written} theory systs + {n_shape_written} shape systs")

        # df0 (the nominal RDataFrame) has been sitting open for this entire
        # process's SF/theory systematics too -- drop it and collect now,
        # once per process, rather than only relying on the next loop
        # iteration's reassignment to eventually trigger GC.
        del df0, sf_lazy
        gc.collect()

    # ---- aggregate into FILE_GROUPS and write to disk ----
    for region_name in REGIONS:
        aggregated = aggregate_by_group(accum[region_name])
        outfile.cd(channel_name(region_name))
        ungrouped = [p for p in accum[region_name] if p != DATA_PROCESS_NAME and p not in PROC_TO_GROUP]
        if ungrouped:
            print(f"  WARNING: [{era_name}/{region_name}] process(es) not in FILE_GROUPS, "
                  f"written standalone: {sorted(set(ungrouped))}")
        for target, hist_dict in aggregated.items():
            for key, h in hist_dict.items():
                out_name = target if key == "nominal" else f"{target}_{key}"
                h.Write(out_name)

    outfile.Close()
    print(f"\n[{era_name}] Done. Wrote {out_path}")
    return out_path


def build_shapes():
    # Each era gets its own process listing, since it's now discovered from
    # that era's own directory (ERA_BASE_DIRS[era_name]) rather than shared
    # across eras via one BASE_DIR + datasetId split. An era whose directory
    # doesn't exist yet (not staged, typo'd path, etc.) is skipped entirely
    # rather than crashing the whole run.
    era_process_map = {}
    for era_name in ERAS:
        base_dir = ERA_BASE_DIRS[era_name]
        if not os.path.isdir(base_dir):
            print(f"WARNING: [{era_name}] base dir does not exist: {base_dir} -- skipping era entirely")
            continue
        procs = sorted(
            d for d in os.listdir(base_dir)
            if os.path.isdir(os.path.join(base_dir, d)) and d not in SKIP_PROCESSES
        )
        print(f"[{era_name}] processes found in {base_dir}: {procs}")
        era_process_map[era_name] = procs

    if not era_process_map:
        raise ValueError("No era directories were found on disk -- check ERA_BASE_DIRS")

    # Binning is computed once (combining all eras' statistics, each read
    # from its own directory) and reused identically across all era output
    # files, so bin edges line up.
    edges_by_region = compute_all_edges(era_process_map)

    out_paths = {}
    for era_name, processes in era_process_map.items():
        base_dir = ERA_BASE_DIRS[era_name]
        print(f"\n===================== ERA {era_name} (dir={base_dir}) =====================")
        out_paths[era_name] = build_shapes_for_era(era_name, base_dir, processes, edges_by_region)
        gc.collect()
    return out_paths


ROOT.gStyle.SetOptStat(0)

OUT_DIR = "plots"

# fill colors, cycled if there are more processes than colors
FILL_COLORS = [
    ROOT.kAzure + 1, ROOT.kOrange + 1, ROOT.kGreen + 2, ROOT.kRed - 4,
    ROOT.kViolet - 3, ROOT.kCyan + 1, ROOT.kSpring - 6, ROOT.kPink + 6,
    ROOT.kYellow + 1, ROOT.kTeal + 2, ROOT.kGray + 1, ROOT.kAzure - 6,
    ROOT.kOrange - 3, ROOT.kMagenta - 5, ROOT.kBlue - 7, ROOT.kRed + 2,
    ROOT.kGreen - 3, ROOT.kViolet + 6, ROOT.kCyan - 6, ROOT.kSpring + 6,
]


def root_color(name_or_hex):
    """Turn a CSS-style color name or '#RRGGBB' hex string into a ROOT color index."""
    hexcode = CSS_COLOR_HEX.get(name_or_hex, name_or_hex)
    return ROOT.TColor.GetColor(hexcode)


CSS_COLOR_HEX = {
    "purple":     "#800080",
    "gray":       "#808080",
    "red":        "#FF0000",
    "green":      "#008000",
    "yellow":     "#FFFF00",
    "saddlebrown": "#8B4513",
    "steelblue":  "#4682B4",
}

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
    "drell_yan":          root_color("purple"),
    "ttg_ttv":            root_color("gray"),
    "ttbar_dileptonic":   root_color("red"),
    "ttbar_semileptonic": root_color("green"),
    "single_tW":          root_color("yellow"),
    "vv":                 root_color("saddlebrown"),
    "wjets":              root_color("steelblue"),
}

# Labels use ROOT's TLatex syntax (not matplotlib mathtext): "#bar{t}" for
# t-bar, "#gamma" for gamma, etc.
GROUP_LABELS = {
    "drell_yan":          "Drell-Yan",
    "ttg_ttv":            "t#bar{t}#gamma / t#bar{t}V",
    "ttbar_dileptonic":   "t#bar{t} dileptonic",
    "ttbar_semileptonic": "t#bar{t} semileptonic",
    "single_tW":          "Single tW",
    "vv":                 "Diboson",
    "wjets":              "W+jets",
}

STACK_ORDER = [
    "wjets", "vv", "single_tW", "ttbar_semileptonic",
    "ttg_ttv", "drell_yan", "ttbar_dileptonic",
]

# ASSUMPTION for datacard generation: which FILE_GROUPS group is the signal
# (process index 0 in the datacard; everything else is a background with a
# positive index). Change this if a different group is actually the signal.
SIGNAL_GROUP = "single_tW"


def get_processes(era_name=None):
    """Returns (bkg_targets, has_data). bkg_targets are the top-level names
    actually written into shapes_<era>.root -- i.e. FILE_GROUPS group names,
    plus any process not covered by FILE_GROUPS (written standalone) -- NOT
    the raw per-process directory names, since those get merged into groups
    at write time now.

    era_name selects which era's directory (ERA_BASE_DIRS[era_name]) to list
    processes from. Pass None or "combined" to union the process lists
    across every era in ERAS instead (used for the all-eras-combined plot)."""
    if era_name is None or era_name == "combined":
        base_dirs = [ERA_BASE_DIRS[e] for e in ERAS if e in ERA_BASE_DIRS]
    else:
        base_dirs = [ERA_BASE_DIRS[era_name]] if era_name in ERA_BASE_DIRS else []

    all_processes = set()
    for base_dir in base_dirs:
        if not os.path.isdir(base_dir):
            continue
        all_processes |= {
            d for d in os.listdir(base_dir)
            if os.path.isdir(os.path.join(base_dir, d)) and d not in SKIP_PROCESSES
        }

    has_data = DATA_PROCESS_NAME in all_processes
    bkg_raw = [p for p in all_processes if p != DATA_PROCESS_NAME]
    bkg_targets = sorted({PROC_TO_GROUP.get(p, p) for p in bkg_raw})
    return bkg_targets, has_data


def find_systematic_names(region_dir, bkg_processes):
    keys = [k.GetName() for k in region_dir.GetListOfKeys()]
    syst_names = set()
    for proc in bkg_processes:
        prefix = proc + "_"
        for k in keys:
            if not k.startswith(prefix):
                continue
            if k.endswith("Up"):
                syst_names.add(k[len(prefix):-2])
            elif k.endswith("Down"):
                syst_names.add(k[len(prefix):-4])
    return sorted(syst_names)


def build_uncertainty_band(region_dir, bkg_processes, total_nominal, syst_names):
    nbins = total_nominal.GetNbinsX()
    up_err_sq = np.zeros(nbins)
    down_err_sq = np.zeros(nbins)

    for syst in syst_names:
        shifted_up = total_nominal.Clone(f"tmp_up_{syst}")
        shifted_down = total_nominal.Clone(f"tmp_down_{syst}")
        shifted_up.Reset()
        shifted_down.Reset()
        found_any = False
        for proc in bkg_processes:
            h_nom = region_dir.Get(proc)
            if not h_nom:
                continue
            h_up = region_dir.Get(f"{proc}_{syst}Up")
            h_dn = region_dir.Get(f"{proc}_{syst}Down")
            shifted_up.Add(h_up if h_up else h_nom)
            shifted_down.Add(h_dn if h_dn else h_nom)
            found_any = found_any or h_up or h_dn
        if not found_any:
            continue
        for b in range(1, nbins + 1):
            nom_val = total_nominal.GetBinContent(b)
            du = shifted_up.GetBinContent(b) - nom_val
            dd = shifted_down.GetBinContent(b) - nom_val
            up_err_sq[b - 1] += max(du, dd, 0.0) ** 2
            down_err_sq[b - 1] += max(-du, -dd, 0.0) ** 2

    return np.sqrt(up_err_sq), np.sqrt(down_err_sq)


def make_display_hist(h, region_cfg, suffix):
    """For fixed_with_overflow regions, clone `h` onto NORMAL fixed-width
    edges [xmin, xmax] (undoing the OVERFLOW_EDGE=1e6 push on the last bin)
    -- see compute_fixed_edges_with_overflow(). For any other binning mode
    (e.g. equal_sb_yield), the histogram's edges ARE the real, meaningful
    bin boundaries, so just clone it unchanged -- remapping onto a uniform
    axis would visually erase the variable bin widths."""
    mode = region_cfg.get("binning_mode", "fixed_with_overflow")
    if mode != "fixed_with_overflow":
        return h.Clone(f"{h.GetName()}_{suffix}")

    edges = np.linspace(region_cfg["xmin"], region_cfg["xmax"], region_cfg["nbins"] + 1)
    hd = ROOT.TH1D(f"{h.GetName()}_{suffix}", h.GetTitle(), len(edges) - 1, edges)
    for b in range(1, h.GetNbinsX() + 1):
        hd.SetBinContent(b, h.GetBinContent(b))
        hd.SetBinError(b, h.GetBinError(b))
    return hd


def make_band_graph(total_nominal, band_up, band_down):
    nbins = total_nominal.GetNbinsX()
    graph = ROOT.TGraphAsymmErrors(nbins)
    for b in range(1, nbins + 1):
        x = total_nominal.GetBinCenter(b)
        w = total_nominal.GetBinWidth(b)
        y = total_nominal.GetBinContent(b)
        graph.SetPoint(b - 1, x, y)
        graph.SetPointError(b - 1, w / 2, w / 2, band_down[b - 1], band_up[b - 1])
    graph.SetFillColorAlpha(ROOT.kGray + 2, 0.4)
    graph.SetFillStyle(3345)
    graph.SetLineColor(ROOT.kGray + 2)
    return graph


def make_ratio_band_graph(total_nominal, band_up, band_down):
    """Same band, but expressed as a ratio around 1.0, for the ratio panel."""
    nbins = total_nominal.GetNbinsX()
    graph = ROOT.TGraphAsymmErrors(nbins)
    for b in range(1, nbins + 1):
        x = total_nominal.GetBinCenter(b)
        w = total_nominal.GetBinWidth(b)
        nom = total_nominal.GetBinContent(b)
        if nom <= 0:
            graph.SetPoint(b - 1, x, 1.0)
            graph.SetPointError(b - 1, w / 2, w / 2, 0.0, 0.0)
            continue
        graph.SetPoint(b - 1, x, 1.0)
        graph.SetPointError(b - 1, w / 2, w / 2, band_down[b - 1] / nom, band_up[b - 1] / nom)
    graph.SetFillColorAlpha(ROOT.kGray + 2, 0.4)
    graph.SetFillStyle(3345)
    graph.SetLineColor(ROOT.kGray + 2)
    return graph


def plot_region(region_name, region_cfg, infile, out_dir, era_name):
    region_dir = infile.Get(channel_name(region_name))
    if not region_dir:
        print(f"WARNING: no directory '{channel_name(region_name)}' found for era {era_name}, skipping")
        return

    bkg_targets, has_data = get_processes(era_name)

    # bkg_targets are already the exact top-level names written into
    # shapes_<era>.root (FILE_GROUPS group names + any standalone leftovers),
    # so we can read them directly -- no re-grouping needed here.
    nominal_hists = {}
    for target in bkg_targets:
        h = region_dir.Get(target)
        if h:
            nominal_hists[target] = h.Clone(f"clone_{target}_{era_name}")
    if not nominal_hists:
        print(f"WARNING: [{era_name}] no background histograms found in region {region_name}, skipping")
        return

    # order by ascending total yield so the biggest sits at the bottom
    ordered_targets = sorted(nominal_hists, key=lambda p: nominal_hists[p].Integral())

    total_nominal = None
    for target in ordered_targets:
        if total_nominal is None:
            total_nominal = nominal_hists[target].Clone(f"total_nominal_{era_name}")
        else:
            total_nominal.Add(nominal_hists[target])

    syst_names = find_systematic_names(region_dir, ordered_targets)
    band_up, band_down = build_uncertainty_band(region_dir, ordered_targets, total_nominal, syst_names)
    print(f"  [{era_name}] {region_name}: {len(syst_names)} distinct systematics found -> {syst_names}")

    h_data = region_dir.Get(DATA_PROCESS_NAME) if has_data else None

    # ---- rebuild everything that gets drawn onto NORMAL [xmin, xmax] edges.
    # All the systematics math above stayed on the original (overflow-
    # widened) edges, which is required for TH1::Add()/bin-by-bin
    # consistency across nominal/up/down histograms sharing the same
    # process. band_up/band_down are plain numpy arrays indexed by bin
    # number, not by x-position, so they carry over unchanged. This is what
    # actually fixes the x-axis running out to 1,000,000 -- previously we
    # only tried to *view*-zoom the huge-range axis with SetRangeUser, but
    # THStack/TH1 kept their true bin edges (and hence axis range) fixed to
    # the overflow-widened ones no matter what. ----
    display_nominal_hists = {
        target: make_display_hist(h, region_cfg, era_name)
        for target, h in nominal_hists.items()
    }
    display_total_nominal = make_display_hist(total_nominal, region_cfg, f"total_{era_name}")
    display_h_data = make_display_hist(h_data, region_cfg, f"data_{era_name}") if h_data else None

    band_graph = make_band_graph(display_total_nominal, band_up, band_down)

    stack = ROOT.THStack(f"stack_{region_name}_{era_name}", "")
    legend = ROOT.TLegend(0.62, 0.55, 0.89, 0.89)
    legend.SetBorderSize(0)
    legend.SetFillStyle(0)
    legend.SetTextSize(0.03)

    # Draw in the requested stacking order, then anything not covered by
    # STACK_ORDER (e.g. an ungrouped standalone process) on top.
    drawn = set()
    mc_order = []  # NEW: (target, label) pairs in the order they were stacked
    for group in STACK_ORDER:
        if group not in display_nominal_hists:
            continue
        h = display_nominal_hists[group]
        h.SetFillColor(GROUP_COLORS.get(group, ROOT.kGray))
        h.SetLineColor(ROOT.kBlack)
        h.SetLineWidth(1)
        stack.Add(h)
        legend.AddEntry(h, GROUP_LABELS.get(group, group), "f")
        drawn.add(group)
        mc_order.append((group, GROUP_LABELS.get(group, group)))  # NEW
    leftover = [t for t in ordered_targets if t not in drawn]
    if leftover:
        print(f"  WARNING: [{era_name}/{region_name}] target(s) not listed in STACK_ORDER, "
              f"appended on top: {leftover}")
    for i, target in enumerate(leftover):
        h = display_nominal_hists[target]
        h.SetFillColor(GROUP_COLORS.get(target, FILL_COLORS[i % len(FILL_COLORS)]))
        h.SetLineColor(ROOT.kBlack)
        h.SetLineWidth(1)
        stack.Add(h)
        legend.AddEntry(h, GROUP_LABELS.get(target, target), "f")
        mc_order.append((target, GROUP_LABELS.get(target, target)))  # NEW
    h_data = display_h_data

    canvas = ROOT.TCanvas(f"c_{region_name}", region_name, 700, 700)
    if h_data:
        pad1 = ROOT.TPad("pad1", "pad1", 0, 0.3, 1, 1.0)
        pad1.SetBottomMargin(0.02)
        pad1.Draw()
        pad2 = ROOT.TPad("pad2", "pad2", 0, 0.0, 1, 0.3)
        pad2.SetTopMargin(0.03)
        pad2.SetBottomMargin(0.35)
        pad2.Draw()
        pad1.cd()
    else:
        pad1 = canvas

    stack.Draw("HIST")
    stack.GetXaxis().SetTitle(region_cfg["variable"])
    stack.GetYaxis().SetTitle("Events")
    stack.SetMaximum(1.5 * max(display_total_nominal.GetMaximum(),
                                h_data.GetMaximum() if h_data else 0))
    band_graph.Draw("2 SAME")

    if h_data:
        h_data.SetMarkerStyle(20)
        h_data.SetMarkerSize(0.9)
        h_data.SetLineColor(ROOT.kBlack)
        h_data.Draw("E1 SAME")
        legend.AddEntry(h_data, "Data", "lep")
        stack.GetXaxis().SetLabelSize(0)
        stack.GetXaxis().SetTitleSize(0)

    legend.AddEntry(band_graph, "Total syst. unc.", "f")
    legend.Draw()

    if h_data:
        pad2.cd()
        ratio = h_data.Clone("ratio")
        ratio.Divide(display_total_nominal)
        ratio.SetMarkerStyle(20)
        ratio.SetMarkerSize(0.9)
        ratio.SetLineColor(ROOT.kBlack)
        ratio.SetTitle("")
        ratio.GetYaxis().SetTitle("Data / MC")
        ratio.GetYaxis().SetRangeUser(0.5, 1.5)
        ratio.GetYaxis().SetNdivisions(505)
        ratio.GetXaxis().SetTitle(region_cfg["variable"])
        ratio.GetXaxis().SetLabelSize(0.09)
        ratio.GetXaxis().SetTitleSize(0.10)
        ratio.GetYaxis().SetLabelSize(0.08)
        ratio.GetYaxis().SetTitleSize(0.09)
        ratio.GetYaxis().SetTitleOffset(0.5)
        ratio.Draw("E1")
        ratio_band = make_ratio_band_graph(display_total_nominal, band_up, band_down)
        ratio_band.Draw("2 SAME")
        ratio.Draw("E1 SAME")
        line = ROOT.TLine(region_cfg["xmin"], 1.0, region_cfg["xmax"], 1.0)
        line.SetLineStyle(2)
        line.Draw()

    out_path = os.path.join(out_dir, era_name, f"stack_{region_name}.pdf")
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    canvas.SaveAs(out_path)
    print(f"  wrote {out_path}")
    save_mc_only_stack(region_name, region_cfg, display_nominal_hists, mc_order,
                        band_graph, display_total_nominal, out_dir, era_name)  # NEW

def run_plots():
    for era_name in ERAS:
        shapes_path = f"shapes_{era_name}.root"
        infile = ROOT.TFile.Open(shapes_path)
        if not infile or infile.IsZombie():
            print(f"WARNING: could not open {shapes_path} -- skipping plots for era {era_name} "
                  f"(did build_shapes() run first?)")
            continue

        for region_name, region_cfg in REGIONS.items():
            print(f"\n=== Plotting [{era_name}] {region_name} ===")
            plot_region(region_name, region_cfg, infile, OUT_DIR, era_name)

        infile.Close()

    # ---- one final all-eras-combined plot per region ----
    combined_path = build_combined_shapes(ERAS)
    infile = ROOT.TFile.Open(combined_path)
    if not infile or infile.IsZombie():
        print(f"WARNING: could not open {combined_path} -- skipping combined plots")
        return
    for region_name, region_cfg in REGIONS.items():
        print(f"\n=== Plotting [combined] {region_name} ===")
        plot_region(region_name, region_cfg, infile, OUT_DIR, "combined")
    infile.Close()
# --------------------------------------------------------------------------
# DATACARD GENERATION
# --------------------------------------------------------------------------

def format_table(rows, min_gap=2):
    """Format a block of datacard rows as aligned, readable columns.

    Unlike a single global column width, each column is sized to the widest
    cell actually seen in THAT column (plus `min_gap` spaces of separation),
    so a long systematic/process/channel name only widens its own column
    instead of either getting truncated-looking spacing or running straight
    into the next column with no gap at all. The last cell of each row is
    never padded, so trailing whitespace isn't left on the line.
    """
    if not rows:
        return []
    ncols = max(len(r) for r in rows)
    widths = [0] * ncols
    for r in rows:
        for i, c in enumerate(r):
            widths[i] = max(widths[i], len(str(c)))
    lines = []
    for r in rows:
        cells = []
        last = len(r) - 1
        for i, c in enumerate(r):
            cell = str(c)
            cells.append(cell if i == last else cell.ljust(widths[i] + min_gap))
        lines.append("".join(cells).rstrip())
    return lines


def write_datacard(era_name, shapes_path):
    """Introspects shapes_<era>.root and emits a combine-format text datacard
    for that era, one bin per region, one column per (bin, process) pair that
    actually has a nominal histogram, and one 'shape' row per systematic name
    found anywhere in the file."""
    infile = ROOT.TFile.Open(shapes_path)
    if not infile or infile.IsZombie():
        print(f"WARNING: could not open {shapes_path} for datacard generation -- skipping")
        return None

    bkg_targets, has_data = get_processes(era_name)
    bins = list(REGIONS.keys())

    bin_process_hists = {}   # region -> {target: TH1}
    bin_syst_map = {}        # region -> {target: set(syst_names)}
    bin_data_hist = {}       # region -> TH1 or None

    for region in bins:
        region_dir = infile.Get(channel_name(region))
        if not region_dir:
            bin_process_hists[region] = {}
            bin_syst_map[region] = {}
            bin_data_hist[region] = None
            continue
        keys = [k.GetName() for k in region_dir.GetListOfKeys()]
        proc_hists, syst_map = {}, {}
        for target in bkg_targets:
            h = region_dir.Get(target)
            if h:
                proc_hists[target] = h
                syst_map[target] = set()
        for target in bkg_targets:
            prefix = target + "_"
            for k in keys:
                if not k.startswith(prefix):
                    continue
                if k.endswith("Up"):
                    syst_map.setdefault(target, set()).add(k[len(prefix):-2])
                elif k.endswith("Down"):
                    syst_map.setdefault(target, set()).add(k[len(prefix):-4])
        bin_process_hists[region] = proc_hists
        bin_syst_map[region] = syst_map
        bin_data_hist[region] = region_dir.Get(DATA_PROCESS_NAME) if has_data else None

    all_systs = sorted({s for region in bins for systs in bin_syst_map[region].values() for s in systs})

    # process ordering: signal first (index 0), then STACK_ORDER, then leftovers
    ordered_targets = []
    if SIGNAL_GROUP in bkg_targets:
        ordered_targets.append(SIGNAL_GROUP)
    else:
        print(f"  WARNING: [{era_name}] SIGNAL_GROUP '{SIGNAL_GROUP}' not found among "
              f"processes {bkg_targets} -- datacard will have no signal (index-0) process")
    for g in STACK_ORDER:
        if g in bkg_targets and g not in ordered_targets:
            ordered_targets.append(g)
    for t in bkg_targets:
        if t not in ordered_targets:
            ordered_targets.append(t)

    proc_index = {}
    idx = 0
    for t in ordered_targets:
        if t == SIGNAL_GROUP:
            proc_index[t] = 0
        else:
            idx += 1
            proc_index[t] = idx

    columns = [(region, t) for region in bins for t in ordered_targets if t in bin_process_hists[region]]
    if not columns:
        print(f"  WARNING: [{era_name}] no (bin, process) columns found -- skipping datacard")
        infile.Close()
        return None

    lines = [
        f"# Auto-generated datacard for era {era_name}",
        f"# ASSUMPTION: signal process = '{SIGNAL_GROUP}' (index 0) -- change SIGNAL_GROUP",
        f"# in the script and rerun if a different group is actually the signal.",
        f"imax {len(bins)}  number of channels",
        "jmax *  number of backgrounds",
        "kmax *  number of nuisance parameters",
        "-" * 40,
        f"shapes data_obs * {shapes_path} $CHANNEL/{DATA_PROCESS_NAME}",
        f"shapes *        * {shapes_path} $CHANNEL/$PROCESS $CHANNEL/$PROCESS_$SYSTEMATIC",
        "-" * 40,
    ]

    # --- observation section: "bin" / "observation" rows share one table so
    # each channel's column lines up between the two rows. ---
    obs_row = ["observation"]
    for region in bins:
        h_data = bin_data_hist[region]
        obs_row.append(f"{h_data.Integral():.0f}" if h_data else "-1")
    header_rows = [
        ["bin"] + [channel_name(b) for b in bins],
        obs_row,
    ]
    lines.extend(format_table(header_rows))
    lines.append("-" * 40)

    # --- process section: "bin" / "process" (name) / "process" (index) /
    # "rate" rows share one table so every (bin, process) column lines up
    # vertically across all four rows. Rate is always "-1" (i.e. "read the
    # rate from the nominal histogram's integral") for every process/channel,
    # signal and background alike -- combine computes yields directly from
    # the shapes file rather than a hardcoded number here. ---
    process_rows = [
        ["bin"] + [channel_name(c[0]) for c in columns],
        ["process"] + [c[1] for c in columns],
        ["process"] + [str(proc_index[c[1]]) for c in columns],
        ["rate"] + ["-1"] * len(columns),
    ]
    lines.extend(format_table(process_rows))
    lines.append("-" * 40)

    # --- shape systematics section: one row per systematic name, all sharing
    # one table so long systematic names get their own column width without
    # disturbing the "shape" / "1" / "-" columns that follow them. Names here
    # already reflect the correlated/uncorrelated choice made in
    # apply_era_correlation() when the histograms were written (e.g.
    # "pileup_2022EE" vs a shared "pdf"), so combineCards.py will decorrelate
    # or correlate them across eras purely based on whether the name matches
    # between the per-era cards -- nothing further to do here. Combining the
    # per-era cards into one is left to you (combineCards.py era1=card1.txt
    # era2=card2.txt ... > combined.txt). ---
    shape_rows = []
    for syst in all_systs:
        row = [syst, "shape"]
        for region, t in columns:
            row.append("1" if syst in bin_syst_map[region].get(t, set()) else "-")
        shape_rows.append(row)
    if shape_rows:
        lines.extend(format_table(shape_rows))

    lines.append("* autoMCStats 10")

    out_path = f"datacard_{era_name}.txt"
    with open(out_path, "w") as fh:
        fh.write("\n".join(lines) + "\n")
    infile.Close()
    print(f"[{era_name}] wrote {out_path} ({len(columns)} (bin,process) columns, {len(all_systs)} systematics)")
    return out_path


def write_datacards():
    for era_name in ERAS:
        write_datacard(era_name, f"shapes_{era_name}.root")


# --------------------------------------------------------------------------
# ENTRY POINT
# --------------------------------------------------------------------------

def main():
    build_shapes()
    run_plots()
    write_datacards()


if __name__ == "__main__":
    main()
