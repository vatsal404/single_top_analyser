/**
 * Get quantile from 2D dR vs tprimeMass histogram
 * Then plot dR vs Quantile..
 * CDozen
*/
#include <iostream>
//#include <cmath>
#include <TH1D.h>
#include <TH2F.h>
#include <TFile.h>
#include <TRandom3.h>
#include <TMath.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TLegendEntry.h>
#include <THStack.h>
#include <TGraph.h>
#include <TGraphAsymmErrors.h>
#include <TMultiGraph.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TString.h>
#include <TMath.h>
void Quantiles_jj(){

  //signal region for full range
  TFile *f1 = new TFile ("/Users/dozen-altuntas/Desktop/GetQuantile_Method/data/FullRange/20230326_3T_2018UL_Mtop140_FullBRew_FULLRANGEcut5pol3_65_cut7pol4_80/BkgEst3Tlistinput2018ULData.root","read"); 
 
  TH2F* dd1 = (TH2F*)f1->Get("TPMassvsDRjjW_CD");

  //create canvas and test print

  TCanvas * can = new TCanvas("can", "can", 600, 450);
  //can->cd();
  dd1->Draw("Colz");
  //dd1->RebinX(2); for rebin*2
  
  TH1F * zz[50];

  for(int i =0;i<dd1->GetNbinsX();i++){

    TString hname= Form("ProjectionY_%d",i+1); //Get DR plots depends on each TprimeMass bins
    zz[i] = (TH1F*) dd1->ProjectionY(hname,i+1,i+1);// calculating the quantiles for the full range of Y-axis bins for  each value of i.

  }

  TCanvas * can2 = new TCanvas("can2", "can2", 800, 600);
  //double n= dd->GetNbinsY()/2;
  can2->Divide(5,10);
  can2->cd();
  std::cout<<"NbinsX:"<<dd1->GetNbinsX()<<std::endl;
  std::cout<<"NbinsY:"<<dd1->GetNbinsY()<<std::endl;

  const int nq  = 50;
  double a,b;

  b=0.80; 
  // y bins array
  Double_t xbins[nq]; //dR
  Double_t ybins[nq];
  Double_t quantile[nq]; // mass quantile
  for(int i=0;i<dd1->GetNbinsX();i++){
    can2->cd(i+1);
    zz[i]->Draw();
    zz[i]->GetQuantiles(1,&a,&b);// calculate the quantiles for zz[i] and store it &a. 
    TAxis* xAxis = dd1->GetXaxis();
    TAxis* yAxis = dd1->GetYaxis();
 
    ybins[i]=yAxis->GetBinUpEdge(i);
    xbins[i]=xAxis->GetBinUpEdge(i);  
    quantile[i]=a; //assign the a value to quantile[i]
    
    std::cout<<"bin"<<i<<"\t LE:"<<xbins[i]<<"\t a:"<<a<<"\t b:"<<b<<std::endl;
  }


  TCanvas * can3 = new TCanvas("can3", "can3", 800, 600);
  can3->cd();
  TGraph *gr = new TGraph(nq,xbins, quantile); 

  gPad->SetGrid();
  gStyle->SetOptFit(0);
  gr->SetMarkerColor(kRed);
  gr->SetMarkerStyle(20);
 
  gr->SetLineWidth(0);
  gr->SetTitle("DR_jjW_2M1L_signal");
  gr->GetXaxis()->SetTitle("TprimeMass(GeV)");
  gr->GetYaxis()->SetTitle("quantile");
  gr->Draw("apl");
  //gPad->BuildLegend();

  gr->Fit("pol4","SR","",280,1270); //for fullrange!!
  TF1* fitFunc = gr->GetFunction("pol4"); // get the fit function
  //TF1* fitFunc5 = gr->GetFunction("pol5");
  //TF1* fitFunc6 = gr->GetFunction("pol6");
  TVirtualFitter* fitter = TVirtualFitter::GetFitter(); // get the TVirtualFitter object
  TH1F* hConfIntervals = new TH1F("hConfIntervals", "Confidence Intervals", fitFunc->GetNpx(), fitFunc->GetXmin(),  fitFunc->GetXmax());
  fitFunc->SetLineColor(kBlue);
  fitFunc->SetLineWidth(2);  

  fitter->GetConfidenceIntervals(hConfIntervals); // get the confidence intervals

  hConfIntervals->SetFillColorAlpha(kBlue, 0.2);
  hConfIntervals->Draw("e6 SAME"); // draw the confidence intervals as a band

  // Create an empty string called "Correction" using the TString class
  TString Correction = "";

  // Loop over the first four parameters of the fit function
  for (int i = 0; i < 5; i++)
  {
    // Get the i-th fit parameter
    double param = fitFunc->GetParameter(i);

    // Check the sign of the parameter and add the appropriate sign symbol to the correction string
    if (param >= 0 && i > 0)
    {
      Correction += " + ";
    }
    else if (param < 0)
    {
      Correction += " - ";
      param = -param;
    }

    // Create a stringstream object to convert the parameter to scientific notation
    std::stringstream myparameter;
    myparameter << std::scientific << param;

    // Define a string indicating what the correction is based on (mass or eta, for example)
    std::string cutTP = "Reconstructed_Tprime->M()";

    // Add a term to the Correction TString that includes the i-th fit parameter
    Correction += Form("%s*pow(%s,%d)", myparameter.str().c_str(), cutTP.c_str(), i);
  }

  // Print the Correction TString to the console
  std::cout << "Correction: " << Correction << std::endl;
  //In this output, there is a minus sign before the first term and a plus sign between each subsequent term, as  appropriate based on the sign of the corresponding fit parameter.

  TLegend* legend = new TLegend(0.40, 0.70, 0.60, 0.90); // (x1, y1, x2, y2) in NDC coordinates

  // set the legend style
  legend->SetBorderSize(1);
  legend->SetFillColor(0);
  legend->SetTextSize(0.03);

  // add an entry to the legend

  legend->AddEntry(gr, "Full Range", "");
  //legend->AddEntry(gr, "RebinX*2", "");
  legend->AddEntry(gr, "80% quantile", "p"); // (object, label, option)
  legend->AddEntry(fitFunc, "Fit: pol4", "l"); //defult for pol3
  //legend->AddEntry(fitFunc5, "Fit: pol5", "l");
  //legend->AddEntry(fitFunc6, "Fit: pol6", "l");
  legend->AddEntry(hConfIntervals, "%95 CL band", "f");
  legend->Draw();

TFile f("Outputs/FullRange/DR_jjw_80_pol4.root","RECREATE");
gr->Write();
f.Write();

}
