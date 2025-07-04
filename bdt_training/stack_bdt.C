#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "THStack.h"
#include "TLegend.h"
#include "TPad.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TLine.h"
#include "TBox.h"
#include <iostream>

void stack_bdt() {
  // Configuration
  const char* inputFile = "bdt2_muon2j1b_histograms.root";
  const char* multijetFile = "multijet_bdt2_QCDmuon2j1b_histograms.root";
  const char* outputImage = "bdt2_muon_postfit.png";

  // Scale factors
  const double sf_signaltop = 1.0300;
  const double sf_signalantitop = 1.0300;
  const double sf_ttbar = 1.0300;

  const double sf_wjets = 1.0300;
  const double sf_multijet = 2.2651;

  // Set ROOT style
  gStyle->SetOptStat(0);
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  gStyle->SetLegendBorderSize(0);
  gStyle->SetErrorX(0);

  // Open ROOT files
  TFile* rootFile = new TFile(inputFile, "READ");
  TFile* mjFile = new TFile(multijetFile, "READ");

  if (!rootFile || rootFile->IsZombie() || !mjFile || mjFile->IsZombie()) {
    std::cerr << "Error: Input file(s) not found or corrupted!" << std::endl;
    return;
  }

  // Load histograms
  TH1D* signaltopHist = (TH1D*)rootFile->Get("signal_top");
  TH1D* signalantitopHist = (TH1D*)rootFile->Get("signal_antitop");
  TH1D* ttbarHist = (TH1D*)rootFile->Get("ttbar");
  TH1D* wjetsHist = (TH1D*)rootFile->Get("wjets");
  TH1D* dataHist = (TH1D*)rootFile->Get("data");
  TH1D* multijetHist = (TH1D*)mjFile->Get("multijet");

  if (!signaltopHist || !signalantitopHist || !ttbarHist || !wjetsHist || !dataHist || !multijetHist) {
    std::cerr << "Error: One or more histograms not found in the file(s)!" << std::endl;
    rootFile->Close();
    mjFile->Close();
    delete rootFile;
    delete mjFile;
    return;
  }

  // Apply scale factors
  signaltopHist->Scale(sf_signaltop);
  signalantitopHist->Scale(sf_signalantitop);
  ttbarHist->Scale(sf_ttbar);
  wjetsHist->Scale(sf_wjets);
  multijetHist->Scale(sf_multijet);

  // Set histogram styles
  signaltopHist->SetFillColor(kRed-7);
  signaltopHist->SetLineColor(kRed);
  signaltopHist->SetLineWidth(2);

  signalantitopHist->SetFillColor(kBlue-7);
  signalantitopHist->SetLineColor(kBlue);
  signalantitopHist->SetLineWidth(2);

  ttbarHist->SetFillColor(kGreen-7);
  ttbarHist->SetLineColor(kGreen+2);
  ttbarHist->SetLineWidth(1);

  wjetsHist->SetFillColor(kAzure-9);
  wjetsHist->SetLineColor(kBlue);
  wjetsHist->SetLineWidth(1);

  multijetHist->SetFillColor(kOrange-3);
  multijetHist->SetLineColor(kOrange+7);
  multijetHist->SetLineWidth(1);

  dataHist->SetMarkerStyle(20);
  dataHist->SetMarkerSize(1.2);
  dataHist->SetMarkerColor(kBlack);
  dataHist->SetLineColor(kBlack);
  dataHist->SetLineWidth(2);

  // Create MC stack
  THStack* mcStack = new THStack("mcStack", "");
  mcStack->Add(multijetHist);
  mcStack->Add(wjetsHist);
  mcStack->Add(ttbarHist);
  mcStack->Add(signaltopHist);
  mcStack->Add(signalantitopHist);

  // Create MC sum for ratio
  TH1D* mcSum = (TH1D*)signaltopHist->Clone("mcSum");
  mcSum->Add(signalantitopHist);
  mcSum->Add(ttbarHist);
  mcSum->Add(wjetsHist);
  mcSum->Add(multijetHist);

  // Canvas setup
  TCanvas* canvas = new TCanvas("canvas", "BDT Analysis", 800, 800);
  canvas->cd();

  TPad* upperPad = new TPad("upperPad", "upperPad", 0, 0.3, 1, 1);
  upperPad->SetBottomMargin(0.02);
  upperPad->SetLeftMargin(0.12);
  upperPad->SetRightMargin(0.05);
  upperPad->Draw();

  TPad* lowerPad = new TPad("lowerPad", "lowerPad", 0, 0.05, 1, 0.3);
  lowerPad->SetTopMargin(0);
  lowerPad->SetBottomMargin(0.25);
  lowerPad->SetLeftMargin(0.12);
  lowerPad->SetRightMargin(0.05);
  lowerPad->SetGridy();
  lowerPad->Draw();

  // Draw upper pad
  upperPad->cd();
  Double_t dataMax = dataHist->GetMaximum();
  mcStack->Draw("HIST");
  Double_t mcMax = mcStack->GetMaximum();
  Double_t maxVal = std::max(dataMax, mcMax) * 1.3;
  mcStack->SetMaximum(maxVal);
  mcStack->SetMinimum(0.1);

  mcStack->Draw("HIST");
  dataHist->Draw("EP SAME");

  mcStack->GetXaxis()->SetLabelSize(0);
  mcStack->GetYaxis()->SetTitle("Events");
  mcStack->GetYaxis()->SetTitleSize(0.05);
  mcStack->GetYaxis()->SetTitleOffset(1.2);
  mcStack->GetYaxis()->SetLabelSize(0.045);

  TLatex lumiText;
  lumiText.SetNDC();
  lumiText.SetTextSize(0.04);
  lumiText.SetTextFont(42);
  lumiText.DrawLatex(0.65, 0.92, "41.48 fb^{-1} (13 TeV)");

	// Legen
  TLegend* legend = new TLegend(0.1, 0.75, 0.9, 0.9);
  legend->SetNColumns(3);
  legend->SetTextSize(0.03);
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);

  legend->AddEntry(signaltopHist, "t-channel top", "f");
  legend->AddEntry(signalantitopHist, "t-channel antitop", "f");
  legend->AddEntry(ttbarHist, "TTbar", "f");
  legend->AddEntry(wjetsHist, "W+jets", "f");
  legend->AddEntry(multijetHist, "QCD", "f");
  legend->AddEntry(dataHist, "Data", "lep");

  legend->Draw();



  // Draw lower pad
  lowerPad->cd();
  TH1D* ratioHist = (TH1D*)dataHist->Clone("ratioHist");
  ratioHist->Divide(mcSum);
  ratioHist->SetTitle("");
  ratioHist->SetMarkerStyle(20);
  ratioHist->SetMarkerSize(1.2);

  ratioHist->GetXaxis()->SetTitle("BDT Score");
  ratioHist->GetXaxis()->SetTitleSize(0.12);
  ratioHist->GetXaxis()->SetTitleOffset(0.9);
  ratioHist->GetXaxis()->SetLabelSize(0.11);

  ratioHist->GetYaxis()->SetTitle("Data/MC");
  ratioHist->GetYaxis()->SetTitleSize(0.12);
  ratioHist->GetYaxis()->SetTitleOffset(0.5);
  ratioHist->GetYaxis()->SetLabelSize(0.11);
  ratioHist->GetYaxis()->SetNdivisions(505);
  ratioHist->GetYaxis()->SetRangeUser(0.5, 1.5);
  ratioHist->Draw("EP");

  TLine* line = new TLine(ratioHist->GetXaxis()->GetXmin(), 1, ratioHist->GetXaxis()->GetXmax(), 1);
  line->SetLineColor(kRed);
  line->SetLineStyle(2);
  line->Draw("SAME");

  // Save the canvas
  canvas->SaveAs(outputImage);
  std::cout << "Plot saved as " << outputImage << std::endl;

  // Statistics
  std::cout << "\nHistogram Statistics:" << std::endl;
  std::cout << "Signal top events: " << signaltopHist->Integral() << std::endl;
  std::cout << "Signal antitop events: " << signalantitopHist->Integral() << std::endl;
  std::cout << "TTbar events: " << ttbarHist->Integral() << std::endl;
  std::cout << "W+Jets events: " << wjetsHist->Integral() << std::endl;
  std::cout << "Multijet events: " << multijetHist->Integral() << std::endl;
  std::cout << "Total MC events: " << mcSum->Integral() << std::endl;
  std::cout << "Data events: " << dataHist->Integral() << std::endl;
  std::cout << "Data/MC ratio: " << dataHist->Integral() / mcSum->Integral() << std::endl;

  // Clean up
  delete line;
  delete canvas;
  rootFile->Close();
  mjFile->Close();
  delete rootFile;
  delete mjFile;
}

