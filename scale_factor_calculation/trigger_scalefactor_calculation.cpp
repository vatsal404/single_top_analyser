#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TPaletteAxis.h"
#include "TROOT.h"

using namespace std;

/********************************************************************/
/* Struct */
/********************************************************************/
struct EffResult {
    TH2D* eff;
    TH2D* num;
    TH2D* den;
};

/********************************************************************/
/* PNG saving helper — auto z-range from non-zero bin percentiles  */
/********************************************************************/
/********************************************************************/
/* PNG saving helper — auto z-range + zero bins shown in white     */
/********************************************************************/
void saveHistogramPNG(TH2D* h, const string& filename, const string& title,
                      double zmin = -1, double zmax = -1,
                      int palette = kBird)
{
    gStyle->SetOptStat(0);
    gStyle->SetNumberContours(255);

    // --- Auto z-range from 5th-95th percentile of non-zero bins ---
    if (zmin < 0 || zmax < 0) {
        vector<double> vals;
        for (int ix = 1; ix <= h->GetNbinsX(); ++ix)
            for (int iy = 1; iy <= h->GetNbinsY(); ++iy) {
                double v = h->GetBinContent(ix, iy);
                if (v > 0) vals.push_back(v);
            }
        if (!vals.empty()) {
            sort(vals.begin(), vals.end());
            int n = vals.size();
            double lo  = vals[max(0,     (int)(0.05 * n))];
            double hi  = vals[min(n - 1, (int)(0.95 * n))];
            double pad = 0.05 * (hi - lo);
            zmin = max(0.0, lo - pad);
            zmax = hi + pad;
        }
    }

    // --- Build a custom palette: index 0 = light grey for zero bins,
    //     indices 1-254 = chosen palette for non-zero values ---
    gStyle->SetPalette(palette);
    // Grab the palette ROOT just set
    const int nBase = 254;
    vector<double> reds(nBase), greens(nBase), blues(nBase);
    for (int i = 0; i < nBase; ++i) {
        TColor* col = gROOT->GetColor(TColor::GetColorPalette(i * (255 / nBase)));
        if (col) {
            reds[i]   = col->GetRed();
            greens[i] = col->GetGreen();
            blues[i]  = col->GetBlue();
        }
    }

    // Build 255-entry custom palette:
    //   slot 0  → light grey  (for zero/empty bins)
    //   slots 1-254 → original palette colours
    const int nTotal = 255;
    vector<Int_t> customPalette(nTotal);

    // Grey for zero
    customPalette[0] = TColor::GetColor(0.82f, 0.82f, 0.82f);

    // Remaining slots from the base palette
    for (int i = 1; i < nTotal; ++i) {
        int src = (int)((i - 1) * (nBase - 1.0) / (nTotal - 2));
        src = min(src, nBase - 1);
        customPalette[i] = TColor::GetColor(
            (float)reds[src], (float)greens[src], (float)blues[src]);
    }

    gStyle->SetPalette(nTotal, customPalette.data());
    gStyle->SetNumberContours(nTotal);

    TCanvas* c = new TCanvas(("c_" + filename).c_str(), title.c_str(), 900, 750);
    c->SetRightMargin(0.15);
    c->SetLeftMargin(0.12);
    c->SetBottomMargin(0.12);

    // Clone to avoid mutating the original histogram
    TH2D* hDraw = (TH2D*)h->Clone(("draw_" + filename).c_str());
    hDraw->SetTitle(title.c_str());
    hDraw->GetXaxis()->SetTitle("Leading electron p_{T} [GeV]");
    hDraw->GetYaxis()->SetTitle("Leading muon p_{T} [GeV]");

    // Set zero bins to a sentinel value just below zmin so they
    // fall into palette slot 0 (grey), while real small values
    // remain in the colour range [zmin, zmax]
    double sentinel = zmin - (zmax - zmin) / (nTotal - 2);
    for (int ix = 1; ix <= hDraw->GetNbinsX(); ++ix)
        for (int iy = 1; iy <= hDraw->GetNbinsY(); ++iy)
            if (hDraw->GetBinContent(ix, iy) == 0)
                hDraw->SetBinContent(ix, iy, sentinel);

    hDraw->SetMinimum(sentinel);
    hDraw->SetMaximum(zmax);

    hDraw->Draw("COLZ");
    c->Update();

    // Widen palette axis
    TPaletteAxis* palette_axis =
        (TPaletteAxis*)hDraw->GetListOfFunctions()->FindObject("palette");
    if (palette_axis) {
        palette_axis->SetX1NDC(0.86);
        palette_axis->SetX2NDC(0.91);
        // Force the displayed axis labels to run from zmin to zmax,
        // hiding the sentinel slot from the user
        c->Modified();
        c->Update();
    }

    c->SaveAs(filename.c_str());
    delete hDraw;
    delete c;

    // Restore default palette so other plots aren't affected
    gStyle->SetPalette(palette);
}
/********************************************************************/
/* Efficiency calculation (UNWEIGHTED ONLY) */
/********************************************************************/
EffResult calculate_efficiency_2d(
    const string& file_num,
    const string& file_den,
    const string& tree_name,
    const string& var_x,
    const string& var_y,
    const vector<double>& bins_x,
    const vector<double>& bins_y,
    const string& weight_branch )
{
    TFile* fNum = TFile::Open(file_num.c_str());
    TFile* fDen = TFile::Open(file_den.c_str());
    if (!fNum || fNum->IsZombie()) throw runtime_error("Cannot open " + file_num);
    if (!fDen || fDen->IsZombie()) throw runtime_error("Cannot open " + file_den);

    TTree* tNum = (TTree*)fNum->Get(tree_name.c_str());
    TTree* tDen = (TTree*)fDen->Get(tree_name.c_str());
    if (!tNum || !tDen) throw runtime_error("Tree not found");

    TH2D* hNum = new TH2D(("num_"+file_num).c_str(),"",
                           bins_x.size()-1, bins_x.data(),
                           bins_y.size()-1, bins_y.data());
    TH2D* hDen = new TH2D(("den_"+file_den).c_str(),"",
                           bins_x.size()-1, bins_x.data(),
                           bins_y.size()-1, bins_y.data());
    hNum->Sumw2();
    hDen->Sumw2();

    Double_t x=0, y=0;
    tNum->SetBranchAddress(var_x.c_str(), &x);
    tNum->SetBranchAddress(var_y.c_str(), &y);
    for (Long64_t i=0; i<tNum->GetEntries(); ++i) {
        tNum->GetEntry(i);
        if (x>0 && y>0) hNum->Fill(x,y);
    }

    tDen->SetBranchAddress(var_x.c_str(), &x);
    tDen->SetBranchAddress(var_y.c_str(), &y);
    for (Long64_t i=0; i<tDen->GetEntries(); ++i) {
        tDen->GetEntry(i);
        if (x>0 && y>0) hDen->Fill(x,y);
    }

    TH2D* hEff = new TH2D(("eff_"+file_num).c_str(),"",
                           bins_x.size()-1, bins_x.data(),
                           bins_y.size()-1, bins_y.data());
    for (int ix=1; ix<=hEff->GetNbinsX(); ++ix)
        for (int iy=1; iy<=hEff->GetNbinsY(); ++iy) {
            double num = hNum->GetBinContent(ix,iy);
            double den = hDen->GetBinContent(ix,iy);
            if (den > 0) {
                double eff = num/den;
                double err = sqrt(eff*(1-eff)/den);
                hEff->SetBinContent(ix,iy,eff);
                hEff->SetBinError(ix,iy,err);
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
/* SF */
/********************************************************************/
TH2D* makeSF(TH2D* d, TH2D* m, const string& name) {
    TH2D* sf = (TH2D*)d->Clone(name.c_str());
    sf->Divide(d, m);
    sf->SetDirectory(0);
    return sf;
}

/********************************************************************/
/* Main */
/********************************************************************/
int main() {
    vector<double> bins = {0,20,30,50,100,150,200,350};
    string weight_branch = "";

    // ================= NOMINAL =================
    auto data_nom = calculate_efficiency_2d(
        "Data_0.root","Data_1.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",
        bins, bins, weight_branch);
    auto mc_nom = calculate_efficiency_2d(
        "TTbar_0.root","TTbar_1.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",
        bins, bins, weight_branch);

    TH2D* sf_nom = makeSF(data_nom.eff, mc_nom.eff, "scale_factor");

    // ================= STAT =================
    cout << "\n========================================" << endl;
    cout << "CALCULATING STATISTICAL UNCERTAINTIES"    << endl;
    cout << "========================================" << endl;

    TH2D* sf_stat = (TH2D*)sf_nom->Clone("scale_factor_stat");
    sf_stat->Reset();

    for (int ix=1; ix<=sf_nom->GetNbinsX(); ++ix)
        for (int iy=1; iy<=sf_nom->GetNbinsY(); ++iy) {
            cout << "\n--- Bin (" << ix << "," << iy << ") ---" << endl;

            double ed = data_nom.eff->GetBinContent(ix,iy);
            double em = mc_nom.eff->GetBinContent(ix,iy);
            cout << "  Data eff (ed) = " << ed << endl;
            cout << "  MC eff (em)   = " << em << endl;
            if (ed<=0 || em<=0) { cout << "  SKIPPED: Efficiency <= 0" << endl; continue; }

            double Xw  = data_nom.num->GetBinContent(ix,iy);
            double Xw2 = pow(data_nom.num->GetBinError(ix,iy), 2);
            double Yw  = mc_nom.num->GetBinContent(ix,iy);
            double Yw2 = pow(mc_nom.num->GetBinError(ix,iy), 2);
            cout << "  Data: Xw = " << Xw << ", Xw2 = " << Xw2 << endl;
            cout << "  MC:   Yw = " << Yw << ", Yw2 = " << Yw2 << endl;

            double Xeff = (Xw2>0) ? Xw*Xw/Xw2 : 0;
            double Yeff = (Yw2>0) ? Yw*Yw/Yw2 : 0;
            cout << "  Xeff = " << Xeff << endl;
            cout << "  Yeff = " << Yeff << endl;
            if (Xeff<1 || Yeff<1) { cout << "  SKIPPED: Xeff or Yeff < 1" << endl; continue; }

            double T      = sf_nom->GetBinContent(ix,iy);
            double term1  = (1-ed)/Xeff;
            double term2  = (1-em)/Yeff;
            double varlnT = term1 + term2;
            cout << "  SF (T) = "       << T      << endl;
            cout << "  (1-ed)/Xeff = "  << term1  << endl;
            cout << "  (1-em)/Yeff = "  << term2  << endl;
            cout << "  varlnT = "       << varlnT << endl;
            if (varlnT < 0) cout << "  ⚠️  WARNING: varlnT < 0! Cannot take sqrt." << endl;

            double stat_unc = T * sqrt(varlnT);
            cout << "  sqrt(varlnT) = "              << sqrt(varlnT) << endl;
            cout << "  stat_unc = T*sqrt(varlnT) = " << stat_unc     << endl;
            if (std::isnan(stat_unc)) cout << "  ❌ NaN DETECTED!" << endl;

            sf_stat->SetBinContent(ix,iy, stat_unc);
        }

    cout << "\n========================================" << endl;
    cout << "STATISTICAL UNCERTAINTY CALCULATION COMPLETE" << endl;
    cout << "========================================\n"    << endl;

    // ================= SYSTEMATICS =================
    auto data_puLT35 = calculate_efficiency_2d("Data_2.root","Data_3.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",bins,bins,weight_branch);
    auto mc_puLT35   = calculate_efficiency_2d("TTbar_2.root","TTbar_3.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",bins,bins,weight_branch);

    auto data_puUD   = calculate_efficiency_2d("Data_4.root","Data_5.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",bins,bins,weight_branch);
    auto mc_puUD     = calculate_efficiency_2d("TTbar_4.root","TTbar_5.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",bins,bins,weight_branch);

    auto data_ngjGE3 = calculate_efficiency_2d("Data_6.root","Data_7.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",bins,bins,weight_branch);
    auto mc_ngjGE3   = calculate_efficiency_2d("TTbar_6.root","TTbar_7.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",bins,bins,weight_branch);

    auto data_ngjLT3 = calculate_efficiency_2d("Data_8.root","Data_9.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",bins,bins,weight_branch);
    auto mc_ngjLT3   = calculate_efficiency_2d("TTbar_8.root","TTbar_9.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",bins,bins,weight_branch);

    TH2D* sf_puLT35 = makeSF(data_puLT35.eff, mc_puLT35.eff, "sf_puLT35");
    TH2D* sf_puUD   = makeSF(data_puUD.eff,   mc_puUD.eff,   "sf_puUD");
    TH2D* sf_ngjGE3 = makeSF(data_ngjGE3.eff, mc_ngjGE3.eff, "sf_ngjGE3");
    TH2D* sf_ngjLT3 = makeSF(data_ngjLT3.eff, mc_ngjLT3.eff, "sf_ngjLT3");

    TH2D* sf_syst = (TH2D*)sf_nom->Clone("scale_factor_syst");
    sf_syst->Reset();
    for (int ix=1; ix<=sf_nom->GetNbinsX(); ++ix)
        for (int iy=1; iy<=sf_nom->GetNbinsY(); ++iy) {
            double sf0 = sf_nom->GetBinContent(ix,iy);
            if (sf0 <= 0) continue;
            double sum2 = 0;
            auto add = [&](TH2D* h) {
                double v = h->GetBinContent(ix,iy);
                if (v > 0) sum2 += (v-sf0)*(v-sf0);
            };
            add(sf_puLT35); add(sf_puUD); add(sf_ngjGE3); add(sf_ngjLT3);
            sf_syst->SetBinContent(ix,iy, sqrt(sum2));
        }

    TH2D* sf_total = (TH2D*)sf_nom->Clone("scale_factor_total");
    sf_total->Reset();
    for (int ix=1; ix<=sf_nom->GetNbinsX(); ++ix)
        for (int iy=1; iy<=sf_nom->GetNbinsY(); ++iy)
            sf_total->SetBinContent(ix,iy,
                sqrt(pow(sf_stat->GetBinContent(ix,iy), 2) +
                     pow(sf_syst->GetBinContent(ix,iy), 2)));

    // ================= OUTPUT (ROOT) =================
    TFile* fout = new TFile("trigger_scale_factors.root","RECREATE");
    sf_nom->Write();
    sf_stat->Write();
    sf_syst->Write();
    sf_total->Write();
    fout->Close();
    cout << "\n✅ SUCCESS: trigger_scale_factors.root written\n";

    // ================= OUTPUT (PNG) =================
    // Nominal SF — kBird palette, auto-ranged from data percentiles
    saveHistogramPNG(sf_nom,    "sf_nominal.png",
                     "Trigger Scale Factor (Nominal)",
                     -1, -1, kBird);

    // Individual systematic SFs — same palette, auto-ranged independently
    saveHistogramPNG(sf_puLT35, "sf_puLT35.png",
                     "SF Systematic: PU < 35",
                     -1, -1, kBird);
    saveHistogramPNG(sf_puUD,   "sf_puUD.png",
                     "SF Systematic: PU Up/Down",
                     -1, -1, kBird);
    saveHistogramPNG(sf_ngjGE3, "sf_ngjGE3.png",
                     "SF Systematic: N_{gj} #geq 3",
                     -1, -1, kBird);
    saveHistogramPNG(sf_ngjLT3, "sf_ngjLT3.png",
                     "SF Systematic: N_{gj} < 3",
                     -1, -1, kBird);

    // Uncertainty histograms — kRainBow so small values near zero are visible
    saveHistogramPNG(sf_stat,   "sf_stat_unc.png",
                     "Trigger SF Statistical Uncertainty",
                     -1, -1, kRainBow);
    saveHistogramPNG(sf_syst,   "sf_syst_unc.png",
                     "Trigger SF Systematic Uncertainty",
                     -1, -1, kRainBow);
    saveHistogramPNG(sf_total,  "sf_total_unc.png",
                     "Trigger SF Total Uncertainty",
                     -1, -1, kRainBow);

    cout << "✅ SUCCESS: PNG histograms written.\n";
    return 0;
}
