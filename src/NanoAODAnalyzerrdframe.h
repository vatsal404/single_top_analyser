/*
 * NanoAODAnalyzerrdframe.h
 *
 *  Created on: Sep 30, 2018
 *      Author: suyong
 * 
 */

#ifndef NANOAODANALYZERRDFRAME_H_
#define NANOAODANALYZERRDFRAME_H_

#include "TTree.h"
#include "TFile.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "Math/Vector4D.h"
#include "BTagCalibrationStandalone.h"
#include "WeightCalculatorFromHistogram.h"

#include <string>
#include "json/json.h"

#include "utility.h" // floats, etc are defined here
#include "RNodeTree.h"
#include "JetCorrectorParameters.h"
#include "FactorizedJetCorrector.h"
#include "JetCorrectionUncertainty.h"
//#include "TauSFTool.h"
//#include "MainDatabase.h"
#include "TCut.h"
using namespace ROOT::RDF;


class NanoAODAnalyzerrdframe {
    using RDF1DHist = RResultPtr<TH1D>;
public:
    NanoAODAnalyzerrdframe(std::string infilename, std::string intreename, std::string outfilename, std::string year="",std::string runtype="",std::string syst="",  std::string jsonfname="", string globaltag="", int nthreads=1, int isDATA=-1);
    NanoAODAnalyzerrdframe(TTree *t, std::string outfilename, std::string year="",std::string runtype="",std::string syst="", std::string jsonfname="", string globaltag="", int nthreads=1, int isDATA=-1);
    virtual ~NanoAODAnalyzerrdframe();
    virtual void setupAnalysis();

    //for the kinematic cuts: initiliaze the text where cuts defined//candan
    //MainDatabase* cutdb = new MainDatabase("Md_cuts.ini");
    // object selectors
    // RNode is in namespace ROOT::RDF
    bool readjson();
    void applyJetMETCorrections();
    //void matchGenReco();
    //virtual void calculateEvWeight();
    //virtual void defineMoreVars() = 0; //subclas // define higher-level variables from basic ones, you must implement this in your subclassed analysis code

    void addVar(varinfo v);

    template <typename T, typename std::enable_if<!std::is_convertible<T, std::string>::value, int>::type = 0>
    void defineVar(std::string varname, T function,  const RDFDetail::ColumnNames_t &columns = {})
    {
        _rlm = _rlm.Define(varname, function, columns);
    };

    void addVartoStore(std::string varname);
    void addCuts(std::string cut, std::string idx); //cut idx
    //virtual void defineCuts() = 0; //subclas // define a series of cuts from defined variables only. you must implement this in your subclassed analysis code
    void add1DHist(TH1DModel histdef, std::string variable, std::string weight, string mincutstep="");
    //virtual void bookHists() = 0; //subclas // book histograms, you must implement this in your subclassed analysis code

    void setupCuts_and_Hists();
    void drawHists(RNode t);
    void run(bool saveAll=true, std::string outtreename="outputTree");
    //virtual void setTree(TTree *t, std::string outfilename);
    void setupTree();

//private:
    ROOT::RDataFrame _rd;

    bool _isSkim;
    bool _isData;
    bool _jsonOK;
    std::string _outfilename;
    std::string _year;
    int run_year;
    std::string _runtype;
    //int isDATA;
    bool _isRun16pre = false;
    bool _isRun16post = false;
    bool _isRun16 = false;
    bool _isRun17 = false;
    bool _isRun18 = false;
    bool _isUL = false;
    bool _isReReco = false;
    bool debug = true;
    std::string _syst;
    std::string _jsonfname;
    std::string _globaltag;
    TFile *_inrootfile;
    TFile *_outrootfile;
    std::vector<std::string> _outrootfilenames;
    RNode _rlm;
    std::map<std::string, RDF1DHist> _th1dhistos;
    //bool helper_1DHistCreator(std::string hname, std::string title, const int nbins, const double xlow, const double xhi, std::string rdfvar, std::string evWeight);
    void helper_1DHistCreator(std::string hname, std::string title, const int nbins, const double xlow, const double xhi, std::string rdfvar, std::string evWeight, RNode *anode);
    std::vector<std::string> _originalvars;
    std::vector<std::string> _selections;

    std::vector<hist1dinfo> _hist1dinfovector;
    std::vector<varinfo> _varinfovector;
    std::vector<cutinfo> _cutinfovector;

    std::vector<std::string> _varstostore;
    std::map<std::string, std::vector<std::string>> _varstostorepertree;

    Json::Value jsonroot;

    // btag weights
    BTagCalibration _btagcalib;
    BTagCalibration _btagcalib2;
    BTagCalibrationReader _btagcalibreader;
    BTagCalibrationReader _btagcalibreader2;

    // pile up weights
    const char * pumcfile;
    const char * pudatafile;
    TH1D *_hpumc;
    TH1D *_hpudata;
    TH1D *_hpudata_plus;
    TH1D *_hpudata_minus;
    WeightCalculatorFromHistogram *_puweightcalc;
    WeightCalculatorFromHistogram *_puweightcalc_plus;
    WeightCalculatorFromHistogram *_puweightcalc_minus;
    RNodeTree _rnt;
    RNodeTree *currentnode;
    bool isDefined(string v);

    // Jet MET corrections
    void setupJetMETCorrection(std::string globaltag, std::string jetalgo="AK4PFchs");
    FactorizedJetCorrector *_jetCorrector;
    JetCorrectionUncertainty *_jetCorrectionUncertainty;
    TH2F* _hmuontrg;
    TH2F* _hmuonid;
    TH2F* _hmuoniso;
    WeightCalculatorFromHistogram* _muontrg;
    WeightCalculatorFromHistogram* _muonid;
    WeightCalculatorFromHistogram* _muoniso;
    //TauIDSFTool* _tauidSFjet;
    //TauIDSFTool* _tauidSFele;
    //TauIDSFTool* _tauidSFmu;
    //TauESTool* _testool;
    //TauFESTool* _festool;

	//initialize HLT names
	std::string setHLT(string str_HLT = "" );
    std::string ctrlBranchName(string str_Branch);
    std::vector< std::string > HLTGlobalNames;
    std::vector< std::string > HLT2016Names;
    std::vector< std::string > HLT2017Names;
    std::vector< std::string > HLT2018Names;

	//initialize object IDs
	std::string ElectronID(int cutbasedID);
    std::string MuonID(int cutbasedID);
    std::string JetID(int cutbasedID);
    //std::string MuonID(int cutbasedID, bool enable_MuonIso = true, bool enable_veto_muon= false);

};

#endif /* NANOAODANALYZERRDFRAME_H_ */
