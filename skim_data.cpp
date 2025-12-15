#include <ROOT/RDataFrame.hxx>
#include <TFile.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <tuple>
#include <vector>
#include <string>
#include <memory>
#include <atomic>
#include <cstdio>
#include <filesystem>

using namespace ROOT;
namespace fs = std::filesystem;

// EventKey struct and hash
struct EventKey {
    UInt_t run;
    ULong64_t event;
    UInt_t lumi;

    bool operator==(EventKey const &o) const {
        return run == o.run && event == o.event && lumi == o.lumi;
    }
};
struct EventKeyHash {
    std::size_t operator()(EventKey const &k) const {
        std::size_t h1 = std::hash<UInt_t>()(k.run);
        std::size_t h2 = std::hash<ULong64_t>()(k.event);
        std::size_t h3 = std::hash<UInt_t>()(k.lumi);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

// Load duplicate events from binary file
std::shared_ptr<std::unordered_set<EventKey, EventKeyHash>>
loadDuplicateEvents(const std::string &filename)
{
    auto dupSet = std::make_shared<std::unordered_set<EventKey, EventKeyHash>>();

    std::ifstream fin(filename, std::ios::binary);
    if (!fin) {
        std::cerr << "[WARNING] Cannot open duplicates binary file: "
                  << filename << " (will process without duplicate removal)" << std::endl;
        return dupSet;
    }

    size_t count = 0;
    while (true) {
        ULong64_t run, event, lumi;

        // Attempt to read 3 values
        fin.read(reinterpret_cast<char*>(&run), sizeof(run));
        if (!fin) break;

        fin.read(reinterpret_cast<char*>(&event), sizeof(event));
        if (!fin) break;

        fin.read(reinterpret_cast<char*>(&lumi), sizeof(lumi));
        if (!fin) break;

        // Convert into EventKey type
        EventKey key{
            static_cast<UInt_t>(run),
            static_cast<ULong64_t>(event),
            static_cast<UInt_t>(lumi)
        };

        dupSet->insert(key);
        count++;
    }

    std::cout << "[INFO] Loaded " << count
              << " duplicate keys from binary " << filename << std::endl;

    return dupSet;
}

// Load file list from batch text file
std::vector<std::string> loadBatchFiles(const std::string &batchFile) {
    std::vector<std::string> files;
    std::ifstream fin(batchFile);
    if (!fin) {
        std::cerr << "[ERROR] cannot open batch file: " << batchFile << std::endl;
        return files;
    }
    std::string line;
    while (std::getline(fin, line)) {
        // trim whitespace
        auto start = line.find_first_not_of(" \t\r\n");
        auto end = line.find_last_not_of(" \t\r\n");
        if (start != std::string::npos && end != std::string::npos) {
            line = line.substr(start, end - start + 1);
        }
        // skip empty lines and comments
        if (!line.empty() && line[0] != '#') {
            files.push_back(line);
        }
    }
    std::cout << "[INFO] Loaded " << files.size() << " files from batch: " << batchFile << std::endl;
    return files;
}

// Extract dataset name from batch filename
std::string extractDatasetFromBatch(const std::string &batchFile) {
    // Example: batch_EGamma0_Run2023C-24Jan2024_v4-v1_NANOAOD_001.txt
    // Extract: EGamma0
    std::string basename = batchFile;
    auto lastSlash = basename.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        basename = basename.substr(lastSlash + 1);
    }
    
    // Remove "batch_" prefix if present
    if (basename.substr(0, 6) == "batch_") {
        basename = basename.substr(6);
    }
    
    // Extract first part before underscore (e.g., "EGamma0")
    auto firstUnderscore = basename.find('_');
    if (firstUnderscore != std::string::npos) {
        return basename.substr(0, firstUnderscore);
    }
    
    return basename;
}

// Template function to process dataset with or without duplicate filter
template<typename NodeType>
void applyFiltersAndSnapshot(NodeType node, const std::string &outName) {
    // Apply physics cuts step-by-step
    auto node_cut1 = node.Filter(
        "nElectron+nMuon>=2 && nJet>0 && PV_npvsGood>=1",
        "Cut1"
    );

    auto node_trigger = node_cut1.Filter(
        "HLT_Ele32_WPTight_Gsf || HLT_IsoMu24 || "
        "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ || "
        "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL",
        "TriggerCut"
    );

    auto node_met = node_trigger.Filter(
        "Flag_goodVertices && Flag_globalSuperTightHalo2016Filter && "
        "Flag_EcalDeadCellTriggerPrimitiveFilter && Flag_BadPFMuonFilter && "
        "Flag_BadPFMuonDzFilter && Flag_hfNoisyHitsFilter && Flag_eeBadScFilter && "
        "Flag_ecalBadCalibFilter",
        "METFilters"
    );

    std::cout << "[INFO] Writing snapshot to: " << outName << std::endl;
    node_met.Snapshot("Events", outName);
}

int main(int argc, char **argv) {
    // Check command line arguments
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <batch_file.txt>" << std::endl;
        std::cerr << "Example: " << argv[0] << " batch_EGamma0_Run2023C-24Jan2024_v4-v1_NANOAOD_001.txt" << std::endl;
        std::cerr << "\nThe program will automatically look for duplicates_<DATASET>.bin files" << std::endl;
        return 1;
    }

    std::string batchFile = argv[1];

    // Load files from batch
    auto files = loadBatchFiles(batchFile);
    if (files.empty()) {
        std::cerr << "[ERROR] No files found in batch file: " << batchFile << std::endl;
        return 1;
    }

    // Extract dataset name
    std::string datasetName = extractDatasetFromBatch(batchFile);
    std::cout << "[INFO] Detected dataset: " << datasetName << std::endl;

    // Construct the duplicate file name for this specific dataset
    std::string dupFile = "duplicates_" + datasetName + ".bin";
    
    // Apply duplicate removal for ALL datasets (EGamma0, EGamma1, Muon0, Muon1, MuonEG)
    bool shouldApplyDuplicateFilter =
        (datasetName.find("EGamma0") != std::string::npos) ||
        (datasetName.find("EGamma1") != std::string::npos) ||
        (datasetName.find("Muon0") != std::string::npos) ||
        (datasetName.find("Muon1") != std::string::npos) ||
        (datasetName.find("MuonEG") != std::string::npos);

    std::cout << "\n=============================================\n";
    std::cout << "[INFO] Processing batch: " << batchFile << "\n";
    std::cout << "[INFO] Dataset: " << datasetName << "\n";
    std::cout << "[INFO] Number of files: " << files.size() << "\n";
    std::cout << "[INFO] Duplicate file: " << dupFile << "\n";
    std::cout << "=============================================\n";

    // Load duplicate events for THIS specific dataset
    auto duplicateEvents = loadDuplicateEvents(dupFile);

    // Base dataframe
    ROOT::RDataFrame df("Events", files);

    // Progress counter as shared atomic
    auto processedEvents = std::make_shared<std::atomic<long long>>(0LL);

    // Define progress counter column
    auto df_progress = df.Define("progress_counter", [processedEvents]() {
        long long x = ++(*processedEvents);
        if ((x % 100000) == 0) {
            std::cout << "[Progress] Processed " << x << " events..." << std::endl;
        }
        return 0;
    });

    // Generate output filename from batch filename
    std::string outName = batchFile;
    // Remove directory path
    auto lastSlash = outName.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        outName = outName.substr(lastSlash + 1);
    }
    // Remove .txt extension
    if (outName.size() > 4 && outName.substr(outName.size() - 4) == ".txt") {
        outName = outName.substr(0, outName.size() - 4);
    }
    // Replace "batch_" with "skimmed_"
    if (outName.substr(0, 6) == "batch_") {
        outName = "skimmed_" + outName.substr(6) + ".root";
    } else {
        outName = "skimmed_" + outName + ".root";
    }

    std::shared_ptr<std::unordered_set<EventKey, EventKeyHash>> dupPtr = duplicateEvents;

    // Branch based on whether to apply duplicate filter
    if (shouldApplyDuplicateFilter && !dupPtr->empty()) {
        std::cout << "[INFO] Applying duplicate-event removal for dataset: " << datasetName << std::endl;
        std::cout << "[INFO] Will remove " << dupPtr->size() << " duplicate events\n";
        
        auto node_dup_removed = df_progress.Filter(
            [dupPtr](UInt_t run, ULong64_t event, UInt_t lumi) -> bool {
                EventKey k{run, event, lumi};
                // Return TRUE to KEEP the event (i.e., if it's NOT in the duplicate list)
                return dupPtr->find(k) == dupPtr->end();
            },
            {"run", "event", "luminosityBlock"},
            "DuplicateEventRemoval"
        );
        applyFiltersAndSnapshot(node_dup_removed, outName);
    } else {
        std::cout << "[INFO] Duplicate-event removal NOT applied for dataset: " << datasetName << std::endl;
        applyFiltersAndSnapshot(df_progress, outName);
    }

    std::cout << "[INFO] Finished processing batch: " << batchFile << "\n";
    std::cout << "[INFO] Output written to: " << outName << "\n";
    std::cout << "[INFO] All done.\n";
    return 0;
}
