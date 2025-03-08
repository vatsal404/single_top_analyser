#include <TFile.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <THStack.h>
#include <TLegend.h>
#include <TStyle.h>

void make_stack_plot() {
    // Open the ROOT file
    TFile *file = TFile::Open("Final_histograms.root", "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Unable to open file!" << std::endl;
        return;
    }

    // Retrieve histograms from the file
    TH1F *DY_Combined = (TH1F*)file->Get("DY_Combined");
    TH1F *QCD_Combined = (TH1F*)file->Get("QCD_Combined");
    TH1F *ST_tW_channel_antitop_UL17 = (TH1F*)file->Get("ST_tW-channel_antitop_UL17");
    TH1F *ST_tW_channel_top_UL17 = (TH1F*)file->Get("ST_tW-channel_top_UL17");
    TH1F *Single_Top_antitop = (TH1F*)file->Get("Single_Top_antitop");
    TH1F *Single_Top_t_channel = (TH1F*)file->Get("Single_Top_t-channel");
    TH1F *TTbar_channel_top_UL17 = (TH1F*)file->Get("TTbar-channel_top_UL17");
    TH1F *WJetsToLNu_UL17 = (TH1F*)file->Get("WJetsToLNu_UL17");

    // Check if histograms are retrieved successfully
    if (!DY_Combined || !QCD_Combined || !ST_tW_channel_antitop_UL17 || 
        !ST_tW_channel_top_UL17 || !Single_Top_antitop || 
        !Single_Top_t_channel || !TTbar_channel_top_UL17 || !WJetsToLNu_UL17) {
        std::cerr << "Error: Failed to retrieve one or more histograms!" << std::endl;
        file->Close();
        return;
    }

    // Create a stack for histograms
    THStack *stack = new THStack("stack", "Stacked Histograms;X-axis Title;Y-axis Title");

    // Assign colors to histograms
    DY_Combined->SetFillColor(kBlue);
    QCD_Combined->SetFillColor(kOrange);
    ST_tW_channel_antitop_UL17->SetFillColor(kRed+1);
    ST_tW_channel_top_UL17->SetFillColor(kRed);
    Single_Top_antitop->SetFillColor(kViolet+1);
    Single_Top_t_channel->SetFillColor(kViolet);
    TTbar_channel_top_UL17->SetFillColor(kYellow+1);
    WJetsToLNu_UL17->SetFillColor(kGreen);

    // Add histograms to the stack
    stack->Add(DY_Combined);
    stack->Add(QCD_Combined);
    stack->Add(ST_tW_channel_antitop_UL17);
    stack->Add(ST_tW_channel_top_UL17);
    stack->Add(Single_Top_antitop);
    stack->Add(Single_Top_t_channel);
    stack->Add(TTbar_channel_top_UL17);
    stack->Add(WJetsToLNu_UL17);

    // Create a canvas
    TCanvas *c1 = new TCanvas("c1", "Stacked Plot", 800, 600);
    gStyle->SetOptStat(0); // Disable statistics box

    // Draw the stack
    stack->Draw("HIST");

    // Create a legend
    TLegend *legend = new TLegend(0.7, 0.6, 0.9, 0.9);
    legend->AddEntry(DY_Combined, "DY", "f");
    legend->AddEntry(QCD_Combined, "QCD", "f");
    legend->AddEntry(ST_tW_channel_antitop_UL17, "ST tW #bar{t}", "f");
    legend->AddEntry(ST_tW_channel_top_UL17, "ST tW t", "f");
    legend->AddEntry(Single_Top_antitop, "Single Top #bar{t}", "f");
    legend->AddEntry(Single_Top_t_channel, "Single Top t", "f");
    legend->AddEntry(TTbar_channel_top_UL17, "t#bar{t}", "f");
    legend->AddEntry(WJetsToLNu_UL17, "W+Jets", "f");
    legend->Draw();

    // Save the plot
    c1->SaveAs("stack_plot.png");

    // Clean up
    file->Close();
    delete c1;
}

