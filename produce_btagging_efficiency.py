import ROOT
import sys
import array

def calculate_btagging_efficiency(input_file, output_file):
    """
    Calculate b-tagging efficiency from NanoAOD analysis output.
    Creates 2D histograms (eta vs pt) for b/c-flavor and light-flavor jets.
    
    Args:
        input_file: Path to input ROOT file
        output_file: Path to output ROOT file
    """
    
    # Open input file
    print(f"Opening input file: {input_file}")
    f_in = ROOT.TFile.Open(input_file, "READ")
    if not f_in or f_in.IsZombie():
        print(f"ERROR: Cannot open input file {input_file}")
        return False
    
    # Get the tree
    tree = f_in.Get("outputTree")
    if not tree:
        print("ERROR: Cannot find outputTree in input file")
        f_in.Close()
        return False
    
    print(f"Tree has {tree.GetEntries()} entries")
    
    # Define binning for eta and pt
    # Recommended binning from CMS b-tagging POG
    pt_bins = [20, 30, 50, 70, 100, 140, 200, 300, 600, 1000]
    
    # Eta binning options (uncomment the one you want to use):
    # Option 1: Single bin (maximum statistics)
    # eta_bins = [-2.5, 2.5]
    
    # Option 2: Barrel and Endcap regions
    eta_bins = [-2.5, -1.5, 1.5, 2.5]
    
    # Option 3: More granular binning (if you have enough statistics)
    # eta_bins = [-2.5, -2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0, 2.5]
    
    n_eta_bins = len(eta_bins) - 1
    n_pt_bins = len(pt_bins) - 1
    
    # Convert to arrays that ROOT can understand
    eta_bins_array = array.array('d', eta_bins)
    pt_bins_array = array.array('d', pt_bins)
    
    # Create histograms for b/c-flavor jets
    hist_btagEff_bcflav = ROOT.TH2D(
        "hist_btagEff_bcflav",
        "B-tagging Efficiency (b/c-flavor);#eta;p_{T} [GeV]",
        n_eta_bins, eta_bins_array,
        n_pt_bins, pt_bins_array
    )
    
    hist_bcflav_pass = ROOT.TH2D(
        "hist_bcflav_pass",
        "B-tagged b/c-flavor jets;#eta;p_{T} [GeV]",
        n_eta_bins, eta_bins_array,
        n_pt_bins, pt_bins_array
    )
    
    hist_bcflav_all = ROOT.TH2D(
        "hist_bcflav_all",
        "All b/c-flavor jets;#eta;p_{T} [GeV]",
        n_eta_bins, eta_bins_array,
        n_pt_bins, pt_bins_array
    )
    
    # Create histograms for light-flavor jets
    hist_btagEff_lflav = ROOT.TH2D(
        "hist_btagEff_lflav",
        "B-tagging Efficiency (light-flavor);#eta;p_{T} [GeV]",
        n_eta_bins, eta_bins_array,
        n_pt_bins, pt_bins_array
    )
    
    hist_lflav_pass = ROOT.TH2D(
        "hist_lflav_pass",
        "B-tagged light-flavor jets;#eta;p_{T} [GeV]",
        n_eta_bins, eta_bins_array,
        n_pt_bins, pt_bins_array
    )
    
    hist_lflav_all = ROOT.TH2D(
        "hist_lflav_all",
        "All light-flavor jets;#eta;p_{T} [GeV]",
        n_eta_bins, eta_bins_array,
        n_pt_bins, pt_bins_array
    )
    
    # Set Sumw2 for proper error calculation
    hist_bcflav_pass.Sumw2()
    hist_bcflav_all.Sumw2()
    hist_lflav_pass.Sumw2()
    hist_lflav_all.Sumw2()
    
    print("Processing events...")
    
    # Loop over tree entries
    for i, event in enumerate(tree):
        if i % 10000 == 0:
            print(f"Processing event {i}/{tree.GetEntries()}")
        
        weight = event.evWeight
        
        # Process b/c-flavor jets that pass b-tagging
        for j in range(len(event.goodJets_btagpass_bcflav_eta)):
            eta = event.goodJets_btagpass_bcflav_eta[j]
            pt = event.goodJets_btagpass_bcflav_pt[j]
            hist_bcflav_pass.Fill(eta, pt, weight)
        
        # Process all b/c-flavor jets
        for j in range(len(event.goodJets_all_bcflav_eta)):
            eta = event.goodJets_all_bcflav_eta[j]
            pt = event.goodJets_all_bcflav_pt[j]
            hist_bcflav_all.Fill(eta, pt, weight)
        
        # Process light-flavor jets that pass b-tagging
        for j in range(len(event.goodJets_btagpass_lflav_eta)):
            eta = event.goodJets_btagpass_lflav_eta[j]
            pt = event.goodJets_btagpass_lflav_pt[j]
            hist_lflav_pass.Fill(eta, pt, weight)
        
        # Process all light-flavor jets
        for j in range(len(event.goodJets_all_lflav_eta)):
            eta = event.goodJets_all_lflav_eta[j]
            pt = event.goodJets_all_lflav_pt[j]
            hist_lflav_all.Fill(eta, pt, weight)
    
    print("Calculating efficiencies...")
    
    # Calculate efficiency = pass / all
    hist_btagEff_bcflav.Divide(hist_bcflav_pass, hist_bcflav_all, 1.0, 1.0, "B")
    hist_btagEff_lflav.Divide(hist_lflav_pass, hist_lflav_all, 1.0, 1.0, "B")
    
    # Create output file
    print(f"Writing output to: {output_file}")
    f_out = ROOT.TFile.Open(output_file, "RECREATE")
    
    # Write efficiency histograms (main histograms needed by getBTaggingEff)
    hist_btagEff_bcflav.Write()
    hist_btagEff_lflav.Write()
    
    # Also write the numerator and denominator for reference
    hist_bcflav_pass.Write()
    hist_bcflav_all.Write()
    hist_lflav_pass.Write()
    hist_lflav_all.Write()
    
    # Print summary statistics
    print("\n=== Summary ===")
    print(f"b/c-flavor jets:")
    print(f"  Total: {hist_bcflav_all.Integral():.2f}")
    print(f"  Passed: {hist_bcflav_pass.Integral():.2f}")
    if hist_bcflav_all.Integral() > 0:
        print(f"  Overall efficiency: {hist_bcflav_pass.Integral()/hist_bcflav_all.Integral()*100:.2f}%")
    
    print(f"\nLight-flavor jets:")
    print(f"  Total: {hist_lflav_all.Integral():.2f}")
    print(f"  Passed: {hist_lflav_pass.Integral():.2f}")
    if hist_lflav_all.Integral() > 0:
        print(f"  Overall efficiency: {hist_lflav_pass.Integral()/hist_lflav_all.Integral()*100:.2f}%")
    
    # Close files
    f_out.Close()
    f_in.Close()
    
    print(f"\nDone! Output written to {output_file}")
    return True


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python calculate_btag_efficiency.py <input_file.root> <output_file.root>")
        print("\nExample:")
        print("  python calculate_btag_efficiency.py input.root btagging_efficiency.root")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    success = calculate_btagging_efficiency(input_file, output_file)
    sys.exit(0 if success else 1)
