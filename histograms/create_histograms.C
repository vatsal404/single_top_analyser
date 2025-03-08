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

void create_histograms(const std::string& filename,const std::string& Tree="outputTree") {
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

        std::map<std::string, std::tuple<double, double, int>> branch_ranges = {
        {"muon_isolation", {0, 0.06, 10}},
        {"ele_isolation", {0, 0.06, 10}},
        {"goodElectrons_leading_pt", {0, 500, 50}},
        {"goodElectrons_leading_eta", {-2.5, 2.5, 100}},
        {"goodElectrons_leading_phi", {-5, 5, 100}},
        {"goodmuons_leading_pt", {0, 300, 50}},
        {"goodmuons_leading_eta", {-2.5, 2.5, 100}},
        {"goodmuons_leading_phi", {-5, 5, 100}},
        {"good_bjet_leading_pt", {0, 300, 50}},
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
        {"rev_iso_el_leading_pt", {0, 300, 50}},
        {"rev_iso_el_leading_eta", {-3.5, 3.5, 100}},
        {"rev_iso_el_leading_phi", {-5, 5, 100}},
        {"rev_iso_mu_leading_pt", {0, 200, 50}},
        {"rev_iso_mu_leading_eta", {-3.5, 3.5, 100}},
        {"rev_iso_mu_leading_phi", {-5, 5, 100}},
        {"bjet_mass", {0, 200, 50}},
        {"Ngood_bjets", {0, 10, 10}},
        {"bdt_wboson_muon_2j0t", {0, 1, 100}},
        {"bdt_wboson_muon_2j1t", {0, 1, 100}},
        {"bdt_wboson_muon_3j2t", {0, 1, 100}},
        {"bdt_wboson_ele_2j0t", {0, 1, 100}},
        {"bdt_wboson_ele_2j1t", {0, 1, 100}},
        {"bdt_wboson_ele_3j2t", {0, 1, 100}},
        {"bdt_top_muon_2j0t", {0, 1, 100}},
        {"bdt_top_muon_2j1t", {0, 1, 100}},
        {"bdt_top_muon_3j2t", {0, 1, 100}},
        {"bdt_top_ele_2j0t", {0, 1, 100}},
        {"bdt_top_ele_2j1t", {0, 1, 100}},
        {"bdt_top_ele_3j2t", {0, 1, 100}},
        {"bdt_specJet_2j0t_leading_eta_muon", {-2.5, 2.5, 100}},
        {"bdt_specJet_2j1t_leading_eta_muon", {-2.5, 2.5, 100}},
        {"bdt_specJet_3j2t_leading_eta_muon", {-2.5, 2.5, 100}},
        {"bdt_specJet_2j1t_leading_eta_ele", {-2.5, 2.5, 100}},
        {"bdt_specJet_2j0t_leading_eta_ele", {-2.5, 2.5, 100}},
        {"bdt_specJet_3j2t_leading_eta_ele", {-2.5, 2.5, 100}},
        {"bdt_delR_muon_2j1t", {0, 10, 100}},
        {"bdt_delR_muon_2j0t", {0, 10, 100}},
        {"bdt_delR_muon_3j2t", {0, 10, 100}},
        {"bdt_delR_ele_2j1t", {0, 10, 100}},
        {"bdt_delR_ele_2j0t", {0, 10, 100}},
        {"bdt_delR_ele_3j2t", {0, 10, 100}},
        {"bdt_deltaEta_muon_2j1t", {0, 10, 100}},
        {"bdt_deltaEta_muon_2j0t", {0, 10, 100}},
        {"bdt_deltaEta_muon_3j2t", {0, 10, 100}},
        {"bdt_deltaEta_ele_2j0t", {0, 10, 100}},
        {"bdt_deltaEta_ele_2j1t", {0, 10, 100}},
        {"bdt_deltaEta_ele_3j2t", {0, 10, 100}},
        {"bdt_eventShape_muon_2j1t", {0, 10, 100}},
        {"bdt_eventShape_muon_2j0t", {0, 10, 100}},
        {"bdt_eventShape_muon_3j2t", {0, 10, 100}},
        {"bdt_eventShape_ele_2j1t", {0, 10, 100}},
        {"bdt_eventShape_ele_2j0t", {0, 10, 100}},
        {"bdt_eventShape_ele_3j2t", {0, 10, 100}},
        {"bdt_WHelicity_muon_2j1t", {0, 1, 100}},
        {"bdt_WHelicity_muon_2j0t", {0, 1, 100}},
        {"bdt_WHelicity_muon_3j2t", {0, 1, 100}},
        {"bdt_WHelicity_ele_2j1t", {0, 1, 100}},
        {"bdt_WHelicity_ele_2j0t", {0, 1, 100}},
        {"bdt_WHelicity_ele_3j2t", {0, 1, 100}},
        {"evWeight", {0, 10, 100}},
        {"top_mass_2j1t", {0, 500, 50}},
        {"top_mass_2j0t", {0, 1000, 50}},
        {"top_mass_3j2t", {0, 1000, 50}},
        {"top_mass", {0, 500, 50}},
        {"top_pt", {0, 1000, 50}},
        {"MET_2j0t_muon", {0, 1500, 50}},
        {"genWeight", {-10, 10, 100}},
        {"Wboson_transversMass", {0, 1000, 50}},
        {"Wboson_transversemass_ele", {0, 500, 50}},
        {"Wboson_transversemass_QCDele", {0, 1000, 50}},
        {"Wboson_transversemass_muon", {0, 500, 50}},
        {"Wboson_transversemass_QCDmuon", {0, 1000, 50}}
    };
   Float_t evWeight = 1.0;
    if (tree->GetBranch("evWeight")) {
        tree->SetBranchAddress("evWeight", &evWeight);
    } else {
        std::cerr << "Warning: evWeight branch not found. Using default weight of 1.0" << std::endl;
    }


    std::string outputFileName = filename;

    // Find the position of the last occurrence of ".root"
    size_t rootPos = outputFileName.rfind(".root");
    if (rootPos != std::string::npos) {
        // Replace ".root" with "_hist.root"
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
    std::cout << "output root file created" << std::endl;

    // Buffers for different data types
    Float_t float_buffer = 0.0;
    Int_t int_buffer = 0;
    Double_t double_buffer = 0.0;
    std::vector<float>* vector_float_buffer = nullptr;  // Changed to pointer

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

                if (strcmp(typeName, "Float_t") == 0) {
                    hist->Fill(float_buffer, evWeight);
                } else if (strcmp(typeName, "Int_t") == 0) {
                    hist->Fill(static_cast<float>(int_buffer), evWeight);
                } else if (strcmp(typeName, "Double_t") == 0) {
                    hist->Fill(static_cast<float>(double_buffer), evWeight);
                } else if (isVector && vector_float_buffer != nullptr) {
                    // Fill histogram with each element of the vector
                    for (const auto& value : *vector_float_buffer) {
                        hist->Fill(value, evWeight);
                    }
                }
            }
            std::cout << "Histogram is filled"<< iterationCount <<"times" << std::endl;

            // Write histogram to the output file
            outputFile->cd();
            hist->Write();
        std::cout << "histogram is written" << std::endl;
        iterationCount++;
        } 
	catch (const std::exception& e) {
            std::cerr << "Error processing branch " << branchName << ": " << e.what() << std::endl;
        }

        delete hist; // Cleanup
        tree->ResetBranchAddress(branch);
        std::cout << "the histogram was deleted and branch is reset" << std::endl;

        // Clean up vector buffer if it was used
        if (isVector) {
            vector_float_buffer = nullptr;
        }
    }

// At the end of your loop over branches, add:
	outputFile->Write();  // Write all histograms
	outputFile->Close();
     	std::cout << "output file was closed " << std::endl;

	delete outputFile;
    	std::cout << "output file was deleted" << std::endl;


// Clean up the input file

std::cout << "Histograms created and saved to " << outputFileName << std::endl;
}
