/*
 * BaseAnalyser.cpp
 *
 *  Created on: May 6, 2022
 *      Author: suyong
 *      Developper: cdozen
 */

#include "Math/GenVector/VectorUtil.h"
#include "BaseAnalyser.h"
#include "utility.h"
#include <fstream>
#include "correction.h"
using correction::CorrectionSet;

BaseAnalyser::BaseAnalyser(TTree *t, std::string outfilename)
:NanoAODAnalyzerrdframe(t, outfilename)
{
    //initiliaze the HLT names in your analyzer class
    HLT2018Names= {"HLT_PFHT380_SixJet32_DoubleBTagCSV_p075",
                    "HLT_PFHT300PT30_QuadPFJet_75_60_45_40_TriplePFBTagCSV_3p0",
                    "HLT_PFHT330PT30_QuadPFJet_75_60_45_40_TriplePFBTagDeepCSV_4p5",
                    "HLT_PFJet550","HLT_PFHT400_FivePFJet_100_100_60_30_30_DoublePFBTagDeepCSV_4p5",
                    "HLT_PFHT400_FivePFJet_120_120_60_30_30_DoublePFBTagDeepCSV_4p5"};
	HLT2017Names= {"Name1","Name2"};
    HLT2016Names= {"Name1","Name2"};
}

// Define your cuts here
void BaseAnalyser::defineCuts()
{
	if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }
	auto Nentry = _rlm.Count();
	// This is how you can express a range of the first 100 entries
    _rlm = _rlm.Range(0, 10000);
    auto Nentry_100 = _rlm.Count();
	std::cout<< "-------------------------------------------------------------------" << std::endl;
    cout << "Usage of ranges:\n"
        << " - All entries: " << *Nentry << endl;
		//<< " - Entries from 0 to 100: " << *Nentry_100 << endl;
	std::cout<< "-------------------------------------------------------------------" << std::endl;

	//MinimalSelection to filter events
    addCuts("nMuon > 2 && nElectron > 0 && nJet>0", "0");
    //addCuts("nJet>0","00");
    //addCuts("ncleanjetspass>0","00");
	//addCuts(setHLT(),"1"); //HLT cut buy checking HLT names in the root file

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
   
    _rlm = _rlm.Define("goodElectronsID", ElectronID(2)); //without pt-eta cuts
	_rlm = _rlm.Define("goodElectrons", "goodElectronsID && Electron_pt>30.0 && abs(Electron_eta)<2.4 && Electron_pfRelIso03_all<0.15");
    _rlm = _rlm.Define("goodElectrons_pt", "Electron_pt[goodElectrons]")
                .Define("goodElectrons_eta", "Electron_eta[goodElectrons]")
                .Define("goodElectrons_phi", "Electron_phi[goodElectrons]")
                .Define("goodElectrons_mass", "Electron_mass[goodElectrons]")
                .Define("goodElectrons_idx", ::good_idx, {"goodElectrons"})
                .Define("NgoodElectrons", "int(goodElectrons_pt.size())");

    //-------------------------------------------------------
    //generate electron 4vector from selected good Electrons
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

    _rlm = _rlm.Define("goodMuonsID", MuonID(2)); //loose muons
    _rlm = _rlm.Define("goodMuons","goodMuonsID && Muon_pt > 15 && abs(Muon_eta) < 2.4 && Muon_miniPFRelIso_all < 0.40");
    _rlm = _rlm.Define("goodMuons_pt", "Muon_pt[goodMuons]") 
                .Define("goodMuons_eta", "Muon_eta[goodMuons]")
                .Define("goodMuons_phi", "Muon_phi[goodMuons]")
                .Define("goodMuons_mass", "Muon_mass[goodMuons]")
                .Define("goodMuons_charge", "Muon_charge[goodMuons]")
                .Define("goodMuons_idx", ::good_idx, {"goodMuons"})
                .Define("NgoodMuons", "int(goodMuons_pt.size())");

    //-------------------------------------------------------
    //generate muon 4vector from selected good Muons
    //-------------------------------------------------------
    _rlm = _rlm.Define("goodMuons_4vecs", ::generate_4vec, {"goodMuons_pt", "goodMuons_eta", "goodMuons_phi", "goodMuons_mass"});
   

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

    _rlm = _rlm.Define("goodJetsID", JetID(6)); //without pt-eta cuts
    _rlm = _rlm.Define("goodJets", "goodJetsID && Jet_pt>30.0 && abs(Jet_eta)<2.4 ");
    _rlm = _rlm.Define("goodJets_pt", "Jet_pt[goodJets]")
                .Define("goodJets_eta", "Jet_eta[goodJets]")
                .Define("goodJets_phi", "Jet_phi[goodJets]")
                .Define("goodJets_mass", "Jet_mass[goodJets]")
                .Define("goodJets_idx", ::good_idx, {"goodJets"});
				_rlm = _rlm.Define("goodJets_hadflav", "Jet_hadronFlavour[goodJets]");
    //goot jets deep-b tag          
	_rlm = _rlm.Define("goodJets_jetdeepbtag", "Jet_btagDeepB[goodJets]")
                .Define("goodJets_deepjetbtag", "Jet_btagDeepFlavB[goodJets]") 
                .Define("NgoodJets", "int(goodJets_pt.size())")
                .Define("goodJets_4vecs", ::generate_4vec, {"goodJets_pt", "goodJets_eta", "goodJets_phi", "goodJets_mass"});

	//select b jest within goodjets
    _rlm = _rlm.Define("btagcuts", "goodJets_deepjetbtag>0.7")
			.Define("good_bjetpt", "goodJets_pt[btagcuts]")
			.Define("good_bjeteta", "goodJets_eta[btagcuts]")
			.Define("good_bjetphi", "goodJets_phi[btagcuts]")
			.Define("good_bjetmass", "goodJets_mass[btagcuts]")
			.Define("good_bjetdeepjet", "goodJets_deepjetbtag[btagcuts]");
	
	_rlm = _rlm.Define("good_bjethadflav", "goodJets_hadflav[btagcuts]");
			
	_rlm = _rlm.Define("Ngood_bjets", "int(good_bjetpt.size())")
			.Define("good_bjet4vecs", ::generate_4vec, {"good_bjetpt", "good_bjeteta", "good_bjetphi", "good_bjetmass"});


}
//=================================Overlap function=================================================//
void BaseAnalyser::removeOverlaps()
{
    cout << "checking overlapss between jets and muons" << endl;
	// lambda function
	// for checking overlapped jets with leptons
    auto checkoverlap = [](FourVectorVec &goodjets, FourVectorVec &goodlep)
		{
			doubles mindrlepton;
			for (auto ajet: goodjets)
			{
                auto mindr = 6.0;
				for (auto alepton: goodlep)
				{
					auto dr = ROOT::Math::VectorUtil::DeltaR(ajet, alepton);
                    if (dr < mindr) mindr = dr;
                }
                int out = mindr > 0.4 ? 1 : 0;
                mindrlepton.emplace_back(out);

			}
            return mindrlepton;
	    };
	//cout << "overlap removal" << endl;
    //==============================Clean Jets==============================================//
    //Use clean jets/bjets for object selections
    //=====================================================================================//

    _rlm = _rlm.Define("muonjetoverlap", checkoverlap, {"goodJets_4vecs","goodMuons_4vecs"});
	_rlm =	_rlm.Define("Selected_jetpt", "goodJets_pt[muonjetoverlap]")
		.Define("Selected_jeteta", "goodJets_eta[muonjetoverlap]")
		.Define("Selected_jetphi", "goodJets_phi[muonjetoverlap]")
		.Define("Selected_jetmass", "goodJets_mass[muonjetoverlap]")
	
		.Define("Selected_jetbtag", "goodJets_deepjetbtag[muonjetoverlap]") //
		.Define("Selected_jethadflav", "goodJets_hadflav[muonjetoverlap]") //
		.Define("ncleanjetspass", "int(Selected_jetpt.size())")
		.Define("cleanjet4vecs", ::generate_4vec, {"Selected_jetpt", "Selected_jeteta", "Selected_jetphi", "Selected_jetmass"})
		.Define("Selected_jetHT", "Sum(Selected_jetpt)");
        
     //==============================Clean b-Jets==============================================//  
	_rlm = _rlm.Define("btagcuts2", "Selected_jetbtag>0.7")
			.Define("Selected_bjetpt", "Selected_jetpt[btagcuts2]")
			.Define("Selected_bjeteta", "Selected_jeteta[btagcuts2]")
			.Define("Selected_bjetphi", "Selected_jetphi[btagcuts2]")
			.Define("Selected_bjetmass", "Selected_jetmass[btagcuts2]")
			.Define("ncleanbjetspass", "int(Selected_bjetpt.size())")
			.Define("Selected_bjetHT", "Sum(Selected_bjetpt)")
			.Define("cleanbjet4vecs", ::generate_4vec, {"Selected_bjetpt", "Selected_bjeteta", "Selected_bjetphi", "Selected_bjetmass"});           

}

void BaseAnalyser::calculateEvWeight(){
	
	bool isCase1 = true; 
	//case1 : fixedWP correction with mujets (here medium WP) # evaluate('systematic', 'working_point', 'flavor', 'abseta', 'pt')
	//for case 1 you should use one of the btvtype = "deepJet_mujets , deepJet_incl"  in jobconfiganalysis.py
    if (!_isData) {
    	if(isCase1){

			cout<<"Case 1 B tagging SF for MC "<<endl;

			auto btv = [this](ints &hadflav,floats &etas, floats &pts)->floats
			{
			
				return ::btv_case1(_correction_btag1, _btvtype, "central","M", hadflav, etas,  pts); // defined in utility.cpp
			
			};

			auto btv_SF_up = [this](ints &hadflav,floats &etas, floats &pts)->floats
			{
				
				return ::btv_case1(_correction_btag1, _btvtype, "up","M", hadflav, etas,  pts); 
			
			};
			auto btv_SF_down = [this](ints &hadflav,floats &etas, floats &pts)->floats
			{
				
				return ::btv_case1(_correction_btag1, _btvtype, "down","M", hadflav, etas,  pts); // 
			
			};

			_rlm = _rlm.Define("btag_SF_case1",btv, {"Selected_jethadflav", "Selected_jeteta","Selected_jetpt"});
			_rlm = _rlm.Define("btag_SF_up_case1",btv_SF_up, {"Selected_jethadflav", "Selected_jeteta","Selected_jetpt"});
			_rlm = _rlm.Define("btag_SF_down_case1",btv_SF_down, {"Selected_jethadflav", "Selected_jeteta","Selected_jetpt"});
	
			// function to calculate event weight for MC events 
			auto btagweightgenerator_case1= [this](ints &hadflav, floats &etas, floats &pts)->float
			{
				double btagWeight = 1.0;
				double sfb_weightup = 1.0;
				double sfb_weightdown = 1.0;


				for (auto i=0; i<pts.size(); i++)
				{
					if (hadflav[i] != 0){
						double wb = _correction_btag1->at("deepJet_mujets")->evaluate({"central","M", int(hadflav[i]), fabs(float(etas[i])), float(pts[i])});
						btagWeight *= wb;
						//cout<<"btag weight from b/c jets :  central (b/c) = " << w  << endl; 
						//up/down
						double wb_up = _correction_btag1->at("deepJet_mujets")->evaluate({"up","M", int(hadflav[i]), fabs(float(etas[i])), float(pts[i])});
						//sfb_weightup*=wb_up;
						btagWeight *=wb_up;
						double wb_down = _correction_btag1->at("deepJet_mujets")->evaluate({"down","M", int(hadflav[i]), fabs(float(etas[i])), float(pts[i])});
						//sfb_weightdown
						btagWeight *=wb_down;
						


					}else{
						double wl = _correction_btag1->at("deepJet_incl")->evaluate({"central","M", int(hadflav[i]), fabs(float(etas[i])), float(pts[i])});
						btagWeight *= wl;
						//cout<< "  btag weight from light jets : central (light) = " <<  w2 <<endl;
						double wl_up = _correction_btag1->at("deepJet_incl")->evaluate({"up","M", int(hadflav[i]), fabs(float(etas[i])), float(pts[i])});
						//sfl_weightup*
						btagWeight *=wl_up;
						double wl_down = _correction_btag1->at("deepJet_incl")->evaluate({"down","M", int(hadflav[i]), fabs(float(etas[i])), float(pts[i])});
						//sfl_weightdown*
						btagWeight *=wl_down;
						
						
					}
				}
				
				return btagWeight;
				
			};
		
			_rlm = _rlm.Define("btagWeight_case1", btagweightgenerator_case1, {"Selected_jethadflav","Selected_jeteta",  "Selected_jetpt"});// jets after overlap
			//_rlm = _rlm.Define("evWeight", "pugenWeight*btagWeight_case1");
			
		}else{
			//for case 3 : use btvtype': 'deepJet_shape' in jobconfiganalysis.py
			cout<<"case 3 Shape correction B tagging SF for MC "<<endl;
			auto btvcentral = [this](ints &hadflav,floats &etas, floats &pts, floats &btags)->floats
			//evaluate('systematic', 'flavor', 'eta', 'pt', 'discriminator')
			{
				return ::btvcorrection(_correction_btag1, _btvtype, "central",hadflav, etas,  pts, btags); // defined in utility.cpp
			}; 
			_rlm = _rlm.Define("btag_SF_case3", btvcentral, { "Selected_jethadflav", "Selected_jeteta", "Selected_jetpt", "Selected_jetbtag"}); 
	
			// function to calculate event weight for MC events based on DeepJet algorithm
			auto btagweightgenerator3= [this](ints &hadflav, floats &etas, floats &pts, floats &btags)->float
			{
				double bweight=1.0;

				for (auto i=0; i<pts.size(); i++)
				{
					double w = _correction_btag1->at(_btvtype)->evaluate({"central", int(hadflav[i]), fabs(float(etas[i])), float(pts[i]), float(btags[i])});
					bweight *= w;
				}
				return bweight;
			};

			cout<<"Generate b-tagging weight"<<endl;
			_rlm = _rlm.Define("btagWeight_case3", btagweightgenerator3, {"Selected_jethadflav", "Selected_jeteta",  "Selected_jetpt", "Selected_jetbtag"});
			//_rlm = _rlm.Define("evWeight", "pugenWeight*btagWeight_case3");
		}
	
	//Muon ID SF and eventweight
		cout<<"muonID SF for MC "<<endl;
	
			auto muonid_sf = [this](floats &etas, floats &pts)->floats
			{
				return ::muoncorrection(_correction_muon, _muontype, "2018_UL", etas, pts, "sf"); // defined in utility.cpp

			};
			
			_rlm = _rlm.Define("muonID_SF",muonid_sf, {"goodMuons_eta","goodMuons_pt"});
			// function to calculate event weight for MC events based on DeepJet algorithm
			
			auto muonid_weightgenerator= [this](floats &etas, floats &pts)->float
			{
				double muonId_w=1.0;

						for (auto i=0; i<pts.size(); i++)
						{ //muontype=NUM_MediumID_DEN_TrackerMuons
							//if (pts[i] < 15 || (fabs(float(etas[i])))>2.4 )continue; //  for muons pts<15 -- testing purpose
							double w = _correction_muon->at(_muontype)->evaluate({"2018_UL", fabs(float(etas[i])), float(pts[i]), "sf"});
							muonId_w *= w;
							cout<<"muonID weight ==  "<< w <<endl;
						}
						return muonId_w;
			};
			_rlm = _rlm.Define("muon_id_weight", muonid_weightgenerator, {"goodMuons_eta","goodMuons_pt"});
			
			//_rlm = _rlm.Define("evWeight", " pugenWeight * btagWeight_case1 * muon_id_weight");
	

//////MUON ISO SF--> need to be updated into the muoncorrection
/*			cout<<"muon ISO SF for MC "<<endl;

			auto muonid_iso = [this](floats &etas, floats &pts)->floats
			{
				return ::muoncorrection(_correction_muon, _muontype, "2018_UL", etas, pts, "sf"); // defined in utility.cpp

			};

			_rlm = _rlm.Define("muonISO_SF",muonid_iso, {"goodMuons_eta","goodMuons_pt"});
*/
			auto muonIso_weightgenerator= [this](floats &etas, floats &pts)->float
			{
				double muonIso_w=1;
				for (auto i=0; i<pts.size(); i++)
				{
					 if (pts[i] <15 || (fabs(float(etas[i])))>2.4 )continue;
					//Muon MediumID ISO UL type: NUM_TightRelIso_DEN_MediumID
					double w = _correction_muon->at("NUM_TightRelIso_DEN_MediumID")->evaluate({"2018_UL", fabs(float(etas[i])), float(pts[i]), "sf"});
					muonIso_w *= w;
					cout<<"muon ISO  weight ==  "<< w <<endl;
				}
				return muonIso_w;
			};

			_rlm = _rlm.Define("muon_iso_weight", muonIso_weightgenerator, {"goodMuons_eta","goodMuons_pt"});

	
		//MuonID+ISO:
		_rlm = _rlm.Define("evWeight_MuonIDISO", " muon_id_weight * muon_iso_weight");
		//evWeight:
		_rlm = _rlm.Define("evWeight", " pugenWeight * btagWeight_case1 * evWeight_MuonIDISO");
		
	
	}

}
//MET

void BaseAnalyser::selectMET()
{
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }

    _rlm = _rlm.Define("goodMET_sumET","MET_sumEt>800")
                .Define("goodMET_pt","MET_pt>5");
                //.Define("goodMET_eta","MET_eta[goodMET]")
                //.Define("goodMET_phi","MET_phi[goodMET]")
                //.Define("NgoodMET","int(goodMET_pt.size())");
    //_rlm = _rlm.Define("goodMet", "MET_sumEt>600 && MET_pt>5");
    //_rlm = _rlm.Define("goodMet_pt", "MET_pt[goodMet]");

    
}

//=============================define variables==================================================//
void BaseAnalyser::defineMoreVars()
{
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }

    addVar({"good_muon1pt", "goodMuons_pt[0]", ""});

    //selected jet candidates
    addVar({"good_jet1pt", "(goodJets_pt.size()>0) ? goodJets_pt[0] : -1", ""});
    addVar({"Selected_jet1pt", "(Selected_jetpt.size()>0) ? Selected_jetpt[0] : -1", ""});
    addVar({"good_jet1eta", "goodJets_eta[0]", ""});
    addVar({"good_jet1mass", "goodJets_mass[0]", ""});

    //================================Store variables in tree=======================================//
    // define variables that you want to store
    //==============================================================================================//
    addVartoStore("run");
    addVartoStore("luminosityBlock");
    addVartoStore("event");
    addVartoStore("evWeight");

    //addVartoStore("genWeight");
    //addVartoStore("genEventSumw");

    //electron
    addVartoStore("nElectron");
    addVartoStore("Electron_charge");
    addVartoStore("NgoodElectrons");

    //muon
    addVartoStore("nMuon");
    addVartoStore("Muon_charge");
    addVartoStore("Muon_mass");
    addVartoStore("NgoodMuons");

    //jet
    addVartoStore("nJet");
    addVartoStore("Jet_pt");
    addVartoStore("NgoodJets");
    addVartoStore("goodJets_pt");
    addVartoStore("Selected_jetpt");
	addVartoStore("Selected_jeteta");
	addVartoStore("Selected_jetbtag");
	addVartoStore("Selected_jethadflav");

	addVartoStore("good_bjetdeepjet");
	addVartoStore("good_bjethadflav");//>btagcut=0.7--tight
	addVartoStore("goodJets_hadflav");
	
	//jetmet corr
    addVartoStore("Jet_pt_corr");
	addVartoStore("Jet_pt_corr_up");
	addVartoStore("Jet_pt_corr_down");
	addVartoStore("Jet_pt_relerror");
    addVartoStore("MET_pt_corr");
    addVartoStore("MET_pt");
    
	//case1 btag correction- fixed wp	
	addVartoStore("btag_SF_case1");
	addVartoStore("btag_SF_up_case1");
	addVartoStore("btag_SF_down_case1");
	addVartoStore("btagWeight_case1");
	//case3 shape correction
	addVartoStore("btag_SF_case3");
	addVartoStore("btagWeight_case3");


	//MUONID - ISO SF & WEIGHT	
	addVartoStore("muonID_SF");
	addVartoStore("muon_id_weight");
	//addVartoStore("muonISO_SF");
	addVartoStore("muon_iso_weight");
	addVartoStore("evWeight_MuonIDISO");
   

}
void BaseAnalyser::bookHists()
{
    //=================================structure of histograms==============================================//
    //add1DHist( {"hnevents", "hist_title; x_axis title; y_axis title", 2, -0.5, 1.5}, "one", "evWeight", "");
    //add1DHist( {"hgoodelectron1_pt", "good electron1_pt; #electron p_{T}; Entries / after ", 18, -2.7, 2.7}, "good_electron1pt", "evWeight", "0");
    //======================================================================================================//
    
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }
    
    //================================gen/LHE weights======================================================//
   // if(!_isData && !isDefined("genWeight")){
   //     add1DHist({"hgenWeight", "genWeight", 1001, -100, 100}, "genWeight", "one", "");
    
    /*if(isDefined("LHEWeight_originalXWGTUP")){
        add1DHist({"hLHEweight", "LHEweight", 1001, -100, 100}, "LHEWeight_originalXWGTUP", "one", "");
    }*/
   // add1DHist({"hgenEventSumw","Sum of gen Weights",1001,-8e+09,8e+09},"one","genEventSumw","");
    //====================================================================================================//
    //}
	
    add1DHist( {"hnevents", "Number of Events", 2, -0.5, 1.5}, "one", "evWeight", "");
	add1DHist( {"hnevents_no_weight", "Number of Events w/o", 2, -0.5, 1.5}, "one", "one", "");
   
    add1DHist( {"hNgoodElectrons", "NumberofGoodElectrons", 5, 0.0, 5.0}, "NgoodElectrons", "evWeight", "");
    
    add1DHist( {"hNgoodMuons", "# of good Muons ", 5, 0.0, 5.0}, "NgoodMuons", "evWeight", "");
    
    add1DHist( {"hgood_jetpt_with weight", "Good Jet pt with weight " , 100, 0, 1000} , "goodJets_pt", "evWeight", "");
	add1DHist( {"hgood_jetpt_NOWeight", "Good Jet pt no weihght " , 100, 0, 1000} , "goodJets_pt", "one", "");

    add1DHist( {"hgood_jet1pt", "Good Jet_1 pt with weight " , 100, 0, 2500} , "good_jet1pt", "evWeight", "");
    add1DHist( {"hselected_jet1pt", "SelectedJet_1 pt no weight" , 100, 0, 1000} , "Selected_jet1pt", "evWeight", "");
    add1DHist( {"hselected_jetptWithweight", "clean-Jets with weight" , 100, 0, 2500} , "Selected_jetpt", "evWeight", "");
	add1DHist( {"hselected_jetptNoweight", "clean-Jets w/o weight" , 100, 0, 2500} , "Selected_jetpt", "one", "");

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
	//_hist1dinfovector.clear();
	_selections.clear();

	this->setupAnalysis();
}
//================================Selected Object Definitions====================================//

void BaseAnalyser::setupObjects()
{
	// Object selection will be defined in sequence.
	// Selected objects will be stored in new vectors.
	selectElectrons();
	selectMuons();
	selectJets();
	removeOverlaps();
	calculateEvWeight();
	selectMET();

}
bool BaseAnalyser::readgoodjson(string goodjsonfname)
{
	auto isgoodjsonevent = [this](unsigned int runnumber, unsigned int lumisection)
		{
			auto key = std::to_string(runnumber).c_str();

			bool goodeventflag = false;


			if (jsonroot.contains(key))
			{
				for (auto &v: jsonroot[key])
				{
					if (v[0]<=lumisection && lumisection <=v[1]) goodeventflag = true;
				}
			}
			return goodeventflag;
		};

	if (goodjsonfname != "")
	{
		std::ifstream jsoninfile;
		jsoninfile.open(goodjsonfname);

		if (jsoninfile.good())
		{
			//using rapidjson
			//rapidjson::IStreamWrapper s(jsoninfile);
			//jsonroot.ParseStream(s);

			//using jsoncpp
			jsoninfile >> jsonroot;
			_rlm = _rlm.Define("goodjsonevent", isgoodjsonevent, {"run", "luminosityBlock"}).Filter("goodjsonevent");
			_jsonOK = true;
			return true;
		}
		else
		{
			cout << "Problem reading json file " << goodjsonfname << endl;
			return false;
		}
	}
	else
	{
		cout << "no JSON file given" << endl;
		return true;
	}
}

void BaseAnalyser::setupJetMETCorrection(string fname, string jettag) //data
{

    cout << "setup JETMET correction" << endl;
	// read from file 
	_correction_jerc = correction::CorrectionSet::from_file(fname);//jercfname=json
	assert(_correction_jerc->validate()); //the assert functionality : check if the parameters passed to a function are valid =1:true
	// correction type(jobconfiganalysis.py)
	cout<<"json file=="<<fname<<endl;
	_jetCorrector = _correction_jerc->compound().at(jettag);//jerctag#JSON (JEC,compound)compoundLevel="L1L2L3Res"
	cout<<"jettag =="<<jettag<<endl;
	_jetCorrectionUnc = _correction_jerc->at(_jercunctag);
}
void BaseAnalyser::applyJetMETCorrections() //data
{
    cout << "apply JETMET correction" << endl;

	auto appcorrlambdaf = [this](floats jetpts, floats jetetas, floats jetAreas, floats jetrawf, float rho)->floats
	{
		floats corrfactors;
		corrfactors.reserve(jetpts.size());
		for (auto i =0; i<jetpts.size(); i++)
		{
			float rawjetpt = jetpts[i]*(1.0-jetrawf[i]);
            //std::cout<<"jetpt===="<< jetpts[i] <<std::endl;
			//float jet_rawmass = jet_mass * (1 - jet.rawFactor)
			//std::cout<<"rawjetpt===="<< rawjetpt <<std::endl;
			float corrfactor = _jetCorrector->evaluate({jetAreas[i], jetetas[i], rawjetpt, rho});
			//std::cout<<"correction factor===="<< corrfactor <<std::endl;
			corrfactors.emplace_back(rawjetpt * corrfactor);
			//std::cout<<"rawjetpt* corrfactor ===="<< rawjetpt * corrfactor <<std::endl;

		}
        //std::cout<<"Facsss===="<< corrfactors <<std::endl;
		return corrfactors;
		
	};

	auto jecuncertaintylambdaf= [this](floats jetpts, floats jetetas, floats jetAreas, floats jetrawf, float rho)->floats
		{
			floats uncertainties;
			uncertainties.reserve(jetpts.size());
			for (auto i =0; i<jetpts.size(); i++)
			{
				float rawjetpt = jetpts[i]*(1.0-jetrawf[i]);
                
				float corrfactor = _jetCorrector->evaluate({jetAreas[i], jetetas[i], rawjetpt, rho});
				//print("\njet SF for shape correction:")
				//print(f"SF: {corrfactor}")
                
				float unc = _jetCorrectionUnc->evaluate({corrfactor*rawjetpt, jetetas[i]});
				uncertainties.emplace_back(unc);

			}
			return uncertainties;
		};

	auto metcorrlambdaf = [](float met, float metphi, floats jetptsbefore, floats jetptsafter, floats jetphis)->float
	{
		auto metx = met * cos(metphi);
		auto mety = met * sin(metphi);
		for (auto i=0; i<jetphis.size(); i++)
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
		for (auto i=0; i<jetphis.size(); i++)
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
        //cout << "jetcorrector==" <<_jetCorrector << endl;

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



//void BaseAnalyser::setupCorrections(string goodjsonfname, string pufname, string putag, string btvfname, string btvtype, string jercfname, string jerctag, string jercunctag)
void BaseAnalyser::setupCorrections(string goodjsonfname, string pufname, string putag, string btvfname, string btvtype,string muon_fname, string muontype, string jercfname, string jerctag, string jercunctag)
{
    cout << "set up Corrections!" << endl;
	if (_isData) _jsonOK = readgoodjson(goodjsonfname); // read golden json file

	if (!_isData) {

		// using correctionlib

        //Muon corrections
		_correction_muon = correction::CorrectionSet::from_file(muon_fname);
		_muontype = muontype;
		cout<< "MUON FNAME====="<<muon_fname << endl;
		cout<< "MUON type====="<<_muontype << endl;
       	assert(_correction_muon->validate());

		
		// btag corrections
		_correction_btag1 = correction::CorrectionSet::from_file(btvfname);
		_btvtype = btvtype;
		cout<< "btv FNAME====="<<btvfname << endl;
		cout<< "btv type====="<<_btvtype << endl;
		assert(_correction_btag1->validate());

		// pile up weights
		_correction_pu = correction::CorrectionSet::from_file(pufname);
		assert(_correction_pu->validate());
		_putag = putag;
		auto punominal = [this](float x) { return pucorrection(_correction_pu, _putag, "nominal", x); };
		auto puplus = [this](float x) { return pucorrection(_correction_pu, _putag, "up", x); };
		auto puminus = [this](float x) { return pucorrection(_correction_pu, _putag, "down", x); };

		if (!isDefined("puWeight")) _rlm = _rlm.Define("puWeight", punominal, {"Pileup_nTrueInt"});
		if (!isDefined("puWeight_plus")) _rlm = _rlm.Define("puWeight_plus", puplus, {"Pileup_nTrueInt"});
		if (!isDefined("puWeight_minus")) _rlm = _rlm.Define("puWeight_minus", puminus, {"Pileup_nTrueInt"});


		if (!isDefined("pugenWeight"))
		{
			_rlm = _rlm.Define("pugenWeight", [this](float x, float y){
					return (x > 0 ? 1.0 : -1.0) *y;
				}, {"genWeight", "puWeight"});
		}
	}
	_jerctag = jerctag;
	_jercunctag = jercunctag;

	setupJetMETCorrection(jercfname, _jerctag);
	applyJetMETCorrections();
}


void BaseAnalyser::setupAnalysis()
{
	if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }
	
 	cout<<"year===="<< _year<< "==runtype=== " <<  _runtype <<endl;

    //==========================================event/gen/ weights==========================================//
    // Event weight for data it's always one. For MC, it depends on the sign
    //=====================================================================================================//
   
	_rlm = _rlm.Define("one", "1.0");
	
	if (_isData && !isDefined("evWeight"))
	{
		_rlm = _rlm.Define("evWeight", [](){
				return 1.0;
			}, {} );
	}
    if(!_isData ) // Only use genWeight
    {
        //_rlm = _rlm.Define("evWeight", "genWeight");
        
       //std::cout<<"Using evWeight = genWeight"<<std::endl;

        auto sumgenweight = _rd.Sum("genWeight");
        string sumofgenweight = Form("%f",*sumgenweight);
        _rlm = _rlm.Define("genEventSumw",sumofgenweight.c_str());
        std::cout<<"Sum of genWeights = "<<sumofgenweight.c_str()<<std::endl;
	}
   
	defineCuts();
	defineMoreVars();
	bookHists();
	setupCuts_and_Hists();
	setupTree();
}

