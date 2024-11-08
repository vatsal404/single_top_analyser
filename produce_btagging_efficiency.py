import os
import ROOT
from array import array
from btagging_efficiency_binning import btageff_dataset_dict

# Define the base directory where the dataset folders are located
base_directory = './PROC_ST_t-channel_top_UL17.root'

# Define the output directory where the new root files will be stored
output_directory = 'data/BTV/2017_UL'

# Loop through the dataset dictionary
for dataset, dataset_info in btageff_dataset_dict.items():
    # Create the input file path
    input_file_path = os.path.join(base_directory, f'PROC{dataset_info["extension"]}.root')

    # Load the input root file
    input_file = ROOT.TFile.Open(input_file_path, 'READ')

    # Create TH2D histograms for bcflav and lflav
    h_btagEff_bcflav = ROOT.TH2D('h_btagEff_bcflav', 'BTag Efficiency BCFlav', len(dataset_info['pt_bins']) - 1, array('d', dataset_info['pt_bins']), len(dataset_info['eta_bins']) - 1, array('d', dataset_info['eta_bins']))
    h_btagEff_lflav = ROOT.TH2D('h_btagEff_lflav', 'BTag Efficiency LFlav', len(dataset_info['pt_bins']) - 1, array('d', dataset_info['pt_bins']), len(dataset_info['eta_bins']) - 1, array('d', dataset_info['eta_bins']))

    # Get the TTree from the input file
    tree = input_file.Get('outputTree')

    # Create histograms for branches
    btagpass_bcflav_pt_hist = ROOT.TH2D('btagpass_bcflav_pt', 'BTagPass BCFlav Pt', len(dataset_info['pt_bins']) - 1, array('d', dataset_info['pt_bins']), len(dataset_info['eta_bins']) - 1, array('d', dataset_info['eta_bins']))
    all_bcflav_pt_hist = ROOT.TH2D('all_bcflav_pt', 'All BCFlav Pt', len(dataset_info['pt_bins']) - 1, array('d', dataset_info['pt_bins']), len(dataset_info['eta_bins']) - 1, array('d', dataset_info['eta_bins']))
    btagpass_lflav_pt_hist = ROOT.TH2D('btagpass_lflav_pt', 'BTagPass LFlav Pt', len(dataset_info['pt_bins']) - 1, array('d', dataset_info['pt_bins']), len(dataset_info['eta_bins']) - 1, array('d', dataset_info['eta_bins']))
    all_lflav_pt_hist = ROOT.TH2D('all_lflav_pt', 'All LFlav Pt', len(dataset_info['pt_bins']) - 1, array('d', dataset_info['pt_bins']), len(dataset_info['eta_bins']) - 1, array('d', dataset_info['eta_bins']))


    # Loop over events and fill histograms
    for i in range(tree.GetEntries()):
        tree.GetEntry(i)
        """
        # Print values of variables for debugging
        print("Event", i)
        print("goodJets_btagpass_bcflav_pt:", tree.goodJets_btagpass_bcflav_pt)
        print("goodJets_btagpass_bcflav_eta:", tree.goodJets_btagpass_bcflav_eta)
        print("goodJets_all_bcflav_pt:", tree.goodJets_all_bcflav_pt)
        print("goodJets_all_bcflav_eta:", tree.goodJets_all_bcflav_eta)
        print("goodJets_btagpass_lflav_pt:", tree.goodJets_btagpass_lflav_pt)
        print("goodJets_btagpass_lflav_eta:", tree.goodJets_btagpass_lflav_eta)
        print("goodJets_all_lflav_pt:", tree.goodJets_all_lflav_pt)
        print("goodJets_all_lflav_eta:", tree.goodJets_all_lflav_eta)
        """
        
        # Fill histograms for all elements in the vectors
        for j in range(len(tree.goodJets_btagpass_bcflav_pt)):
            btagpass_bcflav_pt_hist.Fill(tree.goodJets_btagpass_bcflav_pt[j], tree.goodJets_btagpass_bcflav_eta[j])
        for j in range(len(tree.goodJets_all_bcflav_pt)):
            all_bcflav_pt_hist.Fill(tree.goodJets_all_bcflav_pt[j], tree.goodJets_all_bcflav_eta[j])
        for j in range(len(tree.goodJets_btagpass_lflav_pt)):
            btagpass_lflav_pt_hist.Fill(tree.goodJets_btagpass_lflav_pt[j], tree.goodJets_btagpass_lflav_eta[j])
        for j in range(len(tree.goodJets_all_lflav_pt)):
            all_lflav_pt_hist.Fill(tree.goodJets_all_lflav_pt[j], tree.goodJets_all_lflav_eta[j])
        

    for i in range(len(dataset_info['pt_bins']) - 1):
        for j in range(len(dataset_info['eta_bins']) - 1):
            numerator_bcflav = btagpass_bcflav_pt_hist.GetBinContent(i + 1, j + 1)
            denominator_bcflav = all_bcflav_pt_hist.GetBinContent(i + 1, j + 1)
            bcflav_efficiency = numerator_bcflav / denominator_bcflav if denominator_bcflav != 0 else 0
            
            numerator_lflav = btagpass_lflav_pt_hist.GetBinContent(i + 1, j + 1)
            denominator_lflav = all_lflav_pt_hist.GetBinContent(i + 1, j + 1)
            lflav_efficiency = numerator_lflav / denominator_lflav if denominator_lflav != 0 else 0

            h_btagEff_bcflav.SetBinContent(i + 1, j + 1, bcflav_efficiency)
            h_btagEff_lflav.SetBinContent(i + 1, j + 1, lflav_efficiency)
            
    # Create the output file path
    output_file_path = os.path.join(output_directory, f'BtaggingEfficiency{dataset_info["extension"]}_2Nov.root')

    # Save the histograms to the output file
    output_file = ROOT.TFile.Open(output_file_path, 'RECREATE')
    h_btagEff_bcflav.Write()
    h_btagEff_lflav.Write()
    output_file.Close()

    # Close the input file
    input_file.Close()

print('Process completed.')
