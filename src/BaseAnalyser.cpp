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

BaseAnalyser::BaseAnalyser(TTree *t, std::string outfilename,float crossection,float sumgenWeight)
:NanoAODAnalyzerrdframe(t, outfilename),_crossection(crossection),_sumgenWeight(sumgenWeight)
{
    //initiliaze the HLT names in your analyzer class
    HLT2024Names= {"HLT_PFHT380_SixJet32_DoubleBTagCSV_p075",
                    "HLT_PFHT300PT30_QuadPFJet_75_60_45_40_TriplePFBTagCSV_3p0",
                    "HLT_PFHT330PT30_QuadPFJet_75_60_45_40_TriplePFBTagDeepCSV_4p5",
                    "HLT_PFJet550","HLT_PFHT400_FivePFJet_100_100_60_30_30_DoublePFBTagDeepCSV_4p5",
                    "HLT_PFHT400_FivePFJet_120_120_60_30_30_DoublePFBTagDeepCSV_4p5"};
	HLT2023Names= {"HLT_IsoMu27","HLT_Ele35_WPTight_Gsf","HLT_Ele30_eta2p1_WPTight_Gsf_CentralPFJet35_EleCleaned"};//HLT_IsoTkMu24
    HLT2022Names= {"HLT_Ele30_WPTight_Gsf","HLT_IsoMu24"};
}

void BaseAnalyser::selectChannel(){
    _rlm = _rlm.Define("single_numb_vector",::generate_single_4vec, {"numb", "numb", "numb", "numb"});
    _rlm = _rlm.Define("numbLorentzVector",::generate_TLorentzVector, {"numb", "numb", "numb", "numb"});

//    auto Nentry = _rlm.Count();
//    cout << "Usage of ranges:\n"
//		<< " - Entries from before eu channel cut: " << *Nentry << endl;

// leadingType: 0 = electron leading, 1 = muon leading, 2 = none
// Merge leptons

_rlm = _rlm.Define("lepton_pt",      "Concatenate(goodElectrons_pt, goodmuons_pt)");
_rlm = _rlm.Define("lepton_pdgId",   "Concatenate(goodElectron_pdgId, goodmuons_pdgId)");
_rlm = _rlm.Define("lepton_charge",  "Concatenate(goodElectrons_charge, goodmuons_charge)");

// Indices of leptons sorted by pT
_rlm = _rlm.Define("leading_lepton_index", "ROOT::VecOps::Reverse(ROOT::VecOps::Argsort(lepton_pt))");

// Leading/subleading PDG IDs
_rlm = _rlm.Define("leading_lepton_pdgId",    "leading_lepton_index.size() > 0 ? lepton_pdgId[leading_lepton_index[0]] : numb");
_rlm = _rlm.Define("subleading_lepton_pdgId", "leading_lepton_index.size() > 1 ? lepton_pdgId[leading_lepton_index[1]] : numb");

// Leading/subleading charges
_rlm = _rlm.Define("leading_lepton_charge",    "leading_lepton_index.size() > 0 ? lepton_charge[leading_lepton_index[0]] : numb");
_rlm = _rlm.Define("subleading_lepton_charge", "leading_lepton_index.size() > 1 ? lepton_charge[leading_lepton_index[1]] : numb");

// Select the correct TLorentzVector
_rlm = _rlm.Define("leading_lepton","abs(leading_lepton_pdgId)==11 ? goodElectron_TL4Vecs : abs(leading_lepton_pdgId)==13 ? goodmuons_TL4Vecs : numbLorentzVector");

_rlm = _rlm.Define("subleading_lepton","abs(subleading_lepton_pdgId)==11 ? goodElectron_TL4Vecs : abs(subleading_lepton_pdgId)==13 ? goodmuons_TL4Vecs : numbLorentzVector");
// Leading/subleading pT
_rlm = _rlm.Define("leading_lepton_pt","leading_lepton_index.size() > 0 ?  lepton_pt[leading_lepton_index[0]] :numb");
_rlm = _rlm.Define("subleading_lepton_pt", "leading_lepton_index.size() > 1 ? lepton_pt[leading_lepton_index[1]] : numb");

// Invariant mass
_rlm = _rlm.Define("leptons_invariant_mass", "leading_lepton_index.size() > 1 ? (leading_lepton + subleading_lepton).M() : numb");

_rlm = _rlm.Define("eu_channel","Ngoodmuons >= 1 && NgoodElectrons >= 1 && (abs(leading_lepton_pdgId) != abs(subleading_lepton_pdgId)) && leading_lepton_pt > 25 && (leading_lepton_charge * subleading_lepton_charge) == -1 && leptons_invariant_mass > 20");

//   auto Nentry_eu = _rlm.Count();
 //   cout << "Usage of ranges:\n"
//		<< " - Entries from after eu channel cut: " << *Nentry_eu << endl;

}
void BaseAnalyser::defineCuts(){
	if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }
	auto Nentry = _rlm.Count();
	// This is how you can express a range of the first 100 entries
//	_rlm = _rlm.Range(0, 100000);

//    auto Nentry_nocut = _rlm.Count();
//    cout << "Usage of ranges:\n"
//		<< " no cut " << *Nentry_nocut << endl;
/*
	addCuts("nElectron+nMuon>=2 && nJet>0 && PV_npvsGood>=1", "0");
    addCuts("eu_channel && nElectron+nMuon>=2 && nJet>0 && PV_npvsGood>=1" ,"1");
//    auto Nentry_0 = _rlm.Count();
//    cout << "Usage of ranges:\n"
//		<< " - cut0: " << *Nentry_0 << endl;


	//addCuts(setHLT(),"00"); //HLT cut buy checking HLT names in the root file
    addCuts("(eu_channel && nElectron+nMuon>=2 && nJet>0 && PV_npvsGood>=1) && ( HLT_Ele32_WPTight_Gsf || HLT_IsoMu24 || HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ || HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ)" , "2");
//    auto Nentry_00 = _rlm.Count();
//    cout << "Usage of ranges:\n"
//		<< " - cut00: " << *Nentry_00 << endl;


addCuts("Flag_goodVertices && Flag_globalSuperTightHalo2016Filter && Flag_EcalDeadCellTriggerPrimitiveFilter && Flag_BadPFMuonFilter && Flag_BadPFMuonDzFilter && Flag_hfNoisyHitsFilter && Flag_eeBadScFilter && Flag_ecalBadCalibFilter && (eu_channel && nElectron+nMuon>=2 && nJet>0 && PV_npvsGood>=1) && ( HLT_Ele32_WPTight_Gsf || HLT_IsoMu24 || HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ || HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ)","3");

//    auto Nentry_000 = _rlm.Count();
//    cout << "Usage of ranges:\n"
//		<< " cut 000 " << *Nentry_000 << endl;*/
//addCuts("Flag_goodVertices && Flag_globalSuperTightHalo2016Filter && Flag_EcalDeadCellTriggerPrimitiveFilter && Flag_BadPFMuonFilter && Flag_BadPFMuonDzFilter && Flag_hfNoisyHitsFilter && Flag_eeBadScFilter && Flag_ecalBadCalibFilter && (eu_channel && nElectron+nMuon>=2 && nJet>0 && PV_npvsGood>=1) && ( HLT_Ele32_WPTight_Gsf || HLT_IsoMu24 || HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ || HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ)","0");
addCuts("!loose_vetoed_jets && !vetoed_jets && Flag_goodVertices && Flag_globalSuperTightHalo2016Filter && Flag_EcalDeadCellTriggerPrimitiveFilter && Flag_BadPFMuonFilter && Flag_BadPFMuonDzFilter && Flag_hfNoisyHitsFilter && Flag_eeBadScFilter && Flag_ecalBadCalibFilter && eu_channel && (nElectron+nMuon>=2) && (nJet>0) && (PV_npvsGood>=1) && ( HLT_Ele32_WPTight_Gsf || HLT_IsoMu24 || HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ || HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ) ","0");
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

   std::cout<< "================================0/=================================" << std::endl;
_rlm = _rlm.Define("numb", "std::numeric_limits<double>::quiet_NaN()");
   std::cout<< "================================1/=================================" << std::endl;

    _rlm = _rlm.Define("goodElectrons", "Electron_cutBased==4 && Electron_pt_corr>20 && abs(Electron_eta)<2.5 && !(( abs(Electron_eta + Electron_deltaEtaSC) > 1.4442 && abs(Electron_eta + Electron_deltaEtaSC) < 1.566 ))");
    _rlm = _rlm.Define("goodElectrons_pt", "Electron_pt_corr[goodElectrons]")
        .Define("goodElectrons_leading_pt_index","ArgMax(goodElectrons_pt)")
		.Define("goodElectrons_leading_pt","int(goodElectrons_pt.size())>0 ? static_cast<double> (goodElectrons_pt[goodElectrons_leading_pt_index]) : numb")
        .Define("goodElectron_pdgId","Electron_pdgId[goodElectrons]")

                .Define("goodElectrons_deltaEtaSC", "Electron_deltaEtaSC[goodElectrons]")
                .Define("goodElectrons_leading_deltaEtaSC","int(goodElectrons_deltaEtaSC.size())>0 ? static_cast<double> (goodElectrons_deltaEtaSC[goodElectrons_leading_pt_index]) : numb")


                .Define("goodElectrons_eta", "Electron_eta[goodElectrons]")
                .Define("goodElectrons_leading_eta","int(goodElectrons_eta.size())>0 ? static_cast<double> (goodElectrons_eta[goodElectrons_leading_pt_index]) : numb")
                .Define("goodElectron_eta_supercluster", "goodElectrons_eta + goodElectrons_deltaEtaSC")

		 .Define("goodElectrons_phi", "Electron_phi[goodElectrons]")
                 .Define("goodElectrons_leading_phi", "int(goodElectrons_phi.size())>0 ?static_cast<double>(goodElectrons_phi[goodElectrons_leading_pt_index]) :numb")

		.Define("goodElectrons_mass", "Electron_mass[goodElectrons]")
                .Define("goodElectrons_leading_mass", "int(goodElectrons_mass.size())>0 ?static_cast<double>(goodElectrons_mass[goodElectrons_leading_pt_index]) :numb")

		.Define("goodElectrons_charge","Electron_charge[goodElectrons]")
		.Define("goodElectrons_leading_charge","int(goodElectrons_charge.size())>0 ?static_cast<double>(goodElectrons_charge[goodElectrons_leading_pt_index]) :numb")

		.Define("goodElectrons_idx", ::good_idx, {"goodElectrons"})
                .Define("NgoodElectrons", "int(goodElectrons_pt.size())");
std::cout<< "================================/3/=================================" << std::endl;

    //-------------------------------------------------------
    //generate electron 4vector from selected good Electrons
    //-------------------------------------------------------
    _rlm = _rlm.Define("goodElectron_4vecs", ::generate_4vec, {"goodElectrons_pt", "goodElectrons_eta", "goodElectrons_phi", "goodElectrons_mass"});
    _rlm = _rlm.Define("goodElectron_TL4Vecs", ::generate_TLorentzVector, {"goodElectrons_leading_pt", "goodElectrons_leading_eta", "goodElectrons_leading_phi", "goodElectrons_leading_mass"});
    _rlm = _rlm.Define("goodElectron_4vec", ::generate_single_4vec, {"goodElectrons_leading_pt", "goodElectrons_leading_eta", "goodElectrons_leading_phi", "goodElectrons_leading_mass"});



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

    _rlm = _rlm.Define("goodmuonsID", MuonID(4));
    _rlm = _rlm.Define("goodmuons", "goodmuonsID && Muon_pt_corr > 20 && abs(Muon_eta) < 2.4 && Muon_pfRelIso04_all<0.15");
    _rlm = _rlm.Define("goodmuons_pt", "Muon_pt_corr[goodmuons]")
               .Define("goodmuons_leading_pt_index","ArgMax(goodmuons_pt)")
                .Define("goodmuons_pdgId","Muon_pdgId[goodmuons]")

		 .Define("goodmuons_leading_pt", "int(goodmuons_pt.size())>0 ? static_cast<double>(goodmuons_pt[goodmuons_leading_pt_index]) : numb") 
                
		.Define("goodmuons_eta", "Muon_eta[goodmuons]")
                .Define("goodmuons_leading_eta", "int(goodmuons_eta.size())>0 ? static_cast<double>(goodmuons_eta[goodmuons_leading_pt_index]) :numb")

               .Define("goodmuons_phi", "Muon_phi[goodmuons]")
               .Define("goodmuons_leading_phi", "int(goodmuons_phi.size())>0 ? static_cast<double>(goodmuons_phi[goodmuons_leading_pt_index]) : numb")

                .Define("goodmuons_mass", "Muon_mass[goodmuons]")
		.Define("goodmuons_leading_mass", "int(goodmuons_mass.size())>0 ? static_cast<double>(goodmuons_mass[goodmuons_leading_pt_index]) :numb")
      
                .Define("goodmuons_isolation", "Muon_pfRelIso04_all[goodmuons]")

 		.Define("gooodmuons_charge","Muon_charge[goodmuons]")
                .Define("goodmuons_leading_charge","int(gooodmuons_charge.size())>0 ?static_cast<double>(gooodmuons_charge[goodmuons_leading_pt_index]) :numb")

		.Define("goodmuons_charge", "Muon_charge[goodmuons]")
                .Define("goodmuons_idx", ::good_idx, {"goodmuons"})
                .Define("Ngoodmuons", "int(goodmuons_pt.size())");



    //-------------------------------------------------------
    //generate muon 4vector from selected good Muons
    //-------------------------------------------------------
    _rlm = _rlm.Define("goodmuons_4vecs", ::generate_4vec, {"goodmuons_pt", "goodmuons_eta", "goodmuons_phi", "goodmuons_mass"});
    _rlm = _rlm.Define("goodmuons_TL4Vecs", ::generate_TLorentzVector, {"goodmuons_leading_pt", "goodmuons_leading_eta", "goodmuons_leading_phi", "goodmuons_leading_mass"});
    _rlm = _rlm.Define("goodMuon_4vec", ::generate_single_4vec, {"goodmuons_leading_pt", "goodmuons_leading_eta", "goodmuons_leading_phi", "goodmuons_leading_mass"});

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

//*--------------------- Reconstruct neutrino ---------------------*/

    std::cout<<"Reconstructing neutrino from MET"<<std::endl;
    _rlm = _rlm.Define("nu_pt", "static_cast<float>(goodMET_pt)")
               .Define("nu_phi", "static_cast<float>(goodMET_phi)")
               .Define("nu_phi_double", "static_cast<double>(nu_phi)")

               .Define("nu_px", "nu_pt*cos(nu_phi)")
               .Define("nu_py", "nu_pt*sin(nu_phi)");


    _rlm = _rlm.Define("lambda_reco", ::calculateLambda, {"lepton_TL4vec", "nu_pt", "nu_phi"});

    _rlm = _rlm.Define("delta_reco", ::calculateDelta, {"lepton_TL4vec", "nu_pt", "lambda_reco"})
               .Define("isRealSolution", "delta_reco > 0 ? 1 : -1");

    _rlm = _rlm.Define("nu_pz", ::calculate_nu_z, {"lepton_TL4vec", "lambda_reco", "delta_reco", "nu_pt", "nu_phi"});

    _rlm = _rlm.Define("nu_energy", ::calculate_nu_energy, {"nu_pt", "nu_phi", "nu_pz"});

    _rlm = _rlm.Define("nu_TL4vec", ::get_neutrino_TL4vec, {"nu_pt", "nu_phi", "nu_pz", "nu_energy"});
    _rlm = _rlm.Define("nu_4vecs", ::get_neutrino_4vecs, {"nu_pt", "nu_phi", "nu_pz", "nu_energy"});

    // { 
    //     std::cout << "delta_reco: " << delta_reco << "\n";
    // }

    /*--------------------- Reconstruct W boson ---------------------*/

    _rlm = _rlm.Define("Wboson_4vec", ::reconstructWboson_TL4vec, {"lepton_TL4vec", "nu_TL4vec"})
	       .Define("lepton_pt","lepton_TL4vec.Pt()")
           .Define("nu4vec_pt","nu_TL4vec.Pt()")
           .Define("w_pt","Wboson_4vec.Pt()");

    // _rlm = _rlm.Define("Wboson_transversMass", "Wboson_4vec.Mt()");

   //  _rlm = _rlm.Define("Wboson_transversMass", "sqrt(pow(lepton_TL4vec.Pt()+nu_pt,2)-pow(nu_pt*cos(nu_phi)+lepton_TL4vec.Px(),2) - pow(nu_pt*sin(nu_phi)+lepton_TL4vec.Py(),2))");
    _rlm = _rlm.Define("delta_phi_lep_nu", ::calculate_deltaPhi_scalars, {"lep_phi", "nu_phi_double"})
    .Define("Wboson_transversMass", "sqrt(2*lepton_TL4vec.Pt()*nu_pt*(1-cos(delta_phi_lep_nu)))");

   // _rlm=_rlm.Define("Wboson_transversemass_QCDele","QCDelectronChannel && region_2j0t ? Wboson_transversMass : numb");
       // _rlm=_rlm.Define("Wboson_transversemass_QCDele","QCDelectronChannel && region_2j0t ? Wboson_transversMass : numb");


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

    _rlm = _rlm.Define("goodJetsID", "Jet_jetId == 6"); //without pt-eta cuts
    //_rlm = _rlm.Define("goodJets_low_eta", "goodJetsID && Jet_pt>50.0 && abs(Jet_eta)<3.0 && abs(Jet_eta)>2.5 ");
    _rlm = _rlm.Define("goodJets", "goodJetsID && Jet_pt>30 && abs(Jet_eta)<2.4 ");
    _rlm =_rlm.Define("looseJets", "goodJetsID && Jet_pt<30 && Jet_pt>20 && abs(Jet_eta)<2.4 ");

  //  _rlm = _rlm.Define("goodJets", " goodJets_low_eta || goodJets_high_eta ");


    _rlm = _rlm.Define("goodJets_pt", "Jet_pt[goodJets]")
                .Define("goodJets_eta", "Jet_eta[goodJets]")
                .Define("goodJets_phi", "Jet_phi[goodJets]")
                .Define("goodJets_mass", "Jet_mass[goodJets]")
                .Define("goodJets_idx", ::good_idx, {"goodJets"});

     _rlm = _rlm.Define("looseJets_pt", "Jet_pt[looseJets]")
                .Define("looseJets_eta", "Jet_eta[looseJets]")
                .Define("looseJets_phi", "Jet_phi[looseJets]")
                .Define("looseJets_mass", "Jet_mass[looseJets]");

    if(!_isData){

  _rlm = _rlm.Define("goodJets_hadflav", "Jet_hadronFlavour[goodJets]");
}
    //goot jets deep-b tag
	_rlm = _rlm.Define("goodjets_pnetbtag", "Jet_btagRobustParTAK4B[goodJets]")
                .Define("NgoodJets", "int(goodJets_pt.size())")
                .Define("goodJets_4vecs", ::generate_4vec, {"goodJets_pt", "goodJets_eta", "goodJets_phi", "goodJets_mass"});
    _rlm = _rlm.Define("looseJets_4vecs", ::generate_4vec, {"looseJets_pt", "looseJets_eta", "looseJets_phi", "looseJets_mass"});



	//select b jest within goodjets 
    _rlm = _rlm.Define("btagcuts", "goodjets_pnetbtag>0.3487") //0.2783 -medium, 0.6734 - tight 
      .Define("good_bjetpt", "goodJets_pt[btagcuts]")
      .Define("good_bjet_maxpt_index", "ArgMax(good_bjetpt)")

      .Define("good_bjet_leading_pt", "int(good_bjetpt.size()) > 0 ? good_bjetpt[good_bjet_maxpt_index] : numb")

      .Define("good_bjeteta", "goodJets_eta[btagcuts]")
      .Define("good_bjet_leading_eta", "int(good_bjeteta.size()) > 0 ? good_bjeteta[good_bjet_maxpt_index] : numb")

      .Define("good_bjetphi", "goodJets_phi[btagcuts]")
      .Define("good_bjet_leading_phi", "int(good_bjetphi.size()) > 0 ? good_bjetphi[good_bjet_maxpt_index] : numb")

      .Define("good_bjetmass", "goodJets_mass[btagcuts]")
      .Define("good_bjet_leading_mass", "int(good_bjetmass.size()) > 0 ? good_bjetmass[good_bjet_maxpt_index] : numb")

      .Define("good_bjetdeepjet", "goodjets_pnetbtag[btagcuts]");
    if(!_isData){

    _rlm = _rlm.Define("good_bjethadflav", "goodJets_hadflav[btagcuts]");
}    
    _rlm = _rlm.Define("Ngood_bjets", "int(good_bjetpt.size())")
      .Define("good_bjet_TL4vec", ::generate_TLorentzVector, {"good_bjet_leading_pt","good_bjet_leading_eta","good_bjet_leading_phi","good_bjet_leading_mass"})
      .Define("bjet_mass","good_bjet_TL4vec.M()")
      .Define("good_bjet4vecs", ::generate_4vec, {"good_bjetpt", "good_bjeteta", "good_bjetphi", "good_bjetmass"});
     _rlm = _rlm.Define("goodbjet_single_4vec",::generate_single_4vec,{"good_bjet_leading_pt","good_bjet_leading_eta","good_bjet_leading_phi","good_bjet_leading_mass"});


}

//=================================Overlap function=================================================//
void BaseAnalyser::removeOverlaps()
{
    cout << "checking overlapss between jets and leptons" << endl;

	// lambda function for checking overlapped jets with electrons
	
	auto checkoverlap = [](FourVectorVec &jets, FourVector &lep)
		{
		  doubles mindrlepton;
		  for (auto ajet: jets)
		    {
		      auto mindr = 6.0;
		      // for (auto alepton: leps)
		      // {
		      // 	auto dr = ROOT::Math::VectorUtil::DeltaR(ajet, alepton);
		      //     if (dr < mindr) mindr = dr;
		      // }
		      auto dr = ROOT::Math::VectorUtil::DeltaR(ajet, lep);
		      if (dr < mindr) mindr = dr;
		      int out = mindr > 0.4 ? 1 : 0;
		      mindrlepton.emplace_back(out);
		      
		    }
		  return mindrlepton;
		};
	
	
	_rlm = _rlm.Define("numb4Vector", ::generate_single_4vec, {"numb", "numb", "numb", "numb"});
	
//	_rlm = _rlm.Define("lepton_forOverlapCheck", "electronChannel==1 ? goodElectron_4vec : muonChannel==1 ? goodMuon_4vec : QCDmuonChannel==1 ? rev_iso_mu_4vec : QCDelectronChannel==1 ? rev_iso_el_4vec : numb4Vector");
//	    auto Nentry_nooverlap = _rlm.Count();
//    cout << "Usage of ranges:\n"
//		<< " before overlap removal " << *Nentry_nooverlap << endl;


	_rlm = _rlm.Define("checkOverlap_muon", checkoverlap, {"goodJets_4vecs","goodMuon_4vec"});
    _rlm = _rlm.Define("checkOverlap_ele", checkoverlap, {"goodJets_4vecs","goodElectron_4vec"});
    _rlm = _rlm.Define("checkOverlap","checkOverlap_ele && checkOverlap_muon");

	_rlm = _rlm.Define("checkOverlap_loose_muon", checkoverlap, {"looseJets_4vecs","goodMuon_4vec"});
    _rlm = _rlm.Define("checkOverlap_loose_ele", checkoverlap, {"looseJets_4vecs","goodElectron_4vec"});
    _rlm = _rlm.Define("checkOverlap_loose","checkOverlap_loose_ele && checkOverlap_loose_muon");//.Filter("!checkOverlap || !checkOverlap_loose");
//	    auto Nentry_overlap = _rlm.Count();
//    cout << "Usage of ranges:\n"
//		<< " after overlap removal " << *Nentry_overlap << endl;



	_rlm =	_rlm.Define("Selected_jeteta", "goodJets_eta[checkOverlap]")
		.Define("Selected_jetphi", "goodJets_phi[checkOverlap]")
	    .Define("Selected_jetpt", "goodJets_pt[checkOverlap]")
		.Define("Selected_jetmass", "goodJets_mass[checkOverlap]")
		.Define("Selected_jetbtag", "goodjets_pnetbtag[checkOverlap]") //
	//	.Define("ncleanjetspass", "int(Selected_jetpt.size())")
		.Define("cleanjet4vecs", ::generate_4vec, {"Selected_jetpt", "Selected_jeteta", "Selected_jetphi", "Selected_jetmass"});


    
	_rlm =	_rlm.Define("Selected_loosejeteta", "looseJets_eta[checkOverlap_loose]")
		.Define("Selected_loosejetphi", "looseJets_phi[checkOverlap_loose]")
	    	.Define("Selected_loosejetpt", "looseJets_pt[checkOverlap_loose]")
		.Define("Selected_loosejetmass", "looseJets_mass[checkOverlap_loose]")
	//	.Define("ncleanjetspass", "int(Selected_jetpt.size())")
		.Define("cleanjet4vecs_loose", ::generate_4vec, {"Selected_loosejetpt", "Selected_loosejeteta", "Selected_loosejetphi", "Selected_loosejetmass"});
                
// 	    auto Nentry_nojetveto = _rlm.Count();
//    cout << "Usage of ranges:\n"
//		<< " before jet veto application" << *Nentry_nojetveto << endl;               
                _rlm = applyJetVetoMap(_rlm,"Selected_jeteta","Selected_jetphi");
               _rlm = applyJetVetoMap(_rlm,"Selected_loosejeteta","Selected_loosejetphi","loose_vetoed_jets");
// 	    auto Nentry_jetveto = _rlm.Count();
//    cout << "Usage of ranges:\n"
//		<< " after jet veto application " << *Nentry_jetveto << endl;     

         _rlm=_rlm.Define("ncleanjetspass", "int(Selected_jetpt.size())");
/*        _rlm = _rlm.Define("Selected_jet_maxpt_index", "ArgMax(Selected_jetpt)");
        _rlm = _rlm.Define("Selected_jet_maxpt_index_temp", "int(Selected_jetpt.size())>0");
        _rlm = _rlm.Define("Selected_jet_leading_pt", "int(Selected_jetpt.size())>0 ? Selected_jetpt[Selected_jet_maxpt_index] : numb");
        _rlm = _rlm.Define("Selected_jet_leading_eta", "int(Selected_jetpt.size())>0 ? Selected_jeteta[Selected_jet_maxpt_index] : numb");
        _rlm = _rlm.Define("Selected_jet_leading_phi", "int(Selected_jetpt.size())>0 ? Selected_jetphi[Selected_jet_maxpt_index] : numb");
        _rlm = _rlm.Define("Selected_jet_leading_mass", "int(Selected_jetpt.size())>0 ? Selected_jetmass[Selected_jet_maxpt_index] : numb");
        _rlm = _rlm.Define("selected_leadingcleanjet_TL4vec",:: generate_TLorentzVector,{"Selected_jet_leading_pt", "Selected_jet_leading_eta", "Selected_jet_leading_phi", "Selected_jet_leading_mass"});
*/
_rlm = _rlm.Define("Selected_jet_sorted_indices", "ROOT::VecOps::Reverse(ROOT::VecOps::Argsort(Selected_jetpt))");
_rlm = _rlm.Define("Selected_jet_leading_pt", "int(Selected_jetpt.size())>0 ? Selected_jetpt[Selected_jet_sorted_indices[0]] : numb");
        _rlm = _rlm.Define("Selected_jet_leading_eta", "int(Selected_jetpt.size())>0 ? Selected_jeteta[Selected_jet_sorted_indices[0]] : numb");
        _rlm = _rlm.Define("Selected_jet_leading_phi", "int(Selected_jetpt.size())>0 ? Selected_jetphi[Selected_jet_sorted_indices[0]] : numb");
        _rlm = _rlm.Define("Selected_jet_leading_mass", "int(Selected_jetpt.size())>0 ? Selected_jetmass[Selected_jet_sorted_indices[0]] : numb");
        _rlm = _rlm.Define("selected_leadingcleanjet_TL4vec",:: generate_TLorentzVector,{"Selected_jet_leading_pt", "Selected_jet_leading_eta", "Selected_jet_leading_phi", "Selected_jet_leading_mass"});



_rlm = _rlm.Define("Selected_jet_subleading_pt", "int(Selected_jetpt.size())>1 ? Selected_jetpt[Selected_jet_sorted_indices[1]] : numb");
_rlm = _rlm.Define("Selected_jet_subleading_eta", "int(Selected_jetpt.size())>1 ? Selected_jeteta[Selected_jet_sorted_indices[1]] : numb");
_rlm = _rlm.Define("Selected_jet_subleading_phi", "int(Selected_jetpt.size())>1 ? Selected_jetphi[Selected_jet_sorted_indices[1]] : numb");
_rlm = _rlm.Define("Selected_jet_subleading_mass", "int(Selected_jetpt.size())>1 ? Selected_jetmass[Selected_jet_sorted_indices[1]] : numb");
_rlm = _rlm.Define("selected_subleadingcleanjet_TL4vec",:: generate_TLorentzVector,{"Selected_jet_subleading_pt", "Selected_jet_subleading_eta", "Selected_jet_subleading_phi", "Selected_jet_subleading_mass"});

         //    _rlm=_rlm.Define("ncleanjetspass", "int(Selected_jetpt.size())");
        _rlm=_rlm.Define("ncleanjetspass_loose", "int(Selected_loosejetpt.size())");
        _rlm = _rlm.Define("Selected_loosejet_maxpt_index", "ArgMax(Selected_loosejetpt)");
        _rlm = _rlm.Define("Selected_loosejet_maxpt_index_temp", "int(Selected_loosejetpt.size())>0");
        _rlm = _rlm.Define("Selected_loosejet_leadingpt", "int(Selected_loosejetpt.size())>0 ? Selected_loosejetpt[Selected_loosejet_maxpt_index] : numb");
        if(!_isData){
            _rlm= _rlm .Define("Selected_jethadflav", "goodJets_hadflav[checkOverlap]"); 
}

      
     //==============================Clean b-Jets==============================================// 
	 //--> after remove overlap: use requested btaggedJets for btag-weight SFs && weight_generator. 
	 //=====================================================================================//
//	_rlm = _rlm.Define("btagcuts2", "abs(Selected_jeteta)<=2.4 && Selected_jetbtag>=0.6734")

	_rlm = _rlm.Define("btagcuts2", "Selected_jetbtag>=0.3487")
			.Define("Selected_bjetpt", "Selected_jetpt[btagcuts2]")
                        .Define("Selected_bjet_maxpt_index", "ArgMax(Selected_bjetpt)")

			.Define("Selected_bjet_leading_pt", "int(Selected_bjetpt.size())>0 ? Selected_bjetpt[Selected_bjet_maxpt_index] : numb")

			.Define("Selected_bjeteta", "Selected_jeteta[btagcuts2]")
                        .Define("Selected_bjet_leading_eta", "int(Selected_bjetpt.size())>0 ? Selected_bjeteta[Selected_bjet_maxpt_index] : numb")

			.Define("Selected_bjetphi", "Selected_jetphi[btagcuts2]")
                        .Define("Selected_bjet_leading_phi", "int(Selected_bjetpt.size())>0 ? Selected_bjetphi[Selected_bjet_maxpt_index] : numb")

			.Define("Selected_bjetmass", "Selected_jetmass[btagcuts2]")
                        .Define("Selected_bjet_leading_mass", "int(Selected_bjetpt.size())>0 ? Selected_bjetmass[Selected_bjet_maxpt_index] : numb")

			.Define("ncleanbjetspass", "int(Selected_bjetpt.size())")
			.Define("Selected_bjetHT", "Sum(Selected_bjetpt)")
//			.Define("Selected_bjethadflav", "Selected_jethadflav[btagcuts2]") 
			.Define("cleanbjet4vecs", ::generate_4vec, {"Selected_bjetpt", "Selected_bjeteta", "Selected_bjetphi", "Selected_bjetmass"});
	        _rlm = _rlm.Define("selected_cleanbjet_4vec",::generate_single_4vec, {"Selected_bjet_leading_pt", "Selected_bjet_leading_eta", "Selected_bjet_leading_phi", "Selected_bjet_leading_mass"});
            _rlm = _rlm.Define("selected_cleanbjet_TL4vec",:: generate_TLorentzVector,{"Selected_bjet_leading_pt", "Selected_bjet_leading_eta", "Selected_bjet_leading_phi", "Selected_bjet_leading_mass"});
//                _rlm = applyJetVetoMap(_rlm,"Selected_jeteta","Selected_jetphi").Filter("!vetoed_jets");
           //std::cout << "Number of entries: " << _rlm.Count().GetValue() << std::endl;



    if(!_isData){
            _rlm= _rlm .Define("Selected_bjethadflav", "Selected_jethadflav[btagcuts2]");

    if(!_isData){
      //For Btagging Efficiency    
      _rlm = _rlm.Define("btagpass_bcflav_goodJets", "Selected_jetbtag>0.3487 && Selected_jethadflav==5") //0.2783 -medium, 0.7 - tight 
	.Define("goodJets_btagpass_bcflav_pt", "Selected_jetpt[btagpass_bcflav_goodJets]")
	.Define("goodJets_btagpass_bcflav_eta", "Selected_jeteta[btagpass_bcflav_goodJets]");
      
      _rlm = _rlm.Define("all_bcflav_goodJets", "Selected_jethadflav==5") //0.2783 -medium, 0.7 - tight 
	.Define("goodJets_all_bcflav_pt", "Selected_jetpt[all_bcflav_goodJets]")
	.Define("goodJets_all_bcflav_eta", "Selected_jeteta[all_bcflav_goodJets]");
      
      
      _rlm = _rlm.Define("btagpass_lflav_goodJets", "Selected_jetbtag>0.3487 && Selected_jethadflav==0") //0.2783 -medium, 0.7 - tight 
	.Define("goodJets_btagpass_lflav_pt", "Selected_jetpt[btagpass_lflav_goodJets]")
	.Define("goodJets_btagpass_lflav_eta", "Selected_jeteta[btagpass_lflav_goodJets]");
      
      _rlm = _rlm.Define("all_lflav_goodJets", "Selected_jethadflav==0") //0.2783 -medium, 0.7 - tight 
	.Define("goodJets_all_lflav_pt", "Selected_jetpt[all_lflav_goodJets]")
	.Define("goodJets_all_lflav_eta", "Selected_jeteta[all_lflav_goodJets]");
    }


    }


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


_rlm = _rlm.Define("region_2j2t", "ncleanjetspass == 2 && ncleanbjetspass == 2");
_rlm = _rlm.Define("region_2j1t", "ncleanjetspass == 2 && ncleanbjetspass == 1");
_rlm = _rlm.Define("region_1j1t", "ncleanjetspass== 1 && ncleanbjetspass == 1");
}
/*
void BaseAnalyser::reconstructTop()
{
    if (debug){
    std::cout<<std::endl;
    std::cout<< "================================//=================================" << std::endl;
    std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
    std::cout<< "================================//=================================" << std::endl;
    }

//    _rlm = _rlm.Define("bQuark_forReco", "region == 0.0 ? good_bjet_TL4vec : numbLorentzVector")
    _rlm = _rlm.Define("bQuark_forReco", " region_2j1t || region_3j2t ? selected_cleanbjet_TL4vec : region_2j0t ? selected_cleanjet_TL4vec : numbLorentzVector")
               .Define("topQuark_TL4vec", "Wboson_4vec + bQuark_forReco");

    _rlm = _rlm.Define("top_mass","topQuark_TL4vec.M()" )
               .Define("top_pt", "Wboson_transversMass>50 ? topQuark_TL4vec.Pt() : numb");

}*/
void BaseAnalyser::calculateEvWeight(){

if (_isData )
   {
      _rlm = _rlm.Define("evWeight", [](){
             return 1.0;
       }, {} );
   }
if (!_isData) // Only use genWeight
{
  //Scale Factors for BTag ID	
  int _case = 1;
  std::vector<std::string> Jets_vars_names = {"Selected_jethadflav","Selected_jeteta","Selected_jetpt","Selected_jetbtag"};  
  if(_case !=1){
    Jets_vars_names.emplace_back("Selected_jetbtag");
  }
  std::string output_btag_column_name = "btag_SF_";
//             auto sumgenweight1 = _rlm.Sum("genWeight");
//           string sumofgenweight1 = Form("%f",*sumgenweight1);
//           std::cout<<"Sum of genWeights = "<<sumofgenweight1.c_str()<<std::endl;

//  _rlm = calculateBTagSF(_rlm, Jets_vars_names, _case, 0.2783, "M", output_btag_column_name);
  _rlm = calculateBTagSF(_rlm, Jets_vars_names,1,0.3487,"M", output_btag_column_name);
// ---------- create unified muon eta/pt columns (per-event) ----------
std::vector<std::string> ele_vars_names= {"goodElectron_eta_supercluster", "goodElectrons_pt","goodElectrons_phi"};
std::vector<std::string> muon_vars_names = {"goodmuons_eta", "goodmuons_pt"};

// ---------- call your existing SF functions unchanged ----------
std::string output_mu_column_name  = "muon_SF_";
std::string output_ele_column_name = "ele_SF_";

// apply for muons only if any muon-like channel exists in the dataframe
_rlm = calculateMuSF(_rlm, muon_vars_names, output_mu_column_name);

// apply for electrons only if any electron-like channel exists in the dataframe
_rlm = calculateEleSF(_rlm, ele_vars_names, output_ele_column_name);

  auto sumgenweight = _rd.Sum("genWeight");
  float lumi=(_year == "2023" ? 17794 : 7980 );
  double lumifactor = (_crossection * lumi) / (_sumgenWeight);
  _rlm = _rlm.Define("Lumifactor", [lumifactor]() {
    return lumifactor;
    });
  std::cout << "[DEBUG] In Analyze. crossection = " << _crossection << std::endl;
   std::cout << "[DEBUG] In Analyze. lumifactor = " << lumifactor << std::endl;
   std::cout << "[DEBUG] In Analyze. sum of genweight = " << _sumgenWeight << std::endl;

// _rlm=_rlm .Define("evWeight", "Lumifactor *pugenWeight*ele_SF_central");  	
//     _rlm = _rlm.Define("lepton_SF_central", "(muonChannel || QCDmuonChannel) ? muon_SF_central : (electronChannel || QCDelectronChannel)? ele_SF_central : 1");

  // _rlm = _rlm.Define("evWeight", "Lumifactor * pugenWeight* lepton_SF_central*btag_SF_central");
   _rlm = _rlm.Define("no_puWeight","Lumifactor * genWeight");
   _rlm = _rlm.Define("Weight","Lumifactor * pugenWeight");//* btag_SF_central"); 

        
//  _rlm = _rlm.Define("evWeight", " Lumifactor * btag_SF_bcflav_central  * btag_SF_lflav_central * pugenWeight * muon_SF_central * ele_SF_central"); // btag_SF_bcflav_central * btag_SF_lflav_central
  _rlm = _rlm.Define("evWeight", "Lumifactor * pugenWeight* muon_SF_central * ele_SF_central* btag_SF_bcflav_central  * btag_SF_lflav_central"); 
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

   _rlm = _rlm.Define("goodMET", "PuppiMET_pt_corr")  // Boolean flag
          .Define("goodMET_pt", "goodMET ? PuppiMET_pt_corr : numb")  // Assign numb for events failing cut
          .Define("goodMET_phi", "goodMET ? PuppiMET_phi_corr : numb ");
}

void BaseAnalyser::bdt_variables()
{
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }

  _rlm = _rlm.Define("aplanery_sphericity", ::computeSphericityAplanarity, {"goodElectron_4vecs","goodmuons_4vecs","cleanjet4vecs"});
 _rlm = _rlm.Define("sphericity","aplanery_sphericity.first");
 _rlm = _rlm.Define("aplanery","aplanery_sphericity.second");
 _rlm = _rlm.Define("centrality",::calculate_centrality, {"goodElectron_TL4Vecs","goodmuons_TL4Vecs","selected_cleanbjet_TL4vec"});


_rlm = _rlm.Define("dilepton_invariant_mass","(goodElectron_TL4Vecs+goodmuons_TL4Vecs).M()");
_rlm = _rlm.Define("dilepton_jet_pt","(goodElectron_TL4Vecs+goodmuons_TL4Vecs+selected_cleanbjet_TL4vec).Pt()");
_rlm = _rlm.Define("dilepton_jet_mass","(goodElectron_TL4Vecs+goodmuons_TL4Vecs+selected_cleanbjet_TL4vec).M()");
_rlm = _rlm.Define("dilepton","goodElectron_TL4Vecs+goodmuons_TL4Vecs");
_rlm = _rlm.Define("dijet","selected_subleadingcleanjet_TL4vec + selected_leadingcleanjet_TL4vec");
//_rlm = _rlm.Define("Selected_loosejet_leadingpt", "int(Selected_loosejetpt.size())>0 ? Selected_bjetpt[Selected_loosejet_maxpt_index] : numb");
_rlm = _rlm.Define("dilepton_del_phi", :: calculate_deltaPhi_scalars, {"goodElectrons_leading_phi","goodmuons_leading_phi"});
_rlm = _rlm.Define("leading_lepton_jet_pt","(leading_lepton + selected_cleanbjet_TL4vec).Pt()");
_rlm = _rlm.Define("delR_dilepton_jet",:: calculate_deltaR,{"dilepton","dijet"});
_rlm = _rlm.Define("delR_ele_muon",:: calculate_deltaR,{"goodElectron_TL4Vecs","goodmuons_TL4Vecs"});
_rlm = _rlm.Define("delR_leadinglepton_jet",:: calculate_deltaR,{"leading_lepton","selected_cleanbjet_TL4vec"});

//_rlm = _rlm.Define("subleading_lepton_pt","subleading_lepton.Pt()");


}
//=============================define variables==================================================//

void BaseAnalyser::defineMoreVars()
{
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }

//    addVar({"good_muon1pt", "goodmuons_pt[0]", ""});

    //selected jet candidates
/*    addVar({"good_jet1pt", "(goodJets_pt.size()>0) ? goodJets_pt[0] : -1", ""});
    addVar({"Selected_jet1pt", "(Selected_jetpt.size()>0) ? Selected_jetpt[0] : -1", ""});
    addVar({"good_jet1eta", "goodJets_eta[0]", ""});
    addVar({"good_jet1mass", "goodJets_mass[0]", ""});*/

    //================================Store variables in tree=======================================//
    // define variables that you want to store
    //==============================================================================================//
    
    addVartoStore("genWeight");
    addVartoStore("Weight");
    //addVartoStore("genEventSumw");
    addVartoStore("evWeight");

    addVartoStore("PuppiMET_pt_corr");
    addVartoStore("PuppiMET_phi_corr");
    addVartoStore("no_puWeight");

    addVartoStore("goodElectrons_leading_pt");
    addVartoStore("goodElectrons_leading_eta");
    addVartoStore("goodElectrons_leading_phi");
    /*addVartoStore("goodmuons_leading_pt");
    addVartoStore("goodmuons_leading_eta");
    addVartoStore("goodmuons_leading_phi");
    addVartoStore("Weight");
    addVartoStore("leading_lepton_pt");
    addVartoStore("subleading_lepton_pt");
    addVartoStore("subleading_lepton_charge");
    addVartoStore("leptons_invariant_mass");
    addVartoStore("ele_SF_central");
    addVartoStore("no_puWeight");
    addVartoStore("sphericity");
    addVartoStore("aplanery");*/
    addVartoStore("region_1j1t");
    addVartoStore("region_2j1t");
    addVartoStore("region_2j2t");
    addVartoStore("eu_channel");
    addVartoStore("dilepton_invariant_mass");
    addVartoStore("dilepton_jet_pt");
    addVartoStore("Selected_loosejet_leadingpt");
    addVartoStore("dilepton_del_phi");
    addVartoStore("leading_lepton_jet_pt");
    addVartoStore("dilepton_jet_mass");
    addVartoStore("delR_dilepton_jet");
    addVartoStore("delR_ele_muon");
    addVartoStore("delR_leadinglepton_jet");
    addVartoStore("centrality");
    addVartoStore("Selected_jet_leading_pt");
    addVartoStore("Selected_jet_subleading_pt");
    addVartoStore("Selected_jet_leading_phi");
    addVartoStore("Selected_jet_leading_eta");
    addVartoStore("Selected_jet_leading_mass");
//    addVartoStore("Jet_pt");
    addVartoStore("btagpass_bcflav_goodJets");
    addVartoStore("goodJets_btagpass_bcflav_pt");
    addVartoStore("goodJets_btagpass_bcflav_eta");
    addVartoStore("all_bcflav_goodJets");
    addVartoStore("goodJets_all_bcflav_pt");
    addVartoStore("goodJets_all_bcflav_eta");
    addVartoStore("btagpass_lflav_goodJets");
    addVartoStore("goodJets_btagpass_lflav_pt");
    addVartoStore("goodJets_btagpass_lflav_eta");
    addVartoStore("all_lflav_goodJets");
    addVartoStore("goodJets_all_lflav_pt");
    addVartoStore("goodJets_all_lflav_eta");
    addVartoStore("run");
    addVartoStore("event");
    addVartoStore("luminosityBlock");
    addVartoStore("btag_SF_bcflav_central");
    addVartoStore("btag_SF_lflav_central");

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
//    if(!_isData){// && !isDefined("genWeight")){
//        add1DHist({"hgenWeight", "genWeight", 1001, -100, 100}, "genWeight", "one", "");
//}    
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
//    add1DHist( {"hT_mass_2j1t", "massdistributionof Top quark", 300, 0.0,1000.0}, "top_mass_2j1t", "evWeight", "");
//    add1DHist( {"hT_mass", "massdistributionof Top quark", 300, 0.0,1000.0}, "top_mass", "evWeight", "");  
//    add1DHist( {"hT_mass_2j0t", "massdistributionof Top quark", 300, 0.0,1000.0}, "top_mass_2j0t", "evWeight", "");
//    add1DHist( {"hT_mass_3j2t", "massdistributionof Top quark", 300, 0.0,1000.0}, "top_mass_3j2t", "evWeight", "");
//    add1DHist( {"helec_isolation", "electron isolation", 80, 0,0.1}, "ele_isolation", "evWeight", "");
  
//    add1DHist( {"hmuon_isolation_collection", "muon isolation in 2j0t control region", 10, 0.0, 50.0}, "muon_isolation_collection", "evWeight", "");

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
//    add2DHist( {"btagscalef", "btvcent_sf vs seljet_pt" , 100, 0, 500, 100, 0, 500} ,  "Selected_jetpt","btag_SF_case1", "one","");

    
}
void BaseAnalyser::setTree(TTree *t, std::string outfilename,float crossection,float sumgenWeight)
{
	if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }

	_rd = ROOT::RDataFrame(*t);
	_rlm = RNode(_rd);
	_crossection=crossection;
    _sumgenWeight=sumgenWeight;
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
        selectChannel();
	    selectJets();
        removeOverlaps();
        defineRegion();
	    calculateEvWeight(); // PU, genweight and BTV and Mu and Ele
	    selectMET();
//        reconstructWboson();
	    bdt_variables();
       auto Nentry_2 = _rlm.Count();
//    cout << "Usage of ranges:\n"
//        << " - All entries: " << *Nentry_2 << endl;

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
/*  if(!_isData){ 
		    auto sumgenweight1 = _rd.Sum("genWeight");
	    string sumofgenweight = Form("%f",*sumgenweight1);
	    _rlm = _rlm.Define("genEventSumw",sumofgenweight.c_str());
	    std::cout<<"Sum of genWeights = "<<sumofgenweight.c_str()<<std::endl;
}*/
	defineCuts();
	defineMoreVars();
	bookHists();
	setupCuts_and_Hists();
	setupTree();
}






