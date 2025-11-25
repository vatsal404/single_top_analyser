#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <algorithm>
#include <map>

// Structure to hold event identifier
struct EventID {
    unsigned long long run;
    unsigned long long event;
    unsigned long long luminosityBlock;
    
    bool operator==(const EventID& other) const {
        return run == other.run && event == other.event && luminosityBlock == other.luminosityBlock;
    }
};

// Hash function for EventID
struct EventIDHash {
    std::size_t operator()(const EventID& id) const {
        return std::hash<unsigned long long>()(id.run) ^ 
               (std::hash<unsigned long long>()(id.event) << 1) ^ 
               (std::hash<unsigned long long>()(id.luminosityBlock) << 2);
    }
};

const size_t CHUNK_SIZE = 2000000; // Process 2M events at a time

// Read a chunk of events from binary file
std::vector<EventID> readChunk(std::ifstream& file, size_t chunkSize) {
    std::vector<EventID> chunk;
    chunk.reserve(chunkSize);
    
    unsigned long long run, event, lumi;
    
    for (size_t i = 0; i < chunkSize; ++i) {
        if (file.read(reinterpret_cast<char*>(&run), sizeof(unsigned long long)) &&
            file.read(reinterpret_cast<char*>(&event), sizeof(unsigned long long)) &&
            file.read(reinterpret_cast<char*>(&lumi), sizeof(unsigned long long))) {
            chunk.push_back({run, event, lumi});
        } else {
            break;
        }
    }
    
    return chunk;
}

// Compare MuonEG dataset with another dataset chunk by chunk
void compareMuonEGWithDataset(const std::string& muonEGFile, 
                               const std::string& otherFile, 
                               int otherIdx,
                               const std::string& otherName,
                               std::unordered_map<EventID, std::vector<int>, EventIDHash>& duplicates) {
    
    std::cout << "\n========================================================\n";
    std::cout << "Comparing MuonEG with " << otherName << " (dataset " << (otherIdx+1) << ")...\n";
    std::cout << "========================================================\n";
    
    std::ifstream ifMuonEG(muonEGFile, std::ios::binary);
    if (!ifMuonEG) {
        std::cerr << "Error: Could not open " << muonEGFile << std::endl;
        return;
    }
    
    size_t chunkNum = 0;
    size_t totalMatches = 0;
    
    // Read chunks from MuonEG file
    while (true) {
        auto muonEGChunk = readChunk(ifMuonEG, CHUNK_SIZE);
        if (muonEGChunk.empty()) break;
        
        chunkNum++;
        std::cout << "\n  Processing MuonEG chunk " << chunkNum 
                  << " (" << muonEGChunk.size() << " events)..." << std::endl;
        
        // Build hash set for quick lookup
        std::unordered_set<EventID, EventIDHash> muonEGSet(muonEGChunk.begin(), muonEGChunk.end());
        
        // Compare with chunks from other dataset
        std::ifstream ifOther(otherFile, std::ios::binary);
        if (!ifOther) {
            std::cerr << "Error: Could not open " << otherFile << std::endl;
            break;
        }
        
        size_t otherChunkNum = 0;
        size_t chunkMatches = 0;
        
        while (true) {
            auto otherChunk = readChunk(ifOther, CHUNK_SIZE);
            if (otherChunk.empty()) break;
            
            otherChunkNum++;
            
            // Check for matches
            for (const auto& event : otherChunk) {
                if (muonEGSet.count(event)) {
                    auto& datasets = duplicates[event];
                    
                    // Add dataset indices if not already present (0 = MuonEG)
                    if (std::find(datasets.begin(), datasets.end(), 0) == datasets.end()) {
                        datasets.push_back(0);
                    }
                    if (std::find(datasets.begin(), datasets.end(), otherIdx) == datasets.end()) {
                        datasets.push_back(otherIdx);
                    }
                    chunkMatches++;
                }
            }
            
            if (otherChunkNum % 10 == 0) {
                std::cout << "    Compared with " << otherChunkNum << " chunks from " 
                          << otherName << ", found " << chunkMatches << " matches" << std::endl;
            }
        }
        
        ifOther.close();
        totalMatches += chunkMatches;
        std::cout << "  MuonEG chunk " << chunkNum << " complete: " << chunkMatches << " new matches" << std::endl;
    }
    
    ifMuonEG.close();
    std::cout << "\nComparison with " << otherName << " complete: " 
              << totalMatches << " total duplicate events found" << std::endl;
}

int main() {
    std::cout << "=== MuonEG Duplicate Event Detector ===\n";
    std::cout << "Chunk size: " << CHUNK_SIZE << " events\n\n";
    
    // Define your dataset names for reference
    std::vector<std::string> datasetNames = {
        "EGamma0",          // temp_dataset_0.bin
        "EGamma1",          // temp_dataset_1.bin
        "Muon0",            // temp_dataset_2.bin
        "Muon1",            // temp_dataset_3.bin
        "MuonEG"            // temp_dataset_4.bin
    };
    
    std::vector<std::string> tempFiles = {
        "temp_dataset_0.bin",
        "temp_dataset_1.bin",
        "temp_dataset_2.bin",
        "temp_dataset_3.bin",
        "temp_dataset_4.bin"
    };
    
    // Check if all files exist
    std::cout << "Checking temporary files..." << std::endl;
    for (size_t i = 0; i < tempFiles.size(); ++i) {
        std::ifstream test(tempFiles[i], std::ios::binary);
        if (test) {
            test.seekg(0, std::ios::end);
            size_t fileSize = test.tellg();
            size_t numEvents = fileSize / (3 * sizeof(unsigned long long));
            std::cout << "  [" << (i+1) << "] " << tempFiles[i] << " - " 
                      << datasetNames[i] << " (" << numEvents << " events)" << std::endl;
            test.close();
        } else {
            std::cerr << "  [" << (i+1) << "] " << tempFiles[i] << " - NOT FOUND!" << std::endl;
        }
    }
    
    std::cout << "\n========================================================\n";
    std::cout << "Starting comparison: MuonEG vs all other datasets\n";
    std::cout << "========================================================\n";
    
    // Store all duplicates
    std::unordered_map<EventID, std::vector<int>, EventIDHash> duplicates;
    
    // Compare MuonEG (dataset 4) with all others
    for (size_t i = 0; i < 4; ++i) {  // Compare with datasets 0-3
        compareMuonEGWithDataset(tempFiles[4], tempFiles[i], i, datasetNames[i], duplicates);
    }
    
    // Write results
    std::cout << "\n========================================================\n";
    std::cout << "Writing results...\n";
    std::cout << "========================================================\n\n";
    
    std::ofstream outFile("muoneg_duplicates.txt");
    outFile << "# MuonEG Duplicate Events Report\n";
    outFile << "# Events from MuonEG appearing in other datasets\n";
    outFile << "# Format: Run  Event  LuminosityBlock  Count  Datasets\n";
    outFile << "# ================================================\n";
    outFile << "Run\tEvent\tLuminosityBlock\tCount\tDatasets\n";
    
    std::map<int, int> countHistogram;
    std::map<int, int> datasetCounts; // How many duplicates per other dataset
    
    for (const auto& [id, datasetIndices] : duplicates) {
        int count = datasetIndices.size();
        
        outFile << id.run << "\t" 
               << id.event << "\t" 
               << id.luminosityBlock << "\t" 
               << count << "\t";
        
        // Sort dataset indices for consistent output
        std::vector<int> sortedIndices = datasetIndices;
        std::sort(sortedIndices.begin(), sortedIndices.end());
        
        for (size_t k = 0; k < sortedIndices.size(); ++k) {
            outFile << datasetNames[sortedIndices[k]];
            if (k < sortedIndices.size() - 1) outFile << ",";
            
            // Count per dataset (excluding MuonEG itself)
            if (sortedIndices[k] != 0) {
                datasetCounts[sortedIndices[k]]++;
            }
        }
        outFile << "\n";
        
        countHistogram[count]++;
    }
    outFile.close();
    
    // Write summary
    std::ofstream summaryFile("muoneg_duplicates_summary.txt");
    summaryFile << "=== MuonEG Duplicate Event Analysis Summary ===\n\n";
    
    summaryFile << "Reference Dataset: " << datasetNames[4] << " (MuonEG)\n\n";
    summaryFile << "Compared with:\n";
    for (size_t i = 0; i < 4; ++i) {
        summaryFile << "  [" << (i+1) << "] " << datasetNames[i] << "\n";
    }
    
    summaryFile << "\n--- Statistics ---\n";
    summaryFile << "Total MuonEG events with duplicates: " << duplicates.size() << "\n\n";
    
    summaryFile << "Duplicates per dataset:\n";
    for (size_t i = 0; i < 4; ++i) {
        int count = datasetCounts[i];
        summaryFile << "  " << datasetNames[i] << ": " << count << " events\n";
    }
    
    summaryFile << "\n--- Duplicate Distribution ---\n";
    for (const auto& [count, freq] : countHistogram) {
        summaryFile << "  Events appearing in " << count << " datasets total: " << freq << "\n";
    }
    summaryFile.close();
    
    // Console output
    std::cout << "\n=== Final Results ===\n";
    std::cout << "Total MuonEG events with duplicates: " << duplicates.size() << "\n\n";
    
    std::cout << "Duplicates by dataset:\n";
    for (size_t i = 0; i < 4; ++i) {
        int count = datasetCounts[i];
        std::cout << "  " << datasetNames[i] << ": " << count << " events\n";
    }
    
    if (!duplicates.empty()) {
        std::cout << "\nDuplicate distribution:\n";
        for (const auto& [count, freq] : countHistogram) {
            std::cout << "  " << freq << " events appear in " << count << " datasets\n";
        }
    }
    
    std::cout << "\nOutput files created:\n";
    std::cout << "  - muoneg_duplicates.txt (detailed duplicate list)\n";
    std::cout << "  - muoneg_duplicates_summary.txt (summary statistics)\n";
    std::cout << "\nDone!\n";
    
    return 0;
}
