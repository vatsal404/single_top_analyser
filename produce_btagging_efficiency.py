"""
B-tagging efficiency calculator.
Reads the RVec branches using a pure C++ TTree loop compiled with
gInterpreter.ProcessLine / gROOT.Macro, bypassing all PyROOT/cppyy
template issues with the old RAdoptAllocator type.

Usage:
    python3 produce_btagging_efficiency.py <input.root> <output.root>
"""

import ROOT
import sys
import os
import array
import tempfile

def calculate_btagging_efficiency(input_file, output_file):

    print(f"Opening: {input_file}")

    # Verify the file exists and has the tree
    f_check = ROOT.TFile.Open(input_file, "READ")
    if not f_check or f_check.IsZombie():
        print(f"ERROR: Cannot open {input_file}")
        return False
    tree = f_check.Get("outputTree")
    if not tree:
        print("ERROR: Cannot find outputTree")
        f_check.Close()
        return False
    n_entries = tree.GetEntries()
    print(f"Tree has {n_entries} entries")
    f_check.Close()

    # ------------------------------------------------------------------
    # Write a self-contained C++ macro that does the full loop and writes
    # the output ROOT file.  This completely avoids PyROOT touching any
    # of the RAdoptAllocator branches.
    # ------------------------------------------------------------------
    abs_input  = os.path.abspath(input_file)
    abs_output = os.path.abspath(output_file)

    cpp_code = f"""
#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include "ROOT/RVec.hxx"
#include <iostream>
#include <vector>

void run_btag_efficiency() {{

    // ---- open input ----
    TFile* f_in = TFile::Open("{abs_input}", "READ");
    if (!f_in || f_in->IsZombie()) {{
        std::cerr << "ERROR: cannot open input file" << std::endl;
        return;
    }}
    TTree* tree = (TTree*)f_in->Get("outputTree");
    if (!tree) {{
        std::cerr << "ERROR: cannot find outputTree" << std::endl;
        f_in->Close();
        return;
    }}

    Long64_t nEntries = tree->GetEntries();
    std::cout << "Processing " << nEntries << " events..." << std::endl;

    // ---- branch buffers ----
    // Use std::vector<float> so SetBranchAddress maps cleanly
    // regardless of how the branch was originally written
    std::vector<float> bc_pass_eta, bc_pass_pt;
    std::vector<float> bc_all_eta,  bc_all_pt;
    std::vector<float> lf_pass_eta, lf_pass_pt;
    std::vector<float> lf_all_eta,  lf_all_pt;
    double evWeight = 1.0;

    tree->SetBranchStatus("*", 0);
    tree->SetBranchStatus("goodJets_btagpass_bcflav_eta", 1);
    tree->SetBranchStatus("goodJets_btagpass_bcflav_pt",  1);
    tree->SetBranchStatus("goodJets_all_bcflav_eta",      1);
    tree->SetBranchStatus("goodJets_all_bcflav_pt",       1);
    tree->SetBranchStatus("goodJets_btagpass_lflav_eta",  1);
    tree->SetBranchStatus("goodJets_btagpass_lflav_pt",   1);
    tree->SetBranchStatus("goodJets_all_lflav_eta",       1);
    tree->SetBranchStatus("goodJets_all_lflav_pt",        1);
    tree->SetBranchStatus("evWeight",                     1);

    tree->SetBranchAddress("goodJets_btagpass_bcflav_eta", &bc_pass_eta);
    tree->SetBranchAddress("goodJets_btagpass_bcflav_pt",  &bc_pass_pt);
    tree->SetBranchAddress("goodJets_all_bcflav_eta",      &bc_all_eta);
    tree->SetBranchAddress("goodJets_all_bcflav_pt",       &bc_all_pt);
    tree->SetBranchAddress("goodJets_btagpass_lflav_eta",  &lf_pass_eta);
    tree->SetBranchAddress("goodJets_btagpass_lflav_pt",   &lf_pass_pt);
    tree->SetBranchAddress("goodJets_all_lflav_eta",       &lf_all_eta);
    tree->SetBranchAddress("goodJets_all_lflav_pt",        &lf_all_pt);
    tree->SetBranchAddress("evWeight",                     &evWeight);

    // ---- histograms ----
    double eta_bins[] = {{-2.5, -1.5, 1.5, 2.5}};
    double pt_bins[]  = {{20, 30, 50, 70, 100, 140, 200, 300, 600, 1000}};
    int n_eta = 3;
    int n_pt  = 9;

    TH2D h_bc_pass("hist_bcflav_pass", "B-tagged b/c-flavor jets;#eta;p_{{T}} [GeV]",
                   n_eta, eta_bins, n_pt, pt_bins);
    TH2D h_bc_all ("hist_bcflav_all",  "All b/c-flavor jets;#eta;p_{{T}} [GeV]",
                   n_eta, eta_bins, n_pt, pt_bins);
    TH2D h_lf_pass("hist_lflav_pass",  "B-tagged light-flavor jets;#eta;p_{{T}} [GeV]",
                   n_eta, eta_bins, n_pt, pt_bins);
    TH2D h_lf_all ("hist_lflav_all",   "All light-flavor jets;#eta;p_{{T}} [GeV]",
                   n_eta, eta_bins, n_pt, pt_bins);

    h_bc_pass.Sumw2(); h_bc_all.Sumw2();
    h_lf_pass.Sumw2(); h_lf_all.Sumw2();

    // Detach from any auto-directory so we control where they are written
    h_bc_pass.SetDirectory(0); h_bc_all.SetDirectory(0);
    h_lf_pass.SetDirectory(0); h_lf_all.SetDirectory(0);

    // ---- event loop ----
    for (Long64_t i = 0; i < nEntries; ++i) {{
        if (i % 100000 == 0)
            std::cout << "  Event " << i << " / " << nEntries << std::endl;

        tree->GetEntry(i);

        for (size_t j = 0; j < bc_pass_eta.size(); ++j)
            h_bc_pass.Fill(bc_pass_eta[j], bc_pass_pt[j], evWeight);

        for (size_t j = 0; j < bc_all_eta.size(); ++j)
            h_bc_all.Fill(bc_all_eta[j], bc_all_pt[j], evWeight);

        for (size_t j = 0; j < lf_pass_eta.size(); ++j)
            h_lf_pass.Fill(lf_pass_eta[j], lf_pass_pt[j], evWeight);

        for (size_t j = 0; j < lf_all_eta.size(); ++j)
            h_lf_all.Fill(lf_all_eta[j], lf_all_pt[j], evWeight);
    }}

    std::cout << "Event loop done." << std::endl;
    f_in->Close();

    // ---- efficiency histograms ----
    TH2D h_eff_bc("hist_btagEff_bcflav",
                  "B-tagging Efficiency (b/c-flavor);#eta;p_{{T}} [GeV]",
                  n_eta, eta_bins, n_pt, pt_bins);
    TH2D h_eff_lf("hist_btagEff_lflav",
                  "B-tagging Efficiency (light-flavor);#eta;p_{{T}} [GeV]",
                  n_eta, eta_bins, n_pt, pt_bins);
    h_eff_bc.SetDirectory(0);
    h_eff_lf.SetDirectory(0);

    h_eff_bc.Divide(&h_bc_pass, &h_bc_all, 1.0, 1.0, "B");
    h_eff_lf.Divide(&h_lf_pass, &h_lf_all, 1.0, 1.0, "B");

    // ---- summary ----
    double bc_all_n  = h_bc_all.Integral();
    double bc_pass_n = h_bc_pass.Integral();
    double lf_all_n  = h_lf_all.Integral();
    double lf_pass_n = h_lf_pass.Integral();

    std::cout << "\\n=== Summary ===" << std::endl;
    std::cout << "b/c-flavor jets:" << std::endl;
    std::cout << "  Total:      " << bc_all_n  << std::endl;
    std::cout << "  Passed:     " << bc_pass_n << std::endl;
    if (bc_all_n > 0)
        std::cout << "  Efficiency: " << 100.0 * bc_pass_n / bc_all_n << "%" << std::endl;
    std::cout << "Light-flavor jets:" << std::endl;
    std::cout << "  Total:      " << lf_all_n  << std::endl;
    std::cout << "  Passed:     " << lf_pass_n << std::endl;
    if (lf_all_n > 0)
        std::cout << "  Efficiency: " << 100.0 * lf_pass_n / lf_all_n << "%" << std::endl;

    // ---- write output ----
    TFile* f_out = TFile::Open("{abs_output}", "RECREATE");
    if (!f_out || f_out->IsZombie()) {{
        std::cerr << "ERROR: cannot create output file" << std::endl;
        return;
    }}
    h_eff_bc.Write();
    h_eff_lf.Write();
    h_bc_pass.Write();
    h_bc_all.Write();
    h_lf_pass.Write();
    h_lf_all.Write();
    f_out->Close();

    std::cout << "\\nDone! Output written to {abs_output}" << std::endl;
}}
"""

    # Write C++ to a temp file and execute it with ROOT
    tmp = tempfile.NamedTemporaryFile(suffix=".C", mode="w", delete=False)
    tmp.write(cpp_code)
    tmp.flush()
    tmp.close()

    print("Compiling and running C++ event loop ...")
    # .x executes the macro; + suffix requests ACLiC compilation for speed
    ret = ROOT.gROOT.Macro(tmp.name + "+")
    os.unlink(tmp.name)

    # ACLiC also leaves a shared lib; clean up
    for ext in ["_C.so", "_C.d", "_C_ACLiC_dict_rdict.pcm"]:
        leftover = tmp.name.replace(".C", ext)
        if os.path.exists(leftover):
            os.unlink(leftover)

    if ret != 0:
        print(f"ERROR: C++ macro returned code {ret}")
        return False

    return True


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 produce_btagging_efficiency.py <input.root> <output.root>")
        sys.exit(1)

    ok = calculate_btagging_efficiency(sys.argv[1], sys.argv[2])
    sys.exit(0 if ok else 1)
