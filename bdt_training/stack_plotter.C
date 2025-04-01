#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TPad.h>
#include <TLine.h>
#include <TGraphAsymmErrors.h>

void stack_plotter() {
    // Configuration - modify these as needed
    const int data_tree_number = 11;  // Change this to indicate which tree contains your data
    const int nbins = 50;
    const double xmin = -1.0;
    const double xmax = 1.0;
    
    // Open input file
    TFile *input_file = TFile::Open("bdt_comparison.root");
    if (!input_file) {
        std::cerr << "Error opening input file!" << std::endl;
        return;
    }
    
    // Colors for different MC samples
    int colors[] = {kRed, kBlue, kGreen, kMagenta, kCyan, kYellow, kOrange, 
                   kSpring, kTeal, kViolet, kPink};
    
    // Create stack and legend
    THStack *mc_stack = new THStack("mc_stack", "BDT Score;BDT Score;Events");
    TLegend *legend = new TLegend(0.7, 0.5, 0.9, 0.9);
    
    // Vector to hold all MC histograms
    std::vector<TH1D*> mc_histograms;
    TH1D *h_data = nullptr;
    
    // Process each tree
    for (int i = 1; i <= 11; i++) {
        TString tree_name = Form("tree%d", i);
        TTree *tree = (TTree*)input_file->Get(tree_name);
        
        if (!tree) {
            std::cerr << "Tree " << tree_name << " not found!" << std::endl;
            continue;
        }
        
        if (i == data_tree_number) {
            // This is the data tree
            h_data = new TH1D("h_data", "Data", nbins, xmin, xmax);
            h_data->SetMarkerStyle(20);
            h_data->SetMarkerSize(1.0);
            h_data->SetLineColor(kBlack);
            tree->Draw("BDTscore>>h_data", "", "goff");
        } else {
            // This is an MC tree
            TH1D *h = new TH1D(Form("h_mc%d", i), Form("MC Sample %d", i), 
                              nbins, xmin, xmax);
            h->SetLineColor(colors[i-1]);
            h->SetFillColor(colors[i-1]);
            
            tree->Draw(Form("BDTscore>>h_mc%d", i), "", "goff");
            
            mc_stack->Add(h);
            legend->AddEntry(h, Form("MC %d", i), "f");
            mc_histograms.push_back(h);
        }
    }
    
    if (!h_data) {
        std::cerr << "Data histogram not created! Check data_tree_number." << std::endl;
        return;
    }
    
    // Create canvas and divide it
    TCanvas *c1 = new TCanvas("c1", "Data vs MC", 800, 800);
    TPad *pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
    TPad *pad2 = new TPad("pad2", "pad2", 0, 0.0, 1, 0.3);
    pad1->SetBottomMargin(0.02);
    pad2->SetTopMargin(0.02);
    pad2->SetBottomMargin(0.3);
    pad1->Draw();
    pad2->Draw();
    
    // Draw main plot on pad1
    pad1->cd();
    pad1->SetLogy();  // Optional: use logarithmic scale
    
    // Get maximum for scaling
    double max_data = h_data->GetMaximum();
    double max_mc = mc_stack->GetMaximum();
    double ymax = (max_data > max_mc) ? max_data*1.5 : max_mc*1.5;
    
    mc_stack->SetMaximum(ymax);
    mc_stack->Draw("HIST");
    h_data->Draw("E1 SAME");
    legend->Draw();
    
    // Draw ratio plot on pad2
    pad2->cd();
    
    // Create ratio histogram (data/MC)
    TH1D *h_ratio = (TH1D*)h_data->Clone("h_ratio");
    
    // Sum all MC histograms
    TH1D *h_mc_sum = new TH1D(*mc_histograms[0]);
    for (size_t i = 1; i < mc_histograms.size(); i++) {
        h_mc_sum->Add(mc_histograms[i]);
    }
    
    h_ratio->Divide(h_mc_sum);
    h_ratio->SetTitle("");
    h_ratio->GetYaxis()->SetTitle("Data/MC");
    h_ratio->GetYaxis()->SetRangeUser(0.5, 1.5);
    h_ratio->GetXaxis()->SetLabelSize(0.1);
    h_ratio->GetYaxis()->SetLabelSize(0.1);
    h_ratio->GetYaxis()->SetTitleSize(0.1);
    h_ratio->GetYaxis()->SetTitleOffset(0.4);
    h_ratio->Draw("E1");
    
    // Draw a line at 1
    TLine *line = new TLine(xmin, 1, xmax, 1);
    line->SetLineColor(kRed);
    line->Draw();
    
    // Save the plot
    c1->SaveAs("data_mc_comparison.png");
    
    // Cleanup
    delete h_mc_sum;
    // Note: ROOT will handle the cleanup of other objects when the script ends
}
