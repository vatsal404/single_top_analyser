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
		void selectMuons();
		void selectJets();
		void calculateEvWeight();
		void selectMET();
		void removeOverlaps();



		bool debug = true;
		bool _jsonOK;
		string _outfilename;
		string _jsonfname;
		string _jerctag;
		string _jercunctag;
		string _putag;
		string _btvtype;


		TFile *_outrootfile;
		vector<string> _outrootfilenames;

		// JERC scale factors
		std::unique_ptr<correction::CorrectionSet> _correction_jerc; // json containing all forms of corrections and uncertainties
		std::shared_ptr<const correction::CompoundCorrection> _jetCorrector; // just the combined L1L2L3 correction
		std::shared_ptr<const correction::Correction> _jetCorrectionUnc; // for uncertainty corresponding to the jet corrector
		// btag correction
		std::unique_ptr<correction::CorrectionSet> _correction_btag1;
		// pile up weights
		std::unique_ptr<correction::CorrectionSet> _correction_pu;


		bool readgoodjson(string goodjsonfname); // get ready for applying golden JSON
		json jsonroot;
		
		void setupCorrections(string goodjsonfname, string pufname, string putag, string btvfname, string btvtype, string jercfname, string jerctag, string jercunctag);
		void setupJetMETCorrection(string fname, string jettag);
		void applyJetMETCorrections();
		

};



#endif /* BASEANALYSER_H_ */
