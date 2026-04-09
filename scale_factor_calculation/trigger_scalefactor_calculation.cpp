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
#include "TLatex.h"
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
/* PNG saving helper                                                 */
/*   - Zero/invalid bins shown in grey, no sentinel label printed   */
/*   - Real bin values printed as diagonal text labels              */
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

    // --- Build custom palette: index 0 = light grey for zero/invalid bins,
    //     indices 1-254 = chosen palette for real values ---
    gStyle->SetPalette(palette);
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

    const int nTotal = 255;
    vector<Int_t> customPalette(nTotal);
    customPalette[0] = TColor::GetColor(0.82f, 0.82f, 0.82f); // grey for invalid
    for (int i = 1; i < nTotal; ++i) {
        int src = (int)((i - 1) * (nBase - 1.0) / (nTotal - 2));
        src = min(src, nBase - 1);
        customPalette[i] = TColor::GetColor(
            (float)reds[src], (float)greens[src], (float)blues[src]);
    }

    gStyle->SetPalette(nTotal, customPalette.data());
    gStyle->SetNumberContours(nTotal);

    // --- Larger canvas for readability ---
    TCanvas* c = new TCanvas(("c_" + filename).c_str(), title.c_str(), 1800, 1600);
    c->SetRightMargin(0.15);
    c->SetLeftMargin(0.12);
    c->SetBottomMargin(0.12);

    // Clone for drawing — don't mutate the original
    TH2D* hDraw = (TH2D*)h->Clone(("draw_" + filename).c_str());
    hDraw->SetTitle(title.c_str());
    hDraw->GetXaxis()->SetTitle("Leading electron p_{T} [GeV]");
    hDraw->GetYaxis()->SetTitle("Leading muon p_{T} [GeV]");

    // FIX 1: Use a sentinel that is a FULL range below zmin, not just 1/253 of it.
    // This guarantees zero bins cleanly land in palette slot 0 (grey),
    // never in a gap between slot 0 and slot 1.
    double range    = (zmax > zmin) ? (zmax - zmin) : 1.0;
    double sentinel = zmin - range;

    for (int ix = 1; ix <= hDraw->GetNbinsX(); ++ix)
        for (int iy = 1; iy <= hDraw->GetNbinsY(); ++iy)
            if (hDraw->GetBinContent(ix, iy) <= 0)
                hDraw->SetBinContent(ix, iy, sentinel);

    hDraw->SetMinimum(sentinel);
    hDraw->SetMaximum(zmax);

    // FIX 2: Draw color map WITHOUT text (COLZ only), then manually add
    // TLatex labels only for bins with real (positive) content.
    // This prevents the sentinel value from being printed in grey bins.
    hDraw->Draw("COLZ");
    c->Update();

    // Widen palette axis
    TPaletteAxis* palette_axis =
        (TPaletteAxis*)hDraw->GetListOfFunctions()->FindObject("palette");
    if (palette_axis) {
        palette_axis->SetX1NDC(0.86);
        palette_axis->SetX2NDC(0.91);
        c->Modified();
        c->Update();
    }

    // Manual text labels — only for bins with real content
    TLatex latex;
    latex.SetTextSize(0.022);
    latex.SetTextAlign(22); // centre-centre
    latex.SetTextAngle(45);
    latex.SetNDC(false);

    for (int ix = 1; ix <= h->GetNbinsX(); ++ix) {
        for (int iy = 1; iy <= h->GetNbinsY(); ++iy) {
            double val = h->GetBinContent(ix, iy);
            if (val <= 0) continue; // skip invalid/empty bins — no label

            double x = h->GetXaxis()->GetBinCenter(ix);
            double y = h->GetYaxis()->GetBinCenter(iy);

            char buf[32];
            snprintf(buf, sizeof(buf), "%.3f", val);
            latex.DrawLatex(x, y, buf);
        }
    }

    // pT threshold note
    TLatex note;
    note.SetNDC(true);
    note.SetTextSize(0.030);
    note.SetTextAlign(11);
    note.SetTextColor(kBlack);
    note.DrawLatex(0.13, 0.875, "p_{T}^{e}, p_{T}^{#mu} > 20 GeV (below threshold not shown)");

    c->SaveAs(filename.c_str());
    delete hDraw;
    delete c;

    // Restore default palette
    gStyle->SetPalette(palette);
}

/********************************************************************/
/* Efficiency calculation (WEIGHTED by evWeight)                    */
/*                                                                  */
/* Events are weighted using the branch specified by weight_branch  */
/* (set to "evWeight" in main). Sumw2() is called before filling   */
/* so ROOT accumulates sum-of-squared-weights, which is needed for  */
/* correct error propagation on weighted histograms.                */
/*                                                                  */
/* If weight_branch is empty or the branch is missing in the tree,  */
/* the code falls back gracefully to unweighted filling (w = 1).   */
/* A clear message is printed to stdout in either case so you       */
/* always know which branch is actually being used.                 */
/********************************************************************/
EffResult calculate_efficiency_2d(
    const string& file_num,
    const string& file_den,
    const string& tree_name,
    const string& var_x,
    const string& var_y,
    const vector<double>& bins_x,
    const vector<double>& bins_y,
    const string& weight_branch)
{
    TFile* fNum = TFile::Open(file_num.c_str());
    TFile* fDen = TFile::Open(file_den.c_str());
    if (!fNum || fNum->IsZombie()) throw runtime_error("Cannot open " + file_num);
    if (!fDen || fDen->IsZombie()) throw runtime_error("Cannot open " + file_den);

    TTree* tNum = (TTree*)fNum->Get(tree_name.c_str());
    TTree* tDen = (TTree*)fDen->Get(tree_name.c_str());
    if (!tNum || !tDen) throw runtime_error("Tree not found");

    TH2D* hNum = new TH2D(("num_" + file_num).c_str(), "",
                           bins_x.size() - 1, bins_x.data(),
                           bins_y.size() - 1, bins_y.data());
    TH2D* hDen = new TH2D(("den_" + file_den).c_str(), "",
                           bins_x.size() - 1, bins_x.data(),
                           bins_y.size() - 1, bins_y.data());

    // Sumw2() MUST be called before filling so ROOT stores sum-of-squared-weights
    // per bin. Without this, weighted uncertainties will be wrong.
    hNum->Sumw2();
    hDen->Sumw2();

    // ---- Numerator tree ----
    Double_t x = 0, y = 0, wNum = 1.0;
    tNum->SetBranchAddress(var_x.c_str(), &x);
    tNum->SetBranchAddress(var_y.c_str(), &y);

    bool hasWeightNum = !weight_branch.empty() && (tNum->GetBranch(weight_branch.c_str()) != nullptr);
    if (hasWeightNum) {
        tNum->SetBranchAddress(weight_branch.c_str(), &wNum);
        cout << "[Efficiency] Numerator   (" << file_num << "): "
             << "weighting with branch '" << weight_branch << "'" << endl;
    } else {
        cout << "[Efficiency] Numerator   (" << file_num << "): "
             << "branch '" << weight_branch << "' NOT found — filling UNWEIGHTED (w=1)" << endl;
    }

    for (Long64_t i = 0; i < tNum->GetEntries(); ++i) {
        tNum->GetEntry(i);
        if (!hasWeightNum) wNum = 1.0; // safety: reset each event if branch absent
        if (x >= 0 && y >= 0) hNum->Fill(x, y, wNum);
    }

    // ---- Denominator tree ----
    Double_t wDen = 1.0;
    tDen->SetBranchAddress(var_x.c_str(), &x);
    tDen->SetBranchAddress(var_y.c_str(), &y);

    bool hasWeightDen = !weight_branch.empty() && (tDen->GetBranch(weight_branch.c_str()) != nullptr);
    if (hasWeightDen) {
        tDen->SetBranchAddress(weight_branch.c_str(), &wDen);
        cout << "[Efficiency] Denominator (" << file_den << "): "
             << "weighting with branch '" << weight_branch << "'" << endl;
    } else {
        cout << "[Efficiency] Denominator (" << file_den << "): "
             << "branch '" << weight_branch << "' NOT found — filling UNWEIGHTED (w=1)" << endl;
    }

    for (Long64_t i = 0; i < tDen->GetEntries(); ++i) {
        tDen->GetEntry(i);
        if (!hasWeightDen) wDen = 1.0;
        if (x >= 0 && y >= 0) hDen->Fill(x, y, wDen);
    }

    // ---- Efficiency: ratio num/den with weighted error propagation ----
    TH2D* hEff = new TH2D(("eff_" + file_num).c_str(), "",
                           bins_x.size() - 1, bins_x.data(),
                           bins_y.size() - 1, bins_y.data());
    for (int ix = 1; ix <= hEff->GetNbinsX(); ++ix) {
        for (int iy = 1; iy <= hEff->GetNbinsY(); ++iy) {
            double num = hNum->GetBinContent(ix, iy); // sum of weights in numerator
            double den = hDen->GetBinContent(ix, iy); // sum of weights in denominator
            if (den > 0) {
                double eff = num / den;
                // Error propagation for weighted ratio:
                //   en = sqrt(Sumw2_num), ed = sqrt(Sumw2_den)  [stored by ROOT after Sumw2()]
                //   Var(eff) = en^2/den^2 + num^2 * ed^2 / den^4
                double en  = hNum->GetBinError(ix, iy);
                double ed  = hDen->GetBinError(ix, iy);
                double err = sqrt((en * en) / (den * den) +
                                  (num * num) * (ed * ed) / (den * den * den * den));
                hEff->SetBinContent(ix, iy, eff);
                hEff->SetBinError(ix, iy, err);
            }
            // else: bin stays at 0 — treated as invalid (grey, no label) downstream
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
/* SF: explicit bin-by-bin ratio with protection against 0/0        */
/* FIX 4: Replace TH2D::Divide (which can produce unexpected        */
/* results for 0/0 bins) with an explicit loop that leaves          */
/* invalid bins at exactly 0.                                       */
/********************************************************************/
TH2D* makeSF(TH2D* d, TH2D* m, const string& name)
{
    TH2D* sf = (TH2D*)d->Clone(name.c_str());
    sf->Reset();
    for (int ix = 1; ix <= sf->GetNbinsX(); ++ix) {
        for (int iy = 1; iy <= sf->GetNbinsY(); ++iy) {
            double num = d->GetBinContent(ix, iy);
            double den = m->GetBinContent(ix, iy);
            if (num > 0 && den > 0) {
                double ratio = num / den;
                // Simple error propagation: (sf)^2 * [(err_d/d)^2 + (err_m/m)^2]
                double ed  = d->GetBinError(ix, iy);
                double em  = m->GetBinError(ix, iy);
                double err = ratio * sqrt((ed / num) * (ed / num) + (em / den) * (em / den));
                sf->SetBinContent(ix, iy, ratio);
                sf->SetBinError(ix, iy, err);
            }
            // else: leave at 0 → treated as invalid (grey, no label) in plots
        }
    }
    sf->SetDirectory(0);
    return sf;
}

/********************************************************************/
/* Main */
/********************************************************************/
int main()
{
    // FIX 5: Start bins at 20 GeV — removes the unphysical 0-20 GeV bin.
    vector<double> bins = {20, 30, 50, 100, 150, 200, 350};

    // Event weight branch name used throughout all efficiency calculations.
    // Every call to calculate_efficiency_2d will print to stdout which
    // branch it found (or warn if it falls back to unweighted).
    string weight_branch = "evWeight";

    cout << "========================================" << endl;
    cout << "EVENT WEIGHTING BRANCH: '" << weight_branch << "'" << endl;
    cout << "========================================\n" << endl;

    // ================= NOMINAL =================
    cout << "--- Calculating NOMINAL efficiencies ---" << endl;
    auto data_nom = calculate_efficiency_2d(
        "Data_0.root", "Data_1.root", "outputTree",
        "goodElectrons_leading_pt", "goodmuons_leading_pt",
        bins, bins, weight_branch);
    auto mc_nom = calculate_efficiency_2d(
        "TTbar_0.root", "TTbar_1.root", "outputTree",
        "goodElectrons_leading_pt", "goodmuons_leading_pt",
        bins, bins, weight_branch);

    TH2D* sf_nom = makeSF(data_nom.eff, mc_nom.eff, "scale_factor");

    // ================= STAT =================
    cout << "\n========================================" << endl;
    cout << "CALCULATING STATISTICAL UNCERTAINTIES (KATZ METHOD)" << endl;
    cout << "========================================" << endl;

    TH2D* sf_stat = (TH2D*)sf_nom->Clone("scale_factor_stat");
    sf_stat->Reset();

    for (int ix = 1; ix <= sf_nom->GetNbinsX(); ++ix) {
        for (int iy = 1; iy <= sf_nom->GetNbinsY(); ++iy) {

            cout << "\n--- Bin (" << ix << "," << iy << ") ---" << endl;

            double T = sf_nom->GetBinContent(ix, iy);
            if (T <= 0) {
                cout << "  SKIPPED: SF <= 0" << endl;
                continue;
            }

            // ---- DATA ----
            double xw  = data_nom.num->GetBinContent(ix, iy);
            double xw2 = pow(data_nom.num->GetBinError(ix, iy), 2);

            double mw  = data_nom.den->GetBinContent(ix, iy);
            double mw2 = pow(data_nom.den->GetBinError(ix, iy), 2);

            // Effective counts
            double x = (xw2 > 0) ? (xw * xw / xw2) : 0;
            double m = (mw2 > 0) ? (mw * mw / mw2) : 0;

            // ---- MC ----
            double yw  = mc_nom.num->GetBinContent(ix, iy);
            double yw2 = pow(mc_nom.num->GetBinError(ix, iy), 2);

            double nw  = mc_nom.den->GetBinContent(ix, iy);
            double nw2 = pow(mc_nom.den->GetBinError(ix, iy), 2);

            double y = (yw2 > 0) ? (yw * yw / yw2) : 0;
            double n = (nw2 > 0) ? (nw * nw / nw2) : 0;

            cout << "  Data: x = " << x << ", m = " << m << endl;
            cout << "  MC:   y = " << y << ", n = " << n << endl;

            // ---- Sanity check ----
            if (x < 1 || m < 1 || y < 1 || n < 1) {
                cout << "  SKIPPED: insufficient statistics" << endl;
                continue;
            }

            // ---- Extreme case correction ----
            if (fabs(x - m) < 1e-6) {
                cout << "  Data extreme case detected (x=m), applying -0.5 correction" << endl;
                x = m - 0.5;
            }

            if (fabs(y - n) < 1e-6) {
                cout << "  MC extreme case detected (y=n), applying -0.5 correction" << endl;
                y = n - 0.5;
            }

            // ---- Katz variance ----
            double var = (1.0 / x - 1.0 / m) + (1.0 / y - 1.0 / n);

            if (var < 0) {
                cout << "  WARNING: negative variance, forcing to 0" << endl;
                var = 0;
            }

            double stat_unc = T * sqrt(var);

            cout << "  SF (T) = " << T << endl;
            cout << "  Variance = " << var << endl;
            cout << "  stat_unc = " << stat_unc << endl;

            sf_stat->SetBinContent(ix, iy, stat_unc);
        }
    }

    cout << "\n========================================" << endl;
    cout << "STATISTICAL UNCERTAINTY CALCULATION COMPLETE" << endl;
    cout << "========================================\n" << endl;
    // ================= SYSTEMATICS =================
    cout << "--- Calculating SYSTEMATIC efficiencies (ENVELOPE METHOD) ---" << endl;

    // ---- Variations ----
    auto data_puLT35 = calculate_efficiency_2d("Data_2.root",  "Data_3.root",  "outputTree",
            "goodElectrons_leading_pt", "goodmuons_leading_pt", bins, bins, weight_branch);
    auto mc_puLT35   = calculate_efficiency_2d("TTbar_2.root", "TTbar_3.root", "outputTree",
            "goodElectrons_leading_pt", "goodmuons_leading_pt", bins, bins, weight_branch);

    auto data_puUD   = calculate_efficiency_2d("Data_4.root",  "Data_5.root",  "outputTree",
            "goodElectrons_leading_pt", "goodmuons_leading_pt", bins, bins, weight_branch);
    auto mc_puUD     = calculate_efficiency_2d("TTbar_4.root", "TTbar_5.root", "outputTree",
            "goodElectrons_leading_pt", "goodmuons_leading_pt", bins, bins, weight_branch);

    auto data_ngjGE3 = calculate_efficiency_2d("Data_6.root",  "Data_7.root",  "outputTree",
            "goodElectrons_leading_pt", "goodmuons_leading_pt", bins, bins, weight_branch);
    auto mc_ngjGE3   = calculate_efficiency_2d("TTbar_6.root", "TTbar_7.root", "outputTree",
            "goodElectrons_leading_pt", "goodmuons_leading_pt", bins, bins, weight_branch);

    auto data_ngjLT3 = calculate_efficiency_2d("Data_8.root",  "Data_9.root",  "outputTree",
            "goodElectrons_leading_pt", "goodmuons_leading_pt", bins, bins, weight_branch);
    auto mc_ngjLT3   = calculate_efficiency_2d("TTbar_8.root", "TTbar_9.root", "outputTree",
            "goodElectrons_leading_pt", "goodmuons_leading_pt", bins, bins, weight_branch);

    // ---- SFs for variations ----
    TH2D* sf_puLT35 = makeSF(data_puLT35.eff, mc_puLT35.eff, "sf_puLT35");
    TH2D* sf_puUD   = makeSF(data_puUD.eff,   mc_puUD.eff,   "sf_puUD");

    TH2D* sf_ngjGE3 = makeSF(data_ngjGE3.eff, mc_ngjGE3.eff, "sf_ngjGE3");
    TH2D* sf_ngjLT3 = makeSF(data_ngjLT3.eff, mc_ngjLT3.eff, "sf_ngjLT3");

    // ---- Final systematic histogram ----
    TH2D* sf_syst = (TH2D*)sf_nom->Clone("scale_factor_syst");
    sf_syst->Reset();

    // ---- Loop over bins ----
    for (int ix = 1; ix <= sf_nom->GetNbinsX(); ++ix) {
        for (int iy = 1; iy <= sf_nom->GetNbinsY(); ++iy) {

            double sf0 = sf_nom->GetBinContent(ix, iy);
            if (sf0 <= 0) continue;

            // =========================
            // PU SYSTEMATIC (ENVELOPE)
            // =========================
            double pu1 = sf_puLT35->GetBinContent(ix, iy);
            double pu2 = sf_puUD->GetBinContent(ix, iy);

            double pu_unc = 0.0;
            if (pu1 > 0 || pu2 > 0) {
                double d1 = (pu1 > 0) ? fabs(pu1 - sf0) : 0.0;
                double d2 = (pu2 > 0) ? fabs(pu2 - sf0) : 0.0;
                pu_unc = max(d1, d2);
            }

            // =========================
            // NJETS SYSTEMATIC (ENVELOPE)
            // =========================
            double nj1 = sf_ngjGE3->GetBinContent(ix, iy);
            double nj2 = sf_ngjLT3->GetBinContent(ix, iy);

            double nj_unc = 0.0;
            if (nj1 > 0 || nj2 > 0) {
                double d1 = (nj1 > 0) ? fabs(nj1 - sf0) : 0.0;
                double d2 = (nj2 > 0) ? fabs(nj2 - sf0) : 0.0;
                nj_unc = max(d1, d2);
            }

            // =========================
            // TOTAL SYSTEMATIC (QUADRATURE)
            // =========================
            double total_syst = sqrt(pu_unc * pu_unc + nj_unc * nj_unc);

            sf_syst->SetBinContent(ix, iy, total_syst);

            // ---- Debug print (optional but useful) ----
            cout << "\n--- Bin (" << ix << "," << iy << ") ---" << endl;
            cout << "Nominal SF = " << sf0 << endl;
            cout << "PU unc     = " << pu_unc << endl;
            cout << "Njets unc  = " << nj_unc << endl;
            cout << "Total syst = " << total_syst << endl;
        }
    }
    TH2D* sf_total = (TH2D*)sf_nom->Clone("scale_factor_total");
    sf_total->Reset();
    for (int ix = 1; ix <= sf_nom->GetNbinsX(); ++ix)
        for (int iy = 1; iy <= sf_nom->GetNbinsY(); ++iy)
            sf_total->SetBinContent(ix, iy,
                sqrt(pow(sf_stat->GetBinContent(ix, iy), 2) +
                     pow(sf_syst->GetBinContent(ix, iy), 2)));

    // ================= OUTPUT (ROOT) =================
    TFile* fout = new TFile("trigger_scale_factors.root", "RECREATE");
    sf_nom->Write();
    sf_stat->Write();
    sf_syst->Write();
    sf_total->Write();
    fout->Close();
    cout << "\nSUCCESS: trigger_scale_factors.root written\n";

    // ================= OUTPUT (PNG) =================
    saveHistogramPNG(sf_nom,    "sf_nominal.png",
                     "Trigger Scale Factor (Nominal)",
                     -1, -1, kBird);

    saveHistogramPNG(sf_puLT35, "sf_puLT35.png",
                     "SF Systematic: PU < 35",
                     -1, -1, kBird);
    saveHistogramPNG(sf_puUD,   "sf_puUD.png",
                     "SF Systematic: PU >= 35",
                     -1, -1, kBird);
    saveHistogramPNG(sf_ngjGE3, "sf_ngjGE3.png",
                     "SF Systematic: N_{gj} #geq 3",
                     -1, -1, kBird);
    saveHistogramPNG(sf_ngjLT3, "sf_ngjLT3.png",
                     "SF Systematic: N_{gj} < 3",
                     -1, -1, kBird);

    saveHistogramPNG(sf_stat,   "sf_stat_unc.png",
                     "Trigger SF Statistical Uncertainty",
                     -1, -1, kRainBow);
    saveHistogramPNG(sf_syst,   "sf_syst_unc.png",
                     "Trigger SF Systematic Uncertainty",
                     -1, -1, kRainBow);
    saveHistogramPNG(sf_total,  "sf_total_unc.png",
                     "Trigger SF Total Uncertainty",
                     -1, -1, kRainBow);

    cout << "SUCCESS: PNG histograms written.\n";
    return 0;
}

