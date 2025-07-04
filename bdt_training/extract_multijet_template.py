import ROOT

def extract_multijet_template(input_file, output_file):
    # Open input file
    f_in = ROOT.TFile(input_file, "READ")
    
    # Get all histograms
    data = f_in.Get("data")
    signal_top = f_in.Get("signal_top")
    signal_antitop = f_in.Get("signal_antitop")
    ttbar = f_in.Get("ttbar")
    wjets = f_in.Get("wjets")
#    s_tw = f_in.Get("stw")
#    drell_yen = f_in.Get("dy")
    
    # Clone data histogram for multijet template
    multijet = data.Clone("multijet")
    multijet.SetTitle("Multijet background (Data - Signal - TTbar - Wjets)")
    
    # Subtract all contributions
    multijet.Add(signal_top, -1)
    multijet.Add(signal_antitop, -1)
    multijet.Add(ttbar, -1)
    multijet.Add(wjets, -1)
#    multijet.Add(s_tw, -1)
#    multijet.Add(drell_yen, -1)

    # Replace zero or negative bins with tiny positive number
    tiny_positive = 1e-10
    for i in range(1, multijet.GetNbinsX() + 1):
        if multijet.GetBinContent(i) <= 0:
            multijet.SetBinContent(i, tiny_positive)
    
    # Save to output file
    f_out = ROOT.TFile(output_file, "RECREATE")
    multijet.Write()
    f_out.Close()
    f_in.Close()
    
    print(f" Multijet template saved to {output_file}")

# Example usage
input_file = "eta_QCDele2j1b_histograms.root"
output_file = "multijet_template_ele2j1t_eta.root"
extract_multijet_template(input_file, output_file)
