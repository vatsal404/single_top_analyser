 #include <ROOT/RDataFrame.hxx>
#include <TChain.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdio>

// Structure to hold event identifier with hash function
struct EventID {
    ULong64_t run;
    ULong64_t event;
    ULong64_t luminosityBlock;

    bool operator==(const EventID& other) const {
        return run == other.run && event == other.event && luminosityBlock == other.luminosityBlock;
    }
};

// Hash function for EventID
struct EventIDHash {
    std::size_t operator()(const EventID& id) const {
        return std::hash<ULong64_t>()(id.run) ^
               (std::hash<ULong64_t>()(id.event) << 1) ^
               (std::hash<ULong64_t>()(id.luminosityBlock) << 2);
    }
};

// Structure to track duplicate info
struct DuplicateInfo {
    EventID id;
    std::vector<int> datasets;
};

const size_t CHUNK_SIZE = 2000000; // Process 2M events at a time

// Function to get file list from DAS dataset
std::vector<std::string> getFilesFromDAS(const std::string& dataset) {
    std::vector<std::string> files;

    std::string cmd = "dasgoclient -query='file dataset=" + dataset + "' -limit=0";
    std::cout << "Querying DAS: " << cmd << std::endl;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error: Could not execute dasgoclient" << std::endl;
        return files;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string filename(buffer);
        filename.erase(filename.find_last_not_of(" \n\r\t") + 1);

        if (!filename.empty()) {
            std::string xrootdFile = "root://cms-xrd-global.cern.ch/" + filename;
            files.push_back(xrootdFile);
        }
    }

    pclose(pipe);
    std::cout << "Found " << files.size() << " files for dataset" << std::endl;
    return files;
}

// Read a chunk of events from binary file
std::vector<EventID> readChunk(std::ifstream& file, size_t chunkSize) {
    std::vector<EventID> chunk;
    chunk.reserve(chunkSize);

    ULong64_t run, event, lumi;

    for (size_t i = 0; i < chunkSize; ++i) {
        if (file.read(reinterpret_cast<char*>(&run), sizeof(ULong64_t)) &&
            file.read(reinterpret_cast<char*>(&event), sizeof(ULong64_t)) &&
            file.read(reinterpret_cast<char*>(&lumi), sizeof(ULong64_t))) {
            chunk.push_back({run, event, lumi});
        } else {
            break;
        }
    }

    return chunk;
}

// Process dataset and write events to temporary file
void processDatasetToFile(const std::string& dataset, int datasetIdx, const std::string& tempFile) {
    std::cout << "\n[" << (datasetIdx+1) << "] Processing dataset:\n";
    std::cout << "  " << dataset << std::endl;

    auto files = getFilesFromDAS(dataset);

    if (files.empty()) {
        std::cerr << "  WARNING: No files found for this dataset!\n";
        return;
    }

    TChain chain("Events");
    for (const auto& file : files) {
        chain.Add(file.c_str());
    }

    ROOT::RDataFrame df(chain);
    auto nEntries = df.Count();
    std::cout << "  Total entries: " << *nEntries << std::endl;

    // Extract branches
    auto runs = df.Take<UInt_t>("run");
    auto events = df.Take<ULong64_t>("event");
    auto lumis = df.Take<UInt_t>("luminosityBlock");

    std::cout << "  Processing events..." << std::endl;
    auto runVec = runs.GetValue();
    auto eventVec = events.GetValue();
    auto lumiVec = lumis.GetValue();

    // Write to temporary binary file
    std::ofstream out(tempFile, std::ios::binary);

    for (size_t j = 0; j < runVec.size(); ++j) {
        ULong64_t run = runVec[j];
        ULong64_t event = eventVec[j];
        ULong64_t lumi = lumiVec[j];

        out.write(reinterpret_cast<const char*>(&run), sizeof(ULong64_t));
        out.write(reinterpret_cast<const char*>(&event), sizeof(ULong64_t));
        out.write(reinterpret_cast<const char*>(&lumi), sizeof(ULong64_t));

        if ((j + 1) % 100000 == 0) {
            std::cout << "    Processed " << (j + 1) << " events..." << std::endl;
        }
    }

    out.close();
    std::cout << "  Dataset " << (datasetIdx+1) << " completed: " << runVec.size() << " total events" << std::endl;
    std::cout << "  Temporary file written: " << tempFile << std::endl;
}

// Compare two datasets chunk by chunk
void compareDatasets(const std::string& file1, int idx1,
                     const std::string& file2, int idx2,
                     std::unordered_map<EventID, std::vector<int>, EventIDHash>& duplicates) {

    std::cout << "\nComparing dataset " << (idx1+1) << " with dataset " << (idx2+1) << "..." << std::endl;

    std::ifstream if1(file1, std::ios::binary);

    size_t chunk1Num = 0;
    size_t totalMatches = 0;

    // Read chunks from first file
    while (true) {
        auto chunk1 = readChunk(if1, CHUNK_SIZE);
        if (chunk1.empty()) break;

        chunk1Num++;
        std::cout << "  Processing chunk " << chunk1Num << " from dataset " << (idx1+1)
                  << " (" << chunk1.size() << " events)" << std::endl;

        // Build hash set for quick lookup
        std::unordered_set<EventID, EventIDHash> chunk1Set(chunk1.begin(), chunk1.end());

        // Compare with chunks from second file
        std::ifstream if2(file2, std::ios::binary);
        size_t chunk2Num = 0;
        size_t chunkMatches = 0;

        while (true) {
            auto chunk2 = readChunk(if2, CHUNK_SIZE);
            if (chunk2.empty()) break;

            chunk2Num++;

            // Check for matches
            for (const auto& event : chunk2) {
                if (chunk1Set.count(event)) {
                    auto& datasets = duplicates[event];

                    // Add dataset indices if not already present
                    if (std::find(datasets.begin(), datasets.end(), idx1) == datasets.end()) {
                        datasets.push_back(idx1);
                    }
                    if (std::find(datasets.begin(), datasets.end(), idx2) == datasets.end()) {
                        datasets.push_back(idx2);
                    }
                    chunkMatches++;
                }
            }

            if (chunk2Num % 5 == 0) {
                std::cout << "    Compared with " << chunk2Num << " chunks from dataset "
                          << (idx2+1) << ", found " << chunkMatches << " matches so far" << std::endl;
            }
        }

        if2.close();
        totalMatches += chunkMatches;
        std::cout << "    Chunk " << chunk1Num << " complete: " << chunkMatches << " matches" << std::endl;
    }

    if1.close();
    std::cout << "  Comparison complete: " << totalMatches << " duplicate events found" << std::endl;
}

void findDuplicateEvents() {
    // Define your 5 DAS dataset paths here
    std::vector<std::string> datasets = {
         "/EGamma0/Run2023C-24Jan2024_v4-v1/NANOAOD",
         "/EGamma1/Run2023C-24Jan2024_v4-v1/NANOAOD",
         "/Muon0/Run2023C-24Jan2024_v4-v1/NANOAOD",
         "/Muon1/Run2023C-24Jan2024_v4-v2/NANOAOD",
         "/MuonEG/Run2023C-22Sep2023_v4-v1/NANOAOD"
 
    };

    std::cout << "=== CMS Duplicate Event Detector (Chunk-Based) ===\n";
    std::cout << "Chunk size: " << CHUNK_SIZE << " events\n";
    std::cout << "Datasets to process: " << datasets.size() << "\n";
    std::cout << "========================================================\n\n";

    // Step 1: Process each dataset and write to temporary files
    std::vector<std::string> tempFiles;
    for (size_t i = 0; i < datasets.size(); ++i) {
        std::string tempFile = "temp_dataset_" + std::to_string(i) + ".bin";
        tempFiles.push_back(tempFile);
        processDatasetToFile(datasets[i], i, tempFile);
    }

    // Step 2: Compare all pairs of datasets chunk by chunk
    std::cout << "\n========================================================\n";
    std::cout << "Finding duplicates across all dataset pairs...\n";
    std::cout << "========================================================\n";

    std::unordered_map<EventID, std::vector<int>, EventIDHash> duplicates;

    for (size_t i = 0; i < tempFiles.size(); ++i) {
        for (size_t j = i + 1; j < tempFiles.size(); ++j) {
            compareDatasets(tempFiles[i], i, tempFiles[j], j, duplicates);
        }
    }

    // Step 3: Write results
    std::cout << "\n========================================================\n";
    std::cout << "Writing results...\n";
    std::cout << "========================================================\n\n";

    std::ofstream outFile("duplicate_events.txt");
    outFile << "# Duplicate Events Report\n";
    outFile << "# Events appearing in multiple datasets\n";
    outFile << "# Format: Run  Event  LuminosityBlock  Count  Datasets\n";
    outFile << "# ================================================\n";
    outFile << "Run\tEvent\tLuminosityBlock\tCount\tDatasets\n";

    std::map<int, int> countHistogram;

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
            outFile << (sortedIndices[k] + 1);
            if (k < sortedIndices.size() - 1) outFile << ",";
        }
        outFile << "\n";

        countHistogram[count]++;
    }
    outFile.close();

    // Write summary
    std::ofstream summaryFile("duplicate_summary.txt");
    summaryFile << "=== Duplicate Event Analysis Summary ===\n\n";
    summaryFile << "Datasets analyzed: " << datasets.size() << "\n\n";

    for (size_t i = 0; i < datasets.size(); ++i) {
        summaryFile << "  [" << (i+1) << "] " << datasets[i] << "\n";
    }

    summaryFile << "\n--- Statistics ---\n";
    summaryFile << "Total events with duplicates: " << duplicates.size() << "\n";

    summaryFile << "\n--- Duplicate Distribution ---\n";
    for (const auto& [count, freq] : countHistogram) {
        summaryFile << "  Events appearing in exactly " << count << " datasets: " << freq << "\n";
    }
    summaryFile.close();

    // Console output
    std::cout << "\n=== Final Results ===\n";
    std::cout << "Total duplicate events found: " << duplicates.size() << "\n";

    if (!duplicates.empty()) {
        std::cout << "\nDuplicate distribution:\n";
        for (const auto& [count, freq] : countHistogram) {
            std::cout << "  " << freq << " events appear in " << count << " datasets\n";
        }
    }

    std::cout << "\nOutput files created:\n";
    std::cout << "  - duplicate_events.txt\n";
    std::cout << "  - duplicate_summary.txt\n";

    // Step 4: Clean up temporary files
    std::cout << "\nCleaning up temporary files..." << std::endl;
    for (const auto& tempFile : tempFiles) {
        std::remove(tempFile.c_str());
    }
    std::cout << "Done!\n";
}

int main() {
    ROOT::EnableImplicitMT();

    findDuplicateEvents();

    return 0;
}
