#include <ROOT/RDataFrame.hxx>
#include <TH1F.h>
#include <TFile.h>
#include <iostream>
#include <map>
#include <vector>

void analyze_variables() {
    // Define variable binning and selection criteria
    TString variable_name = "Wboson_transversMass";  // Variable name
    int bins = 32;
    double x_min = 0;
    double x_max = 200;

    // Map to store histograms for different processes
    std::map<TString, TH1F*> histograms;

    // List of input files
    std::vector<TString> INPUT_FILES = {
        "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tchannel_antitop_5f.root",
        "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tchannel_top_5f.root",
        "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_TTbar-channel_top_UL17.root",
        "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_0J_UL17.root",
        "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_1J_UL17.root",
        "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_WJetsToLNu_2J_UL17.root",
        "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tW_top_5f.root",
        "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_ST_tW_antitop_5f.root",
        "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_DYJetsToLL_M-50_UL17.root",
        "/eos/uscms/store/user/vsinha/bdt_variables/PROCESSED_DYJetsToLL_M-10to50_UL17.root",
        "data.root"
    };

    // Loop over input files
    for (const auto& file : INPUT_FILES) {
        // Create a histogram for this file/process
        TString fname = file;
        fname.ReplaceAll(".root", "");
        fname = fname(fName.Last('/') + 1, fname.Length() - fname.Last('/') - 1);  // Extract file name

        // Define histogram for variable of interest
        histograms[fname] = new TH1F(variable_name, variable_name, bins, x_min, x_max);

        // Create RDataFrame from the ROOT file
        ROOT::RDataFrame df("outputTree", file);

        // Apply filter and weight
        auto df_filtered = df.Filter("electronChannel && region_2j1t")
                             .Define("weight", "genWeight * evWeight");  // Define weight

        // Fill histogram with filtered data
        df_filtered.HistoFill(variable_name, histograms[fname], "weight");
    }

    // After processing all files, we can stack and display the histograms

    // Create a canvas to draw histograms
    TCanvas* canvas = new TCanvas("canvas", "Canvas", 800, 600);
    histograms["PROCESSED_ST_tchannel_top_5f"]->SetFillColor(kRed - 7);
    histograms["PROCESSED_ST_tchannel_top_5f"]->SetLineColor(kRed);
    histograms["PROCESSED_ST_tchannel_top_5f"]->SetLineWidth(2);
    
    histograms["PROCESSED_ST_tchannel_antitop_5f"]->SetFillColor(kBlue - 7);
    histograms["PROCESSED_ST_tchannel_antitop_5f"]->SetLineColor(kBlue);
    histograms["PROCESSED_ST_tchannel_antitop_5f"]->SetLineWidth(2);
    
    histograms["PROCESSED_TTbar-channel_top_UL17"]->SetFillColor(kGreen - 7);
    histograms["PROCESSED_TTbar-channel_top_UL17"]->SetLineColor(kGreen + 2);
    histograms["PROCESSED_TTbar-channel_top_UL17"]->SetLineWidth(1);

    histograms["PROCESSED_WJetsToLNu_0J_UL17"]->SetFillColor(kAzure - 9);
    histograms["PROCESSED_WJetsToLNu_0J_UL17"]->SetLineColor(kBlue);
    histograms["PROCESSED_WJetsToLNu_0J_UL17"]->SetLineWidth(1);

    histograms["PROCESSED_DYJetsToLL_M-50_UL17"]->SetFillColor(kOrange - 3);
    
    // Stack histograms
    THStack* hs = new THStack("hs", "Stacked histograms");
    for (const auto& hist : histograms) {
        hs->Add(hist.second);
    }
    hs->Draw("HIST");

    // Create legend
    TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);
    legend->AddEntry(histograms["PROCESSED_ST_tchannel_top_5f"], "Single Top (t-channel)", "f");
    legend->AddEntry(histograms["PROCESSED_ST_tchannel_antitop_5f"], "Single Antitop", "f");
    legend->AddEntry(histograms["PROCESSED_TTbar-channel_top_UL17"], "TTbar", "f");
    legend->AddEntry(histograms["PROCESSED_WJetsToLNu_0J_UL17"], "WJets", "f");
    legend->AddEntry(histograms["PROCESSED_DYJetsToLL_M-50_UL17"], "DYJets", "f");
    legend->Draw();

    // Save the canvas as an image
    canvas->SaveAs("output_histograms.png");
}

