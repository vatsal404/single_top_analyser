import os
import ROOT
from array import array
from btagging_efficiency_binning import btageff_dataset_dict

# Define the base directory where the dataset folders are located
base_directory = ''

# Define the output directory where the new root files will be stored
output_directory = 'data/BTV/2017_UL'

# Loop through the dataset dictionary
for dataset, dataset_info in btageff_dataset_dict.items():
    # Create the input file path
    input_file_path = os.path.join(base_directory, f'{dataset_info["extension"]}.root')

    # Load the input root file
    input_file = ROOT.TFile.Open(input_file_path, 'READ')

    # Check if the file was opened successfully
    if not input_file or input_file.IsZombie():
        print(f'Error opening file: {input_file_path}')
        continue

    # Print the structure of the file
    input_file.ls()

    # Create TH2D histograms for bcflav and lflav efficiencies
    h_btagEff_bcflav = ROOT.TH2D('h_btagEff_bcflav', 'BTag Efficiency BCFlav',
                                  len(dataset_info['pt_bins']) - 1, array('d', dataset_info['pt_bins']),
                                  len(dataset_info['eta_bins']) - 1, array('d', dataset_info['eta_bins']))

    h_btagEff_lflav = ROOT.TH2D('h_btagEff_lflav', 'BTag Efficiency LFlav',
                                  len(dataset_info['pt_bins']) - 1, array('d', dataset_info['pt_bins']),
                                  len(dataset_info['eta_bins']) - 1, array('d', dataset_info['eta_bins']))

    # Create histograms for branches
    btagpass_bcflav_pt_hist = ROOT.TH2D('btagpass_bcflav_pt', 'BTagPass BCFlav Pt',
                                         len(dataset_info['pt_bins']) - 1, array('d', dataset_info['pt_bins']),
                                         len(dataset_info['eta_bins']) - 1, array('d', dataset_info['eta_bins']))

    all_bcflav_pt_hist = ROOT.TH2D('all_bcflav_pt', 'All BCFlav Pt',
                                    len(dataset_info['pt_bins']) - 1, array('d', dataset_info['pt_bins']),
                                    len(dataset_info['eta_bins']) - 1, array('d', dataset_info['eta_bins']))

    btagpass_lflav_pt_hist = ROOT.TH2D('btagpass_lflav_pt', 'BTagPass LFlav Pt',
                                         len(dataset_info['pt_bins']) - 1, array('d', dataset_info['pt_bins']),
                                         len(dataset_info['eta_bins']) - 1, array('d', dataset_info['eta_bins']))

    all_lflav_pt_hist = ROOT.TH2D('all_lflav_pt', 'All LFlav Pt',
                                   len(dataset_info['pt_bins']) - 1, array('d', dataset_info['pt_bins']),
                                   len(dataset_info['eta_bins']) - 1, array('d', dataset_info['eta_bins']))

    # Get the TTree from the input file
    tree = input_file.Get('Events')

    # Print total number of events
    print(f'Processing dataset: {dataset}')
    print(f'Total events in tree: {tree.GetEntries()}')

    # Loop over events and fill histograms
    for event in tree:
        for k in range(len(event.Jet_btagDeepB)):
            # Check if the jet is a b-flavor jet
            if event.Jet_hadronFlavour[k] == 5 or event.Jet_hadronFlavour[k] == 4:  # 5 for b-jets
                all_bcflav_pt_hist.Fill(event.Jet_pt[k], event.Jet_eta[k])
                #if event.Jet_btagDeepB[k] > 0.4506:  # B-tagging threshold
                if event.Jet_btagDeepFlavB[k] > 0.3040:
                    btagpass_bcflav_pt_hist.Fill(event.Jet_pt[k], event.Jet_eta[k])

            # Check if the jet is a light-flavor jet
            elif event.Jet_hadronFlavour[k] == 0:  # 0 for light quarks
                all_lflav_pt_hist.Fill(event.Jet_pt[k], event.Jet_eta[k])
                if event.Jet_btagDeepFlavB[k] > 0.3040:  # B-tagging threshold
                    btagpass_lflav_pt_hist.Fill(event.Jet_pt[k], event.Jet_eta[k])

    # Print number of entries filled in histograms
    print(f'BTagPass BCFlav Pt Histogram Entries: {btagpass_bcflav_pt_hist.GetEntries()}')
    print(f'All BCFlav Pt Histogram Entries: {all_bcflav_pt_hist.GetEntries()}')
    print(f'BTagPass LFlav Pt Histogram Entries: {btagpass_lflav_pt_hist.GetEntries()}')
    print(f'All LFlav Pt Histogram Entries: {all_lflav_pt_hist.GetEntries()}')

    # Calculate efficiencies for bcflav
    for i in range(len(dataset_info['pt_bins']) - 1):
        for j in range(len(dataset_info['eta_bins']) - 1):
            numerator_bcflav = btagpass_bcflav_pt_hist.GetBinContent(i + 1, j + 1)
            denominator_bcflav = all_bcflav_pt_hist.GetBinContent(i + 1, j + 1)
            bcflav_efficiency = numerator_bcflav / denominator_bcflav if denominator_bcflav != 0 else 0
            h_btagEff_bcflav.SetBinContent(i + 1, j + 1, bcflav_efficiency)

    # Calculate efficiencies for lflav
    for i in range(len(dataset_info['pt_bins']) - 1):
        for j in range(len(dataset_info['eta_bins']) - 1):
            numerator_lflav = btagpass_lflav_pt_hist.GetBinContent(i + 1, j + 1)
            denominator_lflav = all_lflav_pt_hist.GetBinContent(i + 1, j + 1)
            lflav_efficiency = numerator_lflav / denominator_lflav if denominator_lflav != 0 else 0
            h_btagEff_lflav.SetBinContent(i + 1, j + 1, lflav_efficiency)

    # Create the output file path
    output_file_path = os.path.join(output_directory, f'BtaggingEfficiency.root')

    # Save all histograms to the output file
    output_file = ROOT.TFile.Open(output_file_path, 'RECREATE')
    all_bcflav_pt_hist.Write()
    btagpass_bcflav_pt_hist.Write()
    all_lflav_pt_hist.Write()
    btagpass_lflav_pt_hist.Write()
    h_btagEff_bcflav.Write()
    h_btagEff_lflav.Write()
    output_file.Close()

    # Close the input file
    input_file.Close()

print('Process completed.')

