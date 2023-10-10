/*
 * NanoAODAnalyzerrdframe.h
 *
 *  Created on: Sep 30, 2018
 *      Author: suyong
 */

#ifndef NANOAODANALYZERRDFRAME_H_
#define NANOAODANALYZERRDFRAME_H_

#include "TTree.h"
#include "TFile.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

//#include "Math/Vector4D.h"

#include "correction.h"

#include <string>
#include <vector>
#include <map>
//#include "rapidjson/document.h"
#include "nlohmann/json.hpp"

#include "utility.h" // floats, etc are defined here
#include "RNodeTree.h"
#include "TCut.h"
#include "TRandom.h"
#include "RoccoR.h"

using namespace ROOT::RDF;
using namespace std;

class TH1D;

using json = nlohmann::json;

class NanoAODAnalyzerrdframe {
	using RDF1DHist = RResultPtr<TH1D>;
	using RDF2DHist = RResultPtr<TH2D>;
public:
	NanoAODAnalyzerrdframe(string infilename, string intreename, string outfilename);
	NanoAODAnalyzerrdframe(TTree *t, string outfilename);
	virtual ~NanoAODAnalyzerrdframe();


	virtual void setupAnalysis();
	virtual void setTree(TTree *t, string outfilename);

	// object selectors
	// RNode is in namespace ROOT::RDF
	bool readgoodjson(string goodjsonfname); // get ready for applying golden JSON
	void selectFatJets();

	void setupCorrections(string goodjsonfname, string pufname, string putag, string btvfname, string btvtype, string muon_roch_fname, string muon_fname, string muon_hlt_type, string muon_reco_type, string muon_id_type, string muon_iso_type, string electron_fname, string electron_reco_type, string electron_id_type, string jercfname, string jerctag, string jercunctag);
	void setupJetMETCorrection(string fname, string jettag);
	void applyJetMETCorrections();
    
	//virtual void applyJetMETCorrections();


	void addVar(varinfo v);

	// define variables
	template <typename T, typename std::enable_if<!std::is_convertible<T, string>::value, int>::type = 0>
	void defineVar(string varname, T function,  const RDFDetail::ColumnNames_t &columns = {})
	{
		_rlm = _rlm.Define(varname, function, columns);
	};

	void addVartoStore(string varname);
	void addCuts(string cut, string idx);
	void add1DHist(TH1DModel histdef, string variable, string weight, string mincutstep="");
	void add2DHist(TH2DModel histdef, string variable1, string variable2, string weight, string mincutstep="");

	ROOT::RDF::RNode calculateBTagSF(RNode _rlm, std::vector<std::string> Jets_vars, int _case, std::string output_var);

	ROOT::RDF::RNode calculateMuSF(RNode _rlm, std::vector<std::string> Muon_vars, std::string output_var);
	ROOT::RDF::RNode calculateEleSF(RNode _rlm, std::vector<std::string> Ele_vars, std::string output_var);

	void setupCuts_and_Hists();
	void drawHists(RNode t);
	void run(bool saveAll=true, string outtreename="outputTree");

	void setupTree();

	//setting parameters for nanoaod
	void setParams(int year, string runtype, int datatype);
	int _year;
	string _runtype;
	int _datatype;
	bool _isUL =false;
	bool _isReReco = false;
	bool _isData;
	TTree* _atree;
	bool debug = true;
	//floats PDFWeights;


	//initialize HLT names
	std::string ctrlBranchName(string str_Branch);
	std::string setHLT(string str_HLT = "" );
    std::vector< std::string > HLTGlobalNames;
    std::vector< std::string > HLT2016Names;
    std::vector< std::string > HLT2017Names;
    std::vector< std::string > HLT2018Names;

	//initialize object IDs
	std::string ElectronID(int cutbasedID);
    std::string MuonID(int cutbasedID);
    std::string JetID(int cutbasedID);

//private:
	ROOT::RDataFrame _rd;

	//bool _isData;
	bool _jsonOK;
	string _outfilename;
	string _jsonfname;
	string _jerctag;
	string _jercunctag;
	string _putag;
	string _btvtype;
	string _muon_hlt_type;
	string _muon_reco_type;
	string _muon_id_type;
	string _muon_iso_type;
	string _electron_reco_type;
	string _electron_id_type;


	TFile *_outrootfile;
	vector<string> _outrootfilenames;
	RNode _rlm;
	
	map<string, RDF1DHist> _th1dhistos;
	bool helper_1DHistCreator(string hname, string title, const int nbins, const double xlow, const double xhi, string rdfvar, string evWeight, RNode *anode);
	vector<hist1dinfo> _hist1dinfovector;

	//for 2D histograms
	map<string, RDF2DHist> _th2dhistos;
	bool helper_2DHistCreator(string hname, string title, const int nbinsx, const double xlow, const double xhi, const int nbinsy, const double ylow, const double yhi, string rdfvarx, string rdfvary, string evWeight, RNode *anode);
	
	vector<hist2dinfo> _hist2dinfovector;


	vector<string> _originalvars;
	vector<string> _selections;
	
	vector<varinfo> _varinfovector;
	vector<cutinfo> _cutinfovector;

	vector<string> _varstostore;
	map<string, vector<std::string>> _varstostorepertree;

	json jsonroot;
	// pile up weights
	std::unique_ptr<correction::CorrectionSet> _correction_pu;

	//muon correction
	std::unique_ptr<correction::CorrectionSet> _correction_muon ;
	RoccoR _Roch_corr;
	void applyMuPtCorrection();

	//electron correction
	std::unique_ptr<correction::CorrectionSet> _correction_electron ;

	// JERC scale factors
	std::unique_ptr<correction::CorrectionSet> _correction_jerc; // json containing all forms of corrections and uncertainties
	std::shared_ptr<const correction::CompoundCorrection> _jetCorrector; // just the combined L1L2L3 correction
	std::shared_ptr<const correction::Correction> _jetCorrectionUnc; // for uncertainty corresponding to the jet corrector

	// btag correction
	std::unique_ptr<correction::CorrectionSet> _correction_btag1;


	RNodeTree _rnt;

	bool isDefined(string v);

};

#endif /* NANOAODANALYZERRDFRAME_H_ */
