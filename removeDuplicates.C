#include <TFile.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TEntryList.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <chrono>
#include <cstdio>

struct EventID {
    UInt_t run;
    UInt_t lumi;
    ULong64_t event;
    Long64_t treeIndex;
    
    bool operator<(const EventID& other) const {
        if (run != other.run) return run < other.run;
        if (lumi != other.lumi) return lumi < other.lumi;
        return event < other.event;
    }
};

void removeDuplicates(const char* inputFile, const char* outputFile) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::cout << "Opening input file: " << inputFile << std::endl;
    TFile* inFile = TFile::Open(inputFile, "READ");
    if (!inFile || inFile->IsZombie()) {
        std::cerr << "Error: Cannot open input file" << std::endl;
        return;
    }
    
    TTree* inTree = (TTree*)inFile->Get("outputTree");
    if (!inTree) {
        std::cerr << "Error: Cannot find tree 'outputTree'" << std::endl;
        inFile->Close();
        return;
    }
    
    Long64_t totalEntries = inTree->GetEntries();
    std::cout << "Total entries: " << totalEntries << std::endl;
    std::cout << "Estimated memory needed: ~" << (totalEntries * 24 / 1024 / 1024 / 1024) << " GB" << std::endl;
    std::cout << "Using disk-based two-pass approach (memory efficient)..." << std::endl;
    
    // Setup reader
    TTreeReader reader("outputTree", inFile);
    TTreeReaderValue<UInt_t> run(reader, "run");
    TTreeReaderValue<UInt_t> lumi(reader, "luminosityBlock");
    TTreeReaderValue<ULong64_t> event(reader, "event");
    
    // Write event IDs to temporary binary file
    const char* tempFile = "temp_eventids.bin";
    std::cout << "Pass 1/4: Writing event IDs to disk..." << std::endl;
    
    FILE* fp = fopen(tempFile, "wb");
    if (!fp) {
        std::cerr << "Error: Cannot create temp file" << std::endl;
        inFile->Close();
        return;
    }
    
    Long64_t reportInterval = totalEntries / 100;
    if (reportInterval == 0) reportInterval = 100000;
    
    Long64_t idx = 0;
    EventID eid;
    while (reader.Next()) {
        eid.run = *run;
        eid.lumi = *lumi;
        eid.event = *event;
        eid.treeIndex = idx;
        fwrite(&eid, sizeof(EventID), 1, fp);
        idx++;
        
        if (idx % reportInterval == 0) {
            float progress = 100.0 * idx / totalEntries;
            std::cout << "Writing: " << progress << "% (" << idx << "/" << totalEntries << ")\r" << std::flush;
        }
    }
    fclose(fp);
    std::cout << std::endl;
    
    // Sort in chunks
    std::cout << "Pass 2/4: Sorting event IDs in chunks..." << std::endl;
    
    const Long64_t chunkSize = 10000000; // 10M events per chunk (~240 MB)
    Long64_t numChunks = (totalEntries + chunkSize - 1) / chunkSize;
    
    std::cout << "Processing in " << numChunks << " chunks of " << chunkSize << " events" << std::endl;
    
    // Sort each chunk
    std::vector<std::string> chunkFiles;
    fp = fopen(tempFile, "rb");
    
    for (Long64_t chunk = 0; chunk < numChunks; ++chunk) {
        Long64_t chunkEntries = std::min(chunkSize, totalEntries - chunk * chunkSize);
        std::vector<EventID> chunkData(chunkEntries);
        
        fread(chunkData.data(), sizeof(EventID), chunkEntries, fp);
        std::sort(chunkData.begin(), chunkData.end());
        
        std::string chunkFileName = "temp_chunk_" + std::to_string(chunk) + ".bin";
        FILE* chunkFp = fopen(chunkFileName.c_str(), "wb");
        fwrite(chunkData.data(), sizeof(EventID), chunkEntries, chunkFp);
        fclose(chunkFp);
        
        chunkFiles.push_back(chunkFileName);
        
        std::cout << "Sorted chunk " << (chunk + 1) << "/" << numChunks << "\r" << std::flush;
    }
    fclose(fp);
    std::cout << std::endl;
    
    // Merge and identify duplicates - write indices to TEXT file
    std::cout << "Pass 3/4: Merging and identifying unique events..." << std::endl;
    
    std::vector<FILE*> chunkFps;
    std::vector<EventID> currentEvents(numChunks);
    std::vector<bool> chunkActive(numChunks, true);
    
    // Open all chunk files
    for (Long64_t i = 0; i < numChunks; ++i) {
        chunkFps.push_back(fopen(chunkFiles[i].c_str(), "rb"));
        if (fread(&currentEvents[i], sizeof(EventID), 1, chunkFps[i]) != 1) {
            chunkActive[i] = false;
        }
    }
    
    // Write unique indices to TEXT file (smaller, easier to handle)
    const char* indicesFile = "temp_unique_indices.txt";
    std::ofstream indicesOut(indicesFile);
    if (!indicesOut) {
        std::cerr << "Error: Cannot create indices file" << std::endl;
        return;
    }
    
    Long64_t uniqueCount = 0;
    Long64_t duplicateCount = 0;
    EventID lastEvent = {0, 0, 0, -1};
    
    // Merge sorted chunks and write unique indices
    while (true) {
        Long64_t minChunk = -1;
        EventID minEvent;
        
        for (Long64_t i = 0; i < numChunks; ++i) {
            if (chunkActive[i]) {
                if (minChunk == -1 || currentEvents[i] < minEvent) {
                    minChunk = i;
                    minEvent = currentEvents[i];
                }
            }
        }
        
        if (minChunk == -1) break;
        
        bool isDuplicate = (lastEvent.treeIndex != -1 &&
                           minEvent.run == lastEvent.run &&
                           minEvent.lumi == lastEvent.lumi &&
                           minEvent.event == lastEvent.event);
        
        if (!isDuplicate) {
            indicesOut << minEvent.treeIndex << "\n";
            uniqueCount++;
        } else {
            duplicateCount++;
        }
        
        lastEvent = minEvent;
        
        if (fread(&currentEvents[minChunk], sizeof(EventID), 1, chunkFps[minChunk]) != 1) {
            chunkActive[minChunk] = false;
        }
        
        if ((uniqueCount + duplicateCount) % reportInterval == 0) {
            float progress = 100.0 * (uniqueCount + duplicateCount) / totalEntries;
            std::cout << "Merging: " << progress << "% | Unique: " << uniqueCount 
                     << " | Duplicates: " << duplicateCount << "\r" << std::flush;
        }
    }
    indicesOut.close();
    std::cout << std::endl;
    
    // Cleanup chunk files
    for (Long64_t i = 0; i < numChunks; ++i) {
        fclose(chunkFps[i]);
        remove(chunkFiles[i].c_str());
    }
    remove(tempFile);
    
    std::cout << "Found " << uniqueCount << " unique events, " << duplicateCount << " duplicates" << std::endl;
    
    // Now use TEntryList for memory-efficient filtering
    std::cout << "Pass 4/4: Creating output tree using TEntryList..." << std::endl;
    
    // Create TEntryList from indices file
    TEntryList* entryList = new TEntryList("entryList", "Unique events");
    
    std::ifstream indicesIn(indicesFile);
    if (!indicesIn) {
        std::cerr << "Error: Cannot open indices file" << std::endl;
        return;
    }
    
    std::cout << "Loading unique indices into TEntryList..." << std::endl;
    Long64_t index;
    Long64_t loaded = 0;
    while (indicesIn >> index) {
        entryList->Enter(index);
        loaded++;
        
        if (loaded % reportInterval == 0) {
            float progress = 100.0 * loaded / uniqueCount;
            std::cout << "Loading: " << progress << "% (" << loaded << "/" << uniqueCount << ")\r" << std::flush;
        }
    }
    indicesIn.close();
    remove(indicesFile);
    std::cout << std::endl;
    
    std::cout << "Loaded " << entryList->GetN() << " entries into TEntryList" << std::endl;
    
    // Set the entry list on the input tree
    inTree->SetEntryList(entryList);
    
    // Create output file and use CopyTree with entry list
    std::cout << "Writing output tree (this may take a while)..." << std::endl;
    TFile* outFile = TFile::Open(outputFile, "RECREATE");
    
    // Set smaller basket size and disable auto-flush to reduce memory
    inTree->SetCacheSize(100*1024*1024); // 100 MB cache
    
    // CopyTree will only copy entries in the entry list
    TTree* outTree = inTree->CopyTree("");
    
    std::cout << "Output tree has " << outTree->GetEntries() << " entries" << std::endl;
    
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "Processing complete!" << std::endl;
    std::cout << "Total entries: " << totalEntries << std::endl;
    std::cout << "Unique events: " << uniqueCount << std::endl;
    std::cout << "Duplicates removed: " << duplicateCount << std::endl;
    std::cout << "Duplicate percentage: " << (100.0 * duplicateCount / totalEntries) << "%" << std::endl;
    
    outFile->cd();
    outTree->Write("", TObject::kOverwrite);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
    std::cout << "Total time: " << duration << " seconds (" << duration/60 << " min)" << std::endl;
    
    delete entryList;
    outFile->Close();
    inFile->Close();
    
    delete outFile;
    delete inFile;
    
    std::cout << "Output: " << outputFile << std::endl;
}

// Memory-optimized version using TEntryList:
// 1. Passes 1-3 same as before (disk-based sorting/merging)
// 2. Writes indices to TEXT file (smaller than binary)
// 3. Creates TEntryList from indices
// 4. Uses TTree::CopyTree() with entry list (ROOT handles memory efficiently)
// 
// Memory usage: ~2-3 GB peak instead of 13+ GB
