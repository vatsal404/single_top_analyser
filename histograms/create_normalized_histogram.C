#include <iostream>
#include <string>
#include <map>
#include <tuple>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TH1F.h>
#include <TLeaf.h>
#include <vector>

void create_normalized_histogram(const std::string& filename, 
	
                       double cross_section, 
                       double sum_gen_weight, 
                       double luminosity=41480.0 ,

		       const std::string& Tree="outputTree") {
    // ROOT file and tree details
    std::string inputFileName = filename;
    std::string treeName = Tree;
    
    // Open the input ROOT file
    TFile* inputFile = TFile::Open(inputFileName.c_str(), "READ");
    if (!inputFile || inputFile->IsZombie()) {
        std::cerr << "Error: Unable to open the ROOT file: " << inputFileName << std::endl;
        return;
    }
    
    // Get the tree
    TTree* tree = static_cast<TTree*>(inputFile->Get(treeName.c_str()));
    if (!tree) {
        std::cerr << "Error: Unable to find the tree: " << treeName << std::endl;
        inputFile->Close();
        return;
    }
    std::cout << "Tree is found" << std::endl;
    
    // Branch ranges definition
    std::map<std::string, std::tuple<double, double, int>> branch_ranges = {
        {"muon_isolation", {0, 0.06, 10}},
        {"ele_isolation", {0, 0.06, 10}},
   /*     {"goodElectrons_leading_pt", {0, 400, 30}},
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
        {"bjet_mass", {0, 200, 50}},*/
        {"Ngood_bjets", {0, 10, 10}},
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
        {"Wboson_transversMass", {0, 200,35}},
        {"Wboson_transversemass_ele", {0, 200, 35}},
        {"Wboson_transversemass_ele_barrel", {0, 200, 35}},
        {"Wboson_transversemass_ele_endcap", {0, 200, 35}},

        {"Wboson_transversemass_QCDele_barrel", {0, 200, 35}},
        {"Wboson_transversemass_QCDele_endcap", {0, 200, 35}},

	{"Wboson_transversemass_muon", {0, 200, 35}},
        {"Wboson_transversemass_QCDmuon", {0, 200, 35}},                
//	{"Pt_2j0t_muon", {0,200, 35}},

//	{"delphi_2j0t_muon", {0,200, 35}},
  //      {"eta_2j0t_muon", {0,200, 35}},*/

  
    };
   
    // Calculate normalization factor
    double normalization_factor = luminosity * cross_section / sum_gen_weight;
    std::cout << "Normalization Factor: " << normalization_factor << std::endl;
   
    // Event weight
    Float_t evWeight = 1.0;
    Float_t genWeight = 1.0;
    if (tree->GetBranch("evWeight")) {
        tree->SetBranchAddress("evWeight", &evWeight);
    } else {
        std::cerr << "Warning: evWeight branch not found. Using default weight of 1.0" << std::endl;
    }
    
    // Check for genWeight branch
    if (tree->GetBranch("genWeight")) {
        tree->SetBranchAddress("genWeight", &genWeight);
    } else {
        std::cerr << "Warning: genWeight branch not found. Using default weight of 1.0" << std::endl;
    }
    
    // Output filename
//    std::string outputFileName = filename;
    size_t lastSlash = filename.find_last_of("/\\");
    std::string outputFileName = (lastSlash != std::string::npos) ? filename.substr(lastSlash + 1) : filename;

    size_t rootPos = outputFileName.rfind(".root");
    if (rootPos != std::string::npos) {
        outputFileName.replace(rootPos, 5, "_hist.root");
    } else {
        std::cerr << "Error: Input file does not have .root extension." << std::endl;
        return;
    }
    
    // Create output ROOT file
    TFile* outputFile = TFile::Open(outputFileName.c_str(), "RECREATE");
    if (!outputFile || outputFile->IsZombie()) {
        std::cerr << "Error: Unable to create output ROOT file." << std::endl;
        inputFile->Close();
        return;
    }
    std::cout << "Output ROOT file created" << std::endl;
    
    // Buffers for different data types
    Float_t float_buffer = 0.0;
    Int_t int_buffer = 0;
    Double_t double_buffer = 0.0;
    std::vector<float>* vector_float_buffer = nullptr;
    
    // Loop over the branches and create histograms
    int iterationCount = 0;
    for (const auto& [branchName, range] : branch_ranges) {
        double minRange = std::get<0>(range);
        double maxRange = std::get<1>(range);
        int bins = std::get<2>(range);
        
        // Create histogram
        TH1D* hist = new TH1D(branchName.c_str(), branchName.c_str(), bins, minRange, maxRange);
        hist->Sumw2(); // Enable proper error calculation
        
        // Check if the branch exists in the tree
        TBranch* branch = tree->GetBranch(branchName.c_str());
        if (!branch) {
            std::cerr << "Warning: Branch " << branchName << " not found in the tree." << std::endl;
            delete hist;
            continue;
        }
        
        // Get branch type
        TLeaf* leaf = branch->GetLeaf(branchName.c_str());
        if (!leaf) {
            std::cerr << "Warning: Leaf not found for branch: " << branchName << std::endl;
            delete hist;
            continue;
        }
        
        const char* typeName = leaf->GetTypeName();
        std::string typeNameStr(typeName);
        bool isVector = typeNameStr.find("vector") != std::string::npos;
        
        try {
            // Set the appropriate branch address based on type
            if (strcmp(typeName, "Float_t") == 0) {
                tree->SetBranchAddress(branchName.c_str(), &float_buffer);
            } else if (strcmp(typeName, "Int_t") == 0) {
                tree->SetBranchAddress(branchName.c_str(), &int_buffer);
            } else if (strcmp(typeName, "Double_t") == 0) {
                tree->SetBranchAddress(branchName.c_str(), &double_buffer);
            } else if (isVector && typeNameStr.find("float") != std::string::npos) {
                tree->SetBranchAddress(branchName.c_str(), &vector_float_buffer);
            } else {
                std::cout << "Info: Skipping branch " << branchName << " with type " << typeName << std::endl;
                delete hist;
                continue;
            }
            
            // Fill histogram
            Long64_t nEntries = tree->GetEntries();
            for (Long64_t i = 0; i < nEntries; ++i) {
                tree->GetEntry(i);
                
                // Calculate total weight with normalization
                double total_weight = evWeight * genWeight * normalization_factor;
                
                if (strcmp(typeName, "Float_t") == 0) {
                    hist->Fill(float_buffer, total_weight);
                } else if (strcmp(typeName, "Int_t") == 0) {
                    hist->Fill(static_cast<float>(int_buffer), total_weight);
                } else if (strcmp(typeName, "Double_t") == 0) {
                    hist->Fill(static_cast<float>(double_buffer), total_weight);
                } else if (isVector && vector_float_buffer != nullptr) {
                    // Fill histogram with each element of the vector
                    for (const auto& value : *vector_float_buffer) {
                        hist->Fill(value, total_weight);
                    }
                }
            }
            
            std::cout << "Histogram for " << branchName << " is filled" << std::endl;
            
            // Write histogram to the output file
            outputFile->cd();
            hist->Write();
            
            iterationCount++;
        }
        catch (const std::exception& e) {
            std::cerr << "Error processing branch " << branchName << ": " << e.what() << std::endl;
        }
        
        delete hist; // Cleanup
        tree->ResetBranchAddress(branch);
        
        // Clean up vector buffer if it was used
        if (isVector) {
            vector_float_buffer = nullptr;
        }
    }
    
    // Write and close files
    outputFile->Write();  // Write all histograms
    outputFile->Close();
    delete outputFile;
    
    std::cout << "Histograms created and saved to " << outputFileName << std::endl;
}
