/*
 * NanoAODAnalyzerrdframe.cpp
 *
 *  Created on: Sep 30, 2018
 *      Author: suyong
 */

#include "NanoAODAnalyzerrdframe.h"
#include <iostream>
#include <algorithm>
#include <typeinfo>

#include "TCanvas.h"
#include "Math/GenVector/VectorUtil.h"
#include <vector>
#include <fstream>
#include "utility.h"
#include <regex>
#include "ROOT/RDFHelpers.hxx"


using namespace std;

NanoAODAnalyzerrdframe::NanoAODAnalyzerrdframe(TTree *atree, std::string outfilename, std::string year, std::string runtype, std::string syst, std::string jsonfname, std::string globaltag, int nthreads,  int isDATA)
:_rd(*atree),_jsonOK(false), _outfilename(outfilename), _year(year), _runtype(runtype), _syst(syst), _jsonfname(jsonfname), _globaltag(globaltag), _inrootfile(0),_outrootfile(0), _rlm(_rd)
    , _btagcalibreader(BTagEntry::OP_RESHAPING, "central", {"up_jes", "down_jes"})
    , _rnt(&_rlm), currentnode(0), _jetCorrector(0), _jetCorrectionUncertainty(0)
{
        // skim switch
        if(_year.find("skim") != std::string::npos){
                _isSkim = true;
                cout << "<< Start Skim NanoAOD >>" << endl;
        }else{
                _isSkim = false;
                cout << "<< Start Process NanoAOD >>" << endl;
        }

        // Year switch
        run_year = atoi(_year.c_str());
        std::cout<< "Data year  == " << run_year << std::endl;
        std::cout<< "-------------------------------------------------------------------" << std::endl;

        if(run_year==2016) {
            cout << "Analysing through Run 2016" << endl;
        }else if(run_year==2017) {
            cout << "Analysing through Run 2017" << endl;
        }else if(run_year==2018){
            cout << "Analysing through Run 2018" << endl;
        }

        // Run type switch : UL or ReReco
        if(_runtype.find("UL") != std::string::npos){
                _isUL = true;
                cout << "Ultra Legacy Selected " << endl;
                std::cout<< "-------------------------------------------------------------------" << std::endl;
        }else if(_runtype.find("ReReco") != std::string::npos){
                _isReReco = true;
                cout << " ReReco  Selected!" << endl;
                std::cout<< "-------------------------------------------------------------------" << std::endl;
        }
        if (!_isUL && !_isReReco){
            std::cout<< "Default run version : UL or ReReco is not selected! "<< std::endl;
            std::cout<< "-------------------------------------------------------------------" << std::endl;
        }

        // Data/mc switch (options: -1:  choose according to the inputfile branch ,  0: is MC , 1: is DATA )

        std::cout<<"You enter --isDATA  =  " <<isDATA<<  std::endl;
        if (isDATA == -1){
            std::cout << "File type selected Automatically by checking the {gen} branch in input rootfiles:\n";

            if(atree->GetBranch("genWeight") == nullptr){
                _isData = true;
                std::cout << "input file is DATA \n";
                std::cout<< "-------------------------------------------------------------------" << std::endl;

            }else{
                _isData = false;
                std::cout << "input file is MC \n";
                std::cout<< "-------------------------------------------------------------------" << std::endl;
            }
        } else if (isDATA == 0)
        {
            std::cout << "File type is FORCED to SELECT from USER:\n";
            std::cout << " isDATA==0 : MC input files Selected!! \n";
            std::cout<< "-------------------------------------------------------------------" << std::endl;
            _isData = false;
        } else if (isDATA==1)
        {
            std::cout << "File type is FORCED to SELECT from USER:\n";
            _isData = true;
            std::cout << " isDATA==1 : DATA input files Selected!! \n";
            std::cout<< "-------------------------------------------------------------------" << std::endl;
        }

    TObjArray *allbranches = atree->GetListOfBranches();
    for (int i =0; i<allbranches->GetSize(); i++)
    {
        TBranch *abranch = dynamic_cast<TBranch *>(allbranches->At(i));
        if (abranch!= nullptr){
            //cout << abranch->GetName() << ", \n";
            _originalvars.push_back(abranch->GetName());
        }
    }
        cout<<endl;
        if(_isSkim)
        {
            // pu weight setup
            cout<<"Loading Pileup profiles"<<endl;
            //if(_isRun17){
            if(run_year==2017){
                pumcfile = "data/Pileup/PileupMC_UL17.root";
                pudatafile = "data/Pileup/PileupDATA_UL17.root";
            }else if(run_year==2018){//if(_isRun18){
                pumcfile = "data/Pileup/PileupMC_UL18.root";
                pudatafile = "data/Pileup/PileupDATA_UL18.root";
            }
            TFile tfmc(pumcfile);
            _hpumc = dynamic_cast<TH1D *>(tfmc.Get("pu_mc"));
            _hpumc->SetDirectory(0);
            tfmc.Close();

            TFile tfdata(pudatafile);
            _hpudata = dynamic_cast<TH1D *>(tfdata.Get("pileup"));
            _hpudata_plus = dynamic_cast<TH1D *>(tfdata.Get("pileup_plus"));
            _hpudata_minus = dynamic_cast<TH1D *>(tfdata.Get("pileup_minus"));

            _hpudata->SetDirectory(0);
            _hpudata_plus->SetDirectory(0);
            _hpudata_minus->SetDirectory(0);
            tfdata.Close();

            _puweightcalc = new WeightCalculatorFromHistogram(_hpumc, _hpudata);
            _puweightcalc_plus = new WeightCalculatorFromHistogram(_hpumc, _hpudata_plus);
            _puweightcalc_minus = new WeightCalculatorFromHistogram(_hpumc, _hpudata_minus);
        }else
        {
            cout<<"Loading jetmet Correction"<<endl;
            setupJetMETCorrection(_globaltag);

            if(!_isData){
                if(run_year==2017){
                      //  _btagcalib = {"DeepJet","data/btagSF/DeepJet_106XUL17SF_V2p1.csv"};
                }else if(run_year==2018){//if(_isRun18){
                       // _btagcalib = {"DeepJet","data/btagSF/DeepJet_106XUL18SF.csv"};
                }

                if(run_year==2017){
                    TFile muontrg("data/MuonSF/UL2017/Efficiencies_muon_generalTracks_Z_Run2017_UL_SingleMuonTriggers.root");
                    _hmuontrg = dynamic_cast<TH2F *>(muontrg.Get("NUM_IsoMu27_DEN_CutBasedIdTight_and_PFIsoTight_abseta_pt"));
                    _hmuontrg->SetDirectory(0);
                    muontrg.Close();

                    TFile muonid("data/MuonSF/UL2017/Efficiencies_muon_generalTracks_Z_Run2017_UL_ID.root");
                    _hmuonid = dynamic_cast<TH2F *>(muonid.Get("NUM_TightID_DEN_TrackerMuons_abseta_pt"));
                    _hmuonid->SetDirectory(0);
                    muonid.Close();

                    TFile muoniso("data/MuonSF/UL2017/Efficiencies_muon_generalTracks_Z_Run2017_UL_ISO.root");
                    _hmuoniso = dynamic_cast<TH2F *>(muoniso.Get("NUM_TightRelIso_DEN_TightIDandIPCut_abseta_pt"));
                    _hmuoniso->SetDirectory(0);
                    muoniso.Close();

                    _muontrg = new WeightCalculatorFromHistogram(_hmuontrg);
                    _muonid = new WeightCalculatorFromHistogram(_hmuonid);
                    _muoniso = new WeightCalculatorFromHistogram(_hmuoniso);

                }else if(run_year==2018){
                //if(_isRun18){
                    TFile muontrg("data/MuonSF/UL2018/Efficiencies_muon_generalTracks_Z_Run2018_UL_SingleMuonTriggers.root");
                    _hmuontrg = dynamic_cast<TH2F *>(muontrg.Get("NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight_abseta_pt"));
                    _hmuontrg->SetDirectory(0);
                    muontrg.Close();

                    TFile muonid("data/MuonSF/UL2018/Efficiencies_muon_generalTracks_Z_Run2018_UL_ID.root");
                    _hmuonid = dynamic_cast<TH2F *>(muonid.Get("NUM_TightID_DEN_TrackerMuons_abseta_pt"));
                    _hmuonid->SetDirectory(0);
                    muonid.Close();

                    TFile muoniso("data/MuonSF/UL2018/Efficiencies_muon_generalTracks_Z_Run2018_UL_ISO.root");
                    _hmuoniso = dynamic_cast<TH2F *>(muoniso.Get("NUM_TightRelIso_DEN_TightIDandIPCut_abseta_pt"));
                    _hmuoniso->SetDirectory(0);
                    muoniso.Close();

                    _muontrg = new WeightCalculatorFromHistogram(_hmuontrg);
                    _muonid = new WeightCalculatorFromHistogram(_hmuonid);
                    _muoniso = new WeightCalculatorFromHistogram(_hmuoniso);
                }
            }
        }
}

NanoAODAnalyzerrdframe::~NanoAODAnalyzerrdframe() {
    // TODO Auto-generated destructor stub
    //

    std::cout<< "-------------------------------------------------------------------" << std::endl;
    cout << "Creating output root file :  " << endl;

        for (auto afile:_outrootfilenames)
        {
                cout << afile << " ";
                cout<<endl;
                std::cout<< "-------------------------------------------------------------------" << std::endl;

                //_outrootfile = new TFile(afile.c_str(), "UPDATE");
        }
        std::cout<< "-------------------------------------------------------------------" << std::endl;
        std::cout << "END...  :) " << std::endl;
}

bool NanoAODAnalyzerrdframe::isDefined(string v)
{
    auto result = std::find(_originalvars.begin(), _originalvars.end(), v);
    if (result != _originalvars.end()) return true;
    else return false;
}

/*void NanoAODAnalyzerrdframe::setTree(TTree *t, std::string outfilename)
{

    this->setupAnalysis();
}*/

void NanoAODAnalyzerrdframe::setupAnalysis()
{
    // Must sequentially define objects.
    //defineMoreVars();
    //defineCuts();
    //bookHists();
    setupCuts_and_Hists();
    setupTree();
}

bool NanoAODAnalyzerrdframe::readjson()
{
    auto isgoodjsonevent = [this](unsigned int runnumber, unsigned int lumisection)
        {
            auto key = std::to_string(runnumber);

            bool goodeventflag = false;

            if (jsonroot.isMember(key))
            {
                Json::Value runlumiblocks = jsonroot[key];
                for (unsigned int i=0; i<runlumiblocks.size() && !goodeventflag; i++)
                {
                    auto lumirange = runlumiblocks[i];
                    if (lumisection >= lumirange[0].asUInt() && lumisection <= lumirange[1].asUInt()) goodeventflag = true;
                }
                return goodeventflag;
            }
            else
            {
                //cout << "Run not in json " << runnumber << endl;
                return false;
            }

        };

    if (_jsonfname != "")
    {
        std::ifstream jsoninfile;
        jsoninfile.open(_jsonfname);

        if (jsoninfile.good())
        {
            jsoninfile >> jsonroot;
            /*
            auto runlumiblocks =  jsonroot["276775"];
            for (auto i=0; i<runlumiblocks.size(); i++)
            {
                auto lumirange = runlumiblocks[i];
                cout << "lumi range " << lumirange[0] << " " << lumirange[1] << endl;
            }
            */
            _rlm = _rlm.Define("goodjsonevent", isgoodjsonevent, {"run", "luminosityBlock"}).Filter("goodjsonevent");
            _jsonOK = true;
            return true;
        }
        else
        {
            cout << "Problem reading json file " << _jsonfname << endl;
            return false;
        }
    }
    else
    {
        cout << "no JSON file given" << endl;
        return true;
    }
}

void NanoAODAnalyzerrdframe::setupJetMETCorrection(string globaltag, string jetalgo)
{
    if (globaltag != "")
    {
        cout << "Applying new JetMET corrections. GT: "+globaltag+" on jetAlgo: "+jetalgo << endl;
        string basedirectory = "data/jme/";

        string datamcflag = "";
        if (_isData) datamcflag = "DATA";
        else datamcflag = "MC";

        // set file names that contain the parameters for corrections
        string dbfilenamel1 = basedirectory+globaltag+"_"+datamcflag+"_L1FastJet_"+jetalgo+".txt";
        string dbfilenamel2 = basedirectory+globaltag+"_"+datamcflag+"_L2Relative_"+jetalgo+".txt";
        string dbfilenamel3 = basedirectory+globaltag+"_"+datamcflag+"_L3Absolute_"+jetalgo+".txt";
        string dbfilenamel2l3 = basedirectory+globaltag+"_"+datamcflag+"_L2L3Residual_"+jetalgo+".txt";

        JetCorrectorParameters *L1JetCorrPar = new JetCorrectorParameters(dbfilenamel1);

        if (!L1JetCorrPar->isValid())
        {
            std::cerr << "L1FastJet correction parameters not read" << std::endl;
            exit(1);
        }
                JetCorrectorParameters *L2JetCorrPar = new JetCorrectorParameters(dbfilenamel2);
        if (!L2JetCorrPar->isValid())
        {
            std::cerr << "L2Relative correction parameters not read" << std::endl;
            exit(1);
        }
        JetCorrectorParameters *L3JetCorrPar = new JetCorrectorParameters(dbfilenamel3);
        if (!L3JetCorrPar->isValid())
        {
            std::cerr << "L3Absolute correction parameters not read" << std::endl;
            exit(1);
        }
                JetCorrectorParameters *L2L3JetCorrPar = new JetCorrectorParameters(dbfilenamel2l3);
        if (!L2L3JetCorrPar->isValid())
        {
            std::cerr << "L2L3Residual correction parameters not read" << std::endl;
            exit(1);
        }

        // to apply all the corrections, first collect them into a vector
        std::vector<JetCorrectorParameters> jetc;
        jetc.push_back(*L1JetCorrPar);
        jetc.push_back(*L2JetCorrPar);
        jetc.push_back(*L3JetCorrPar);
        jetc.push_back(*L2L3JetCorrPar);

        // apply the various corrections
        _jetCorrector = new FactorizedJetCorrector(jetc);

        // object to calculate uncertainty
        string dbfilenameunc = basedirectory+globaltag+"_"+datamcflag+"_Uncertainty_"+jetalgo+".txt";
        _jetCorrectionUncertainty = new JetCorrectionUncertainty(dbfilenameunc);
    }
    else
    {
        cout << "Not applying new JetMET corrections. Using NanoAOD as is." << endl;
        std::cout<< "-------------------------------------------------------------------" << std::endl;

    }
}


// Adapted from https://github.com/cms-nanoAOD/nanoAOD-tools/blob/master/python/postprocessing/modules/jme/jetRecalib.py
// and https://github.com/cms-nanoAOD/nanoAOD-tools/blob/master/python/postprocessing/modules/jme/JetRecalibrator.py
void NanoAODAnalyzerrdframe::applyJetMETCorrections()
{
    auto appcorrlambdaf = [this](floats jetpts, floats jetetas, floats jetAreas, floats jetrawf, float rho)->floats
    {
        floats corrfactors;
        corrfactors.reserve(jetpts.size());
        for (unsigned int i =0; i<jetpts.size(); i++)
        {
            float rawjetpt = jetpts[i]*(1.0-jetrawf[i]);
            _jetCorrector->setJetPt(rawjetpt);
            _jetCorrector->setJetEta(jetetas[i]);
            _jetCorrector->setJetA(jetAreas[i]);
            _jetCorrector->setRho(rho);
            float corrfactor = _jetCorrector->getCorrection();
            corrfactors.emplace_back(rawjetpt * corrfactor);
        }
        return corrfactors;
    };

    auto jecuncertaintylambdaf= [this](floats jetpts, floats jetetas, floats jetAreas, floats jetrawf, float rho)->floats
        {
            floats uncertainties;
            uncertainties.reserve(jetpts.size());
            for (unsigned int i =0; i<jetpts.size(); i++)
            {
                float rawjetpt = jetpts[i]*(1.0-jetrawf[i]);
                _jetCorrector->setJetPt(rawjetpt);
                _jetCorrector->setJetEta(jetetas[i]);
                _jetCorrector->setJetA(jetAreas[i]);
                _jetCorrector->setRho(rho);
                float corrfactor = _jetCorrector->getCorrection();
                _jetCorrectionUncertainty->setJetPt(corrfactor*rawjetpt);
                _jetCorrectionUncertainty->setJetEta(jetetas[i]);
                float unc = _jetCorrectionUncertainty->getUncertainty(true);
                uncertainties.emplace_back(unc);

            }
            return uncertainties;
        };

    auto metcorrlambdaf = [](float met, float metphi, floats jetptsbefore, floats jetptsafter, floats jetphis)->float
    {
        auto metx = met * cos(metphi);
        auto mety = met * sin(metphi);
        for (unsigned int i=0; i<jetphis.size(); i++)
        {
            if (jetptsafter[i]>15.0)
            {
                metx -= (jetptsafter[i] - jetptsbefore[i])*cos(jetphis[i]);
                mety -= (jetptsafter[i] - jetptsbefore[i])*sin(jetphis[i]);
            }
        }
        return float(sqrt(metx*metx + mety*mety));
    };

    auto metphicorrlambdaf = [](float met, float metphi, floats jetptsbefore, floats jetptsafter, floats jetphis)->float
    {
        auto metx = met * cos(metphi);
        auto mety = met * sin(metphi);
        for (unsigned int i=0; i<jetphis.size(); i++)
        {
            if (jetptsafter[i]>15.0)
            {
                metx -= (jetptsafter[i] - jetptsbefore[i])*cos(jetphis[i]);
                mety -= (jetptsafter[i] - jetptsbefore[i])*sin(jetphis[i]);
            }
        }
        return float(atan2(mety, metx));
    };

    if (_jetCorrector != 0)
    {
        _rlm = _rlm.Define("Jet_pt_corr", appcorrlambdaf, {"Jet_pt", "Jet_eta", "Jet_area", "Jet_rawFactor", "fixedGridRhoFastjetAll"});
        _rlm = _rlm.Define("Jet_pt_relerror", jecuncertaintylambdaf, {"Jet_pt", "Jet_eta", "Jet_area", "Jet_rawFactor", "fixedGridRhoFastjetAll"});
        _rlm = _rlm.Define("Jet_pt_corr_up", "Jet_pt_corr*(1.0f + Jet_pt_relerror)");
        _rlm = _rlm.Define("Jet_pt_corr_down", "Jet_pt_corr*(1.0f - Jet_pt_relerror)");
        _rlm = _rlm.Define("MET_pt_corr", metcorrlambdaf, {"MET_pt", "MET_phi", "Jet_pt", "Jet_pt_corr", "Jet_phi"});
        _rlm = _rlm.Define("MET_phi_corr", metphicorrlambdaf, {"MET_pt", "MET_phi", "Jet_pt", "Jet_pt_corr", "Jet_phi"});
        _rlm = _rlm.Define("MET_pt_corr_up", metcorrlambdaf, {"MET_pt", "MET_phi", "Jet_pt", "Jet_pt_corr_up", "Jet_phi"});
        _rlm = _rlm.Define("MET_phi_corr_up", metphicorrlambdaf, {"MET_pt", "MET_phi", "Jet_pt", "Jet_pt_corr_up", "Jet_phi"});
        _rlm = _rlm.Define("MET_pt_corr_down", metcorrlambdaf, {"MET_pt", "MET_phi", "Jet_pt", "Jet_pt_corr_down", "Jet_phi"});
        _rlm = _rlm.Define("MET_phi_corr_down", metphicorrlambdaf, {"MET_pt", "MET_phi", "Jet_pt", "Jet_pt_corr_down", "Jet_phi"});
    }
}



// This function is newly added for getting event weight with selected objects
//void NanoAODAnalyzerrdframe::calculateEvWeight()
//{
/*
 */
//}


void NanoAODAnalyzerrdframe::helper_1DHistCreator(std::string hname, std::string title, const int nbins, const double xlow, const double xhi, std::string rdfvar, std::string evWeight, RNode *anode)
{
    cout << "1DHistCreator " << hname  << endl;
    RDF1DHist histojets = anode->Histo1D({hname.c_str(), title.c_str(), nbins, xlow, xhi}, rdfvar, evWeight); // Fill with weight given by evWeight
    _th1dhistos[hname] = histojets;
    //return true;
};

// Automatically loop to create
void NanoAODAnalyzerrdframe::setupCuts_and_Hists()
{
    cout << "setting up definitions, cuts, and histograms" <<endl;

    for ( auto &c : _varinfovector)
    {
        //addvar
        if (c.mincutstep.length()==0) _rlm = _rlm.Define(c.varname, c.vardefinition);
    }

    for (auto &x : _hist1dinfovector)
    {
        std::string hpost = "_nocut";

        if (x.mincutstep.length()==0)
        {
            helper_1DHistCreator(std::string(x.hmodel.fName)+hpost,  std::string(x.hmodel.fTitle)+hpost, x.hmodel.fNbinsX, x.hmodel.fXLow, x.hmodel.fXUp, x.varname, x.weightname, &_rlm);
        }
    }

    _rnt.setRNode(&_rlm);

    for (auto acut : _cutinfovector)
    {

        std::string cutname = "cut"+ acut.idx;
        std::cout << "cut name  : " << cutname  << std::endl;
        std::cout << "cut id  : " << acut.idx  << std::endl;

        std::string hpost = "_"+cutname;
        RNode *r = _rnt.getParent(acut.idx)->getRNode();
        auto rnext = new RNode(r->Define(cutname, acut.cutdefinition));
        *rnext = rnext->Filter(cutname);


        for ( auto &c : _varinfovector)
        
        {
            if (acut.idx.compare(c.mincutstep)==0) *rnext = rnext->Define(c.varname, c.vardefinition);
        }
        for (auto &x : _hist1dinfovector)
        
        {
            if (acut.idx.compare(0, x.mincutstep.length(), x.mincutstep)==0)
            {
                helper_1DHistCreator(std::string(x.hmodel.fName)+hpost,  std::string(x.hmodel.fTitle)+hpost, x.hmodel.fNbinsX, x.hmodel.fXLow, x.hmodel.fXUp, x.varname, x.weightname, rnext);
            }
        }
        _rnt.addDaughter(rnext, acut.idx);
    }
}

void NanoAODAnalyzerrdframe::add1DHist(TH1DModel histdef, std::string variable, std::string weight,string mincutstep)
{
    _hist1dinfovector.push_back({histdef, variable, weight, mincutstep});
}

void NanoAODAnalyzerrdframe::drawHists(RNode t)
{
    cout << "processing" <<endl;
    t.Count();

}

void NanoAODAnalyzerrdframe::addVar(varinfo v)
{
    _varinfovector.push_back(v);
}

void NanoAODAnalyzerrdframe::addVartoStore(string varname)
{
    // varname is assumed to be a regular expression.
    // e.g. if varname is "Muon_eta" then "Muon_eta" will be stored
    // if varname=="Muon_.*", then any branch name that starts with "Muon_" string willmbe saved
    _varstostore.push_back(varname);

}

void NanoAODAnalyzerrdframe::setupTree()
{   if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }


    vector<RNodeTree *> rntends;
    _rnt.getRNodeLeafs(rntends);
    for (auto arnt: rntends)
    {
        RNode *arnode = arnt->getRNode();
        string nodename = arnt->getIndex();
        vector<string> varforthistree;
        std::map<string, int> varused;

        for (auto varname: _varstostore)
        {
            bool foundmatch = false;
            std::regex b(varname);
            for (auto a: arnode->GetColumnNames())
            {
                if (std::regex_match(a, b) && varused[a]==0)
                {
                    varforthistree.push_back(a);
                    varused[a]++;
                    foundmatch = true;
                }
            }
            if (!foundmatch)
            {
                cout << varname << " not found at "<< nodename << endl;
            }

        }
        _varstostorepertree[nodename]  = varforthistree;
    }

}

void NanoAODAnalyzerrdframe::addCuts(string cut, string idx)
{
    _cutinfovector.push_back({cut, idx});

}


void NanoAODAnalyzerrdframe::run(bool saveAll, string outtreename)
{
     //ROOT::EnableImplicitMT();

    vector<RNodeTree *> rntends;
    _rnt.getRNodeLeafs(rntends);
    //_rnt.Print();
    //cout << " rntends size ==" <<rntends.size() << endl;

    for (auto arnt: rntends)
    {
        string nodename = arnt->getIndex();
        string outname = _outfilename;
        if (rntends.size()>1) outname.replace(outname.find(".root"), 5, "_"+nodename+".root");

        _outrootfilenames.push_back(outname);
        RNode *arnode = arnt->getRNode();
        std::cout<< "-------------------------------------------------------------------" << std::endl;
        cout<<" cut " ;
        cout << arnt->getIndex();
        //std::cout<< "-------------------------------------------------------------------" << std::endl;
        if (saveAll) {
            arnode->Snapshot(outtreename, outname);
        }
        else {
            // use the following if you want to store only a few variables
            //arnode->Snapshot(outtreename, outname, _varstostore);
            cout << " writing branches" << endl;
            std::cout<< "-------------------------------------------------------------------" << std::endl;
            for (auto bname: _varstostorepertree[nodename])
            {
                //std::cout << bname << " " <<std::endl;
            }
                        //cout<<endl;
            arnode->Snapshot(outtreename, outname, _varstostorepertree[nodename]);
        } 
        _outrootfile = new TFile(outname.c_str(), "UPDATE");
        for (auto &h : _th1dhistos)
        {
			if (h.second.GetPtr() != nullptr) {
				h.second.GetPtr()->Print();
				h.second.GetPtr()->Write();
			}
        }
        _outrootfile->Write(0, TObject::kOverwrite);
        _outrootfile->Close();

    }

}
//--------------------------------------------------------------------------------------------------------//
// Trigger selection - added for FLY project group
//Checking HLTs in the input root file
//--------------------------------------------------------------------------------------------------------//
std::string NanoAODAnalyzerrdframe::setHLT(std::string str_HLT){
    if(debug){
    std::cout<< "================================//=================================" << std::endl;
    std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
    std::cout<< "================================//=================================" << std::endl;
    }
    if(str_HLT != "" ){

        bool ctrl_HLT =isDefined(str_HLT);
        std::string output;
        if(ctrl_HLT){
            output=str_HLT;
            std::cout<<"HLT : " <<  str_HLT  << " : SUCCESSFULLY FOUND!!"<< std::endl;
        }else{
            std::cout<<"HLT : " <<  str_HLT  << " : CAN NOT BE FOUND "<< std::endl;
            std::cout<< "Check HLT branches in the input root file!!" << std::endl;
            std::cout<< "EXITING PROGRAM!!" << std::endl;

            exit(1);
        }
        return output;

    }else{ // fill the HLT names in a vector according to each year
            std::vector<string> V_output;
            if(run_year==2016){
                HLTGlobalNames=HLT2016Names;
            }else if (run_year==2017){
                HLTGlobalNames=HLT2017Names;
            }else if(run_year==2018){
                HLTGlobalNames=HLT2018Names;
            }

            //loop on HLTs
            for (size_t i = 0; i < HLTGlobalNames.size(); i++)
            {
                /* code */
                bool ctrl_HLT = isDefined(HLTGlobalNames[i]);
                if(ctrl_HLT){
                    V_output.push_back(HLTGlobalNames[i]);
                }

            }
            std::string output_HLT;
            if(!V_output.empty()){
                for (size_t i = 0; i < V_output.size() ; i++)
                {
                    if(i!=V_output.size()-1){
                    output_HLT += V_output[i] + "==1 || " ;
                    }else{
                        output_HLT += V_output[i] + "==1 " ;
                    }
                }
            }else{
                std::cout<< " Not matched with any HLT Triggers! Please check the HLT Names in the inputfile " << std::endl;
                std::cout<< "EXITING PROGRAM!!" << std::endl;
                exit(1);
            }
            std::cout<< " HLT names =  " <<  output_HLT  << std::endl;
            return output_HLT;

    }

}
// added for FLY project group
//control all branch names using in the addCuts function
std::string NanoAODAnalyzerrdframe::ctrlBranchName(std::string str_Branch){
    bool ctrl_Branch =isDefined(str_Branch);
    std::string output;
    if(ctrl_Branch){
        output=str_Branch;
    }else{
        std::cout<<"Branch : " <<  str_Branch  << " : CAN NOT BE FOUND "<< std::endl;
        std::cout<< "Check your branches in the input root file!!" << std::endl;
        std::cout<< "EXITING PROGRAM!!" << std::endl;

        exit(1);
    }
    return output;
}
//--------------------------------------------------------------------------------------------------------//
// ObjectID definitions - added for FLY project group
//cut-based ID Fall17 V2 (1:veto, 2:loose, 3:medium, 4:tight)
//--------------------------------------------------------------------------------------------------------//
std::string NanoAODAnalyzerrdframe::ElectronID(int cutbasedID){
    //double Electron_eta;
    //double Electron_pt;
    if (cutbasedID==1)std::cout<< " VETO Electron ID requested == " << cutbasedID <<std::endl;
    if (cutbasedID==2)std::cout<< " LOOSE Electron ID requested == " << cutbasedID <<std::endl;
    if (cutbasedID==3)std::cout<< " MEDIUM Electron ID requested == " << cutbasedID <<std::endl;
    if (cutbasedID==4)std::cout<< " TIGHT Electron ID requested == " << cutbasedID <<std::endl;
    std::cout<< "-------------------------------------------------------------------" << std::endl;

    if (cutbasedID<0 || cutbasedID>4){
        std::cout<< "ERROR!! Wrong Electron ID requested == " << cutbasedID << "!! Can't be applied" <<std::endl;
        std::cout<< "Please select ElectronID from 1 to 4 " <<std::endl;
        std::cout<< "-------------------------------------------------------------------" << std::endl;
        std::cout<< "EXITING PROGRAM!!" << std::endl;
        exit(1);
    }
/*if(run_year==2018 && _isUL){
    if(cutbasedID==2 ){
        Electron_eta=2.5;
    Electron_pt=10;
    }else if (cutbasedID==3){ 
        Electron_eta=2.4;
        Electron_pt=10;

    }
}*/

//Rdataframe look for the variables in the intput Ttree..
std::string output = Form("Electron_cutBased == %d ",cutbasedID);
//std::string output = Form("Electron_cutBased == %d &&  abs(Electron_eta)<%f && Electron_pt<%f",cutbasedID,  Electron_eta, Electron_pt);

return output;
}

//--------------------------------------------------------------------------------------------------------//
// MuonID defintion should be added
//--------------------------------------------------------------------------------------------------------//
std::string NanoAODAnalyzerrdframe::MuonID(int cutbasedID){

    //if (cutbasedID==1)std::cout<< "You selected Veto Muon ID == " << cutbasedID <<std::endl;
    if (cutbasedID==2)std::cout<< " LOOSE Muon ID requested == " << cutbasedID <<std::endl;
    if (cutbasedID==3)std::cout<< " MEDIUM Muon ID requested == " << cutbasedID <<std::endl;
    if (cutbasedID==4)std::cout<< " TIGHT Muon ID requested == " << cutbasedID <<std::endl;
    std::cout<< "-------------------------------------------------------------------" << std::endl;

    if (cutbasedID<1 || cutbasedID>4){
        std::cout<< "ERROR!! Wrong Muon ID requested == " << cutbasedID << "!! Can't be applied" <<std::endl;
        std::cout<< "Please select Muon ID from 2 to 4 " <<std::endl;
        std::cout<< "-------------------------------------------------------------------" << std::endl;
        std::cout<< "EXITING PROGRAM!!" << std::endl;

        exit(1);
    }

    /*if(cutbasedID<1 || cutbasedID>4){
        std::cout<< "Error Wrong Muon ID requested == " << cutbasedID << "!! Can't be applied" <<std::endl;
        std::cout<< "Please select number from 1 to 4 " <<std::endl;
    }*/

    /*
      code
    */
}

//--------------------------------------------------------------------------------------------------------//
// JetID defintion should be added
//--------------------------------------------------------------------------------------------------------//
/*In nanoAOD format, there is flag called Jet_jetId corresponding to the different working points described in the recommendations above. 
This flag represents: passlooseID*1+passtightID*2+passtightLepVetoID*4. Then,

For 2016 samples:
jetId==1 means: pass loose ID, fail tight, fail tightLepVeto
jetId==3 means: pass loose and tight ID, fail tightLepVeto
jetId==7 means: pass loose, tight, tightLepVeto ID.

For 2017 and 2018 samples:

jetId==2 means: pass tight ID, fail tightLepVeto
jetId==6 means: pass tight and tightLepVeto ID.
*/
//--------------------------------------------------------------------------------------------------------//

std::string NanoAODAnalyzerrdframe::JetID(int cutbasedID){


    if(cutbasedID<1 || cutbasedID>7){
        std::cout<< "Error Wrong JET ID requested == " << cutbasedID << "!! Can't be applied" <<std::endl;
        std::cout<< "Please select number from 1 to 7 " <<std::endl;

    }

    /*
    code
    */
}


