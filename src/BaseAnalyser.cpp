/*
 * BaseAnalyser.cpp
 *
 *  Created on: May 6, 2022
 *      Author: suyong
 *      Developper: cdozen
 */

#include "TCanvas.h"
#include "TH1D.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TDirectory.h"
#include "TStyle.h"
#include <TSystem.h>
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
	HLT2017Names= {"HLT_IsoMu24","HLT_Ele32_WPTight_Gsf"};
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
	//_rlm = _rlm.Range(0, 100000);
    auto Nentry_100 = _rlm.Count();
	std::cout<< "-------------------------------------------------------------------" << std::endl;
    cout << "Usage of ranges:\n"
        << " - All entries: " << *Nentry << endl;
		//<< " - Entries from 0 to 100: " << *Nentry_100 << endl;
	std::cout<< "-------------------------------------------------------------------" << std::endl;

	//MinimalSelection to filter events
	addCuts("nMuon + nElectron == 1 && nJet>2 && PV_ndof>4", "0");
	//addCuts("Ngoodmuons>=2","00");
//    addCuts("ncleanjetspass>0","00");
	addCuts(setHLT(),"2"); //HLT cut buy checking HLT names in the root file

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
    _rlm = _rlm.Define("goodElectrons", "goodElectronsID && Electron_pt>35.0 && abs(Electron_eta)<2.1 && Electron_pfRelIso03_all<0.0588 && Electron_dxy<0.5 && Electron_dz<1 && Electron_hoe<0.0414 && Electron_lostHits==0");
    _rlm = _rlm.Define("goodElectrons_pt", "Electron_pt[goodElectrons]")
		.Define("goodElectrons_leading_pt","int(goodElectrons_pt.size())>0 ? static_cast<double> (goodElectrons_pt[0]) : -999.9")

                .Define("goodElectrons_eta", "Electron_eta[goodElectrons]")
                .Define("goodElectrons_leading_eta","int(goodElectrons_eta.size())>0 ? static_cast<double> (goodElectrons_eta[0]) : -999.9")
               
		 .Define("goodElectrons_phi", "Electron_phi[goodElectrons]")
                 .Define("goodElectrons_leading_phi", "int(goodElectrons_phi.size())>0 ?static_cast<double>(goodElectrons_phi[0]) :-999.9")

		.Define("goodElectrons_mass", "Electron_mass[goodElectrons]")
                .Define("goodElectrons_leading_mass", "int(goodElectrons_mass.size())>0 ?static_cast<double>(goodElectrons_mass[0]) :-999.9")

		.Define("goodElectrons_idx", ::good_idx, {"goodElectrons"})
                .Define("NgoodElectrons", "int(goodElectrons_pt.size())");

    //-------------------------------------------------------
    //generate electron 4vector from selected good Electrons
    //-------------------------------------------------------
    _rlm = _rlm.Define("goodElectron_4Vecs", ::generate_4vec, {"goodElectrons_pt", "goodElectrons_eta", "goodElectrons_phi", "goodElectrons_mass"});
    _rlm = _rlm.Define("goodElectron_TL4Vecs", ::generate_TLorentzVector, {"goodElectrons_leading_pt", "goodElectrons_leading_eta", "goodElectrons_leading_phi", "goodElectrons_leading_mass"});
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

    _rlm = _rlm.Define("goodmuonsID", MuonID(2)); //loose muons
    _rlm = _rlm.Define("goodmuons", "goodmuonsID && Muon_highPurity && Muon_pt > 29 && abs(Muon_eta) < 2.4 && Muon_pfRelIso04_all < 0.06 && Muon_dxy<2 && Muon_dz<5 && Muon_isGlobal==1");
//   _rlm = _rlm.Define("goodmuons","Muon_pt > 29 && abs(Muon_eta) < 2.4 && Muon_miniPFRelIso_all < 0.06");
      _rlm = _rlm.Define("goodmuons_pt", "Muon_pt[goodmuons]")
		 .Define("goodmuons_leading_pt", "int(goodmuons_pt.size())>0 ? static_cast<double>(goodmuons_pt[0]) : -999.9") 
                
		.Define("goodmuons_eta", "Muon_eta[goodmuons]")
                .Define("goodmuons_leading_eta", "int(goodmuons_eta.size())>0 ? static_cast<double>(goodmuons_eta[0]) :-999.9")
 
               .Define("goodmuons_phi", "Muon_phi[goodmuons]")
               .Define("goodmuons_leading_phi", "int(goodmuons_phi.size())>0 ? static_cast<double>(goodmuons_phi[0]) : -999.9")

                .Define("goodmuons_mass", "Muon_mass[goodmuons]")
		.Define("goodmuons_leading_mass", "int(goodmuons_mass.size())>0 ? static_cast<double>(goodmuons_mass[0]) :-999.9")
                
		.Define("goodmuons_charge", "Muon_charge[goodmuons]")
                .Define("goodmuons_idx", ::good_idx, {"goodmuons"})
                .Define("Ngoodmuons", "int(goodmuons_pt.size())");

    //-------------------------------------------------------
    //generate muon 4vector from selected good Muons
    //-------------------------------------------------------
    _rlm = _rlm.Define("goodmuons_4vecs", ::generate_4vec, {"goodmuons_pt", "goodmuons_eta", "goodmuons_phi", "goodmuons_mass"});
    _rlm = _rlm.Define("goodmuons_TL4Vecs", ::generate_TLorentzVector, {"goodmuons_leading_pt", "goodmuons_leading_eta", "goodmuons_leading_phi", "goodmuons_leading_mass"});



}

//=============================Reconstruct W boson=============================================//
//: Reconstruct the W boson
//=============================================================================================//
void BaseAnalyser::reconstructWboson()
{
    if (debug){
    std::cout<<std::endl;
    std::cout<< "================================//=================================" << std::endl;
    std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
    std::cout<< "================================//=================================" << std::endl;
    }

    /*--------------------- Reconstruct neutrino ---------------------*/

    std::cout<<"Reconstructing neutrino from MET"<<std::endl;
    _rlm = _rlm.Define("nu_pt", "MET_pt")
               .Define("nu_phi", "MET_phi")
               .Define("nu_phi_double", "static_cast<double>(nu_phi)")

               .Define("nu_px", "nu_pt*cos(nu_phi)")
               .Define("nu_py", "nu_pt*sin(nu_phi)");

    _rlm = _rlm.Define("electronChannel","NgoodElectrons==1 ? 1 : 0");
    _rlm = _rlm.Define("muonChannel", "Ngoodmuons==1 ? 1 : 0");


    _rlm = _rlm.Define("numb","-999.99");
    _rlm = _rlm.Define("numbLorentzVector",::generate_TLorentzVector, {"numb", "numb", "numb", "numb"});
    _rlm = _rlm.Define("lepton_TL4vec", "muonChannel==1 ? goodmuons_TL4Vecs : electronChannel==1 ?goodElectron_TL4Vecs : numbLorentzVector")
               .Define("lep_phi", "electronChannel==1 ? goodElectrons_phi[0] : muonChannel==1 ? goodmuons_phi[0] : -999.9"); 

    _rlm = _rlm.Define("lambda_reco", ::calculateLambda, {"lepton_TL4vec", "nu_pt", "nu_phi"});

    _rlm = _rlm.Define("delta_reco", ::calculateDelta, {"lepton_TL4vec", "nu_pt", "lambda_reco"})
               .Define("isRealSolution", "delta_reco > 0 ? 1 : -1");

    _rlm = _rlm.Define("nu_pz", ::calculate_nu_z, {"lepton_TL4vec", "lambda_reco", "delta_reco", "nu_pt", "nu_phi"});

    _rlm = _rlm.Define("nu_energy", ::calculate_nu_energy, {"nu_pt", "nu_phi", "nu_pz"});

    _rlm = _rlm.Define("nu_TL4vec", ::get_neutrino_TL4vec, {"nu_pt", "nu_phi", "nu_pz", "nu_energy"});


    // for (const auto &delta_reco: _rlm.Take<float>("delta_reco"))
    // { 
    //     std::cout << "delta_reco: " << delta_reco << "\n";
    // }

    /*--------------------- Reconstruct W boson ---------------------*/

    _rlm = _rlm.Define("Wboson_4vec", ::reconstructWboson_TL4vec, {"lepton_TL4vec", "nu_TL4vec"})
               .Define("w_mass","Wboson_4vec.M()")
               .Define("w_pt","Wboson_4vec.Pt()");

    // _rlm = _rlm.Define("Wboson_transversMass", "Wboson_4vec.Mt()");

    // _rlm = _rlm.Define("Wboson_transversMass", "sqrt(pow(lepton_TL4vec.Pt()+nu_pt,2)-pow(nu_pt*cos(nu_phi)+lepton_TL4vec.Px(),2) - pow(nu_pt*sin(nu_phi)+lepton_TL4vec.Py(),2))");
    _rlm = _rlm.Define("delta_phi_lep_nu", ::calculate_deltaPhi_scalars, {"lep_phi", "nu_phi_double"})
    .Define("Wboson_transversMass", "sqrt(2*lepton_TL4vec.Pt()*nu_pt*(1-cos(delta_phi_lep_nu)))");


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
//    _rlm = _rlm.Define("goodJets", "goodJetsID && Jet_pt>30.0 && abs(Jet_eta)<2.4 ");
    _rlm = _rlm.Define("goodJets_high_eta", "Jet_pt>40.0 && (abs(Jet_eta)<4.7 && abs(Jet_eta)>3.0) || (abs(Jet_eta)>0.0 && abs(Jet_eta)<2.7) ");
    _rlm = _rlm.Define("goodJets_low_eta", "Jet_pt>50.0 && abs(Jet_eta)<3.0 && abs(Jet_eta)>2.7 ");
    _rlm = _rlm.Define("goodJets", " goodJets_high_eta || goodJets_low_eta ");


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
    _rlm = _rlm.Define("btagcuts", "goodJets_deepjetbtag>0.7") //0.2783 -medium, 0.7 - tight 
      .Define("good_bjetpt", "goodJets_pt[btagcuts]")
      .Define("good_bjet_leading_pt", "int(good_bjetpt.size()) > 0 ? good_bjetpt[0] : -999.9")

      .Define("good_bjeteta", "goodJets_eta[btagcuts]")
      .Define("good_bjet_leading_eta", "int(good_bjeteta.size()) > 0 ? good_bjeteta[0] : -999.9")

     .Define("good_bjetphi", "goodJets_phi[btagcuts]")
      .Define("good_bjet_leading_phi", "int(good_bjetphi.size()) > 0 ? good_bjetphi[0] : -999.9")

      .Define("good_bjetmass", "goodJets_mass[btagcuts]")
      .Define("good_bjet_leading_mass", "int(good_bjetmass.size()) > 0 ? good_bjetmass[0] : -999.9")

      .Define("good_bjetdeepjet", "goodJets_deepjetbtag[btagcuts]");

    _rlm = _rlm.Define("good_bjethadflav", "goodJets_hadflav[btagcuts]");
    
    _rlm = _rlm.Define("Ngood_bjets", "int(good_bjetpt.size())")
      .Define("good_bjet_TL4vec", ::generate_TLorentzVector, {"good_bjet_leading_pt","good_bjet_leading_eta","good_bjet_leading_phi","good_bjet_leading_mass"})
      .Define("bjet_pt","good_bjet_TL4vec.Pt()")
      .Define("good_bjet4vecs", ::generate_4vec, {"good_bjetpt", "good_bjeteta", "good_bjetphi", "good_bjetmass"});
    


    if(!_isData){
      //For Btagging Efficiency    
      _rlm = _rlm.Define("btagpass_bcflav_goodJets", "goodJets_deepjetbtag>0.2783 && goodJets_hadflav!=0") //0.2783 -medium, 0.7 - tight 
	.Define("goodJets_btagpass_bcflav_pt", "goodJets_pt[btagpass_bcflav_goodJets]")
	.Define("goodJets_btagpass_bcflav_eta", "goodJets_eta[btagpass_bcflav_goodJets]");
      
      _rlm = _rlm.Define("all_bcflav_goodJets", "goodJets_hadflav!=0") //0.2783 -medium, 0.7 - tight 
	.Define("goodJets_all_bcflav_pt", "goodJets_pt[all_bcflav_goodJets]")
	.Define("goodJets_all_bcflav_eta", "goodJets_eta[all_bcflav_goodJets]");
      
      
      _rlm = _rlm.Define("btagpass_lflav_goodJets", "goodJets_deepjetbtag>0.2783 && goodJets_hadflav==0") //0.2783 -medium, 0.7 - tight 
	.Define("goodJets_btagpass_lflav_pt", "goodJets_pt[btagpass_lflav_goodJets]")
	.Define("goodJets_btagpass_lflav_eta", "goodJets_eta[btagpass_lflav_goodJets]");
      
      _rlm = _rlm.Define("all_lflav_goodJets", "goodJets_hadflav==0") //0.2783 -medium, 0.7 - tight 
	.Define("goodJets_all_lflav_pt", "goodJets_pt[all_lflav_goodJets]")
	.Define("goodJets_all_lflav_eta", "goodJets_eta[all_lflav_goodJets]");
    }
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

    _rlm = _rlm.Define("muonjetoverlap", checkoverlap, {"goodJets_4vecs","goodmuons_4vecs"});
	_rlm =	_rlm.Define("Selected_jetpt", "goodJets_pt[muonjetoverlap]")
		.Define("Selected_jeteta", "goodJets_eta[muonjetoverlap]")
		.Define("Selected_jetphi", "goodJets_phi[muonjetoverlap]")
		.Define("Selected_jetmass", "goodJets_mass[muonjetoverlap]")
	
		.Define("Selected_jetbtag", "goodJets_deepjetbtag[muonjetoverlap]") //
		.Define("Selected_jethadflav", "goodJets_hadflav[muonjetoverlap]") 
		.Define("ncleanjetspass", "int(Selected_jetpt.size())")
		.Define("cleanjet4vecs", ::generate_4vec, {"Selected_jetpt", "Selected_jeteta", "Selected_jetphi", "Selected_jetmass"})
		.Define("Selected_jetHT", "Sum(Selected_jetpt)");
        
     //==============================Clean b-Jets==============================================// 
	 //--> after remove overlap: use requested btaggedJets for btag-weight SFs && weight_generator. 
	 //=====================================================================================//
	_rlm = _rlm.Define("btagcuts2", "Selected_jetbtag>0.3040") //medium wp -->as an example. 
			.Define("Selected_bjetpt", "Selected_jetpt[btagcuts2]")
			.Define("Selected_bjeteta", "Selected_jeteta[btagcuts2]")
			.Define("Selected_bjetphi", "Selected_jetphi[btagcuts2]")
			.Define("Selected_bjetmass", "Selected_jetmass[btagcuts2]")
			.Define("ncleanbjetspass", "int(Selected_bjetpt.size())")
			.Define("Selected_bjetHT", "Sum(Selected_bjetpt)")
			.Define("Selected_bjethadflav", "Selected_jethadflav[btagcuts2]") 
			.Define("cleanbjet4vecs", ::generate_4vec, {"Selected_bjetpt", "Selected_bjeteta", "Selected_bjetphi", "Selected_bjetmass"});           

}


//=================================Define regions=================================================//
void BaseAnalyser::defineRegion()
{
    if (debug){
    std::cout<<std::endl;
    std::cout<< "================================//=================================" << std::endl;
    std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
    std::cout<< "================================//=================================" << std::endl;
    }

    _rlm = _rlm.Define("region_2j1t", "NgoodJets == 2 && Ngood_bjets== 1 ? 1 : 0");
    _rlm = _rlm.Define("region_2j0t", "NgoodJets == 2 && Ngood_bjets == 0 ? 1 : 0");
    _rlm = _rlm.Define("region_3j2t", "NgoodJets >= 3 && Ngood_bjets >= 2 ? 1 : 0");

    _rlm = _rlm.Define("region", "region_2j1t == 1 ? 0.0 : region_2j0t == 1 ? 1.0 : region_3j2t == 1 ? 2.0 :-1.0");

}

void BaseAnalyser::reconstructTop()
{
    if (debug){
    std::cout<<std::endl;
    std::cout<< "================================//=================================" << std::endl;
    std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
    std::cout<< "================================//=================================" << std::endl;
    }

//    _rlm = _rlm.Define("bQuark_forReco", "region == 0.0 ? good_bjet_TL4vec : numbLorentzVector")
    _rlm = _rlm.Define("bQuark_forReco", " good_bjet_TL4vec")
               .Define("topQuark_TL4vec", "Wboson_4vec + bQuark_forReco");

    _rlm = _rlm.Define("top_mass", "topQuark_TL4vec.M()")
               .Define("top_pt", "topQuark_TL4vec.Pt()");

}
void BaseAnalyser::calculateEvWeight(){

  //Scale Factors for BTag ID	
  int _case = 1;
  std::vector<std::string> Jets_vars_names = {"Selected_jethadflav", "Selected_jeteta",  "Selected_jetpt", "Selected_jetbtag"};  
  if(_case !=1){
    Jets_vars_names.emplace_back("Selected_jetbtag");
  }
  std::string output_btag_column_name = "btag_SF_";
//  _rlm = calculateBTagSF(_rlm, Jets_vars_names, _case, 0.2783, "M", output_btag_column_name);

  _rlm = calculateBTagSF(_rlm, Jets_vars_names, 0.3040,output_btag_column_name);

  //Scale Factors for Muon HLT, RECO, ID and ISO
  std::vector<std::string> Muon_vars_names = {"goodmuons_eta", "goodmuons_pt"};
  std::string output_mu_column_name = "muon_SF_";
  _rlm = calculateMuSF(_rlm, Muon_vars_names, output_mu_column_name);

  //Scale Factors for Electron RECO and ID
  std::vector<std::string> Electron_vars_names = {"goodElectrons_eta", "goodElectrons_pt"};
  std::string output_ele_column_name = "ele_SF_";
  _rlm = calculateEleSF(_rlm, Electron_vars_names, output_ele_column_name);

//   _rlm = _rlm.Define("evWeight_wobtagSF", " pugenWeight * muon_SF_central * ele_SF_central"); 
//  _rlm = _rlm.Define("totbtagSF", "btag_SF_bcflav_central * btag_SF_lflav_central"); 
  //Total event Weight:

  //Prefiring Weight for 2016 and 2017
  _rlm = applyPrefiringWeight(_rlm);
  //Total event Weight:
  //_rlm = _rlm.Define("evWeight", " pugenWeight * prefiring_SF_central * btag_SF_bcflav_central * btag_SF_lflav_central * muon_SF_central * ele_SF_central"); 
  _rlm = _rlm.Define("evWeight", " pugenWeight * prefiring_SF_central * muon_SF_central * ele_SF_central"); 

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

    addVar({"good_muon1pt", "goodmuons_pt[0]", ""});

    //selected jet candidates
/*    addVar({"good_jet1pt", "(goodJets_pt.size()>0) ? goodJets_pt[0] : -1", ""});
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
    addVartoStore("PV");
    //electron
    addVartoStore("nElectron");
    addVartoStore("Electron_charge");
    addVartoStore("NgoodElectrons");

    //muon
//    addVartoStore("nMuon");
//  addVartoStore("Muon_charge");
//    addVartoStore("Muon_mass");
//    addVartoStore("Ngoodmuons");

    //jet
    addVartoStore("nJet");
    addVartoStore("Jet_pt");
    addVartoStore("NgoodJets");
    addVartoStore("goodJets_pt");
    addVartoStore("Selected_jetpt");
    addVartoStore("Selected_jeteta");
    addVartoStore("Selected_jetbtag");
    addVartoStore("Wboson_transversMass");

    
    addVartoStore("good_bjetdeepjet");
    
    //jetmet corr
    addVartoStore("Jet_pt_corr");
    addVartoStore("Jet_pt_corr_up");
    addVartoStore("Jet_pt_corr_down");
    addVartoStore("Jet_pt_relerror");
    addVartoStore("MET_pt_corr");
    addVartoStore("MET_pt");

    if(!_isData){
      //case1 btag correction- fixed wp	
      //case3 shape correction
      //addVartoStore("btagWeight_case3");
    addVartoStore("Selected_jethadflav");
    addVartoStore("Selected_bjethadflav");//after overlap and btag cut

    addVartoStore("good_bjethadflav");
    addVartoStore("goodJets_hadflav");
      

    //For Btagging Efficiency
    addVartoStore("goodJets_btagpass_bcflav_pt");
    addVartoStore("goodJets_btagpass_bcflav_eta");
    addVartoStore("goodJets_all_bcflav_pt");
    addVartoStore("goodJets_all_bcflav_eta");

    addVartoStore("goodJets_btagpass_lflav_pt");
    addVartoStore("goodJets_btagpass_lflav_eta");
    addVartoStore("goodJets_all_lflav_pt");
    addVartoStore("goodJets_all_lflav_eta");

      
    //"evWeight", " pugenWeight * btag_SF_bcflav_central * btag_SF_lflav_central * muon_SF_central * ele_SF_central"
    //case1 btag correction- fixed wp	
//    addVartoStore("btag_SF_bcflav_central");
//    addVartoStore("btag_SF_lflav_central");
    addVartoStore("totbtagSF");
    //addVartoStore("btag_SF_down");
    addVartoStore("evWeight_wobtagSF");
    
    //case3 shape correction
    //addVartoStore("btagWeight_case3");
    
    
    //MUONID - ISO SF & WEIGHT	
//    addVartoStore("muon_SF_central");
    //addVartoStore("muon_id_weight");
//    addVartoStore("muon_SF_id_sf");
//    addVartoStore("muon_SF_id_syst");
//    addVartoStore("muon_SF_id_systup");
//    addVartoStore("muon_SF_id_systdown");
    //addVartoStore("muonISO_SF");
//    addVartoStore("muon_SF_iso_sf");
    addVartoStore("nu_pt");
    addVartoStore("good_bjet_TL4vec");
    addVartoStore("top_pt");
    addVartoStore("top_mass");
    addVartoStore("w_mass");
    addVartoStore("w_pt"); */
    addVartoStore("bjet_pt");


  }

//    addVartoStore("evWeight");   
    

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
	
    // add1DHist( {"hnevents", "Number of Events", 2, -0.5, 1.5}, "one", "evWeight", "");
    // add1DHist( {"hnevents_no_weight", "Number of Events w/o", 2, -0.5, 1.5}, "one", "one", "");
    
//    add1DHist( {"hNgoodElectrons", "NumberofGoodElectrons", 5, 0.0, 10.0}, "NgoodElectrons", "evWeight", "");
//    add1DHist( {"hW_mass", "massdistributionof W boson", 200, 0.0, 1000.0}, "Wboson_transversMass", "evWeight", "");  
//    add1DHist( {"hT_mass", "massdistributionof Top quark", 300, 0.0,1000.0}, "top_mass", "evWeight", "");  

//    add1DHist( {"hNgoodmuons", "# of good Muons ", 5, 0.0, 10.0}, "Ngoodmuons", "evWeight", "");
    
    // add1DHist( {"hgood_jetpt_with weight", "Good Jet pt with weight " , 100, 0, 1000} , "goodJets_pt", "evWeight", "");
    // add1DHist( {"hgood_jetpt_NOWeight", "Good Jet pt no weihght " , 100, 0, 1000} , "goodJets_pt", "one", "");
    
    // add1DHist( {"hgood_jet1pt", "Good Jet_1 pt with weight " , 100, 0, 2500} , "good_jet1pt", "evWeight", "");
    // add1DHist( {"hselected_jet1pt", "SelectedJet_1 pt no weight" , 100, 0, 1000} , "Selected_jet1pt", "evWeight", "");
    // add1DHist( {"hselected_jetptWithweight", "clean-Jets with weight" , 100, 0, 2500} , "Selected_jetpt", "evWeight", "");
    // add1DHist( {"hselected_jetptNoweight", "clean-Jets w/o weight" , 100, 0, 2500} , "Selected_jetpt", "one", "");
/*    if(!_isData){
      add1DHist( {"hbtag_SF_bcflav_central", "btag SF bcflav central" , 100, 0, 2} , "btag_SF_bcflav_central", "one", "");
      add1DHist( {"hbtag_SF_lflav_central", "btag SF lflav central" , 100, 0, 2} , "btag_SF_lflav_central", "one", "");
    }
  */  
    //add2DHist( {"btagscalef", "btvcent_sf vs seljet_pt" , 100, 0, 500, 100, 0, 500} ,  "Selected_jetpt","btag_SF_case1", "one","");

    
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
	_hist2dinfovector.clear();
	_th2dhistos.clear();
	_varstostore.clear();
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

	if(!_isData){
	  this->calculateEvWeight(); // PU, genweight and BTV and Mu and Ele
	}
	selectMET();
        reconstructWboson();
        defineRegion();
        reconstructTop();
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

