//============================================================================
// Name        : nanoaodrdataframe.cpp
// Author      : Suyong Choi
// Version     :
// Copyright   : suyong@korea.ac.kr, Korea University, Department of Physics
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "NanoAODAnalyzerrdframe.h"
#include "BaseAnalyser.h"
//#include "SkimEvents.h"
#include "TChain.h"

using namespace std;
using namespace ROOT;

int main(void) {

	TChain c1("Events");
	//c1.Add("testinputdata/data/JetHT_2017C_DA05CA1A-3265-EE47-84F9-10CB09D22BDA.root"); //data
	//c1.Add("testinputdata/MC/2017/ttJets-2017MC-A258F579-5EC0-D840-95D7-4327595FC3DE.root"); // MC
	c1.Add("testinputdata/MC/2018/BG/TTbarBKGMC.root"); // MC
	BaseAnalyser nanoaodrdf(&c1, "testout.root");

	string goodjsonfname = "data/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt";
	string pileupfname = "'data/LUM/2018_UL/puWeights.json";
	string pileuptag = "Collisions18_UltraLegacy_goldenJSON";
	string btvfname = "data/BTV/2018_UL/btagging.json";
	string btvtype = "deepJet_shape";
	string muon_fname = "data/MUON/2018_UL/muon_Z.json";
	string muontype = "NUM_MediumID_DEN_TrackerMuons";
	string electron_fname = "data/ELECTRON/2018_UL/electron_Z.json";
	string electrontype = "UL-Electron-ID-SF";
	string jercfname = "data/JERC/UL18_jerc.json";
	string jerctag = "Summer19UL18_V5_MC_L1L2L3Res_AK4PFchs";
	string jercunctag = "Summer19UL18_V5_MC_Total_AK4PFchs";



	nanoaodrdf.setupCorrections(goodjsonfname, pileupfname, pileuptag, btvfname, btvtype, jercfname, jerctag, jercunctag);
	nanoaodrdf.setupObjects();
	nanoaodrdf.setupAnalysis();
	nanoaodrdf.run(false, "outputTree");

	return EXIT_SUCCESS;
}
