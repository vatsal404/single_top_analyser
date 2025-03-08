import ROOT
import numpy as np
import array

def create_efficiency_plots():
    # Open the ROOT file
    file = ROOT.TFile("/eos/uscms/store/user/vsinha/results/signal.root", "READ")
    tree = file.Get("outputTree")
    
    # Define binning - convert numpy arrays to ROOT-compatible arrays
    pt_bins = array.array('d', [30, 50, 70, 100, 140, 200, 300, 600])
    eta_bins = array.array('d', [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0])
    
    # Create 2D histograms for b/c-flavor jets
    h_pass_bc = ROOT.TH2F("h_pass_bc", "B-tagging Efficiency (b/c-flavor);#eta;p_{T} [GeV]", 
                          len(eta_bins)-1, eta_bins,
                          len(pt_bins)-1, pt_bins)
    h_all_bc = ROOT.TH2F("h_all_bc", "B-tagging Efficiency (b/c-flavor);#eta;p_{T} [GeV]",
                         len(eta_bins)-1, eta_bins,
                         len(pt_bins)-1, pt_bins)
    
    # Create 2D histograms for light-flavor jets
    h_pass_light = ROOT.TH2F("h_pass_light", "B-tagging Efficiency (light-flavor);#eta;p_{T} [GeV]", 
                            len(eta_bins)-1, eta_bins,
                            len(pt_bins)-1, pt_bins)
    h_all_light = ROOT.TH2F("h_all_light", "B-tagging Efficiency (light-flavor);#eta;p_{T} [GeV]",
                           len(eta_bins)-1, eta_bins,
                           len(pt_bins)-1, pt_bins)
    
    # Fill histograms
    for event in tree:
        # b/c-flavored jets
        for pt, eta in zip(event.goodJets_btagpass_bcflav_pt, 
                          event.goodJets_btagpass_bcflav_eta):
            if eta < 0:
                eta = -eta
            if eta <= 3.0:
                h_pass_bc.Fill(eta, pt)
        
        for pt, eta in zip(event.goodJets_all_bcflav_pt,
                          event.goodJets_all_bcflav_eta):
            if eta < 0:
                eta = -eta
            if eta <= 3.0:
                h_all_bc.Fill(eta, pt)
            
        # light-flavored jets
        for pt, eta in zip(event.goodJets_btagpass_lflav_pt, 
                          event.goodJets_btagpass_lflav_eta):
            if eta < 0:
                eta = -eta
            if eta <= 3.0:
                h_pass_light.Fill(eta, pt)
                
        for pt, eta in zip(event.goodJets_all_lflav_pt,
                          event.goodJets_all_lflav_eta):
            if eta < 0:
                eta = -eta
            if eta <= 3.0:
                h_all_light.Fill(eta, pt)
    ROOT.gStyle.SetOptStat(0)    
    # Create efficiency plots
    h_eff_bc = h_pass_bc.Clone("h_eff_bc")
    h_eff_bc.Divide(h_all_bc)
    h_eff_bc.Scale(100)  # Convert to percentage
    
    h_eff_light = h_pass_light.Clone("h_eff_light")
    h_eff_light.Divide(h_all_light)
    h_eff_light.Scale(100)  # Convert to percentage
    
    # Set style options
#    ROOT.gStyle.SetPaintTextFormat("%.1f%%")  # Show percentage with 1 decimal place
#    ROOT.gStyle.SetOptStat(0)
#    ROOT.gStyle.SetPalette(1)
    
    # Create and save b/c-flavor efficiency plot
    c1 = ROOT.TCanvas("c1", "c1", 800, 600)
    ROOT.gPad.SetRightMargin(0.15)
    ROOT.gPad.SetLeftMargin(0.15)
    h_eff_bc.SetMarkerSize(1.5)  # Increase text size
    h_eff_bc.Draw("colz text")  # Draw with both color and text
    
    c1.SaveAs("btag_efficiency_bc_2D.pdf")
    c1.SaveAs("btag_efficiency_bc_2D.png")
    
    # Create and save light-flavor efficiency plot
    c2 = ROOT.TCanvas("c2", "c2", 800, 600)
    ROOT.gPad.SetRightMargin(0.15)
    ROOT.gPad.SetLeftMargin(0.15)
    h_eff_light.SetMarkerSize(1.5)  # Increase text size
    h_eff_light.Draw("colz text")  # Draw with both color and text
    
    c2.SaveAs("btag_efficiency_light_2D.pdf")
    c2.SaveAs("btag_efficiency_light_2D.png")
    
    file.Close()

if __name__ == "__main__":
    create_efficiency_plots()
