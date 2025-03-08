#include <TFile.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <vector>
#include <iostream>

void extractMultijetTemplate() {
    std::vector<TString> dataFiles = {"hist_files/data_hist.root"};
    std::vector<TString> bkgFiles = {
    "bdt_hists/PROCESSED_DYJetsToLL_M-10to50_UL17_hist.root",
    "bdt_hists/PROCESSED_DYJetsToLL_M-50_UL17_hist.root",
    "bdt_hists/PROCESSED_ST_tW_antitop_5f_hist.root",
    "bdt_hists/PROCESSED_ST_tW_top_5f_hist.root",
    "bdt_hists/PROCESSED_ST_tchannel_antitop_5f_hist.root",
    "bdt_hists/PROCESSED_ST_tchannel_top_5f_hist.root",
    "bdt_hists/PROCESSED_TTbar-channel_top_UL17_hist.root",
    "bdt_hists/PROCESSED_WJetsToLNu_0J_UL17_hist.root",
    "bdt_hists/PROCESSED_WJetsToLNu_1J_UL17_hist.root",
    "bdt_hists/PROCESSED_WJetsToLNu_2J_UL17_hist.root"
    };
    TString dataHistName = "Wboson_transversemass_QCDmuon";//data for determining multijet template
    TString dataNormalHistName = "Wboson_transversemass_muon"; // data to fit to multijet template 
    TString bkgHistName = "Wboson_transversemass_QCDmuon"; //background to be subtracted from data to get multijet template  
    TString controlBkgHistName = "Wboson_transversemass_muon";// background which will be fitted along with multijet template

    TFile *dataFile = TFile::Open(dataFiles[0]);
    if (!dataFile || dataFile->IsZombie()) {
        std::cerr << "Error opening data file: " << dataFiles[0] << std::endl;
        return;
    }

    TH1D* h_data = (TH1D*)dataFile->Get(dataHistName);
    TH1D* h_data_normal = (TH1D*)dataFile->Get(dataNormalHistName); // Get the additional histogram

    if (!h_data || !h_data_normal) {
        std::cerr << "Error: One or more data histograms not found." << std::endl;
        dataFile->Close();
        return;
    }

    h_data_normal->SetDirectory(0); // Ensure histogram persists when file is closed
    TH1D* h_multijet = (TH1D*)h_data->Clone("h_multijet");
    h_multijet->Reset();

    TH1D* h_background = nullptr;
    TH1D* h_background_control = nullptr;

    for (const auto& file : bkgFiles) {
        TFile *f = TFile::Open(file);
        if (!f || f->IsZombie()) continue;

        TH1D* h_bg = (TH1D*)f->Get(bkgHistName);
        if (h_bg) {
            if (!h_background) {
                h_background = (TH1D*)h_bg->Clone("h_background");
                h_background->SetDirectory(0);
            } else {
                h_background->Add(h_bg);
            }
        }

        TH1D* h_bg_control = (TH1D*)f->Get(controlBkgHistName);
        if (h_bg_control) {
            if (!h_background_control) {
                h_background_control = (TH1D*)h_bg_control->Clone("h_background_control");
                h_background_control->SetDirectory(0);
            } else {
                h_background_control->Add(h_bg_control);
            }
        }
        f->Close();
    }

    if (!h_background) {
        std::cerr << "No valid background histograms found." << std::endl;
        return;
    }

for (int i = 1; i <= h_data->GetNbinsX(); i++) {
    double dataVal = h_data->GetBinContent(i);
    double bgVal = h_background->GetBinContent(i);
    double multijetVal = dataVal - bgVal;

    if (multijetVal < 0) multijetVal = 0;  // Ensure non-negative values

    h_multijet->SetBinContent(i, multijetVal);
}
    for (int i = 1; i <= h_multijet->GetNbinsX(); i++) {
        std::cout << "Bin " << i << " -> Multijet: " << h_multijet->GetBinContent(i) << std::endl;
    }

    TFile *outFile = new TFile("multijet_template.root", "RECREATE");
    h_multijet->Write("h_multijet");
    h_background->Write("h_background");
    h_background_control->Write("h_background_control");
    h_data->Write("h_data");
    h_data_normal->Write("h_data_control"); // Write the additional histogram

    outFile->Close();

    TCanvas *c1 = new TCanvas("c1", "Multijet Extraction", 800, 600);
    h_data->SetLineColor(kBlack);
    h_data->SetLineWidth(2);
    h_data->Draw("HIST");
    h_background->SetLineColor(kRed);
    h_background->SetLineWidth(2);
    h_background->Draw("HIST SAME");
    h_multijet->SetLineColor(kBlue);
    h_multijet->SetLineWidth(2);
    h_multijet->Draw("HIST SAME");
    TLegend *legend = new TLegend(0.7, 0.7, 0.9, 0.9);  // Adjust position (x1, y1, x2, y2)
    legend->SetBorderSize(1);  // Remove legend border (use 0 to remove it completely)
    legend->SetFillColor(0);   // Transparent background
    legend->SetTextSize(0.03); // Adjust text size

// Add entries to the legend
    legend->AddEntry(h_data, "Data", "l");        // "l" means line
    legend->AddEntry(h_background, "Background", "l");
    legend->AddEntry(h_multijet, "Multijet", "l");

// Draw the legend
    legend->Draw();
    c1->SaveAs("multijet_template.png");

    delete c1;
    dataFile->Close();
}
