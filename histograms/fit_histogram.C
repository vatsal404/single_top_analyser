#include <TFile.h>
#include <TH1D.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TRatioPlot.h>
#include <RooRealVar.h>
#include <RooDataHist.h>
#include <RooHistPdf.h>
#include <RooAddPdf.h>
#include <RooFitResult.h>
#include <RooPlot.h>
#include <RooLognormal.h>
#include <RooConstVar.h>
#include <RooProdPdf.h>
#include <TMath.h>

void fit_histogram() {
    // Open the ROOT file
    TFile *file = TFile::Open("multijet_template.root");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Cannot open file!" << std::endl;
        return;
    }

    // Retrieve histograms
    TH1D *h_multijet = (TH1D*)file->Get("h_multijet");
    TH1D *h_background_control = (TH1D*)file->Get("h_background_control");
    TH1D *h_data_control = (TH1D*)file->Get("h_data_control");

    if (!h_multijet || !h_background_control || !h_data_control) {
        std::cerr << "Error: One or more histograms not found!" << std::endl;
        return;
    }

    // Define observable
    RooRealVar x("x", "Observable", h_data_control->GetXaxis()->GetXmin(), h_data_control->GetXaxis()->GetXmax());

    // Convert histograms to RooFit objects
    RooDataHist dh_data_control("dh_data_control", "Data", RooArgList(x), h_data_control);
    RooDataHist dh_multijet("dh_multijet", "QCD", RooArgList(x), h_multijet);
    RooDataHist dh_background("dh_background", "Background", RooArgList(x), h_background_control);

    RooHistPdf pdf_multijet("pdf_multijet", "QCD PDF", RooArgSet(x), dh_multijet);
    RooHistPdf pdf_background("pdf_background", "Background PDF", RooArgSet(x), dh_background);

    // Define normalization parameters
    double nominal_background = h_background_control->Integral();
    double nominal_qcd = h_multijet->Integral();
    
    // Set up log-normal constraint parameters for 30% uncertainty
    double sigma_rel = 0.30;
    
    // Calculate proper log-normal parameters
    // For log-normal: if X ~ LogN(μ,σ), then:
    // median = exp(μ)
    // kappa = exp(σ)
    // and relative uncertainty = sqrt(exp(σ^2) - 1)
    
    double sigma = sqrt(log(1 + sigma_rel * sigma_rel));
    double mu = log(nominal_background);
    
    // Create RooConstVar for the log-normal parameters
    RooConstVar mu_const("mu_const", "mu", mu);
    RooConstVar sigma_const("sigma_const", "sigma", sigma);

    // Define normalization parameters with appropriate ranges
    RooRealVar norm_multijet("norm_multijet", "QCD norm", nominal_qcd, 0, 2 * h_data_control->Integral());
    RooRealVar norm_background("norm_background", "Background norm", nominal_background, 
                              0.5 * nominal_background,  // Lower bound
                              1.5 * nominal_background); // Upper bound

    // Create log-normal constraint
    RooLognormal background_constraint("background_constraint", "Background Constraint", 
                                     norm_background, mu_const, sigma_const);

    // Define model with constraint
    RooAddPdf sum_model("sum_model", "Sum Model", 
                       RooArgList(pdf_multijet, pdf_background),
                       RooArgList(norm_multijet, norm_background));
    
    RooProdPdf model("model", "Total Model", RooArgSet(sum_model, background_constraint));

    // Print initial constraint value
    std::cout << "Initial constraint value: " << background_constraint.getVal() << std::endl;

    // Perform ML fit with more verbose output
    RooFitResult *fitResult = model.fitTo(dh_data_control, 
                                         RooFit::Save(), 
                                         RooFit::PrintLevel(2),
                                         RooFit::Extended(true));
    
    // Print detailed fit results
    fitResult->Print("v");
    
    // Print final values and their relation to nominal
    std::cout << "Nominal background: " << nominal_background << std::endl;
    std::cout << "Fitted background: " << norm_background.getVal() 
              << " (" << (norm_background.getVal()/nominal_background - 1)*100 << "% change)" << std::endl;
    std::cout << "Final constraint value: " << background_constraint.getVal() << std::endl;

    // Scale histograms to fit results
    h_multijet->Scale(norm_multijet.getVal() / nominal_qcd);
    h_background_control->Scale(norm_background.getVal() / nominal_background);

    // Set colors & styles
    h_multijet->SetFillColor(kGray + 2);
    h_background_control->SetFillColor(kRed);
    h_data_control->SetMarkerStyle(20);

    // Create combined histogram
    TH1D *h_fit = (TH1D*)h_multijet->Clone("h_fit");
    h_fit->Add(h_background_control);

    // Canvas and pads
    TCanvas *c1 = new TCanvas("c1", "Fit Results", 800, 800);
    TPad *pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
    TPad *pad2 = new TPad("pad2", "pad2", 0, 0.05, 1, 0.3);
    pad1->SetBottomMargin(0.02);
    pad2->SetTopMargin(0);
    pad2->SetBottomMargin(0.25);
    pad1->Draw(); 
    pad2->Draw();

    // Draw main plot
    pad1->cd();
    THStack *hs = new THStack("hs", "");
    hs->Add(h_background_control);
    hs->Add(h_multijet);
    h_data_control->Draw("E");
    hs->Draw("HIST SAME");
    h_data_control->Draw("E SAME");

    // Add legend
    TLegend *leg = new TLegend(0.65, 0.65, 0.88, 0.85);
    leg->AddEntry(h_data_control, "Data", "lep");
    leg->AddEntry(h_multijet, "QCD (fitted)", "f");
    leg->AddEntry(h_background_control, "Background (fitted)", "f");
    leg->SetBorderSize(0);
    leg->Draw();

    // Draw ratio plot
    pad2->cd();
    TH1D *h_ratio = (TH1D*)h_data_control->Clone("h_ratio");
    h_ratio->Divide(h_fit);
    h_ratio->SetStats(0);
    h_ratio->GetYaxis()->SetTitle("Data/Fit");
    h_ratio->GetYaxis()->SetRangeUser(0.5, 1.5);
    h_ratio->GetXaxis()->SetTitle(h_data_control->GetXaxis()->GetTitle());
    h_ratio->Draw("EP");
    
    TLine *line = new TLine(h_ratio->GetXaxis()->GetXmin(), 1, h_ratio->GetXaxis()->GetXmax(), 1);
    line->SetLineStyle(2);
    line->Draw();

    c1->SaveAs("fit_result.png");
    file->Close();
}
