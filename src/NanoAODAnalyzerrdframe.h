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


using namespace ROOT::RDF;
using namespace std;

class TH1D;

using json = nlohmann::json;

class NanoAODAnalyzerrdframe {
	using RDF1DHist = RResultPtr<TH1D>;
public:
	NanoAODAnalyzerrdframe(string infilename, string intreename, string outfilename);
	NanoAODAnalyzerrdframe(TTree *t, string outfilename);
	virtual ~NanoAODAnalyzerrdframe();


	virtual void setupAnalysis();
	virtual void setTree(TTree *t, string outfilename);

	// object selectors
	// RNode is in namespace ROOT::RDF
	bool readgoodjson(string goodjsonfname); // get ready for applying golden JSON


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


	TFile *_outrootfile;
	vector<string> _outrootfilenames;
	RNode _rlm;
	map<string, RDF1DHist> _th1dhistos;
	//bool helper_1DHistCreator(std::string hname, std::string title, const int nbins, const double xlow, const double xhi, std::string rdfvar, std::string evWeight);
	bool helper_1DHistCreator(string hname, string title, const int nbins, const double xlow, const double xhi, string rdfvar, string evWeight, RNode *anode);
	vector<string> _originalvars;
	vector<string> _selections;

	vector<hist1dinfo> _hist1dinfovector;
	vector<varinfo> _varinfovector;
	vector<cutinfo> _cutinfovector;

	vector<string> _varstostore;
	map<string, vector<std::string>> _varstostorepertree;

	json jsonroot;

	RNodeTree _rnt;

	bool isDefined(string v);

};

#endif /* NANOAODANALYZERRDFRAME_H_ */
