#include <ROOT/RDataFrame.hxx>
#include <TChain.h>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdio>
#include <algorithm>

// ---------------------------
// Event key structure
// ---------------------------
struct EventID {
    UInt_t run;
    ULong64_t event;
    UInt_t lumi;

    bool operator==(const EventID &other) const {
        return run == other.run && event == other.event && lumi == other.lumi;
    }
    
    bool operator<(const EventID &other) const {
        if (run != other.run) return run < other.run;
        if (event != other.event) return event < other.event;
        return lumi < other.lumi;
    }
};

struct EventIDHash {
    std::size_t operator()(const EventID &id) const {
        return std::hash<UInt_t>()(id.run) ^
               (std::hash<ULong64_t>()(id.event) << 1) ^
               (std::hash<UInt_t>()(id.lumi) << 2);
    }
};

// ---------------------------
// Write dataset events to temp file (chunked)
// ---------------------------
void processDataset(const std::string &dataset, int idx, const std::string &outfile)
{
    std::cout << "Processing dataset " << idx << ": " << dataset << "\n";
    
    std::string dascmd = "dasgoclient -query='file dataset=" + dataset + "' -limit=0";

    FILE *pipe = popen(dascmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed DAS query\n";
        return;
    }

    std::vector<std::string> files;
    char buffer[300];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string f(buffer);
        f.erase(f.find_last_not_of(" \n\t") + 1);
        files.push_back("root://cms-xrd-global.cern.ch/" + f);
    }
    pclose(pipe);

    if (files.empty()) {
        std::cerr << "No files for dataset " << dataset << "\n";
        return;
    }

    std::cout << "  Found " << files.size() << " files\n";

    // Process files in batches
    std::ofstream out(outfile, std::ios::binary);
    const size_t filesPerBatch = 15;
    
    for (size_t start = 0; start < files.size(); start += filesPerBatch) {
        size_t end = std::min(start + filesPerBatch, files.size());
        
        std::cout << "  Processing files " << start << " to " << end << "\n";
        
        TChain chain("Events");
        for (size_t i = start; i < end; i++) {
            chain.Add(files[i].c_str());
        }

        ROOT::RDataFrame df(chain);
        auto runs = df.Take<UInt_t>("run");
        auto events = df.Take<ULong64_t>("event");
        auto lumis = df.Take<UInt_t>("luminosityBlock");

        auto vr = runs.GetValue();
        auto ve = events.GetValue();
        auto vl = lumis.GetValue();

        for (size_t i = 0; i < vr.size(); i++) {
            UInt_t run = vr[i];
            ULong64_t ev = ve[i];
            UInt_t lu = vl[i];

            out.write((char *)&run, sizeof(run));
            out.write((char *)&ev, sizeof(ev));
            out.write((char *)&lu, sizeof(lu));
        }
    }
    
    out.close();
    std::cout << "  Completed dataset " << idx << "\n";
}

// ---------------------------
// Sort temp file on disk
// ---------------------------
void sortTempFile(const std::string &infile, const std::string &outfile) {
    std::cout << "  Sorting " << infile << "...\n";
    
    // Read all events
    std::vector<EventID> events;
    std::ifstream in(infile, std::ios::binary);
    
    UInt_t run, lumi;
    ULong64_t event;
    while (in.read((char *)&run, sizeof(run)) &&
           in.read((char *)&event, sizeof(event)) &&
           in.read((char *)&lumi, sizeof(lumi))) {
        events.push_back({run, event, lumi});
    }
    in.close();
    
    std::cout << "    Read " << events.size() << " events, sorting...\n";
    
    // Sort
    std::sort(events.begin(), events.end());
    
    // Write sorted
    std::ofstream out(outfile, std::ios::binary);
    for (const auto &ev : events) {
        out.write((char *)&ev.run, sizeof(ev.run));
        out.write((char *)&ev.event, sizeof(ev.event));
        out.write((char *)&ev.lumi, sizeof(ev.lumi));
    }
    out.close();
    
    std::cout << "    Sorted file written\n";
}

// ---------------------------
// Compare two sorted files and write duplicates
// ---------------------------
size_t findDuplicatesInSortedFiles(const std::string &file1, const std::string &file2, 
                                   const std::string &outfile) {
    std::ifstream in1(file1, std::ios::binary);
    std::ifstream in2(file2, std::ios::binary);
    std::ofstream out(outfile, std::ios::binary);
    
    size_t dupCount = 0;
    
    EventID ev1, ev2;
    bool has1 = false, has2 = false;
    
    // Read first event from each file
    has1 = in1.read((char *)&ev1.run, sizeof(ev1.run)) &&
           in1.read((char *)&ev1.event, sizeof(ev1.event)) &&
           in1.read((char *)&ev1.lumi, sizeof(ev1.lumi));
    
    has2 = in2.read((char *)&ev2.run, sizeof(ev2.run)) &&
           in2.read((char *)&ev2.event, sizeof(ev2.event)) &&
           in2.read((char *)&ev2.lumi, sizeof(ev2.lumi));
    
    // Merge-style comparison
    while (has1 && has2) {
        if (ev1 == ev2) {
            // Duplicate found - write to output
            out.write((char *)&ev2.run, sizeof(ev2.run));
            out.write((char *)&ev2.event, sizeof(ev2.event));
            out.write((char *)&ev2.lumi, sizeof(ev2.lumi));
            dupCount++;
            
            // Advance both
            has1 = in1.read((char *)&ev1.run, sizeof(ev1.run)) &&
                   in1.read((char *)&ev1.event, sizeof(ev1.event)) &&
                   in1.read((char *)&ev1.lumi, sizeof(ev1.lumi));
            has2 = in2.read((char *)&ev2.run, sizeof(ev2.run)) &&
                   in2.read((char *)&ev2.event, sizeof(ev2.event)) &&
                   in2.read((char *)&ev2.lumi, sizeof(ev2.lumi));
        } else if (ev1 < ev2) {
            // Advance file1
            has1 = in1.read((char *)&ev1.run, sizeof(ev1.run)) &&
                   in1.read((char *)&ev1.event, sizeof(ev1.event)) &&
                   in1.read((char *)&ev1.lumi, sizeof(ev1.lumi));
        } else {
            // Advance file2
            has2 = in2.read((char *)&ev2.run, sizeof(ev2.run)) &&
                   in2.read((char *)&ev2.event, sizeof(ev2.event)) &&
                   in2.read((char *)&ev2.lumi, sizeof(ev2.lumi));
        }
    }
    
    out.close();
    return dupCount;
}

// ---------------------------
// Main duplicate comparison
// ---------------------------
void findDuplicateEvents()
{
    std::vector<std::string> datasets = {
        "/EGamma0/Run2023C-24Jan2024_v4-v1/NANOAOD",
        "/EGamma1/Run2023C-24Jan2024_v4-v1/NANOAOD",
        "/Muon0/Run2023C-24Jan2024_v4-v1/NANOAOD",
        "/Muon1/Run2023C-24Jan2024_v4-v2/NANOAOD",
        "/MuonEG/Run2023C-22Sep2023_v4-v1/NANOAOD"
    };

    std::vector<std::string> datasetNames = {
        "EGamma0", "EGamma1", "Muon0", "Muon1", "MuonEG"
    };

    std::vector<std::string> tempFiles;
    std::vector<std::string> sortedFiles;
    
    // Step 1: Create temp files for each dataset
    std::cout << "\n=== Step 1: Creating temp files ===\n";
    for (size_t i = 0; i < datasets.size(); i++) {
        std::string tmp = "temp_" + std::to_string(i) + ".bin";
        tempFiles.push_back(tmp);
        processDataset(datasets[i], i, tmp);
    }

    // Step 2: Sort each temp file
    std::cout << "\n=== Step 2: Sorting temp files ===\n";
    for (size_t i = 0; i < tempFiles.size(); i++) {
        std::string sorted = "sorted_" + std::to_string(i) + ".bin";
        sortedFiles.push_back(sorted);
        std::cout << "Sorting " << datasetNames[i] << "\n";
        sortTempFile(tempFiles[i], sorted);
    }

    // Step 3: Find duplicates by comparing sorted files
    std::cout << "\n=== Step 3: Finding duplicates ===\n";
    
    std::vector<size_t> removeCounts(datasets.size(), 0);
    
    // For each dataset, compare with all previous datasets
    for (size_t i = 0; i < sortedFiles.size(); i++) {
        std::string outfile = "duplicates_" + datasetNames[i] + ".bin";
        std::ofstream finalOut(outfile, std::ios::binary);
        
        std::cout << "Finding duplicates for " << datasetNames[i] << ":\n";
        
        // Compare with all previous datasets
        for (size_t j = 0; j < i; j++) {
            std::string tmpDup = "tmp_dup_" + std::to_string(i) + "_" + std::to_string(j) + ".bin";
            std::cout << "  Comparing with " << datasetNames[j] << "...\n";
            
            size_t count = findDuplicatesInSortedFiles(sortedFiles[j], sortedFiles[i], tmpDup);
            std::cout << "    Found " << count << " duplicates\n";
            
            // Append to final output
            std::ifstream tmpIn(tmpDup, std::ios::binary);
            finalOut << tmpIn.rdbuf();
            tmpIn.close();
            
            removeCounts[i] += count;
            
            // Clean up temp file
            std::remove(tmpDup.c_str());
        }
        
        finalOut.close();
    }

    // Summary
    std::cout << "\n=== Summary ===\n";
    size_t totalRemoved = 0;
    for (size_t i = 0; i < datasets.size(); i++) {
        std::cout << "Dataset " << datasetNames[i] << ": "
                  << removeCounts[i] << " events to remove (duplicates_"
                  << datasetNames[i] << ".bin)\n";
        totalRemoved += removeCounts[i];
    }
    
    std::cout << "\nTotal events marked for removal: " << totalRemoved << "\n";
    std::cout << "\nYou can now delete temp_*.bin and sorted_*.bin files if desired.\n";
}

// ---------------------------
// main()
// ---------------------------
int main()
{
    findDuplicateEvents();
    return 0;
}
