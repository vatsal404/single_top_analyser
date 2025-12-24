#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"

using namespace std;

/********************************************************************/
/* Struct to hold efficiency results                                */
/********************************************************************/
struct EffResult {
    TH2D* eff;
    TH2D* num;
    TH2D* den;
};

/********************************************************************/
/* Efficiency calculation (weighted-safe, bug-free)                 */
/********************************************************************/
EffResult calculate_efficiency_2d(
    const string& file_num,
    const string& file_den,
    const string& tree_name,
    const string& var_x,
    const string& var_y,
    const vector<double>& bins_x,
    const vector<double>& bins_y,
    const string& weight_branch = "evWeight"
) {
    TFile* fNum = TFile::Open(file_num.c_str());
    TFile* fDen = TFile::Open(file_den.c_str());

    if (!fNum || fNum->IsZombie())
        throw runtime_error("Cannot open " + file_num);
    if (!fDen || fDen->IsZombie())
        throw runtime_error("Cannot open " + file_den);

    TTree* tNum = (TTree*)fNum->Get(tree_name.c_str());
    TTree* tDen = (TTree*)fDen->Get(tree_name.c_str());
    if (!tNum || !tDen)
        throw runtime_error("Tree not found");

    cout << "  Numerator entries   : " << tNum->GetEntries() << endl;
    cout << "  Denominator entries : " << tDen->GetEntries() << endl;

    TH2D* hNum = new TH2D(
        "h_num", "",
        bins_x.size() - 1, bins_x.data(),
        bins_y.size() - 1, bins_y.data()
    );
    TH2D* hDen = new TH2D(
        "h_den", "",
        bins_x.size() - 1, bins_x.data(),
        bins_y.size() - 1, bins_y.data()
    );
    hNum->Sumw2();
    hDen->Sumw2();

    Double_t x = 0.0, y = 0.0, w = 1.0;

    // --- Numerator ---
    tNum->SetBranchAddress(var_x.c_str(), &x);
    tNum->SetBranchAddress(var_y.c_str(), &y);
    if (tNum->GetBranch(weight_branch.c_str()))
        tNum->SetBranchAddress(weight_branch.c_str(), &w);

    for (Long64_t i = 0; i < tNum->GetEntries(); ++i) {
        tNum->GetEntry(i);
        if (x > 0 && y > 0)
            hNum->Fill(x, y, w);
    }

    // --- Denominator ---
    tDen->SetBranchAddress(var_x.c_str(), &x);
    tDen->SetBranchAddress(var_y.c_str(), &y);
    if (tDen->GetBranch(weight_branch.c_str()))
        tDen->SetBranchAddress(weight_branch.c_str(), &w);

    for (Long64_t i = 0; i < tDen->GetEntries(); ++i) {
        tDen->GetEntry(i);
        if (x > 0 && y > 0)
            hDen->Fill(x, y, w);
    }

    cout << "  Numerator integral   : " << hNum->Integral() << endl;
    cout << "  Denominator integral : " << hDen->Integral() << endl;

    TH2D* hEff = new TH2D(
        "h_eff", "",
        bins_x.size() - 1, bins_x.data(),
        bins_y.size() - 1, bins_y.data()
    );

    for (int ix = 1; ix <= hEff->GetNbinsX(); ++ix) {
        for (int iy = 1; iy <= hEff->GetNbinsY(); ++iy) {
            double num = hNum->GetBinContent(ix, iy);
            double den = hDen->GetBinContent(ix, iy);
            if (den > 0) {
                double eff = std::clamp(num / den, 0.0, 1.0);
                double err = sqrt(eff * (1.0 - eff) / den);
                hEff->SetBinContent(ix, iy, eff);
                hEff->SetBinError(ix, iy, err);
            }
        }
    }

    hNum->SetDirectory(0);
    hDen->SetDirectory(0);
    hEff->SetDirectory(0);

    fNum->Close();
    fDen->Close();

    return {hEff, hNum, hDen};
}

/********************************************************************/
/* Build SF histogram                                                */
/********************************************************************/
TH2D* makeSF(TH2D* data_eff, TH2D* mc_eff, const string& name) {
    TH2D* sf = (TH2D*)data_eff->Clone(name.c_str());
    sf->Divide(data_eff, mc_eff);
    sf->SetDirectory(0);
    return sf;
}

/********************************************************************/
/* Main                                                             */
/********************************************************************/
int main() {

    vector<double> bins_el = {0,30,35,40,45,50,60,70,80,100,120,150,200,300,500};
    vector<double> bins_mu = {0,25,30,35,40,45,50,60,70,80,100,120,150,200,300,500};

    cout << "\n=== NOMINAL DATA ===\n";
    auto data_nom = calculate_efficiency_2d(
        "Data_0.root", "Data_1.root",
        "outputTree",
        "goodElectrons_leading_pt",
        "goodmuons_leading_pt",
        bins_el, bins_mu
    );

    cout << "\n=== NOMINAL MC ===\n";
    auto mc_nom = calculate_efficiency_2d(
        "TTbar_0.root", "TTbar_1.root",
        "outputTree",
        "goodElectrons_leading_pt",
        "goodmuons_leading_pt",
        bins_el, bins_mu
    );

    TH2D* sf_nom = makeSF(data_nom.eff, mc_nom.eff, "scale_factor");

    // --- Statistical uncertainty ---
    TH2D* sf_stat = (TH2D*)sf_nom->Clone("scale_factor_stat");
    sf_stat->Reset();

    for (int ix = 1; ix <= sf_nom->GetNbinsX(); ++ix)
        for (int iy = 1; iy <= sf_nom->GetNbinsY(); ++iy) {
            double d = data_nom.eff->GetBinContent(ix, iy);
            double m = mc_nom.eff->GetBinContent(ix, iy);
            if (d > 0 && m > 0) {
                double de = data_nom.eff->GetBinError(ix, iy);
                double me = mc_nom.eff->GetBinError(ix, iy);
                sf_stat->SetBinContent(ix, iy,
                    sf_nom->GetBinContent(ix, iy) *
                    sqrt(pow(de/d,2) + pow(me/m,2))
                );
            }
        }

    cout << "\n=== SYSTEMATICS (DATA ONLY) ===\n";

    auto data_puLT35 = calculate_efficiency_2d("Data_2.root","Data_3.root","outputTree",
                                               "goodElectrons_leading_pt","goodmuons_leading_pt",
                                               bins_el,bins_mu);

    auto data_puUD   = calculate_efficiency_2d("Data_4.root","Data_5.root","outputTree",
                                               "goodElectrons_leading_pt","goodmuons_leading_pt",
                                               bins_el,bins_mu);

    auto data_ngjGE3 = calculate_efficiency_2d("Data_6.root","Data_7.root","outputTree",
                                               "goodElectrons_leading_pt","goodmuons_leading_pt",
                                               bins_el,bins_mu);

    auto data_ngjLT3 = calculate_efficiency_2d("Data_8.root","Data_9.root","outputTree",
                                               "goodElectrons_leading_pt","goodmuons_leading_pt",
                                               bins_el,bins_mu);

    TH2D* sf_puLT35 = makeSF(data_puLT35.eff, mc_nom.eff, "sf_puLT35");
    TH2D* sf_puUD   = makeSF(data_puUD.eff,   mc_nom.eff, "sf_puUD");
    TH2D* sf_ngjGE3 = makeSF(data_ngjGE3.eff, mc_nom.eff, "sf_ngjGE3");
    TH2D* sf_ngjLT3 = makeSF(data_ngjLT3.eff, mc_nom.eff, "sf_ngjLT3");

    TH2D* sf_syst = (TH2D*)sf_nom->Clone("scale_factor_syst");
    sf_syst->Reset();

    for (int ix = 1; ix <= sf_nom->GetNbinsX(); ++ix)
        for (int iy = 1; iy <= sf_nom->GetNbinsY(); ++iy) {

            double sf0 = sf_nom->GetBinContent(ix, iy);
            if (sf0 <= 0) continue;

            double sum2 = 0;
            auto add = [&](TH2D* h) {
                double v = h->GetBinContent(ix, iy);
                if (v > 0) sum2 += pow(v - sf0, 2);
            };

            add(sf_puLT35);
            add(sf_puUD);
            add(sf_ngjGE3);
            add(sf_ngjLT3);

            sf_syst->SetBinContent(ix, iy, sqrt(sum2));
        }

    TH2D* sf_total = (TH2D*)sf_nom->Clone("scale_factor_total");
    sf_total->Reset();

    for (int ix = 1; ix <= sf_nom->GetNbinsX(); ++ix)
        for (int iy = 1; iy <= sf_nom->GetNbinsY(); ++iy)
            sf_total->SetBinContent(ix, iy,
                sqrt(pow(sf_stat->GetBinContent(ix, iy),2) +
                     pow(sf_syst->GetBinContent(ix, iy),2)));

    TFile* fout = new TFile("trigger_scale_factors.root","RECREATE");

    sf_nom->Write();
    sf_stat->Write();
    sf_syst->Write();
    sf_total->Write();

    sf_puLT35->Write();
    sf_puUD->Write();
    sf_ngjGE3->Write();
    sf_ngjLT3->Write();

    fout->Close();

    cout << "\nSUCCESS: trigger_scale_factors.root written\n";
    return 0;
}

