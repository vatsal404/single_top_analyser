


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
	HLT2017Names= {"HLT_IsoMu24","HLT_Ele32_WPTight_Gsf"};//HLT_IsoTkMu24
    HLT2016Names= {"Name1","Name2"};
}

void BaseAnalyser::selectChannel()
{
   _rlm = _rlm.Define("muonChannel", "(Ngoodmuons==1) && (N_iso_loose_mu==1) && (N_veto_el==0) ");

    _rlm = _rlm.Define("QCDmuonChannel", "(N_rev_iso_mu==1) && (N_veto_el==0) && (N_iso_loose_mu==1)");

    _rlm = _rlm.Define("electronChannel", "(NgoodElectrons==1) && (N_veto_el==1) && (N_iso_loose_mu==0)");
   
    _rlm = _rlm.Define("QCDelectronChannel", "(N_rev_iso_el==1) && (N_iso_loose_mu==0) && (N_veto_el==1)");

    _rlm = _rlm.Define("numbLorentzVector",::generate_TLorentzVector, {"numb", "numb", "numb", "numb"});
     _rlm = _rlm.Define("single_numb_vector",::generate_single_4vec, {"numb", "numb", "numb", "numb"});

    _rlm = _rlm.Define("lepton_TL4vec", "muonChannel==1 ? goodmuons_TL4Vecs : electronChannel==1 ?goodElectron_TL4Vecs : QCDmuonChannel==1? rev_iso_mu_TL4vec :QCDelectronChannel==1? rev_iso_el_TL4vec :numbLorentzVector")
	       .Define("lep_eta", "electronChannel==1 ? goodElectrons_leading_eta : muonChannel==1 ? goodmuons_leading_eta : QCDmuonChannel==1 ? rev_iso_mu_leading_eta : QCDelectronChannel==1 ? rev_iso_el_leading_eta : numb")
               .Define("lep_phi", "electronChannel==1 ? goodElectrons_leading_phi : muonChannel==1 ? goodmuons_leading_phi : QCDmuonChannel==1 ? rev_iso_mu_leading_phi : QCDelectronChannel==1 ? rev_iso_el_leading_phi : numb"); 
    _rlm = _rlm.Define("goodlepton_4vecs", "muonChannel==1 ? goodmuons_4vecs : electronChannel==1 ? goodElectron_4vecs : QCDmuonChannel==1? rev_iso_mu_4vecs :QCDelectronChannel==1? rev_iso_el_4vecs :std::vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >()");
_rlm = _rlm.Define("goodlepton_4vec", "muonChannel == 1 ? goodMuon_4vec : electronChannel == 1 ? goodElectron_4vec : QCDmuonChannel == 1 ? rev_iso_mu_4vec : QCDelectronChannel == 1 ? rev_iso_el_4vec : single_numb_vector");
    _rlm = _rlm.Define("lepton_charge", "(muonChannel==1) ? goodmuons_leading_charge : (electronChannel==1) ? goodElectrons_leading_charge  :QCDmuonChannel==1? rel_iso_muon_leading_charge : QCDelectronChannel==1? rel_iso_Electrons_leading_charge : numb");

}
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
	addCuts("nElectron+nMuon>=1 && nJet>1 && PV_ndof>4", "0");
//	addCuts("region_2j1t","1");
//    addCuts("ncleanjetspass>0","00");
	addCuts(setHLT(),"00"); //HLT cut buy checking HLT names in the root file

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
    _rlm = _rlm.Define("numb", "std::numeric_limits<double>::quiet_NaN()");
   
    _rlm = _rlm.Define("goodElectronsID", "Electron_cutBased==4"); //without pt-eta cuts
    _rlm = _rlm.Define("goodElectrons", "Electron_cutBased==4 && Electron_pt>35.0 && abs(Electron_eta+Electron_deltaEtaSC)<2.1 && Electron_pfRelIso03_all<0.0588 && abs(Electron_dxy)<0.5 && abs(Electron_dz)<1");
    _rlm = _rlm.Define("goodElectrons_pt", "Electron_pt[goodElectrons]")
		.Define("goodElectrons_leading_pt","int(goodElectrons_pt.size())>0 ? static_cast<double> (goodElectrons_pt[0]) : numb")

                .Define("goodElectrons_eta", "Electron_eta[goodElectrons]")
                .Define("goodElectrons_leading_eta","int(goodElectrons_eta.size())>0 ? static_cast<double> (goodElectrons_eta[0]) : numb")
               
		 .Define("goodElectrons_phi", "Electron_phi[goodElectrons]")
                 .Define("goodElectrons_leading_phi", "int(goodElectrons_phi.size())>0 ?static_cast<double>(goodElectrons_phi[0]) :numb")

		.Define("goodElectrons_mass", "Electron_mass[goodElectrons]")
                .Define("goodElectrons_leading_mass", "int(goodElectrons_mass.size())>0 ?static_cast<double>(goodElectrons_mass[0]) :numb")

		.Define("goodElectrons_charge","Electron_charge[goodElectrons]")
		.Define("goodElectrons_leading_charge","int(goodElectrons_charge.size())>0 ?static_cast<double>(goodElectrons_charge[0]) :numb")

		.Define("goodElectrons_idx", ::good_idx, {"goodElectrons"})
                .Define("NgoodElectrons", "int(goodElectrons_pt.size())");

    //-------------------------------------------------------
    //generate electron 4vector from selected good Electrons
    //-------------------------------------------------------
    _rlm = _rlm.Define("goodElectron_4vecs", ::generate_4vec, {"goodElectrons_pt", "goodElectrons_eta", "goodElectrons_phi", "goodElectrons_mass"});
    _rlm = _rlm.Define("goodElectron_TL4Vecs", ::generate_TLorentzVector, {"goodElectrons_leading_pt", "goodElectrons_leading_eta", "goodElectrons_leading_phi", "goodElectrons_leading_mass"});
    _rlm = _rlm.Define("goodElectron_4vec", ::generate_single_4vec, {"goodElectrons_leading_pt", "goodElectrons_leading_eta", "goodElectrons_leading_phi", "goodElectrons_leading_mass"});


    /*-----------------revert isolated Electron----------*/
    _rlm = _rlm.Define("rev_iso_el", "Electron_cutBased<=2 && Electron_pt>35 && abs(Electron_eta+Electron_deltaEtaSC)<2.1 && Electron_pfRelIso03_all<0.85");
    _rlm = _rlm.Define("rev_iso_el_pt_collection", "Electron_pt[rev_iso_el]")
               .Define("rev_iso_el_leading_pt", "int(rev_iso_el_pt_collection.size())>0 ? rev_iso_el_pt_collection[0] : numb")

               .Define("rev_iso_el_eta_collection", "Electron_eta[rev_iso_el]")
               .Define("rev_iso_el_leading_eta", "int(rev_iso_el_eta_collection.size())>0 ? rev_iso_el_eta_collection[0] : numb")

               .Define("rev_iso_el_phi_collection", "Electron_phi[rev_iso_el]")
               .Define("rev_iso_el_leading_phi", "int(rev_iso_el_phi_collection.size())>0 ? rev_iso_el_phi_collection[0] : numb")

               .Define("rev_iso_el_mass_collection", "Electron_mass[rev_iso_el]")
               .Define("rev_iso_el_leading_mass", "int(rev_iso_el_mass_collection.size())>0 ? rev_iso_el_mass_collection[0] : numb")

               .Define("rev_iso_el_rel_iso_collection","Electron_pfRelIso03_all[rev_iso_el]")
               .Define("log10_rev_iso_el_rel_iso_collection", "log10(rev_iso_el_rel_iso_collection)")

	       .Define("rel_iso_Electrons_charge","Electron_charge[rev_iso_el]")
		.Define("rel_iso_Electrons_leading_charge","int(rel_iso_Electrons_charge.size())>0 ?static_cast<double>(rel_iso_Electrons_charge[0]) :numb")


               .Define("N_rev_iso_el", "int(rev_iso_el_pt_collection.size())");

    _rlm = _rlm.Define("rev_iso_el_TL4vec", ::generate_TLorentzVector, {"rev_iso_el_leading_pt", "rev_iso_el_leading_eta", "rev_iso_el_leading_phi", "rev_iso_el_leading_mass"});
    _rlm = _rlm.Define("rev_iso_el_4vec", ::generate_single_4vec, {"rev_iso_el_leading_pt", "rev_iso_el_leading_eta", "rev_iso_el_leading_phi", "rev_iso_el_leading_mass"});
    _rlm = _rlm.Define("rev_iso_el_4vecs", ::generate_4vec, {"rev_iso_el_pt_collection", "rev_iso_el_eta_collection", "rev_iso_el_phi_collection", "rev_iso_el_mass_collection"});


   /*--------------- veto Electron ID ---------------*/
    _rlm = _rlm.Define("veto_el", "Electron_cutBased>=1 && Electron_pt>15 && abs(Electron_eta+Electron_deltaEtaSC)<2.5");
    _rlm = _rlm.Define("veto_el_pt_collection", "Electron_pt[veto_el]")
               .Define("N_veto_el", "int(veto_el_pt_collection.size())");


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

    _rlm = _rlm.Define("goodmuonsID", MuonID(2));
    _rlm = _rlm.Define("iso_loose_mu"," goodmuonsID && Muon_pfRelIso04_all<0.25"); //loose muons
    _rlm = _rlm.Define("iso_loose_mu_pT_collection"," Muon_pt_corr[iso_loose_mu]");
    _rlm = _rlm.Define("N_iso_loose_mu", "int(iso_loose_mu_pT_collection.size())");

 //loose muons
    _rlm = _rlm.Define("goodmuons", "goodmuonsID && Muon_highPurity && Muon_pt_corr > 29 && abs(Muon_eta) < 2.4 && Muon_pfRelIso04_all < 0.06 && abs(Muon_dxy)<2 && abs(Muon_dz<5) && Muon_isGlobal==1");
      _rlm = _rlm.Define("goodmuons_pt", "Muon_pt_corr[goodmuons]")
		 .Define("goodmuons_leading_pt", "int(goodmuons_pt.size())>0 ? static_cast<double>(goodmuons_pt[0]) : numb") 
                
		.Define("goodmuons_eta", "Muon_eta[goodmuons]")
                .Define("goodmuons_leading_eta", "int(goodmuons_eta.size())>0 ? static_cast<double>(goodmuons_eta[0]) :numb")
 
               .Define("goodmuons_phi", "Muon_phi[goodmuons]")
               .Define("goodmuons_leading_phi", "int(goodmuons_phi.size())>0 ? static_cast<double>(goodmuons_phi[0]) : numb")

                .Define("goodmuons_mass", "Muon_mass[goodmuons]")
		.Define("goodmuons_leading_mass", "int(goodmuons_mass.size())>0 ? static_cast<double>(goodmuons_mass[0]) :numb")
      
                .Define("goodmuons_isolation", "Muon_pfRelIso04_all[goodmuons]")

 		.Define("gooodmuons_charge","Muon_charge[goodmuons]")
                .Define("goodmuons_leading_charge","int(gooodmuons_charge.size())>0 ?static_cast<double>(gooodmuons_charge[0]) :numb")

		.Define("goodmuons_charge", "Muon_charge[goodmuons]")
                .Define("goodmuons_idx", ::good_idx, {"goodmuons"})
                .Define("Ngoodmuons", "int(goodmuons_pt.size())");



    //-------------------------------------------------------
    //generate muon 4vector from selected good Muons
    //-------------------------------------------------------
    _rlm = _rlm.Define("goodmuons_4vecs", ::generate_4vec, {"goodmuons_pt", "goodmuons_eta", "goodmuons_phi", "goodmuons_mass"});
    _rlm = _rlm.Define("goodmuons_TL4Vecs", ::generate_TLorentzVector, {"goodmuons_leading_pt", "goodmuons_leading_eta", "goodmuons_leading_phi", "goodmuons_leading_mass"});
    _rlm = _rlm.Define("goodMuon_4vec", ::generate_single_4vec, {"goodmuons_leading_pt", "goodmuons_leading_eta", "goodmuons_leading_phi", "goodmuons_leading_mass"});


    /*--------------- Reverted isolated Muons ID ---------------*/
    _rlm = _rlm.Define("rev_iso_mu", "goodmuonsID && Muon_pt_corr > 29 && abs(Muon_eta) < 2.4 && Muon_pfRelIso04_all > 0.2 && Muon_isGlobal==1");
//    _rlm = _rlm.Define("rev_iso_mu","Muon_looseId && Muon_pfRelIso04_all>0.20"); // Reversed Isolated Muon Cut
 // Reversed Isolated Muon Cut
    _rlm = _rlm.Define("rev_iso_mu_pt_collection", "Muon_pt_corr[rev_iso_mu]")
               .Define("rev_iso_mu_leading_pt", "int(rev_iso_mu_pt_collection.size())>0 ? rev_iso_mu_pt_collection[0] : numb")

               .Define("rev_iso_mu_eta_collection", "Muon_eta[rev_iso_mu]")
               .Define("rev_iso_mu_leading_eta", "int(rev_iso_mu_eta_collection.size())>0 ? rev_iso_mu_eta_collection[0] : numb")

               .Define("rev_iso_mu_phi_collection", "Muon_phi[rev_iso_mu]")
               .Define("rev_iso_mu_leading_phi", "int(rev_iso_mu_phi_collection.size())>0 ? rev_iso_mu_phi_collection[0] : numb")

               .Define("rev_iso_mu_mass_collection", "Muon_mass[rev_iso_mu]")
               .Define("rev_iso_mu_leading_mass", "int(rev_iso_mu_mass_collection.size())>0 ? rev_iso_mu_mass_collection[0] : numb")

               .Define("rev_iso_mu_charge_collection", "Muon_charge[rev_iso_mu]")
               .Define("rev_iso_mu_leading_charge", "int(rev_iso_mu_charge_collection.size())>0 ? rev_iso_mu_charge_collection[0] : numb")

               .Define("rev_iso_mu_relIso_dR0p3_collection", "Muon_pfRelIso03_all[rev_iso_mu]")

	       .Define("rel_iso_muon_charge","Muon_charge[rev_iso_mu]")
                .Define("rel_iso_muon_leading_charge","int(rel_iso_muon_charge.size())>0 ?static_cast<double>(rel_iso_muon_charge[0]) :numb")

               .Define("N_rev_iso_mu", "int(rev_iso_mu_pt_collection.size())");

    _rlm = _rlm.Define("rev_iso_mu_4vec", ::generate_single_4vec, {"rev_iso_mu_leading_pt", "rev_iso_mu_leading_eta", "rev_iso_mu_leading_phi", "rev_iso_mu_leading_mass"});
    _rlm = _rlm.Define("rev_iso_mu_TL4vec", ::generate_TLorentzVector, {"rev_iso_mu_leading_pt", "rev_iso_mu_leading_eta", "rev_iso_mu_leading_phi", "rev_iso_mu_leading_mass"});
    _rlm = _rlm.Define("rev_iso_mu_4vecs", ::generate_4vec, {"rev_iso_mu_pt_collection", "rev_iso_mu_eta_collection", "rev_iso_mu_phi_collection", "rev_iso_mu_mass_collection"});

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
    _rlm = _rlm.Define("nu_pt", "static_cast<float>(goodMET_pt_corr)")
               .Define("nu_phi", "static_cast<float>(goodMET_phi_corr)")
               .Define("nu_phi_double", "static_cast<double>(nu_phi)")

               .Define("nu_px", "nu_pt*cos(nu_phi)")
               .Define("nu_py", "nu_pt*sin(nu_phi)");


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
	       .Define("lepton_pt","lepton_TL4vec.Pt()")
               .Define("nu4vec_pt","nu_TL4vec.Pt()")
               .Define("w_pt","Wboson_4vec.Pt()");

    // _rlm = _rlm.Define("Wboson_transversMass", "Wboson_4vec.Mt()");

   //  _rlm = _rlm.Define("Wboson_transversMass", "sqrt(pow(lepton_TL4vec.Pt()+nu_pt,2)-pow(nu_pt*cos(nu_phi)+lepton_TL4vec.Px(),2) - pow(nu_pt*sin(nu_phi)+lepton_TL4vec.Py(),2))");
    _rlm = _rlm.Define("delta_phi_lep_nu", ::calculate_deltaPhi_scalars, {"lep_phi", "nu_phi_double"})
    .Define("Wboson_transversMass", "sqrt(2*lepton_TL4vec.Pt()*nu_pt*(1-cos(delta_phi_lep_nu)))")
    .Define("Wboson_transversMass_45", "sqrt(2*45*nu_pt*(1-cos(delta_phi_lep_nu)))");

    _rlm=_rlm.Define("Wboson_transversemass_ele","electronChannel && region_2j0t ? Wboson_transversMass : numb");
    _rlm=_rlm.Define("Wboson_transversemass_ele_barrel","electronChannel && region_2j0t && lep_eta<1.47 ? Wboson_transversMass : numb");
    _rlm=_rlm.Define("Wboson_transversemass_ele_endcap","electronChannel && region_2j0t && lep_eta>1.47 ? Wboson_transversMass : numb");

    _rlm=_rlm.Define("Wboson_transversemass_muon","muonChannel && region_2j0t ? Wboson_transversMass : numb");
   // _rlm=_rlm.Define("Wboson_transversemass_QCDele","QCDelectronChannel && region_2j0t ? Wboson_transversMass : numb");
       // _rlm=_rlm.Define("Wboson_transversemass_QCDele","QCDelectronChannel && region_2j0t ? Wboson_transversMass : numb");

    _rlm=_rlm.Define("Wboson_transversemass_QCDele_barrel","QCDelectronChannel && region_2j0t && lep_eta<1.47 ? Wboson_transversMass : numb");
    _rlm=_rlm.Define("Wboson_transversemass_QCDele_endcap","QCDelectronChannel && region_2j0t && lep_eta>1.47 ? Wboson_transversMass : numb");

    _rlm=_rlm.Define("Wboson_transversemass_QCDmuon","QCDmuonChannel && region_2j0t ? Wboson_transversMass : numb");

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
//    _rlm = _rlm.Define("goodJets", "goodJetsID && Jet_pt_corr>30.0 && abs(Jet_eta)<2.4 ");
    _rlm = _rlm.Define("goodJets_high_eta", "Jet_pt_corr>40.0 && ((abs(Jet_eta)<4.7 && abs(Jet_eta)>3.0) || (abs(Jet_eta)>0.0 && abs(Jet_eta)<2.7)) ");
    _rlm = _rlm.Define("goodJets_low_eta", "Jet_pt_corr>50.0 && abs(Jet_eta)<3.0 && abs(Jet_eta)>2.7 ");
    _rlm = _rlm.Define("goodJets", " goodJets_high_eta || goodJets_low_eta ");


    _rlm = _rlm.Define("goodJets_pt", "Jet_pt_corr[goodJets]")
                .Define("goodJets_eta", "Jet_eta[goodJets]")
                .Define("goodJets_phi", "Jet_phi[goodJets]")
                .Define("goodJets_mass", "Jet_mass[goodJets]")
                .Define("goodJets_idx", ::good_idx, {"goodJets"});
    if(!_isData){

  _rlm = _rlm.Define("goodJets_hadflav", "Jet_hadronFlavour[goodJets]");
}
    //goot jets deep-b tag
	_rlm = _rlm.Define("goodJets_jetdeepbtag", "Jet_btagDeepB[goodJets]")
                .Define("goodJets_deepjetbtag", "Jet_btagDeepFlavB[goodJets]")
                .Define("NgoodJets", "int(goodJets_pt.size())")
                .Define("goodJets_4vecs", ::generate_4vec, {"goodJets_pt", "goodJets_eta", "goodJets_phi", "goodJets_mass"});


	//select b jest within goodjets 
    _rlm = _rlm.Define("btagcuts", "goodJets_deepjetbtag>0.7476") //0.2783 -medium, 0.7476 - tight 
      .Define("good_bjetpt", "goodJets_pt[btagcuts]")
      .Define("good_bjet_maxpt_index", "ArgMax(good_bjetpt)")

      .Define("good_bjet_leading_pt", "int(good_bjetpt.size()) > 0 ? good_bjetpt[good_bjet_maxpt_index] : numb")

      .Define("good_bjeteta", "goodJets_eta[btagcuts]")
      .Define("good_bjet_leading_eta", "int(good_bjeteta.size()) > 0 ? good_bjeteta[good_bjet_maxpt_index] : numb")

     .Define("good_bjetphi", "goodJets_phi[btagcuts]")
      .Define("good_bjet_leading_phi", "int(good_bjetphi.size()) > 0 ? good_bjetphi[good_bjet_maxpt_index] : numb")

      .Define("good_bjetmass", "goodJets_mass[btagcuts]")
      .Define("good_bjet_leading_mass", "int(good_bjetmass.size()) > 0 ? good_bjetmass[good_bjet_maxpt_index] : numb")

      .Define("good_bjetdeepjet", "goodJets_deepjetbtag[btagcuts]");
    if(!_isData){

    _rlm = _rlm.Define("good_bjethadflav", "goodJets_hadflav[btagcuts]");
}    
    _rlm = _rlm.Define("Ngood_bjets", "int(good_bjetpt.size())")
      .Define("good_bjet_TL4vec", ::generate_TLorentzVector, {"good_bjet_leading_pt","good_bjet_leading_eta","good_bjet_leading_phi","good_bjet_leading_mass"})
      .Define("bjet_mass","good_bjet_TL4vec.M()")
      .Define("good_bjet4vecs", ::generate_4vec, {"good_bjetpt", "good_bjeteta", "good_bjetphi", "good_bjetmass"});
     _rlm = _rlm.Define("goodbjet_single_4vec",::generate_single_4vec,{"good_bjet_leading_pt","good_bjet_leading_eta","good_bjet_leading_phi","good_bjet_leading_mass"});


    if(!_isData){
      //For Btagging Efficiency    
      _rlm = _rlm.Define("btagpass_bcflav_goodJets", "goodJets_deepjetbtag>0.7476 && goodJets_hadflav!=0") //0.2783 -medium, 0.7 - tight 
	.Define("goodJets_btagpass_bcflav_pt", "goodJets_pt[btagpass_bcflav_goodJets]")
	.Define("goodJets_btagpass_bcflav_eta", "goodJets_eta[btagpass_bcflav_goodJets]");
      
      _rlm = _rlm.Define("all_bcflav_goodJets", "goodJets_hadflav!=0") //0.2783 -medium, 0.7 - tight 
	.Define("goodJets_all_bcflav_pt", "goodJets_pt[all_bcflav_goodJets]")
	.Define("goodJets_all_bcflav_eta", "goodJets_eta[all_bcflav_goodJets]");
      
      
      _rlm = _rlm.Define("btagpass_lflav_goodJets", "goodJets_deepjetbtag>0.7476 && goodJets_hadflav==0") //0.2783 -medium, 0.7 - tight 
	.Define("goodJets_btagpass_lflav_pt", "goodJets_pt[btagpass_lflav_goodJets]")
	.Define("goodJets_btagpass_lflav_eta", "goodJets_eta[btagpass_lflav_goodJets]");
      
      _rlm = _rlm.Define("all_lflav_goodJets", "goodJets_hadflav==0") //0.2783 -medium, 0.7 - tight 
	.Define("goodJets_all_lflav_pt", "goodJets_pt[all_lflav_goodJets]")
	.Define("goodJets_all_lflav_eta", "goodJets_eta[all_lflav_goodJets]");
    }
}

void BaseAnalyser::spectatorJets()
{
    cout << "select good jets" << endl;
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }
    _rlm = _rlm.Define("specJet", "Selected_jetbtag <= 0.7476")
    
               // First get the filtered collections
               .Define("specJet_eta_collection", "Selected_jeteta[specJet]")
	       .Define("specJet_maxeta_index", "ArgMax(specJet_eta_collection)")
               // Then get the leading values, checking collection size first
               .Define("specJet_leading_eta", "specJet_eta_collection.size() > 0 ? specJet_eta_collection[specJet_maxeta_index] : numb")
               
               .Define("specJet_pt_collection", "Selected_jetpt[specJet]")
               .Define("specJet_leading_pt", "specJet_pt_collection.size() > 0 ? specJet_pt_collection[specJet_maxeta_index] : numb")
               
               .Define("specJet_phi_collection", "Selected_jetphi[specJet]")
               .Define("specJet_leading_phi", "specJet_phi_collection.size() > 0 ? specJet_phi_collection[specJet_maxeta_index] : numb")
               
               .Define("specJet_mass_collection", "Selected_jetmass[specJet]")
               .Define("specJet_leading_mass","specJet_mass_collection.size() > 0 ? specJet_mass_collection[specJet_maxeta_index] : numb");
               
           //    .Define("NspecJets_2j0t", "int(specJet_2j0t_pt_collection.size())");
    _rlm = _rlm.Define("specJet_single_4vec",::generate_single_4vec,{"specJet_leading_pt","specJet_leading_eta","specJet_leading_phi","specJet_leading_mass"});
 _rlm = _rlm.Define("specJet_single_TL4vec",:: generate_TLorentzVector,{"specJet_leading_pt", "specJet_leading_eta", "specJet_leading_phi", "specJet_leading_mass"});

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
	
	_rlm = _rlm.Define("lepton_forOverlapCheck", "electronChannel==1 ? goodElectron_4vec : muonChannel==1 ? goodMuon_4vec : QCDmuonChannel==1 ? rev_iso_mu_4vec : QCDelectronChannel==1 ? rev_iso_el_4vec : numb4Vector");
	
	_rlm = _rlm.Define("checkOverlap", checkoverlap, {"goodJets_4vecs","lepton_forOverlapCheck"});
//        _rlm = _rlm.Define("checkOverlap", "ROOT::RVec<int>(numb, 1)");
	
std::cout << "Number of entries: " << _rlm.Count().GetValue() << std::endl;

  //  _rlm = _rlm.Define("muonjetoverlap", checkoverlap, {"goodJets_4vecs","goodmuons_4vecs"});
	_rlm =	_rlm.Define("Selected_jeteta", "goodJets_eta[checkOverlap]")
		   .Define("Selected_jeteta_mineta_index", "ArgMin(Selected_jeteta)")
                   .Define("Selected_jet_leading_eta", "int(Selected_jeteta.size())>0 ? Selected_jeteta[Selected_jeteta_mineta_index] : numb")

		.Define("Selected_jetphi", "goodJets_phi[checkOverlap]")
	        .Define("Selected_jet_leading_phi", "int(Selected_jetphi.size())>0 ? Selected_jetphi[Selected_jeteta_mineta_index] : numb")

	    	.Define("Selected_jetpt", "goodJets_pt[checkOverlap]")
                .Define("Selected_jet_leading_pt", "int(Selected_jetpt.size())>0 ? Selected_jetpt[Selected_jeteta_mineta_index] : numb")

		.Define("Selected_jetmass", "goodJets_mass[checkOverlap]")
                .Define("Selected_jet_leading_mass", "int(Selected_jetphi.size())>0 ? Selected_jetmass[Selected_jeteta_mineta_index] : numb")

		.Define("Selected_jetbtag", "goodJets_deepjetbtag[checkOverlap]") //
//		.Define("Selected_jethadflav", "goodJets_hadflav[checkOverlap]") 
		.Define("ncleanjetspass", "int(Selected_jetpt.size())")
		.Define("cleanjet4vecs", ::generate_4vec, {"Selected_jetpt", "Selected_jeteta", "Selected_jetphi", "Selected_jetmass"})
		.Define("Selected_jetHT", "Sum(Selected_jetpt)");
	 _rlm = _rlm.Define("selected_cleanjet_TL4vec",:: generate_TLorentzVector,{"Selected_jet_leading_pt", "Selected_jet_leading_eta", "Selected_jet_leading_phi", "Selected_jet_leading_mass"});




    if(!_isData){
            _rlm= _rlm .Define("Selected_jethadflav", "goodJets_hadflav[checkOverlap]"); 

}      
     //==============================Clean b-Jets==============================================// 
	 //--> after remove overlap: use requested btaggedJets for btag-weight SFs && weight_generator. 
	 //=====================================================================================//
	_rlm = _rlm.Define("btagcuts2", "Selected_jetbtag>=0.7476") //medium wp -->as an example. 
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
		

    if(!_isData){
            _rlm= _rlm .Define("Selected_bjethadflav", "Selected_jethadflav[btagcuts2]");


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
/*
    _rlm = _rlm.Define("region_2j1t", "ncleanjetspass == 2 && Ngood_bjets== 1 ? 1 : 0");
    _rlm = _rlm.Define("region_2j0t", "ncleanjetspass == 2 && Ngood_bjets == 0 ? 1 : 0");
    _rlm = _rlm.Define("region_3j2t", "ncleanjetspass >= 3 && Ngood_bjets >= 2 ? 1 : 0");

    _rlm = _rlm.Define("region", "region_2j1t == 1 ? 0.0 : region_2j0t == 1 ? 1.0 : region_3j2t == 1 ? 2.0 :-1.0");
*/

_rlm = _rlm.Define("region_2j1t", "ncleanjetspass == 2 && ncleanbjetspass == 1");
_rlm = _rlm.Define("region_2j0t", "ncleanjetspass== 2 && ncleanbjetspass == 0");
_rlm = _rlm.Define("region_3j2t", "ncleanjetspass== 3 && ncleanbjetspass >= 1");

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
    _rlm = _rlm.Define("bQuark_forReco", " region_2j1t || region_3j2t ? selected_cleanbjet_TL4vec : region_2j0t ? selected_cleanjet_TL4vec : numbLorentzVector")
               .Define("topQuark_TL4vec", "Wboson_4vec + bQuark_forReco");

    _rlm = _rlm.Define("top_mass","topQuark_TL4vec.M()" )
               .Define("top_mass_2j1t_ele","electronChannel && region_2j1t ? top_mass : numb")
               .Define("top_mass_2j1t_muon","muonChannel && region_2j1t ? top_mass : numb")

               .Define("top_mass_2j0t","Wboson_transversMass >50 && region_2j0t ? top_mass : numb")
               .Define("top_mass_3j2t","Wboson_transversMass >50 && region_3j2t ? top_mass : numb")

               .Define("top_pt", "Wboson_transversMass>50 ? topQuark_TL4vec.Pt() : numb");

}
void BaseAnalyser::calculateEvWeight(){


  //Scale Factors for BTag ID	
  int _case = 1;
  std::vector<std::string> Jets_vars_names = {"Selected_jethadflav", "Selected_jeteta",  "Selected_jetpt"};  
  if(_case !=1){
    Jets_vars_names.emplace_back("Selected_jetbtag");
  }
  std::string output_btag_column_name = "btag_SF_";

//  _rlm = calculateBTagSF(_rlm, Jets_vars_names, _case, 0.2783, "M", output_btag_column_name);
  _rlm = calculateBTagSF(_rlm, Jets_vars_names, 1, output_btag_column_name);

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
//  _rlm = _rlm.Define("evWeight", " pugenWeight * prefiring_SF_central * btag_SF_bcflav_central * btag_SF_lflav_central * muon_SF_central * ele_SF_central"); 
  _rlm = _rlm.Define("evWeight", " puWeight * prefiring_SF_central * muon_SF_central * ele_SF_central * btag_SF_central"); 

}
//MET

void BaseAnalyser::selectMET()
{
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }

   _rlm = _rlm.Define("goodMET", "MET_pt_corr>=20")  // Boolean flag
          .Define("goodMET_pt_corr", "goodMET ? MET_pt_corr : numb")  // Assign numb for events failing cut
          .Define("goodMET_phi_corr", "goodMET ? MET_phi_corr : numb ");

    //            .Define("goodMET_eta","MET_eta[goodMET]")
   //
     //           .Define("goodMET_phi_corr","MET_phi_corr[goodMET]");
                //.Define("NgoodMET","int(goodMET_pt_corr.size())");
    //_rlm = _rlm.Define("goodMet", "MET_sumEt>600 && MET_pt_corr>5");
    //_rlm = _rlm.Define("goodMET_pt_corr", "MET_pt_corr[goodMet]");

    
}

/*
double numb=std::numeric_limits<double>::quiet_NaN();


double calculate_deltaEtaWrapper(bool region, bool channel, const FourVector& lepton, const FourVector& goodbjet , double Wboson_transversMass) {
    return region && channel && Wboson_transversMass>=50 ? calculate_deltaEta(lepton, goodbjet) : numb;
}


double computeDeltaRWrapper(const bool region, const bool channel, const FourVector& specJet, const FourVector& goodbjet, double Wboson_transversMass) {
    return (region && channel && Wboson_transversMass>=50) ? calculate_deltaR(specJet, goodbjet) : numb;
}

float eventShapeWrapper(const bool region,const bool channel,
                        const FourVectorVec &leptons,
                        const FourVectorVec &jets, double Wboson_transversMass)
{
    FourVectorVec combined = leptons;
    combined.insert(combined.end(), jets.begin(), jets.end());

    // Call the event_shape function to compute event shape
    return (region && channel && Wboson_transversMass>=50) ? event_shape(combined):numb;
}

float calculateWHelicityAngle_TL_wrapper(const TLorentzVector &lepton, const TLorentzVector &wBoson, const TLorentzVector &top,bool region, bool channel, double Wboson_transversMass){
    return (region && channel && Wboson_transversMass>=50) ? calculateWHelicityAngle(lepton,wBoson,top):numb;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//--------------------------------------------------------------------------------------------------------------------------------------

*/

void BaseAnalyser::Background_Estimation()
{
    if (debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }


//_rlm = _rlm.Define("muon_2j0t", "region_2j0t && QCDmuonChannel")

//           .Define("muon_isolation_collection","Muon_pfRelIso04_all[muon_2j0t]");
//           .Define("muon_isolation", "int(muon_isolation_collection.size())>0 ? muon_isolation_collection[0] : numb");
std::cout << "Type of muon_isolation_collection: " 
          << _rlm.GetColumnType("Muon_pt_corr") 
          << std::endl;
_rlm = _rlm.Define("muon_isolation", "region_2j0t && muonChannel  ? static_cast<double>(Muon_pfRelIso04_all[0]) : numb");
_rlm = _rlm.Define("ele_isolation", "region_2j0t && electronChannel ? static_cast<double>(Electron_pfRelIso03_all[0]) : numb");
_rlm = _rlm.Define("ele_isolation_barrel", "region_2j0t && electronChannel && abs(Electron_eta[0])<1.47 ?static_cast<double>(Electron_pfRelIso03_all[0]) : numb");
_rlm = _rlm.Define("ele_isolation_endcap", "region_2j0t && electronChannel && abs(Electron_eta[0])>1.6 ?static_cast<double>(Electron_pfRelIso03_all[0]) : numb");
_rlm = _rlm.Define("MET_pt_2j0t_muon", "region_2j0t && muonChannel ?static_cast<double>(MET_pt_corr) : numb");
_rlm = _rlm.Define("MET_pt_2j0t_ele_barrel", "region_2j0t && electronChannel && abs(Electron_eta[0])<1.47 ? static_cast<double>(MET_pt_corr) : numb");
_rlm = _rlm.Define("MET_pt_2j0t_ele_endcap", "region_2j0t && electronChannel && abs(Electron_eta[0])>1.47 ? static_cast<double>(MET_pt_corr) : numb");
_rlm = _rlm.Define("Pt_2j0t_muon", "region_2j0t && muonChannel ? static_cast<double>(Muon_pt_corr[0]) : numb");
_rlm = _rlm.Define("eta_2j0t_muon", "region_2j0t && muonChannel ? static_cast<double>(Muon_eta[0]) : numb");
_rlm = _rlm.Define("eta_2j0t_ele", "region_2j0t && electronChannel ? static_cast<double>(Electron_eta[0]) : numb");
_rlm = _rlm.Define("delphi_2j0t_muon", "region_2j0t && muonChannel ? static_cast<double>(goodmuons_leading_phi-MET_phi_corr) : numb");
_rlm = _rlm.Define("delphi_2j0t_ele", "region_2j0t && electronChannel ? static_cast<double>(goodElectrons_leading_phi-MET_phi_corr) : numb");
_rlm = _rlm.Define("delphi_2j0t_ele_barrel", "region_2j0t && electronChannel && abs(Electron_eta[0])<1.47 ? static_cast<double>(goodElectrons_leading_phi-MET_phi_corr) : numb");
_rlm = _rlm.Define("delphi_2j0t_ele_endcap", "region_2j0t && electronChannel && abs(Electron_eta[0])>1.47? static_cast<double>(goodElectrons_leading_phi-MET_phi_corr) : numb");






_rlm = _rlm.Define("bdt_MET_pt_muon_2j0t", "region_2j0t && muonChannel && Wboson_transversMass>=50 ? MET_pt_corr : numb");
_rlm = _rlm.Define("bdt_MET_pt_muon_2j1t", "region_2j1t && muonChannel && Wboson_transversMass>=50 ? MET_pt_corr : numb");
_rlm = _rlm.Define("bdt_MET_pt_muon_3j2t", "region_3j2t && muonChannel && Wboson_transversMass>=50 ? MET_pt_corr : numb");
_rlm = _rlm.Define("bdt_MET_pt_ele_2j0t", "region_2j0t && electronChannel && Wboson_transversMass>=50 ? MET_pt_corr : numb");
_rlm = _rlm.Define("bdt_MET_pt_ele_2j1t", "region_2j1t && electronChannel && Wboson_transversMass>=50 ? MET_pt_corr : numb");
_rlm = _rlm.Define("bdt_MET_pt_ele_3j2t", "region_3j2t && electronChannel && Wboson_transversMass>=50 ? MET_pt_corr : numb");

_rlm = _rlm.Define("bdt_wboson_muon_2j0t", "region_2j0t && muonChannel ? Wboson_transversMass : numb");
_rlm = _rlm.Define("bdt_wboson_muon_2j1t", "region_2j1t && muonChannel ? Wboson_transversMass : numb");
_rlm = _rlm.Define("bdt_wboson_muon_3j2t", "region_3j2t && muonChannel ? Wboson_transversMass : numb");
_rlm = _rlm.Define("bdt_wboson_ele_2j0t", "region_2j0t && electronChannel ? Wboson_transversMass : numb");
_rlm = _rlm.Define("bdt_wboson_ele_2j1t", "region_2j1t && electronChannel ? Wboson_transversMass : numb");
_rlm = _rlm.Define("bdt_wboson_ele_3j2t", "region_3j2t && electronChannel ? Wboson_transversMass : numb");

_rlm = _rlm.Define("bdt_top_muon_2j0t", "region_2j0t && muonChannel && Wboson_transversMass>=50 ? top_mass-172.5 : numb");
_rlm = _rlm.Define("bdt_top_muon_2j1t", "region_2j1t && muonChannel && Wboson_transversMass>=50 ? top_mass-172.5 : numb");
_rlm = _rlm.Define("bdt_top_muon_3j2t", "region_3j2t && muonChannel && Wboson_transversMass>=50 ? top_mass-172.5 : numb");
_rlm = _rlm.Define("bdt_top_ele_2j0t", "region_2j0t && electronChannel && Wboson_transversMass>=50 ? top_mass-172.5 : numb");
_rlm = _rlm.Define("bdt_top_ele_2j1t", "region_2j1t && electronChannel && Wboson_transversMass>=50 ? top_mass-172.5 : numb");
_rlm = _rlm.Define("bdt_top_ele_3j2t", "region_3j2t && electronChannel && Wboson_transversMass>=50 ? top_mass-172.5 : numb");

_rlm = _rlm.Define("bdt_specJet_2j0t_leading_eta_muon", "region_2j0t &&  muonChannel && Wboson_transversMass>=50 ? specJet_leading_eta : numb");
_rlm = _rlm.Define("bdt_specJet_2j0t_leading_eta_ele", "region_2j0t &&  electronChannel && Wboson_transversMass>=50 ? specJet_leading_eta : numb");
_rlm = _rlm.Define("bdt_specJet_2j1t_leading_eta_muon", "region_2j1t &&  muonChannel && Wboson_transversMass>=50 ? specJet_leading_eta : numb");
_rlm = _rlm.Define("bdt_specJet_2j1t_leading_eta_ele", "region_2j1t &&  electronChannel && Wboson_transversMass>=50 ? specJet_leading_eta : numb");
_rlm = _rlm.Define("bdt_specJet_3j2t_leading_eta_muon", "region_3j2t &&  muonChannel && Wboson_transversMass>=50 ? specJet_leading_eta : numb");
_rlm = _rlm.Define("bdt_specJet_3j2t_leading_eta_ele", "region_3j2t &&  electronChannel && Wboson_transversMass>=50 ? specJet_leading_eta : numb");

_rlm = _rlm.Define("bdt_delR", ::calculate_deltaR, {"specJet_single_4vec", "selected_cleanbjet_4vec"});

_rlm = _rlm.Define("bdt_delR_muon_2j1t", "region_2j1t &&  muonChannel && Wboson_transversMass>=50 ? bdt_delR : numb");
_rlm = _rlm.Define("bdt_delR_muon_2j0t", "region_2j0t &&  muonChannel && Wboson_transversMass>=50 ? bdt_delR : numb");
_rlm = _rlm.Define("bdt_delR_muon_3j2t", "region_3j2t &&  muonChannel && Wboson_transversMass>=50 ? bdt_delR : numb");
_rlm = _rlm.Define("bdt_delR_ele_2j1t", "region_2j1t &&  electronChannel && Wboson_transversMass>=50 ? bdt_delR : numb");
_rlm = _rlm.Define("bdt_delR_ele_2j0t", "region_2j0t &&  electronChannel && Wboson_transversMass>=50 ? bdt_delR : numb");
_rlm = _rlm.Define("bdt_delR_ele_3j2t", "region_3j2t &&  electronChannel && Wboson_transversMass>=50 ? bdt_delR : numb");

_rlm = _rlm.Define("bdt_deltaEta", ::calculate_deltaEta, {"goodlepton_4vec", "selected_cleanbjet_4vec"});


_rlm = _rlm.Define("bdt_deltaEta_muon_2j1t", "region_2j1t &&  muonChannel && Wboson_transversMass>=50 ? bdt_deltaEta : numb");
_rlm = _rlm.Define("bdt_deltaEta_muon_2j0t", "region_2j0t &&  muonChannel && Wboson_transversMass>=50 ? bdt_deltaEta : numb");
_rlm = _rlm.Define("bdt_deltaEta_muon_3j2t", "region_3j2t &&  muonChannel && Wboson_transversMass>=50 ? bdt_deltaEta : numb");
_rlm = _rlm.Define("bdt_deltaEta_ele_2j1t", "region_2j1t && electronChannel && Wboson_transversMass>=50 ? bdt_deltaEta : numb");
_rlm = _rlm.Define("bdt_deltaEta_ele_2j0t", "region_2j0t &&  electronChannel && Wboson_transversMass>=50 ? bdt_deltaEta : numb");
_rlm = _rlm.Define("bdt_deltaEta_ele_3j2t", "region_3j2t &&  electronChannel && Wboson_transversMass>=50 ? bdt_deltaEta : numb");

//_rlm = _rlm.Define("bdt_eventShape", ::event_shape, {"goodlepton_4vecs", "cleanbjet4vecs"});
_rlm = _rlm.Define("bdt_eventShape", 
                   [](const FourVectorVec &leptons, const FourVectorVec &jets) {
                        FourVectorVec combined = leptons;
                        combined.insert(combined.end(), jets.begin(), jets.end());

                        return  event_shape(combined); 
                   }, 
                   {"goodlepton_4vecs", "cleanbjet4vecs"});


_rlm = _rlm.Define("bdt_eventShape_muon_2j1t", "region_2j1t &&  muonChannel && Wboson_transversMass>=50 ? bdt_eventShape : numb");
_rlm = _rlm.Define("bdt_eventShape_muon_2j0t", "region_2j0t &&  muonChannel && Wboson_transversMass>=50 ? bdt_eventShape : numb");
_rlm = _rlm.Define("bdt_eventShape_muon_3j2t", "region_3j2t &&  muonChannel && Wboson_transversMass>=50 ? bdt_eventShape : numb");
_rlm = _rlm.Define("bdt_eventShape_ele_2j1t", "region_2j1t &&  electronChannel && Wboson_transversMass>=50 ? bdt_eventShape : numb");
_rlm = _rlm.Define("bdt_eventShape_ele_2j0t", "region_2j0t &&  electronChannel && Wboson_transversMass>=50 ? bdt_eventShape : numb");
_rlm = _rlm.Define("bdt_eventShape_ele_3j2t", "region_3j2t &&  electronChannel && Wboson_transversMass>=50 ?  bdt_eventShape: numb");

_rlm = _rlm.Define("bdt_WHelicity", ::calculateWHelicityAngle, {"lepton_TL4vec", "Wboson_4vec", "topQuark_TL4vec"});


_rlm = _rlm.Define("bdt_WHelicity_muon_2j1t", "region_2j1t &&  muonChannel && Wboson_transversMass>=50 ? bdt_WHelicity : numb");
_rlm = _rlm.Define("bdt_WHelicity_muon_2j0t", "region_2j0t &&  muonChannel && Wboson_transversMass>=50 ? bdt_WHelicity : numb");
_rlm = _rlm.Define("bdt_WHelicity_muon_3j2t", "region_3j2t &&  muonChannel && Wboson_transversMass>=50 ? bdt_WHelicity : numb");
_rlm = _rlm.Define("bdt_WHelicity_ele_2j1t", "region_2j1t &&  electronChannel && Wboson_transversMass>=50 ? bdt_WHelicity : numb");
_rlm = _rlm.Define("bdt_WHelicity_ele_2j0t", "region_2j0t &&  electronChannel && Wboson_transversMass>=50 ? bdt_WHelicity : numb");
_rlm = _rlm.Define("bdt_WHelicity_ele_3j2t", "region_3j2t &&  electronChannel && Wboson_transversMass>=50 ? bdt_WHelicity : numb");

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
    addVar({"good_jet1mass", "goodJets_mass[0]", ""});

    //================================Store variables in tree=======================================//
    // define variables that you want to store
    //==============================================================================================//
    addVartoStore("run");
    addVartoStore("luminosityBlock");
    addVartoStore("event");
    addVartoStore("evWeight");

    //addVartoStore("genWeight");
    addVartoStore("genEventSumw");
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
    addVartoStore("Jet_pt_corr");
    addVartoStore("NgoodJets");
    addVartoStore("goodJets_pt");
    addVartoStore("Selected_jetpt");
    addVartoStore("Selected_jeteta");
    addVartoStore("Selected_jetbtag");
    addVartoStore("Wboson_transversMass");

    
    addVartoStore("good_bjetdeepjet");
    
    //jetmet corr
    addVartoStore("Jet_pt_corr_corr");
    addVartoStore("lep_phi");
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
    addVartoStore("pugenWeight");
    addVartoStore("muon_SF_central");
    addVartoStore("ele_SF_central");
    addVartoStore("btag_SF_central");

      
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
   
// my variables start drom here -------------------------------------------------------------------------	   
  //  addVartoStore("muon_isolation");
    //addVartoStore("ele_isolation");
   /* addVartoStore("goodElectrons_leading_pt");
    addVartoStore("goodElectrons_leading_eta");
    addVartoStore("goodElectrons_leading_phi");
    addVartoStore("goodmuons_leading_pt");
    addVartoStore("goodmuons_leading_eta");
    addVartoStore("goodmuons_leading_phi");
    addVartoStore("good_bjet_leading_pt");
    addVartoStore("good_bjet_leading_phi");
    addVartoStore("good_bjet_leading_eta");
    addVartoStore("goodJets_btagpass_bcflav_pt");
    addVartoStore("goodJets_btagpass_bcflav_eta");
    addVartoStore("goodJets_all_bcflav_pt");
    addVartoStore("goodJets_all_bcflav_eta");
    addVartoStore("goodJets_btagpass_lflav_pt");
    addVartoStore("goodJets_btagpass_lflav_eta");
    addVartoStore("goodJets_all_lflav_pt");
    addVartoStore("goodJets_all_lflav_eta");
    addVartoStore("good_bjet_leading_mass");
    addVartoStore("rev_iso_el_leading_pt");
    addVartoStore("rev_iso_el_leading_eta");
    addVartoStore("rev_iso_el_leading_phi");
    addVartoStore("rev_iso_mu_leading_pt");
    addVartoStore("rev_iso_mu_leading_eta");
    addVartoStore("rev_iso_mu_leading_phi");
    addVartoStore("bjet_mass");*/
    addVartoStore("lepton_charge");    
    addVartoStore("bdt_wboson_muon_2j0t");
    addVartoStore("bdt_wboson_muon_2j1t");
    addVartoStore("bdt_wboson_muon_3j2t");
    addVartoStore("bdt_wboson_ele_2j0t");
    addVartoStore("bdt_wboson_ele_2j1t");
    addVartoStore("bdt_wboson_ele_3j2t");
    addVartoStore("bdt_top_muon_2j0t");
    addVartoStore("bdt_top_muon_2j1t");
    addVartoStore("bdt_top_muon_3j2t");
    addVartoStore("bdt_top_ele_2j0t");
    addVartoStore("bdt_top_ele_2j1t");
    addVartoStore("bdt_top_ele_3j2t");
    addVartoStore("bdt_specJet_2j0t_leading_eta_muon");
    addVartoStore("bdt_specJet_2j1t_leading_eta_muon");
    addVartoStore("bdt_specJet_3j2t_leading_eta_muon");
    addVartoStore("bdt_specJet_2j1t_leading_eta_ele");
    addVartoStore("bdt_specJet_2j0t_leading_eta_ele");
    addVartoStore("bdt_specJet_3j2t_leading_eta_ele");
    addVartoStore("bdt_delR_muon_2j1t");
    addVartoStore("bdt_delR_muon_2j0t");
    addVartoStore("bdt_delR_muon_3j2t");
    addVartoStore("bdt_delR_ele_2j1t");
    addVartoStore("bdt_delR_ele_2j0t");
    addVartoStore("bdt_delR_ele_3j2t");
    addVartoStore("bdt_deltaEta_muon_2j1t");
    addVartoStore("bdt_deltaEta_muon_2j0t");
    addVartoStore("bdt_deltaEta_muon_3j2t");
    addVartoStore("bdt_deltaEta_ele_2j0t");
    addVartoStore("bdt_deltaEta_ele_2j1t");
    addVartoStore("bdt_deltaEta_ele_3j2t");
    addVartoStore("bdt_eventShape_muon_2j1t");
    addVartoStore("bdt_eventShape_muon_2j0t");
    addVartoStore("bdt_eventShape_muon_3j2t");
    addVartoStore("bdt_eventShape_ele_2j1t");
    addVartoStore("bdt_eventShape_ele_2j0t");
    addVartoStore("bdt_eventShape_ele_3j2t");
    addVartoStore("bdt_WHelicity_muon_2j1t");
    addVartoStore("bdt_WHelicity_muon_2j0t");
    addVartoStore("bdt_WHelicity_muon_3j2t");
    addVartoStore("bdt_WHelicity_ele_2j1t");
    addVartoStore("bdt_WHelicity_ele_2j0t");
    addVartoStore("bdt_WHelicity_ele_3j2t");
    addVartoStore("bdt_MET_pt_muon_2j0t");
    addVartoStore("bdt_MET_pt_muon_2j1t");
    addVartoStore("bdt_MET_pt_muon_3j2t");
    addVartoStore("bdt_MET_pt_ele_2j0t");
    addVartoStore("bdt_MET_pt_ele_2j1t");
    addVartoStore("bdt_MET_pt_ele_3j2t");
    
   addVartoStore("ncleanjetspass");
   addVartoStore("Jet_pt_corr");
    addVartoStore("nJet");
    addVartoStore("evWeight");
    addVartoStore("genWeight");
    addVartoStore("genEventSumw");

    addVartoStore("ncleanbjetspass");
   /* addVartoStore("top_mass_2j1t");
    addVartoStore("top_mass_2j1t_ele");
    addVartoStore("top_mass_2j1t_muon");
    addVartoStore("top_mass_2j0t");
    addVartoStore("top_mass_3j2t");
    addVartoStore("top_mass");
   // addVartoStore("top_pt");
    addVartoStore("evWeight");*/


    addVartoStore("Wboson_transversMass");
    addVartoStore("Wboson_transversemass_ele");
    addVartoStore("Wboson_transversemass_ele_barrel");
    addVartoStore("Wboson_transversemass_ele_endcap");
    addVartoStore("Wboson_transversemass_QCDele");
    addVartoStore("Wboson_transversemass_QCDele_barrel");
    addVartoStore("Wboson_transversemass_QCDele_endcap");
   
    addVartoStore("Wboson_transversemass_muon");
    addVartoStore("Wboson_transversemass_QCDmuon");
   // addVartoStore("good_bjet_leading_pt");
    
    addVartoStore("region_2j0t");
    addVartoStore("region_2j1t");
    addVartoStore("region_3j2t");
    addVartoStore("muonChannel");
    addVartoStore("electronChannel");
    addVartoStore("MET_pt_corr");
    addVartoStore("top_mass");
    addVartoStore("specJet_leading_eta");
    addVartoStore("bdt_delR");
    addVartoStore("bdt_deltaEta");
    addVartoStore("bdt_eventShape");
    addVartoStore("bdt_WHelicity");
    
    addVartoStore("QCDmuonChannel");
    addVartoStore("QCDelectronChannel");
     // addVartoStore("genWeight");
    //addVartoStore("top_pt");
    //addVartoStore("w_pt");
      // addVartoStore("ele_SF_central");
     /* addVartoStore("genWeight");
    addVartoStore("genEventSumw");
    addVartoStore("delphi_2j0t_muon");
    addVartoStore("eta_2j0t_muon");
    addVartoStore("Pt_2j0t_muon");
    addVartoStore("eta_2j0t_muon");
    addVartoStore("eta_2j0t_ele");
    addVartoStore("MET_pt_2j0t_muon");
    addVartoStore("MET_pt_2j0t_ele_barrel");
    addVartoStore("MET_pt_2j0t_ele_endcap");
    addVartoStore("muon_SF_central");
    addVartoStore("ele_SF_central");
*/
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
        selectChannel();
	selectJets();
        removeOverlaps();
        defineRegion();
        spectatorJets();

	if(!_isData){
	  this->calculateEvWeight(); // PU, genweight and BTV and Mu and Ele
	}
	selectMET();
        reconstructWboson();
        reconstructTop();
	Background_Estimation();
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
//	    _rlm = _rlm.Define("evWeight", "genWeight");
	    
//	    std::cout<<"Using evWeight = genWeight"<<std::endl;

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

