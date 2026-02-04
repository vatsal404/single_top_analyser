#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"

using namespace std;

/********************************************************************/
/* Struct                                                           */
/********************************************************************/
struct EffResult {
    TH2D* eff;
    TH2D* num;
    TH2D* den;
};

/********************************************************************/
/* Efficiency calculation using BOOLEAN BRANCH                      */
/********************************************************************/
EffResult calculate_efficiency_2d(
    const string& file_num,
    const string& file_den,
    const string& tree_name,
    const string& var_x,
    const string& var_y,
    const vector<double>& bins_x,
    const vector<double>& bins_y,
    const string& boolean_branch  // "eu_channel"
) {
    TFile* fNum = TFile::Open(file_num.c_str());
    TFile* fDen = TFile::Open(file_den.c_str());
    if (!fNum || fNum->IsZombie()) throw runtime_error("Cannot open " + file_num);
    if (!fDen || fDen->IsZombie()) throw runtime_error("Cannot open " + file_den);

    TTree* tNum = (TTree*)fNum->Get(tree_name.c_str());
    TTree* tDen = (TTree*)fDen->Get(tree_name.c_str());
    if (!tNum || !tDen) throw runtime_error("Tree not found");

    TH2D* hNum = new TH2D(("num_"+file_num).c_str(),"",
        bins_x.size()-1,bins_x.data(),
        bins_y.size()-1,bins_y.data());
    TH2D* hDen = new TH2D(("den_"+file_den).c_str(),"",
        bins_x.size()-1,bins_x.data(),
        bins_y.size()-1,bins_y.data());
    hNum->Sumw2();
    hDen->Sumw2();

    Double_t x=0, y=0;
    Bool_t pass_boolean_num = false;
    Bool_t pass_boolean_den = false;

    Long64_t num_total = tNum->GetEntries();
    Long64_t den_total = tDen->GetEntries();
    Long64_t num_used  = 0;
    Long64_t den_used  = 0;
    Long64_t num_true  = 0;
    Long64_t den_true  = 0;

    // ================= NUMERATOR =================
    tNum->SetBranchAddress(var_x.c_str(),&x);
    tNum->SetBranchAddress(var_y.c_str(),&y);
    tNum->SetBranchAddress(boolean_branch.c_str(), &pass_boolean_num);

    for(Long64_t i=0;i<num_total;++i){
        tNum->GetEntry(i);
        if(x>0 && y>0){
            num_used++;
            // Only fill if boolean is TRUE
            if(pass_boolean_num){
                hNum->Fill(x,y);
                num_true++;
            }
        }
    }

    // ================= DENOMINATOR =================
    tDen->SetBranchAddress(var_x.c_str(),&x);
    tDen->SetBranchAddress(var_y.c_str(),&y);
    tDen->SetBranchAddress(boolean_branch.c_str(), &pass_boolean_den);

    for(Long64_t i=0;i<den_total;++i){
        tDen->GetEntry(i);
        if(x>0 && y>0){
            den_used++;
            // Only fill if boolean is TRUE
            if(pass_boolean_den){
                hDen->Fill(x,y);
                den_true++;
            }
        }
    }

    // ================= EVENT-LEVEL DEBUG =================
    cout << "\n========================================" << endl;
    cout << "EFFICIENCY EVENT DEBUG" << endl;
    cout << "========================================" << endl;
    cout << "NUM file: " << file_num << endl;
    cout << "DEN file: " << file_den << endl;
    cout << "Boolean branch: " << boolean_branch << endl;
    cout << "Total NUM entries                = " << num_total << endl;
    cout << "Total DEN entries                = " << den_total << endl;
    cout << "NUM entries used (x>0,y>0)       = " << num_used << endl;
    cout << "DEN entries used (x>0,y>0)       = " << den_used << endl;
    cout << "NUM entries with boolean=TRUE    = " << num_true << endl;
    cout << "DEN entries with boolean=TRUE    = " << den_true << endl;
    cout << "NUM histogram integral           = " << hNum->Integral() << endl;
    cout << "DEN histogram integral           = " << hDen->Integral() << endl;
    cout << "========================================\n" << endl;

    // ================= EFFICIENCY =================
    TH2D* hEff = new TH2D(("eff_"+file_num).c_str(),"",
        bins_x.size()-1,bins_x.data(),
        bins_y.size()-1,bins_y.data());

    for(int ix=1;ix<=hEff->GetNbinsX();++ix)
        for(int iy=1;iy<=hEff->GetNbinsY();++iy){
            double num = hNum->GetBinContent(ix,iy);
            double den = hDen->GetBinContent(ix,iy);
            if(den>0){
                double eff = num/den;
                double err = sqrt(eff*(1-eff)/den);
                hEff->SetBinContent(ix,iy,eff);
                hEff->SetBinError(ix,iy,err);
            }
        }

    // ================= BIN-BY-BIN DEBUG =================
    cout << "========== BIN-BY-BIN DEBUG ==========" << endl;
    for(int ix=1; ix<=hEff->GetNbinsX(); ++ix){
        for(int iy=1; iy<=hEff->GetNbinsY(); ++iy){
            double n = hNum->GetBinContent(ix,iy);
            double d = hDen->GetBinContent(ix,iy);
            cout << "Bin (" << ix << "," << iy << ")  "
                 << "Num=" << n << "  "
                 << "Den=" << d;
            if(d>0)
                cout << "  Eff=" << n/d << endl;
            else
                cout << "  Eff=UNDEFINED" << endl;
        }
    }
    cout << "=====================================\n" << endl;


    cout << "Global check: "
     << "Num total = " << hNum->Integral()
     << " Den total = " << hDen->Integral()
     << endl;


    hNum->SetDirectory(0);
    hDen->SetDirectory(0);
    hEff->SetDirectory(0);
    fNum->Close();
    fDen->Close();

    return {hEff,hNum,hDen};
}

/********************************************************************/
/* SF                                                               */
/********************************************************************/
TH2D* makeSF(TH2D* d, TH2D* m, const string& name){
    TH2D* sf = (TH2D*)d->Clone(name.c_str());
    sf->Divide(d,m);
    sf->SetDirectory(0);
    return sf;
}

/********************************************************************/
/* Main                                                             */
/********************************************************************/
int main(){

    vector<double> bins = {0,20,30,50,100,150,200,350};

    // ================= NOMINAL =================
    auto data_nom = calculate_efficiency_2d(
        "Data_0.root","Data_1.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",
        bins,bins,"eu_channel");

    auto mc_nom = calculate_efficiency_2d(
        "TTbar_0.root","TTbar_1.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",
        bins,bins,"eu_channel");

    TH2D* sf_nom = makeSF(data_nom.eff, mc_nom.eff, "scale_factor");

    // ================= STAT =================
    TH2D* sf_stat = (TH2D*)sf_nom->Clone("scale_factor_stat");
    sf_stat->Reset();

    for(int ix=1;ix<=sf_nom->GetNbinsX();++ix)
        for(int iy=1;iy<=sf_nom->GetNbinsY();++iy){

            double ed = data_nom.eff->GetBinContent(ix,iy);
            double em = mc_nom.eff->GetBinContent(ix,iy);
            if(ed<=0 || em<=0) continue;

            double Xw  = data_nom.num->GetBinContent(ix,iy);
            double Xw2 = pow(data_nom.num->GetBinError(ix,iy),2);
            double Yw  = mc_nom.num->GetBinContent(ix,iy);
            double Yw2 = pow(mc_nom.num->GetBinError(ix,iy),2);

            double Xeff = (Xw2>0)?Xw*Xw/Xw2:0;
            double Yeff = (Yw2>0)?Yw*Yw/Yw2:0;
            if(Xeff<1 || Yeff<1) continue;

            double T = sf_nom->GetBinContent(ix,iy);
            double varlnT = (1-ed)/Xeff + (1-em)/Yeff;
            sf_stat->SetBinContent(ix,iy, T*sqrt(varlnT));
        }

    // ================= OUTPUT =================
    TFile* fout = new TFile("trigger_scale_factors.root","RECREATE");
    sf_nom->Write();
    sf_stat->Write();
    fout->Close();

    cout << "\n✅ SUCCESS: trigger_scale_factors.root written\n";
    return 0;
}
