#include <iostream>
#include <map>
#include <tuple>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>

void create_raw_histogram(const char* output_file = "data_hist.root") {

    // Define the dictionary with branch names and corresponding min, max, and bin values inside the function
       std::map<std::string, std::tuple<double, double, int>> branch_ranges = {
/*        {"muon_isolation", {0, 0.06, 10}},
        {"ele_isolation", {0, 0.06, 10}},
        {"goodElectrons_leading_pt", {0, 400, 30}},
        {"goodElectrons_leading_eta", {-2.5, 2.5, 50}},
        {"goodElectrons_leading_phi", {-5, 5, 50}},
        {"goodmuons_leading_pt", {0, 500, 30}},
        {"goodmuons_leading_eta", {-2.5, 2.5, 100}},
        {"goodmuons_leading_phi", {-5, 5, 100}},
        {"good_bjet_leading_pt", {0, 450, 30}},
        {"good_bjet_leading_eta", {-2.5, 2.5, 100}},
        {"good_bjet_leading_phi", {-5, 5, 100}},
        {"goodJets_btagpass_bcflav_pt", {0, 300, 50}},
        {"goodJets_btagpass_bcflav_eta", {-5, 5, 100}},
        {"goodJets_all_bcflav_pt", {0, 300, 50}},
        {"goodJets_all_bcflav_eta", {-5, 5, 100}},
        {"goodJets_btagpass_lflav_pt", {0, 300, 50}},
        {"goodJets_btagpass_lflav_eta", {-5, 5, 100}},
        {"goodJets_all_lflav_pt", {0, 300, 50}},
        {"goodJets_all_lflav_eta", {-5, 5, 100}},
        {"good_bjet_leading_mass", {0, 200, 50}},
        {"rev_iso_el_leading_pt", {0, 400, 30}},
        {"rev_iso_el_leading_eta", {-3.5, 3.5, 100}},
        {"rev_iso_el_leading_phi", {-5, 5, 100}},
        {"rev_iso_mu_leading_pt", {0, 200, 30}},
        {"rev_iso_mu_leading_eta", {-3.5, 3.5, 100}},
        {"rev_iso_mu_leading_phi", {-5, 5, 100}},
        {"bjet_mass", {0, 200, 50}},
        {"Ngood_bjets", {0, 10, 10}},*/
	
        {"bdt_wboson_muon_2j0t", {0, 200,35}},
        {"bdt_wboson_muon_2j1t", {0, 200, 35}},
        {"bdt_wboson_muon_3j2t", {0, 200, 35}},
        {"bdt_wboson_ele_2j0t", {0, 200, 35}},
        {"bdt_wboson_ele_2j1t", {0, 200, 35}},
        {"bdt_wboson_ele_3j2t", {0, 200, 35}},
        {"bdt_top_muon_2j0t", {0, 300,40}},
        {"bdt_top_muon_2j1t", {0, 300, 40}},
        {"bdt_top_muon_3j2t", {0, 300,40}},
        {"bdt_top_ele_2j0t", {0, 300, 40}},
        {"bdt_top_ele_2j1t", {0, 300, 40}},
        {"bdt_top_ele_3j2t", {0, 300, 40}},
        {"bdt_specJet_2j0t_leading_eta_muon", {0, 5, 20}},
        {"bdt_specJet_2j1t_leading_eta_muon", {0, 5, 20}},
        {"bdt_specJet_3j2t_leading_eta_muon", {0, 5, 20}},
        {"bdt_specJet_2j1t_leading_eta_ele", {0, 5, 20}},
        {"bdt_specJet_2j0t_leading_eta_ele", {0, 5, 20}},
        {"bdt_specJet_3j2t_leading_eta_ele", {0, 5, 20}},
        {"bdt_delR_muon_2j1t", {0, 6, 30}},
        {"bdt_delR_muon_2j0t", {0, 6, 30}},
        {"bdt_delR_muon_3j2t", {0,6, 30}},
        {"bdt_delR_ele_2j1t", {0, 6, 30}},
        {"bdt_delR_ele_2j0t", {0, 6, 30}},
        {"bdt_delR_ele_3j2t", {0, 6, 30}},
        {"bdt_deltaEta_muon_2j1t", {0, 4, 25}},
        {"bdt_deltaEta_muon_2j0t", {0, 4, 25}},
        {"bdt_deltaEta_muon_3j2t", {0, 4, 25}},
        {"bdt_deltaEta_ele_2j0t", {0, 4, 25}},
        {"bdt_deltaEta_ele_2j1t", {0, 4, 25}},
        {"bdt_deltaEta_ele_3j2t", {0, 4, 25}},
        {"bdt_eventShape_muon_2j1t", {0, 1, 30}},
        {"bdt_eventShape_muon_2j0t", {0, 1, 30}},
        {"bdt_eventShape_muon_3j2t", {0, 1, 30}},
        {"bdt_eventShape_ele_2j1t", {0, 1, 30}},
        {"bdt_eventShape_ele_2j0t", {0, 1, 30}},
        {"bdt_eventShape_ele_3j2t", {0, 1, 30}},
        {"bdt_WHelicity_muon_2j1t", {-1, 1, 30}},
        {"bdt_WHelicity_muon_2j0t", {-1, 1, 30}},
        {"bdt_WHelicity_muon_3j2t", {-1, 1,30}},
        {"bdt_WHelicity_ele_2j1t", {-1, 1, 30}},
        {"bdt_WHelicity_ele_2j0t", {-1, 1, 30}},
        {"bdt_WHelicity_ele_3j2t", {-1, 1, 30}},

        {"bdt_MET_pt_muon_2j1t", {0, 200, 30}},
        {"bdt_MET_pt_muon_2j0t", {0, 200, 30}},
        {"bdt_MET_pt_muon_3j2t", {0, 200, 30}},
        {"bdt_MET_pt_ele_2j1t", {0, 200, 30}},
        {"bdt_MET_pt_ele_2j0t", {0, 200, 30}},
        {"bdt_MET_pt_ele_3j2t", {0, 200, 30}},

        {"ncleanbjetspass",{0,10,10}},
        {"ncleanjetspass",{0,10,10}},
        {"nJet",{0,15,15}},

        {"muonChannel",{0,2,2}},
        {"electronChannel",{0,2,2}},
        {"QCDmuonChannel",{0,2,2}},
        {"QCDelectronChannel",{0,2,2}},
        {"region_2j0t",{0,2,2}},
        {"region_2j1t",{0,2,2}},
        {"region_3j2t",{0,2,2}},

	{"Wboson_transversMass", {0, 200,35}},
        {"Wboson_transversemass_ele", {0, 200, 35}},
        {"Wboson_transversemass_ele_barrel", {0, 200, 35}},
        {"Wboson_transversemass_ele_endcap", {0, 200, 35}},

        {"Wboson_transversemass_QCDele_barrel", {0, 200, 35}},
        {"Wboson_transversemass_QCDele_endcap", {0, 200, 35}},

        {"Wboson_transversemass_muon", {0, 200, 35}},
        {"Wboson_transversemass_QCDmuon", {0, 200, 35}},

	       
	       /*{"evWeight", {0, 200, 100}},
        {"top_mass_2j1t_ele", {0, 200, 100}},
        {"top_mass_2j1t_muon", {0, 200, 100}},
        {"Wboson_transversemass_ele_barrel", {0, 600, 40}},
        {"Wboson_transversemass_ele_endcap", {0, 600, 40}},
        {"Wboson_transversemass_QCDele_barrel", {0, 600, 40}},
        {"Wboson_transversemass_QCDele_endcap", {0, 600, 40}},
       // {"ele_isolation_barrel", {0, 200, 100}},
       // {"ele_isolation_endcap", {0, 200, 100}},
        {"MET_pt_2j0t_muon", {0, 350, 50}},
        {"MET_pt_2j0t_ele_barrel", {0, 200, 100}},
        {"MET_pt_2j0t_ele_endcap", {0, 200, 100}},
        {"Pt_2j0t_muon", {0, 300, 30}},
        {"eta_2j0t_muon", {-3, 3, 40}},
        {"eta_2j0t_ele", {-3, 3, 40}},
        {"delphi_2j0t_muon", {-7, 7, 50}},
        {"delphi_2j0t_ele", {-7, 7, 50}},
        {"delphi_2j0t_ele_barrel", {-7, 7, 50}},
        {"delphi_2j0t_ele_endcap", {-7, 7, 50}},

        {"top_mass_2j1t", {0, 500, 40}},
        {"top_mass_2j0t", {0, 500, 40}},
        {"top_mass_3j2t", {0, 500, 40}},
        {"top_mass", {0, 500, 40}},
        {"top_pt", {0, 1000, 50}},
        //{"MET_2j0t_muon", {0, 1500, 50}},
        //{"genWeight", {-10, 10, 100}},
        {"Wboson_transversMass", {0, 200, 35}},
        {"Wboson_transversemass_ele", {0, 200, 35}},
        {"Wboson_transversemass_QCDele", {0, 200, 35}},
        {"Wboson_transversemass_muon", {0, 200, 35}},
        {"Wboson_transversemass_QCDmuon", {0,200, 35}},
     	{"Wboson_transversemass_ele_barrel", {0, 200, 35}},
        {"Wboson_transversemass_ele_endcap", {0, 200, 35}},
        {"Wboson_transversemass_QCDele", {0, 200, 35}},
        {"Wboson_transversemass_QCDele_barrel", {0, 200, 35}},
        {"Wboson_transversemass_QCDele_endcap", {0, 200, 35}},
        {"delphi_2j0t_muon", {0, 200, 35}},
        {"eta_2j0t_muon", {0, 200, 35}},
        {"Pt_2j0t_muon", {0, 200, 35}},*/
	       
       };       
   
   
       TFile* file = TFile::Open("data.root", "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Couldn't open the ROOT file." << std::endl;
        return;
    }

    // Get the tree from the file
    TTree* tree = (TTree*)file->Get("outputTree");
    if (!tree) {
        std::cerr << "Error: Couldn't find tree " << tree << " in the ROOT file." << std::endl;
        return;
    }

    // Create an output ROOT file to store the histograms
    TFile* output = new TFile(output_file, "RECREATE");

    // Loop through each branch and create a histogram
    for (const auto& entry : branch_ranges) {
        const std::string& branch_name = entry.first;
        double min_val = std::get<0>(entry.second);
        double max_val = std::get<1>(entry.second);
        int n_bins = std::get<2>(entry.second);

        // Create a histogram for the branch
        TH1D* hist = new TH1D(branch_name.c_str(), branch_name.c_str(), n_bins, min_val, max_val);

        // Project data from the tree into the histogram
        tree->Project(branch_name.c_str(), branch_name.c_str());

        // Write the histogram to the output file
        hist->Write();
    }

    // Close the output file
    output->Close();

    // Close the input ROOT file
    file->Close();

    std::cout << "Histograms saved to " << output_file << std::endl;
}

int main() {
    // Call the function to create histograms
    create_raw_histogram();
    return 0;
}

