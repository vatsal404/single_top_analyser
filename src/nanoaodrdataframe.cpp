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
#include "TChain.h"

using namespace std;
using namespace ROOT;

int main(void) {

	//ROOT::EnableImplicitMT();
	TChain c1("outputTree");
	c1.Add("");


	BaseAnalyser nanoaodrdf(&c1, "testout.root", "");
	nanoaodrdf.setupAnalysis();
	nanoaodrdf.run(false, "outputTree2");

	return EXIT_SUCCESS;
}
