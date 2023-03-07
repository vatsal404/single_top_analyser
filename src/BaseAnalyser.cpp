/*
 * BaseAnalyser.cpp
 *
 *  Created on: May 6, 2022
 *      Author: suyong
 *      Developper: cdozen
 */

#include "Math/GenVector/VectorUtil.h"
#include "BaseAnalyser.h"
#include "TCanvas.h"
#include "utility.h"
#include <fstream>
#include "correction.h"
using correction::CorrectionSet;

using FourVectorRVec = ROOT::VecOps::RVec<FourVector>;
using namespace ROOT::VecOps;
using RNode = ROOT::RDF::RNode;

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
    //_rlm = _rlm.Range(0, 100);
    //auto Nentry_100 = _rlm.Count();
	std::cout<< "-------------------------------------------------------------------" << std::endl;
    cout << "Usage of ranges:\n"
        << " - All entries: " << *Nentry << endl;
		//<< " - Entries from 0 to 100: " << *Nentry_100 << endl;
	std::cout<< "-------------------------------------------------------------------" << std::endl;

	//MinimalSelection to filter events
    addCuts("nMuon > 2 && nElectron > 0 && nJet>0", "0");
    addCuts("nJet>0","00");
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
    //string veto_electron_str = Form ("Electron_pt>%f && abs(Electron_eta)<%f && %s",cutdb->Md_Electron_pT,cutdb->Md_Electron_eta, ElectronID(2).c_str());
    //_rlm = _rlm.Define("goodElectrons", veto_electron_str.c_str());

    _rlm = _rlm.Define("goodElectrons", ElectronID(2)); //without pt-eta cuts
	//_rlm = _rlm.Define("goodElectrons", "Electron_pt>30.0 && abs(Electron_eta)<2.4 && Electron_pfRelIso03_all<0.15");
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

    //_rlm = _rlm.Define("goodMuons", "Muon_pt>30.0 && abs(Muon_eta)<2.4 && Muon_pfRelIso03_all<0.15");
    _rlm = _rlm.Define("goodMuons", MuonID(4));
    _rlm = _rlm.Define("goodMuons_pt", "Muon_pt[goodMuons]") 
				//.Define("goodMuons_pair_pt", "(goodMuons_pt.size()>0)? goodMuons_pt[0]+goodMuons_pt[1]:-100")
				 //.Define("selectedMuon_pt",::ClearEmptyEvent,{"goodMuons_pt"});
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
    //_rlm = _rlm.Define("goodMuons_4vecs", ::generate_4vec, {"goodMuons_pt", "goodMuons_eta", "goodMuons_phi", "goodMuons_mass"});

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

    //string jetcuts_str = Form ("Jet_pt>%f && abs(Jet_eta)<%f && Jet_jetId >= %d",cutdb->Md_jet_pT,cutdb->Md_jet_eta,6 );
    //_rlm = _rlm.Define("goodJets", jetcuts_str);

   // _rlm = _rlm.Define("goodJets", "Jet_pt>30.0 && abs(Jet_eta)<2.4 && Jet_jetId >= 6");// 
    _rlm = _rlm.Define("goodJetsID", JetID(6)); //without pt-eta cuts
     _rlm = _rlm.Define("goodJets", "goodJetsID && Jet_pt>30.0 && abs(Jet_eta)<2.4 ");
    _rlm = _rlm.Define("goodJets_pt", "Jet_pt[goodJets]")
				//.Define("goodJets_HT", "Sum(goodJets_pt)")
                .Define("goodJets_eta", "Jet_eta[goodJets]")
				//.Define("goodJets_hadflvr", "Jet_hadronFlavour[goodJets]")
                .Define("goodJets_phi", "Jet_phi[goodJets]")
                .Define("goodJets_mass", "Jet_mass[goodJets]")
                .Define("goodJets_idx", ::good_idx, {"goodJets"})
                .Define("goodJets_btag", "Jet_btagCSVV2[goodJets]")
                //.Define("Selected_jetbtag", "Jet_btagDeepFlavB[goodJets]") //select bjets within goodjets
				.Define("goodJets_deepb", "Jet_btagDeepB[goodJets]")//AK4 b-tagging DeepCSV NanoAOD name #https://twiki.cern.ch/twiki/bin/view/CMS/BtagRecommendation106XUL18
                .Define("NgoodJets", "int(goodJets_pt.size())")
                .Define("goodJets_4vecs", ::generate_4vec, {"goodJets_pt", "goodJets_eta", "goodJets_phi", "goodJets_mass"});
    

	/*///combination among goodJets:
    _rlm = _rlm.Define("dis_comb", ::distinct_comb, {"NgoodJets"})
                .Define("goodJets_4Rvecs", ::generate_4Rvec, {"goodJets_pt", "goodJets_eta", "goodJets_phi", "goodJets_mass"})
                //combination of good Jets 
                .Define("Comb_goodJet1_phi", "ROOT::VecOps::Take(goodJets_phi, dis_comb[0])")
                .Define("Comb_goodJet2_phi", "ROOT::VecOps::Take(goodJets_phi, dis_comb[1])")
                .Define("Comb_goodJets_DeltaPhi", "ROOT::VecOps::abs(ROOT::VecOps::DeltaPhi(Comb_goodJet1_phi, Comb_goodJet2_phi))") 
                .Define("Comb_goodJets_DeltaPhiMin", "ROOT::VecOps::Min(Comb_goodJets_DeltaPhi)")
                //combination of good jetvec
                .Define("Comb_goodJet1_vec", "ROOT::VecOps::Take(goodJets_4Rvecs, dis_comb[0])")
                .Define("Comb_goodJet2_vec", "ROOT::VecOps::Take(goodJets_4Rvecs, dis_comb[1])")
                .Define("Comb_goodjet12", "Comb_goodJet1_vec + Comb_goodJet2_vec");
                //inv. mass J12 from combination 
    _rlm = _rlm.Define("Comb_goodjet12_Mass", "ROOT::VecOps::Map(Comb_goodjet12, FourVecMass)") ;         
	*/

    //btagging
    _rlm = _rlm.Define("btagcuts", "goodJets_btag>0.8")
			.Define("good_bjetpt", "goodJets_pt[btagcuts]")
			.Define("good_bjeteta", "goodJets_eta[btagcuts]")
			.Define("good_bjetphi", "goodJets_phi[btagcuts]")
			.Define("good_bjetmass", "goodJets_mass[btagcuts]")
			.Define("Ngood_bjets", "int(good_bjetpt.size())")
			.Define("good_bjet4vecs", ::generate_4vec, {"good_bjetpt", "good_bjeteta", "good_bjetphi", "good_bjetmass"});



}
void BaseAnalyser::calculateEvWeight()
{
    // calculate event weight for MC only
    if (!_isData && !isDefined("evWeight")){

		_rlm = _rlm.Define("jetcutsforsf", "Jet_pt>25.0 && abs(Jet_eta)<2.4 ")
				.Define("Sel_jetforsfpt", "Jet_pt[jetcutsforsf]")
				.Define("Sel_jetforsfeta", "Jet_eta[jetcutsforsf]")
				.Define("Sel_jetforsfhad", "Jet_hadronFlavour[jetcutsforsf]")
				.Define("Sel_jetcsvv2", "Jet_btagCSVV2[jetcutsforsf]")
				.Define("Sel_jetdeepb", "Jet_btagDeepB[jetcutsforsf]");//AK4 b-tagging DeepCSV NanoAOD name #https://twiki.cern.ch/twiki/bin/view/CMS/BtagRecommendation106XUL18

		//shape correction SF		

		auto btvcentral = [this](floats &pts, floats &etas, ints &hadflav, floats &btags)->floats
		{
			return ::btvcorrection(_correction_btag1, _btvtype, "central", pts, etas, hadflav, btags); // defined in utility.cpp
		};


		_rlm = _rlm.Define("Sel_jet_deepJet_shape_central", btvcentral, {"Sel_jetforsfpt", "Sel_jetforsfeta", "Sel_jetforsfhad", "Sel_jetdeepb"});

		//_rlm = _rlm.Define("Goodjet_deepJet_shape_central", btvcentral, {"goodJets_pt", "goodJets_eta", "goodJets_hadflvr", "goodJets_deepb"});
		//_rlm = _rlm.Define("Sel_jet_deepJet_shape_central",[this](floats &pts, floats &etas, ints &hadflav, floats &btags){return ::btvcorrection(_correction_btag1, "deepJet_shape", "central", pts, etas, hadflav, btags);}
			//				, {"Sel_jetforsfpt", "Sel_jetforsfeta", "Sel_jetforsfhad", "Sel_jetdeepb"});

		// function to calculate event weight for MC events 
		auto btagweightgenerator3= [this](floats &pts, floats &etas, ints &hadflav, floats &btags)->float
		{
			double bweight=1.0;

			for (auto i=0; i<pts.size(); i++)
			{
				double w = _correction_btag1->at(_btvtype)->evaluate({"central", int(hadflav[i]), fabs(float(etas[i])), float(pts[i]), float(btags[i])});
				bweight *= w;
			}
			return bweight;
		};
		_rlm = _rlm.Define("btagWeight_DeepJetrecalc", btagweightgenerator3, {"Sel_jetforsfpt", "Sel_jetforsfeta", "Sel_jetforsfhad", "Sel_jetdeepb"});
		//_rlm = _rlm.Define("btagWeight_DeepJetrecalc", btagweightgenerator3, {"goodJets_pt", "goodJets_eta", "goodJets_hadflvr", "goodJets_deepb"});
		//_rlm = _rlm.Define("evWeight", "genWeight * btagWeight_DeepJetrecalc");
        _rlm = _rlm.Define("evWeight", "pugenWeight * btagWeight_DeepJetrecalc");
		
	
    }

}
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
		.Define("Selected_jetbtag", "goodJets_btag[muonjetoverlap]")
		.Define("ncleanjetspass", "int(Selected_jetpt.size())")
		.Define("cleanjet4vecs", ::generate_4vec, {"Selected_jetpt", "Selected_jeteta", "Selected_jetphi", "Selected_jetmass"})
		.Define("Selected_jetHT", "Sum(Selected_jetpt)")
		.Define("Selected_jetweight", "std::vector<double>(ncleanjetspass, evWeight)"); //selected jetpt weight //doesn't work for data
        
     //==============================Clean b-Jets==============================================//  
	_rlm = _rlm.Define("btagcuts2", "Selected_jetbtag>0.8")
			.Define("Selected_bjetpt", "Selected_jetpt[btagcuts2]")
			.Define("Selected_bjeteta", "Selected_jeteta[btagcuts2]")
			.Define("Selected_bjetphi", "Selected_jetphi[btagcuts2]")
			.Define("Selected_bjetmass", "Selected_jetmass[btagcuts2]")
			.Define("ncleanbjetspass", "int(Selected_bjetpt.size())")
			.Define("Selected_bjetHT", "Sum(Selected_bjetpt)")
			.Define("cleanbjet4vecs", ::generate_4vec, {"Selected_bjetpt", "Selected_bjeteta", "Selected_bjetphi", "Selected_bjetmass"});           

}


//=============================define variables==================================================//
void BaseAnalyser::defineMoreVars()
{
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }

	//addVar({"good_electron1pt", "goodElectrons_pt[0]", ""});
    addVar({"good_muon1pt", "goodMuons_pt[0]", ""});

    //selected jet candidates
    addVar({"good_jet1pt", "(goodJets_pt.size()>0) ? goodJets_pt[0] : -1", ""});
    addVar({"Selected_jet1pt", "(Selected_jetpt.size()>0) ? Selected_jetpt[0] : -1", ""});
    addVar({"good_jet1eta", "goodJets_eta[0]", ""});
    addVar({"good_jet1mass", "goodJets_mass[0]", ""});

	//addVar({"Muon_pfRelIso03_all","Muon_pfRelIso03_all",""});
    //addVar({"Muon_miniPFRelIso_all","Muon_miniPFRelIso_all",""});
    //addVar({"Muon_pfRelIso04_all","Muon_pfRelIso04_all",""});
	//addVar({"Muon_ip3d","Muon_ip3d",""});
	



    //================================Store variables in tree=======================================//
    // define variables that you want to store
    //==============================================================================================//
    addVartoStore("run");
    addVartoStore("luminosityBlock");
    addVartoStore("event");
    //addVartoStore("evWeight.*");

	addVartoStore("evWeight");
    addVartoStore("genWeight"); // doesn't work for data
	addVartoStore("pugenWeight");// doesn't work for data
	addVartoStore("puWeight");// doesn't work for data
	addVartoStore("btagWeight_DeepJetrecalc");// doesn't work for data
	addVartoStore("Selected_jetweight");
    
	addVartoStore("Sel_jetforsfpt"); //Selected  jet pt Jet_pt_corr>30 Gev
    addVartoStore("Sel_jetforsfeta"); //Selected  jet eta for Jet_pt_corr>30 Gev 
	addVartoStore("Sel_jet_deepJet_shape_central");// selected jet shape after btv correction //// doesn't work for data
	addVartoStore("Goodjet_deepJet_shape_central"); //// doesn't work for data
    addVartoStore("Sel_jetcsvv2");//// doesn't work for data
    addVartoStore("Sel_jetdeepb");// doesn't work for data
    //addVartoStore("Sel_jet_deepJet_shape_central");
    
	addVartoStore("Jet_pt_corr"); //Jet_pt corrected
	addVartoStore("Jet_pt_corr_up");
	addVartoStore("Jet_pt_corr_down");
	addVartoStore("Jet_pt_relerror");
    
	addVartoStore("MET_pt_corr"); //MET_pt corrected
	addVartoStore("MET_pt_corr_up"); 
	addVartoStore("MET_pt_corr_down"); 
    addVartoStore("MET_pt");
	addVartoStore("MET_sumEt");
    addVartoStore("MET_phi");
	addVartoStore("MET_phi_corr"); 
	addVartoStore("MET_phi_corr_up"); 
	addVartoStore("MET_phi_corr_down"); 
	


    //electron
    addVartoStore("nElectron");
    addVartoStore("Electron_charge");
    addVartoStore("NgoodElectrons");

    //muon
    addVartoStore("nMuon");
    addVartoStore("Muon_charge");
    addVartoStore("Muon_mass");
    addVartoStore("NgoodMuons");
	addVartoStore("Muon_ip3d");//3D impact parameter wrt first PV, in cm
	addVartoStore("Muon_pfRelIso03_all");
    addVartoStore("Muon_miniPFRelIso_all");
    addVartoStore("Muon_pfRelIso04_all");

    //jet
    addVartoStore("nJet");
    addVartoStore("Jet_pt");
    addVartoStore("NgoodJets");
    addVartoStore("goodJets_pt");
    addVartoStore("Selected_jetpt");
	//addVartoStore("goodJets_HT");

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
    if(!_isData && isDefined("genWeight")){
		add1DHist({"hgenWeight", "genWeight", 1001, -100, 100}, "genWeight", "one", ""); //weight per event 
    
	add1DHist({"hgenEventSumw","Sum of gen Weights",1001,-8e+09,8e+09},"one","genEventSumw",""); //sum of gen weight
	//LHE weight
	//add1DHist( {"hnLHE", "LHE weight dist ", 100, 1e-4, 1e-6}, "LHEweight", "one", ""); //LHE weight per event: //x-axis: find LHE weight, increase with 1.
    //totalLHEweight_nocut
    //add1DHist( {"htotLHE", "total LHE weight dist ", 2, 0, 2}, "one", "LHEweight", ""); //increase the bin as LHE weight.
 	//====================================================================================================//
	}
	//add1DHist( {"btagscalef", "btvcentral_sf " , 100, 0, 2} , "Sel_jet_deepJet_shape_central", "Sel_jetforsfpt", "");
	add1DHist( {"hnevents_weighted", "Number of Events", 2, -0.5, 1.5}, "one", "evWeight", "");
	add1DHist( {"hnevents", "Number of Events not weighted", 2, -0.5, 1.5}, "one", "one", "");
   
    add1DHist( {"hNgoodElectrons_weighted", "NumberofGoodElectrons", 5, 0.0, 5.0}, "NgoodElectrons", "evWeight", "");
    
    add1DHist( {"hNgoodMuons_weighted", "# of good Muons ", 5, 0.0, 5.0}, "NgoodMuons", "evWeight", "");
    
    add1DHist( {"hgood_jetpt_weighted", "Good Jet pt " , 100, 0, 1000} , "goodJets_pt", "evWeight", "");
	add1DHist( {"hgood_jetpt", "Good Jet pt after not weighted " , 100, 0, 1000} , "goodJets_pt", "one", "");
	

    add1DHist( {"hgood_jet1pt_weighted", "Good Jet_1 pt " , 100, 0, 1000} , "good_jet1pt", "evWeight", "");
    add1DHist( {"hselected_jet1pt_weighted", "SelectedJet_1 pt" , 100, 0, 1000} , "Selected_jet1pt", "evWeight", "");
    add1DHist( {"hselected_jetpt_weighted", "No overlap muon-Jets " , 100, 0, 1000} , "Selected_jetpt", "evWeight", "");
	add1DHist( {"hselected_jetpt", "No overlap muon-Jets not weighted" , 100, 0, 1000} , "Selected_jetpt", "one", "");
	


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
	calculateEvWeight();
	selectMET();
	removeOverlaps();

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
	printf("\n\nCompound JEC:\n===================");
	_jetCorrector = _correction_jerc->compound().at(jettag);//jerctag#JSON (JEC,compound)compoundLevel="L1L2L3Res"
	cout<<"jettag =="<<jettag<<endl;
	//printf("jerc tag correction use count: %lu\n", deepcsv.use_count());
	//printf("SF : %f", corrfactor);
	//printf([input.name for input in sf.inputs]);
	_jetCorrectionUnc = _correction_jerc->at(_jercunctag);
}
void BaseAnalyser::applyJetMETCorrections() //data
	//For UL/NanoAofv9 :Jets are corrected to latest available JECs. No systematic variations of JECs are stored, but the quantities (ρ, area, pT, η) needed to recompute JECs or JEC systematic variations are available.
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
        //std::cout<<"SCALE Facsss===="<< corrfactors <<std::endl;
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
				//printf("\njet SF for shape correction:");
				//printf("SF : %f", corrfactor);
                
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
		//fixedGridRhoFastjetAll: rho from all PF Candidates, used e.g. for JECs"
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



void BaseAnalyser::setupCorrections(string goodjsonfname, string pufname, string putag, string btvfname, string btvtype, string jercfname, string jerctag, string jercunctag)
//In this function the correction is evaluated for each jet. The correction depends on the momentum, pseudorapidity, energy, and cone area of the jet, as well as the value of “rho” (the average momentum per area) and number of interactions in the event. The correction is used to scale the momentum of the jet.
//void BaseAnalyser::setupCorrections(string goodjsonfname, string btvfname, string btvtype, string jercfname, string jerctag, string jercunctag)
{
    cout << "set up Corrections!" << endl;
	if (_isData) _jsonOK = readgoodjson(goodjsonfname); // read golden json file

	if (!_isData) {
		// using correctionlib
		// btag corrections
		_correction_btag1 = correction::CorrectionSet::from_file(btvfname);
		_btvtype = btvtype;
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
    /*if(!_isData && isDefined("genWeight")) 
	{
		_rlm = _rlm.Define("evWeight","genWeight");
        std::cout<<"Not Data! evweight== genWeight"<<std::endl;

	}*/
            
    //==========================================sum of gen weight==========================================//
    // store sum of gen weight in outputtree. 
    //PS:"genweight" stored in "Events" tree and "genEventSumw" stored in "Runs" tree in the inputfile
    //=====================================================================================================//   
    if(!_isData){
    auto sumgenweight = _rd.Sum("genWeight");
    cout<<"sum of gen weight "<< *sumgenweight <<endl;
    string sumofgenweight = Form("%f",*sumgenweight);
    _rlm = _rlm.Define("genEventSumw",sumofgenweight.c_str());
    }
   
    /*--------------------------------------------------------*/

	defineCuts();
	defineMoreVars();
	bookHists();
	setupCuts_and_Hists();
	setupTree();
}

