#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <stdexcept>

#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include "TEfficiency.h"

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
/* Efficiency calculation (weighted OR unweighted)                  */
/********************************************************************/
EffResult calculate_efficiency_2d(
    const string& file_num,
    const string& file_den,
    const string& tree_name,
    const string& var_x,
    const string& var_y,
    const vector<double>& bins_x,
    const vector<double>& bins_y,
    const string& weight_branch   // <-- empty = unweighted
) {
    cout << "\n[INFO] Processing:\n"
         << "  Num: " << file_num << "\n"
         << "  Den: " << file_den << "\n"
         << "  Weight branch: "
         << (weight_branch.empty() ? "UNWEIGHTED" : weight_branch) << endl;

    TFile* fNum = TFile::Open(file_num.c_str());
    TFile* fDen = TFile::Open(file_den.c_str());

    if (!fNum || fNum->IsZombie()) throw runtime_error("Cannot open " + file_num);
    if (!fDen || fDen->IsZombie()) throw runtime_error("Cannot open " + file_den);

    TTree* tNum = (TTree*)fNum->Get(tree_name.c_str());
    TTree* tDen = (TTree*)fDen->Get(tree_name.c_str());
    if (!tNum || !tDen) throw runtime_error("Tree not found");

    cout << "  Entries num/den = "
         << tNum->GetEntries() << " / "
         << tDen->GetEntries() << endl;

    TH2D* hNum = new TH2D(
        ("num_" + file_num).c_str(), "",
        bins_x.size()-1, bins_x.data(),
        bins_y.size()-1, bins_y.data()
    );
    TH2D* hDen = new TH2D(
        ("den_" + file_den).c_str(), "",
        bins_x.size()-1, bins_x.data(),
        bins_y.size()-1, bins_y.data()
    );
    hNum->Sumw2();
    hDen->Sumw2();

    Double_t x=0,y=0,w=1.0;

    // ---------------- NUMERATOR ----------------
    tNum->SetBranchAddress(var_x.c_str(), &x);
    tNum->SetBranchAddress(var_y.c_str(), &y);

    bool useWeightNum = false;
    if (!weight_branch.empty() && tNum->GetBranch(weight_branch.c_str())) {
        tNum->SetBranchAddress(weight_branch.c_str(), &w);
        useWeightNum = true;
    }

    for (Long64_t i=0;i<tNum->GetEntries();++i) {
        tNum->GetEntry(i);
        double weight = useWeightNum ? w : 1.0;
        if (x>0 && y>0) hNum->Fill(x,y,weight);
    }

    // ---------------- DENOMINATOR ----------------
    tDen->SetBranchAddress(var_x.c_str(), &x);
    tDen->SetBranchAddress(var_y.c_str(), &y);

    bool useWeightDen = false;
    if (!weight_branch.empty() && tDen->GetBranch(weight_branch.c_str())) {
        tDen->SetBranchAddress(weight_branch.c_str(), &w);
        useWeightDen = true;
    }

    for (Long64_t i=0;i<tDen->GetEntries();++i) {
        tDen->GetEntry(i);
        double weight = useWeightDen ? w : 1.0;
        if (x>0 && y>0) hDen->Fill(x,y,weight);
    }

    cout << "  Integral num/den = "
         << hNum->Integral() << " / "
         << hDen->Integral() << endl;

    // ---------------- EFFICIENCY ----------------
    TH2D* hEff = new TH2D(
        ("eff_" + file_num).c_str(), "",
        bins_x.size()-1, bins_x.data(),
        bins_y.size()-1, bins_y.data()
    );

    for (int ix=1;ix<=hEff->GetNbinsX();++ix)
        for (int iy=1;iy<=hEff->GetNbinsY();++iy) {
            double num = hNum->GetBinContent(ix,iy);
            double den = hDen->GetBinContent(ix,iy);
            if (den>0) {
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

    // =================== SWITCH HERE ===================
    string weight_branch = "evWeight";        // unweighted (debug)
    // string weight_branch = "evWeight"; // weighted (final)
    // ===================================================

    cout << "\n========== NOMINAL ==========\n";
    auto data_nom = calculate_efficiency_2d(
        "Data_0.root","Data_1.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",
        bins,bins,weight_branch
    );

    auto mc_nom = calculate_efficiency_2d(
        "TTbar_0.root","TTbar_1.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",
        bins,bins,weight_branch
    );

    TH2D* sf_nom = makeSF(data_nom.eff, mc_nom.eff, "scale_factor");

    // =================== STAT (KATZ) ===================
    TH2D* sf_stat = (TH2D*)sf_nom->Clone("scale_factor_stat");
    sf_stat->Reset();

    for(int ix=1;ix<=sf_nom->GetNbinsX();++ix)
        for(int iy=1;iy<=sf_nom->GetNbinsY();++iy){

            double eps_d = data_nom.eff->GetBinContent(ix,iy);
            double eps_m = mc_nom.eff->GetBinContent(ix,iy);
            if(eps_d<=0||eps_m<=0) continue;

            double Xw=data_nom.num->GetBinContent(ix,iy);
            double Xw2=pow(data_nom.num->GetBinError(ix,iy),2);
            double Yw=mc_nom.num->GetBinContent(ix,iy);
            double Yw2=pow(mc_nom.num->GetBinError(ix,iy),2);

            double Xeff=(Xw2>0)?Xw*Xw/Xw2:0;
            double Yeff=(Yw2>0)?Yw*Yw/Yw2:0;

            if(Xeff < 1.0 || Yeff < 1.0){
                sf_stat->SetBinContent(ix,iy,0.0);
                continue;
            }

            double T = sf_nom->GetBinContent(ix,iy);
            double varlnT = (1-eps_d)/Xeff + (1-eps_m)/Yeff;
            sf_stat->SetBinContent(ix,iy,T*sqrt(varlnT));
        }

    // =================== SYSTEMATICS (unchanged) ===================
    cout << "\n========== SYSTEMATICS ==========\n";

    auto data_puLT35 = calculate_efficiency_2d("Data_2.root","Data_3.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",bins,bins,weight_branch);

    auto data_puUD   = calculate_efficiency_2d("Data_4.root","Data_5.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",bins,bins,weight_branch);

    auto data_ngjGE3 = calculate_efficiency_2d("Data_6.root","Data_7.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",bins,bins,weight_branch);

    auto data_ngjLT3 = calculate_efficiency_2d("Data_8.root","Data_9.root","outputTree",
        "goodElectrons_leading_pt","goodmuons_leading_pt",bins,bins,weight_branch);

    TH2D* sf_puLT35 = makeSF(data_puLT35.eff, mc_nom.eff, "sf_puLT35");
    TH2D* sf_puUD   = makeSF(data_puUD.eff,   mc_nom.eff, "sf_puUD");
    TH2D* sf_ngjGE3 = makeSF(data_ngjGE3.eff, mc_nom.eff, "sf_ngjGE3");
    TH2D* sf_ngjLT3 = makeSF(data_ngjLT3.eff, mc_nom.eff, "sf_ngjLT3");

    TH2D* sf_syst = (TH2D*)sf_nom->Clone("scale_factor_syst");
    sf_syst->Reset();

    for(int ix=1;ix<=sf_nom->GetNbinsX();++ix)
        for(int iy=1;iy<=sf_nom->GetNbinsY();++iy){
            double sf0=sf_nom->GetBinContent(ix,iy);
            if(sf0<=0) continue;
            double sum2=0;
            auto add=[&](TH2D* h){
                double v=h->GetBinContent(ix,iy);
                if(v>0) sum2+=pow(v-sf0,2);
            };
            add(sf_puLT35); add(sf_puUD); add(sf_ngjGE3); add(sf_ngjLT3);
            sf_syst->SetBinContent(ix,iy,sqrt(sum2));
        }

    TH2D* sf_total = (TH2D*)sf_nom->Clone("scale_factor_total");
    sf_total->Reset();
    for(int ix=1;ix<=sf_nom->GetNbinsX();++ix)
        for(int iy=1;iy<=sf_nom->GetNbinsY();++iy)
            sf_total->SetBinContent(ix,iy,
                sqrt(pow(sf_stat->GetBinContent(ix,iy),2)+
                     pow(sf_syst->GetBinContent(ix,iy),2)));

    // =================== OUTPUT ===================
    TFile* fout = new TFile("trigger_scale_factors.root","RECREATE");

    data_nom.eff->Write("eff_data_nom");
    mc_nom.eff->Write("eff_mc_nom");

    data_nom.num->Write("num_data_nom");
    data_nom.den->Write("den_data_nom");
    mc_nom.num->Write("num_mc_nom");
    mc_nom.den->Write("den_mc_nom");

    sf_nom->Write();
    sf_stat->Write();
    sf_syst->Write();
    sf_total->Write();

    sf_puLT35->Write();
    sf_puUD->Write();
    sf_ngjGE3->Write();
    sf_ngjLT3->Write();

    fout->Close();
    cout << "\n✅ SUCCESS: trigger_scale_factors.root written\n";
    return 0;
}

