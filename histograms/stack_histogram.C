#include <TFile.h>
#include <TH1F.h>
#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TRatioPlot.h>
#include <TStyle.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>

// Define histogram name to plot (can be changed to any histogram in your files)
std::string histogram_name =  "bdt_delR_muon_2j1t";// Change this variable to plot different histograms

// Define process groups with colors
struct ProcessGroup {
    std::string name;
    int color;
    std::vector<std::string> files;
};

// Define all process groups with their colors and files
std::vector<ProcessGroup> process_groups = {
    {
        "QCD (MuEnriched)", 
        kBlue, 
        {
            "PROCESSED_QCD_Pt-15To20_MuEnrichedPt5_hist.root",
            "PROCESSED_QCD_Pt-20To30_MuEnrichedPt5_hist.root",
            "PROCESSED_QCD_Pt-30To50_MuEnrichedPt5_hist.root",
            "PROCESSED_QCD_Pt-50To80_MuEnrichedPt5_hist.root",
            "PROCESSED_QCD_Pt-80To120_MuEnrichedPt5_hist.root",
            "PROCESSED_QCD_Pt-120To170_MuEnrichedPt5_hist.root",
            "PROCESSED_QCD_Pt-170To300_MuEnrichedPt5_hist.root",
            "PROCESSED_QCD_Pt-300To470_MuEnrichedPt5_hist.root",
            "PROCESSED_QCD_Pt-470To600_MuEnrichedPt5_hist.root",
            "PROCESSED_QCD_Pt-600To800_MuEnrichedPt5_hist.root",
            "PROCESSED_QCD_Pt-800To1000_MuEnrichedPt5_hist.root",
            "PROCESSED_QCD_Pt-1000_MuEnrichedPt5_hist.root"
        }
    },
    {
        "QCD (EMEnriched)", 
        kGreen, 
        {
            "PROCESSED_QCD_Pt-15to20_EMEnriched_hist.root",
            "PROCESSED_QCD_Pt-20to30_EMEnriched_hist.root",
            "PROCESSED_QCD_Pt-30to50_EMEnriched_hist.root",
            "PROCESSED_QCD_Pt-50to80_EMEnriched_hist.root",
            "PROCESSED_QCD_Pt-80to120_EMEnriched_hist.root",
            "PROCESSED_QCD_Pt-120to170_EMEnriched_hist.root",
            "PROCESSED_QCD_Pt-70to300_EMEnriched_hist.root"
        }
    },
    {
        "W+Jets", 
        kOrange+1, 
        {
            "PROCESSED_WJetsToLNu_0J_UL17_hist.root",
            "PROCESSED_WJetsToLNu_1J_UL17_hist.root",
            "PROCESSED_WJetsToLNu_2J_UL17_hist.root"
        }
    },
    {
        "DY", 
        kYellow, 
        {
            "PROCESSED_DYJetsToLL_M-10to50_UL17_hist.root",
            "PROCESSED_DYJetsToLL_M-50_UL17_hist.root"
        }
    },
    {
        "Single Top (t-channel)", 
        kRed, 
        {
            "PROCESSED_ST_tchannel_antitop_5f_hist.root",
            "PROCESSED_ST_tchannel_top_5f_hist.root"
        }
    },
    {
        "Single Top (tW)", 
        kMagenta, 
        {
            "PROCESSED_ST_tW_antitop_5f_hist.root",
            "PROCESSED_ST_tW_top_5f_hist.root"
        }
    },
    {
        "TTbar", 
        kCyan, 
        {
            "PROCESSED_TTbar-channel_top_UL17_hist.root"
        }
    }
};

// Path to your data file
std::string data_file_path = "data_hist.root";

// Function to extract process name from filename
std::string extractProcessName(const std::string& filename) {
    // Extract just the file name without path
    size_t lastSlash = filename.find_last_of("/\\");
    std::string file = filename.substr(lastSlash + 1);
    
    // Extract the part after PROCESSED_ and before _hist.root
    size_t startPos = file.find("PROCESSED_") + 10;
    size_t endPos = file.find("_hist.root");
    
    return file.substr(startPos, endPos - startPos);
}

void stack_histogram() {
    gStyle->SetOptStat(0);
    
    // Create stack and legend
    THStack* stack = new THStack("stack", (histogram_name + " Distribution").c_str());
    TLegend* legend = new TLegend(0.6, 0.6, 0.9, 0.9);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    
    // Map to store the grouped histograms
    std::map<std::string, TH1F*> group_histograms;
    
    // Process each group
    for (const auto& group : process_groups) {
        TH1F* group_hist = nullptr;
        bool added_files = false;
        
        // Process each file in the group
        for (const auto& file_path : group.files) {
            TFile* file = TFile::Open(file_path.c_str(), "READ");
            if (!file || file->IsZombie()) {
                std::cerr << "Error: Failed to open " << file_path << std::endl;
                continue;
            }
            
            TH1F* hist = (TH1F*)file->Get(histogram_name.c_str());
            if (!hist) {
                std::cerr << "Error: Histogram '" << histogram_name << "' not found in " << file_path << std::endl;
                file->Close();
                continue;
            }
            
            // Clone the histogram to keep it after file close
            TH1F* hist_clone = (TH1F*)hist->Clone(extractProcessName(file_path).c_str());
            hist_clone->SetDirectory(0);
            
            // Add to group histogram or create it
            if (group_hist == nullptr) {
                group_hist = (TH1F*)hist_clone->Clone((group.name + "_hist").c_str());
                group_hist->SetDirectory(0);
                added_files = true;
            } else {
                group_hist->Add(hist_clone);
                added_files = true;
            }
            
            delete hist_clone;
            file->Close();
        }
        
        // Add the group histogram to the stack only if we actually added files
        if (group_hist && added_files) {
            group_hist->SetFillColor(group.color);
            group_hist->SetLineColor(group.color);
            stack->Add(group_hist);
            legend->AddEntry(group_hist, group.name.c_str(), "f");
            
            // Store in map to prevent garbage collection
            group_histograms[group.name] = group_hist;
        }
    }
    
    // Load data histogram
    TFile* data_file = TFile::Open(data_file_path.c_str(), "READ");
    if (!data_file || data_file->IsZombie()) {
        std::cerr << "Error: Failed to open data file!" << std::endl;
        return;
    }
    
    TH1F* data_hist = (TH1F*)data_file->Get(histogram_name.c_str());
    if (!data_hist) {
        std::cerr << "Error: Data histogram '" << histogram_name << "' not found!" << std::endl;
        data_file->Close();
        return;
    }
    
    // Clone the data histogram to keep it after file close
    TH1F* data_hist_clone = (TH1F*)data_hist->Clone("data_hist");
    data_hist_clone->SetDirectory(0);
    data_hist_clone->SetMarkerStyle(20);
    data_hist_clone->SetMarkerColor(kBlack);
    data_hist_clone->SetLineColor(kBlack);
    
    data_file->Close();
    
    // Create canvas with proper divisions for ratio plot
    TCanvas* canvas = new TCanvas("canvas", "Stacked Histogram with Ratio Plot", 800, 800);
    
    // Get total MC histogram (stack sum)
    stack->Draw("HIST");
    TH1F* mc_total = (TH1F*)stack->GetStack()->Last()->Clone("mc_total");
    mc_total->SetDirectory(0);
    
    // Create and draw ratio plot
    TRatioPlot* ratio_plot = new TRatioPlot(data_hist_clone, mc_total, "pois");
    ratio_plot->Draw();
    
    // Configure the upper pad (main plot)
    ratio_plot->GetUpperPad()->cd();
    stack->Draw("HIST");
    data_hist_clone->Draw("SAME E1");
    legend->AddEntry(data_hist_clone, "Data", "lep");
    legend->Draw();
    
    // Set axis titles for the main plot
    ratio_plot->GetUpperRefYaxis()->SetTitle("Events");
    ratio_plot->GetUpperRefXaxis()->SetTitle(histogram_name.c_str());
    
    // Configure the lower pad (ratio plot)
    ratio_plot->GetLowerPad()->cd();
    ratio_plot->GetLowerRefGraph()->GetYaxis()->SetTitle("Data/MC");
    ratio_plot->GetLowerRefGraph()->SetMinimum(0.5);
    ratio_plot->GetLowerRefGraph()->SetMaximum(1.5);
    
    // Set overall plot title
    canvas->cd();
    TPaveText* title = new TPaveText(0.1, 0.95, 0.9, 0.99, "brNDC");
    title->SetFillColor(0);
    title->SetBorderSize(0);
    title->SetTextAlign(21);
    title->AddText((histogram_name + " Distribution").c_str());
    title->Draw();
    
    // Save plot
    canvas->SaveAs(("stacked_" + histogram_name + "_with_ratio.png").c_str());
    
    std::cout << "Generated plot for histogram: " << histogram_name << std::endl;
    std::cout << "To plot a different histogram, change the 'histogram_name' variable." << std::endl;
}
