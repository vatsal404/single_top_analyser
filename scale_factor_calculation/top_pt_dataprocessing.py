#!/usr/bin/env python3
"""
gen_top_pt.py
=============
Read a text file of XRootD file paths (one per line), open each ROOT /
NanoAOD file via RDataFrame, find gen-level top quarks (pdgId == 6,
GenPart_status == 62), fill a top-pT histogram, and save it to an output
ROOT file.

Usage
-----
    python gen_top_pt.py <filelist.txt> <output_file.root>

Arguments
---------
    filelist.txt  : plain text file with one XRootD path per line (e.g.
                    root://cmsxrootd.fnal.gov///store/mc/.../file.root)
                    Lines starting with '#' and blank lines are ignored.
    output_file   : path for the output ROOT file that will hold the
                    histogram h_gen_top_pt

Requirements
------------
    ROOT >= 6.20 with PyROOT enabled  (comes with CMSSW or standalone ROOT)
"""

import sys
import os
import ROOT

# ── helpers ──────────────────────────────────────────────────────────────────

def collect_file_paths(input_txt: str) -> list:
    """
    Read *input_txt* and return every non-empty, non-comment line
    as a list of XRootD (or local) paths.
    """
    paths = []
    with open(input_txt) as fh:
        for line in fh:
            line = line.strip()
            if line and not line.startswith("#"):
                paths.append(line)
    return paths


def build_chain(file_paths: list, tree_name: str = "Events") -> ROOT.TChain:
    """
    Build a TChain from a list of file paths.
    XRootD paths are used as-is; ROOT resolves them transparently.
    """
    chain = ROOT.TChain(tree_name)
    for p in file_paths:
        chain.Add(p)
    print(f"[info] Chained {len(file_paths)} file(s) into TChain '{tree_name}'")
    return chain


# ── main ─────────────────────────────────────────────────────────────────────

def main():
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)

    input_txt   = sys.argv[1]
    output_file = sys.argv[2]

    if not os.path.isfile(input_txt):
        sys.exit(f"[error] '{input_txt}' is not a file")

    # ── 1. collect file paths ────────────────────────────────────────────────
    file_paths = collect_file_paths(input_txt)
    if not file_paths:
        sys.exit(f"[error] No file paths found in '{input_txt}'")
    print(f"[info] Found {len(file_paths)} file path(s)")

    # ── 2. enable multi-threading (call before RDataFrame construction) ──────
    ROOT.EnableImplicitMT()

    # ── 3. build TChain -> RDataFrame ────────────────────────────────────────
    chain = build_chain(file_paths, tree_name="Events")
    rdf   = ROOT.RDataFrame(chain)

    # ── 4. define GenTop_pt column ───────────────────────────────────────────
    #   NanoAOD branches used:
    #     GenPart_pdgId  : PDG ID of each gen particle
    #     GenPart_pt     : pT of each gen particle
    #     GenPart_status : Pythia8 status code
    #
    #   Top quark selection:
    #     * |pdgId| == 6    -> top or anti-top
    #     * status == 62    -> top just before decay (last copy in hard process)
    # ─────────────────────────────────────────────────────────────────────────

    rdf_with_top_pt = rdf.Define(
        "GenTop_pt",
        """
        ROOT::VecOps::RVec<float> top_pt;
        for (unsigned int i = 0; i < GenPart_pt.size(); ++i) {
            bool isTop      = (abs(GenPart_pdgId[i]) == 6);
            bool isPreDecay = (GenPart_status[i] == 62);
            if (isTop && isPreDecay) {
                top_pt.push_back(GenPart_pt[i]);
            }
        }
        return top_pt;
        """
    )

    # ── 5. filter events with at least one gen top ───────────────────────────
    rdf_has_top = rdf_with_top_pt.Filter(
        "GenTop_pt.size() > 0",
        "At least one gen-level top quark"
    )

    # ── 6. histogram of top pT ───────────────────────────────────────────────
    #   Histo1D flattens the RVec automatically -- each element is one entry,
    #   so both the top and anti-top fill the same histogram.
    h_top_pt = rdf_has_top.Histo1D(
        ROOT.RDF.TH1DModel(
            "h_gen_top_pt",
            "Gen-level top quark p_{T};p_{T} [GeV];Entries",
            100,     # bins
            0.0,     # x-min [GeV]
            1000.0   # x-max [GeV]
        ),
        "GenTop_pt"
    )

    # ── 7. write output ──────────────────────────────────────────────────────
    print("[info] Processing events ... (this may take a while for remote files)")

    out = ROOT.TFile.Open(output_file, "RECREATE")
    if not out or out.IsZombie():
        sys.exit(f"[error] Cannot create output file '{output_file}'")

    # Trigger the event loop
    hist = h_top_pt.GetValue()
    hist.SetDirectory(out)
    hist.Write()

    # Print cut-flow report
    print("\n[info] Cut-flow report:")
    rdf_has_top.Report().Print()

    out.Close()
    print(f"\n[done] Histogram written to '{output_file}'  ->  h_gen_top_pt")


if __name__ == "__main__":
    main()
