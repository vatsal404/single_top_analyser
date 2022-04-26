/*
 * BaseAnalyser.cpp: subclas -> inherited from main class NanoAODAnalyzerrdframe (Created on: Sep 30, 2018-Author: suyong)
 *
 *  Created on: Dec 18, 2021
 *      modified:cdozen
 */

#include "BaseAnalyser.h"
#include "utility.h"
#include "Math/GenVector/VectorUtil.h"
#include <ROOT/RDataFrame.hxx>
#include <ROOT/TProcessExecutor.hxx>
#include <TStopwatch.h>




BaseAnalyser::BaseAnalyser(TTree *t, std::string outfilename, std::string year, std::string runtype,std::string syst, std::string jsonfname, string globaltag, int nthreads,int isDATA)
:NanoAODAnalyzerrdframe(t, outfilename, year, runtype,syst, jsonfname, globaltag, nthreads, isDATA)
{//initiliaze the HLT names in your analyzer class
    HLT2018Names= {"HLT_PFHT380_SixJet32_DoubleBTagCSV_p075",
                    "HLT_PFHT300PT30_QuadPFJet_75_60_45_40_TriplePFBTagCSV_3p0",
                    "HLT_PFHT330PT30_QuadPFJet_75_60_45_40_TriplePFBTagDeepCSV_4p5",
                    "HLT_PFJet550","HLT_PFHT400_FivePFJet_100_100_60_30_30_DoublePFBTagDeepCSV_4p5",
                    "HLT_PFHT400_FivePFJet_120_120_60_30_30_DoublePFBTagDeepCSV_4p5"};
	HLT2016Names= {"Name1","Name2"};
	HLT2017Names= {"Name1","Name2"};

}
//=============================Event Selection==================================================//
// Cuts to be applied in order
//==============================================================================================//
void BaseAnalyser::defineCuts()
{
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }
   // count some entries using ranges
    auto Nentry = _rlm.Count();
    // This is how you can express a range of the first 100 entries
    //_rlm = _rlm.Range(0, 100);
    //auto Nentry_100 = _rlm.Count();
    // This is how you pick all entries from 10 onwards
    //_rlm = _rlm.Range(10, 0);
    //auto Nentry_10_end = _rlm.Count();
    // We can use a stride too, in this case we pick an event every 3
    //_rlm = _rlm.Range(10, 0, 3);
    //auto Nentry_10_end_3 = _rlm.Count();
    std::cout<< "-------------------------------------------------------------------" << std::endl;
    cout << "Usage of ranges:\n"
        << " - All entries: " << *Nentry << endl;
        //<< " - Entries from 0 to 100: " << *Nentry_100 << endl;
        //<< " - Entries from 10 onwards: " << *Nentry_10_end << endl;
        //<< " - Entries from 100 onwards in steps of 3: " << *Nentry_100_end_3 << endl;
    std::cout<< "-------------------------------------------------------------------" << std::endl;


    //MinimalSelection to filter events
    addCuts("nMuon > 0 && nElectron > 0", "0");
    addCuts(setHLT(),"1"); //HLT cut buy checking HLT names in the root file

    //addCuts("Sum(goodMuons)>0 && (Selected_muon_charge[0]!=Selected_muon_charge[1])", "00");
    //addCuts("Sum(Electron_charge>0)==2 && (Sum(Muon_charge==1)>=2 || Sum(Muon_charge==-1)>=2)","00");


}

//===============================Find Good Electrons===========================================//
//: Define Good Electrons in rdata frame
//=============================================================================================//

void BaseAnalyser::selectElectrons()
{
    cout << "select good electrons" << endl;
    if (debug){
    std::cout<< "================================//=================================" << std::endl;
    std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
    std::cout<< "================================//=================================" << std::endl;
    }
    //string veto_electron_str = Form ("Electron_pt>%f && abs(Electron_eta)<%f && %s",cutdb->Md_Electron_pT,cutdb->Md_Electron_eta, ElectronID(2).c_str());
    //_rlm = _rlm.Define("goodElectrons", veto_electron_str.c_str());

    _rlm = _rlm.Define("goodElectrons", ElectronID(2)); //without pt-eta cuts
    _rlm = _rlm.Define("goodElectrons_pt", "Electron_pt[goodElectrons]")
                .Define("goodElectrons_eta", "Electron_eta[goodElectrons]")
                .Define("goodElectrons_phi", "Electron_phi[goodElectrons]")
                .Define("goodElectrons_mass", "Electron_mass[goodElectrons]")
                .Define("goodElectrons_idx", ::good_idx, {"goodElectrons"})
                .Define("NgoodElectrons", "int(goodElectrons_pt.size())");

    //-------------------------------------------------------
    //generate electron 4vector
    //-------------------------------------------------------
    _rlm = _rlm.Define("goodElectron_4Vecs", ::generate_4vec, {"goodElectrons_pt", "goodElectrons_eta", "goodElectrons_phi", "goodElectrons_mass"});

}

//===============================Find Good Muons===============================================//
//: Define Good Muons in rdata frame
//=============================================================================================//
void BaseAnalyser::selectMuons()
{

    cout << "select good muons" << endl;
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }



}

//=================================Select Jets=================================================//
//check the twiki page :    https://twiki.cern.ch/twiki/bin/view/CMS/JetID
//to find jetId working points for the purpose of  your analysis.
    //jetId==2 means: pass tight ID, fail tightLepVeto
    //jetId==6 means: pass tight ID and tightLepVeto ID.
//=============================================================================================//
void BaseAnalyser::selectJets()
{

    cout << "select good jets" << endl;
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }


}

//=================================Check Overlaps==============================================//
//To find the clean jtes: remove jets overlapping with any lepton with lambda function
//=============================================================================================//
void BaseAnalyser::removeOverlaps()
{
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }

    //-------------------------------------------------------
    // lambda function for checking overlapped jets with leptons
    //-------------------------------------------------------
    auto checkoverlap = [](FourVectorVec &seljets, FourVectorVec &sellep)
        {
            doubles mindrlepton;
                for (auto ajet: seljets)
                {
                    auto mindr = 6.0;
                    for ( auto alepton : sellep )
                    {
                        auto dr = ROOT::Math::VectorUtil::DeltaR(ajet, alepton);
                        if (dr < mindr) mindr = dr;
                        }
                    int out = mindr > 0.4 ? 1 : 0;
                    mindrlepton.emplace_back(out);
                }
            return mindrlepton;
        };
    //------------------jet-muon separation-------------------
    //Overlap removal with muon (used for btagging SF)
    //-------------------------------------------------------
    cout << "checking overlap with muons and jets" << endl;

    //=================================Find b-tag jets==============================================//
    if(run_year==2016){
       // _rlm = _rlm.Define("btagcuts", "Selmu_jetbtag>0.7221");
    }else if (run_year==2017){//(_isRun17){
       // _rlm = _rlm.Define("btagcuts", "Selmu_jetbtag>0.7476");
    }else if(run_year==2018){//if(_isRun18){

       // _rlm = _rlm.Define("btagcuts", "Selmu_jetbtag>0.7100");
    }
    //------------------btag-jets selection-------------------


}

//-------------------------------------------------------
//Event weight calculation
//-------------------------------------------------------
void BaseAnalyser::calculateEvWeight()
{
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }
    //-------------------------------------------------------
    // Muon Scale Factor
    //-------------------------------------------------------
    //cout<<"Getting Muon Scale Factors"<<endl;
    auto muonSF = [this](floats &pt, floats &eta)->float {
        float weight = 1.0;
        if(pt.size() > 0){
            for(unsigned int i=0; i<pt.size(); i++){
                float trg_SF = _muontrg->getWeight(std::abs(eta[i]),pt[i]);
                float ID_SF = _muonid->getWeight(std::abs(eta[i]),pt[i]);
                float Iso_SF = _muoniso->getWeight(std::abs(eta[i]),pt[i]);
                weight *= trg_SF * ID_SF * Iso_SF;
            }
        }
        return weight;
    };
    //_rlm = _rlm.Define("evWeight_muonSF",muonSF,{"Selected_muon_pt","Selected_muon_eta"});

}
//=============================define variables==================================================//
void BaseAnalyser::defineMoreVars()
{
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }

    addVar({"good_electron1pt", "goodElectrons_pt[0]", ""});

    //================================Store variables in tree=======================================//
    // define variables that you want to store
    //==============================================================================================//
    addVartoStore("run");
    addVartoStore("luminosityBlock");
    addVartoStore("event");
    addVartoStore("evWeight.*");

    //electron
    addVartoStore("nElectron");
    addVartoStore("Electron_charge");
    addVartoStore("NgoodElectrons");

    //muon
    addVartoStore("nMuon");


}
//================================Histogram Definitions===========================================//
//add1DHist(TH1DModel histdef, std::string variable, std::string weight, string mincutstep="");
//==============================================================================================//
void BaseAnalyser::bookHists()
{
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }
    //add1DHist( {"hdimuon_dr", "Dimuon DR", 20, 0, 4}, "SSdimuon_dr", "one", "");
   // add1DHist( {"hdimuon_mass", "Dimuon mass", 20, 0, 4}, "SSdimuon_mass", "one", "");


    add1DHist( {"hnevents", "Number of Events", 2, -0.5, 1.5}, "one", "evWeight", "");

    add1DHist( {"hNgoodElectrons", "NumberofGoodElectrons", 5, 0.0, 5.0}, "NgoodElectrons", "evWeight", "");


    //===reco w candidate no cut =====//
    //add1DHist( {"hrecow_mass", "Reco Wmass", 100, 0, 200}, "reco_w_mass", "one", "");
    //add1DHist( {"hdijet_dR", "Dijet DR", 20, 0, 4}, "reco_dijet_dr", "one", "");


    //===minimum cut step:0 : defined in defineCuts() function =====//
    add1DHist( {"hgoodelectron1_pt", "good electron1_pt", 18, -2.7, 2.7}, "good_electron1pt", "evWeight", "0");

}

void BaseAnalyser::setTree(TTree *t, std::string outfilename)
{
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }

    _rd = ROOT::RDataFrame(*t);
    _rlm = RNode(_rd);

    _outfilename = outfilename;
    _hist1dinfovector.clear();
    _th1dhistos.clear();
    _varstostore.clear();
    _hist1dinfovector.clear();
    _selections.clear();

    this->setupAnalysis();
}
//================================Selected Object Definitions====================================//
void BaseAnalyser::setupAnalysis()
{
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }

    if (_isData) _jsonOK = readjson();
    // Event weight for data it's always one. For MC, it depends on the sign

    _rlm = _rlm.Define("one", "1.0");
        if(_isSkim){
                if(_isData){
                        _rlm = _rlm.Define("unitGenWeight","one")
                                    .Define("pugenWeight","one");
                }else{
                        _rlm = _rlm.Define("unitGenWeight","genWeight != 0 ? genWeight/abs(genWeight) : 0")
                                    .Define("puWeight",[this](float x) {return _puweightcalc->getWeight(x);}, {"Pileup_nTrueInt"})
                                    .Define("puWeight_plus",[this](float x) {return _puweightcalc_plus->getWeight(x);}, {"Pileup_nTrueInt"})
                                    .Define("puWeight_minus",[this](float x) {return _puweightcalc_minus->getWeight(x);}, {"Pileup_nTrueInt"})
                                    .Define("pugenWeight", "unitGenWeight * puWeight");
                }
        }else if(_isData){
                _rlm = _rlm.Define("evWeight","one")
                            .Define("evWeight_muonSF","one")
                            .Define("evWeight_leptonSF","one");
                            //.Define("btagWeight_DeepFlavBrecalc","one");
        }else{
                _rlm = _rlm.Define("unitGenWeight","genWeight != 0 ? genWeight/abs(genWeight) : 0")
                            .Define("puWeight",[this](float x) {return _puweightcalc->getWeight(x);}, {"Pileup_nTrueInt"})
                            .Define("puWeight_plus",[this](float x) {return _puweightcalc_plus->getWeight(x);}, {"Pileup_nTrueInt"})
                            .Define("puWeight_minus",[this](float x) {return _puweightcalc_minus->getWeight(x);}, {"Pileup_nTrueInt"})
                            .Define("pugenWeight", "unitGenWeight * puWeight")
                            .Define("evWeight_leptonSF","one")
                            //.Define("evWeight_pglep","one")
                            .Define("evWeight","one");
        }
//=======================================================
    // Object selection will be defined in sequence.
    // Selected objects will be stored in new vectors.
    selectElectrons();
    selectMuons();
    selectJets();
    removeOverlaps();
    //if(!_isData && !_isSkim)
    if(!_isData){
        //matchGenReco();
        calculateEvWeight();
    }
    defineCuts();
    defineMoreVars();
    bookHists();
    setupCuts_and_Hists();
    setupTree();


}

