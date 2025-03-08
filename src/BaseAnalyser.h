/*
 * BaseAnalyser.h
 *
 *  Created on: May 6, 2022
 *      Author: suyong
 *		Developper: cdozen
 */

#ifndef BASEANALYSER_H_
#define BASEANALYSER_H_

#include "NanoAODAnalyzerrdframe.h"

class BaseAnalyser: public NanoAODAnalyzerrdframe
{
	public:
		BaseAnalyser(TTree *t, std::string outfilename);
		void defineCuts();		//define a series of cuts from defined variables only. you must implement this in your subclassed analysis 
		void defineMoreVars(); 	//define higher-level variables from basic ones, you must implement this in your subclassed analysis code
		void bookHists(); 		//book histograms, you must implement this in your subclassed analysis code

		void setTree(TTree *t, std::string outfilename);
		void setupObjects();
		void setupAnalysis();
		// object selectors
		void selectElectrons();
		void spectatorJets();
                void Background_Estimation();
                void selectChannel();
		void reconstructWboson();	
        	void selectMuons();
		void selectJets();
		void calculateEvWeight();
		void selectMET();
                void plotWBosonMass();
		void removeOverlaps();
                void reconstructTop();
                void defineRegion();
		bool debug = true;
		bool _jsonOK;
		string _outfilename;


		TFile *_outrootfile;
		vector<string> _outrootfilenames;

                int _redefine;  
};



#endif /* BASEANALYSER_H_ */
