#include <ROOT/RDataFrame.hxx>
#include <TFile.h>
#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TKey.h>
#include <iostream>

int Plots() {
    std::string fileName = "Final_histograms.root";
    TFile *inputFile = TFile::Open(fileName.c_str());
    if (!inputFile || inputFile->IsZombie()) {
        std::cerr << "Error: Cannot open file " << fileName << std::endl;
        return 1;
    }

    // Debug: Print all keys in the file
    std::cout << "Available histograms in the file:" << std::endl;
    TIter next(inputFile->GetListOfKeys());
    TKey *key;
    while ((key = (TKey*)next())) {
        std::cout << "- \"" << key->GetName() << "\" (type: " << key->GetClassName() << ")" << std::endl;
    }

    std::vector<std::string> samples = {
        "SingleTop",
        "TTbar",  // Removed leading space
        "WJets",
        "DY",
        "ST_tW",
     //
    };

    std::vector<TH1*> histograms;
    for (const auto& sample : samples) {
        TH1 *hist = dynamic_cast<TH1*>(inputFile->Get(sample.c_str()));
        if (hist) {
            std::cout << "Successfully loaded histogram: " << sample << std::endl;
            hist->SetFillColorAlpha(histograms.size() + 2, 0.6);
            hist->SetLineColor(histograms.size() + 2);
            histograms.push_back(hist);
        } else {
            std::cerr << "Error: Histogram \"" << sample << "\" not found in the file!" << std::endl;
        }
    }

    if (histograms.empty()) {
        std::cerr << "No histograms were loaded successfully!" << std::endl;
        inputFile->Close();
        delete inputFile;
        return 1;
    }

    TCanvas *canvas = new TCanvas("canvas", "Top mass Stacked Plot", 800, 600);
    THStack *stack = new THStack("stack", "Stacked Histogram for Top Pt; Top Pt [GeV]; Events");
    
    for (auto hist : histograms) {
        stack->Add(hist);
    }
    
    stack->Draw("HIST");
    
    TLegend *legend = new TLegend(0.7, 0.7, 0.9, 0.9);
    for (size_t i = 0; i < histograms.size(); ++i) {
        legend->AddEntry(histograms[i], samples[i].c_str(), "f");
    }
    legend->Draw();
    
    canvas->SaveAs("good_bjet_leading_eta_stack_plot.png");
    
    inputFile->Close();
    delete inputFile;
    delete canvas;
    return 0;
}
