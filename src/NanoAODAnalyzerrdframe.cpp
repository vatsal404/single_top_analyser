/*
 * NanoAODAnalyzerrdframe.cpp
 *
 *  Created on: Sep 30, 2018
 *      Author: suyong
 *  Updated on: 10Oct, 2023
 *      Author: Arnab PUROHIT, IP2I, Lyon 
 */

#include "NanoAODAnalyzerrdframe.h"
#include <iostream>
#include <algorithm>
#include <typeinfo>
#include <random>

#include "TCanvas.h"
#include "Math/GenVector/VectorUtil.h"
#include <vector>
#include <fstream>
#include "utility.h"
#include <regex>
#include "ROOT/RDFHelpers.hxx"

using namespace std;

NanoAODAnalyzerrdframe::NanoAODAnalyzerrdframe(TTree *atree, std::string outfilename)
:_rd(*atree), _jsonOK(false),_outfilename(outfilename)
	, _outrootfile(0), _rlm(_rd)
	, _rnt(&_rlm) //PDFWeights(103, 0.0) 
{
	_atree=atree;
	//cout<< " run year=====" << _year <<endl;
	// if genWeight column exists, then it is not real data
	//

}

NanoAODAnalyzerrdframe::~NanoAODAnalyzerrdframe() {
	// TODO Auto-generated destructor stub
}

bool NanoAODAnalyzerrdframe::isDefined(string v)
{
	auto result = std::find(_originalvars.begin(), _originalvars.end(), v);
	if (result != _originalvars.end()) return true;
	else return false;
}

void NanoAODAnalyzerrdframe::setTree(TTree *t, std::string outfilename)
{
	_rd = ROOT::RDataFrame(*t);
	_rlm = RNode(_rd);
	_outfilename = outfilename;
	_hist1dinfovector.clear();
	_th1dhistos.clear();
	_varstostore.clear();
	//_hist1dinfovector.clear();
	_hist2dinfovector.clear();
	_th2dhistos.clear();
	_selections.clear();

	this->setupAnalysis();
}


void NanoAODAnalyzerrdframe::setupAnalysis()
{
	// Event weight for data it's always one. For MC, it depends on the sign
 	//cout<<"year===="<< _year<< "==runtype=== " <<  _runtype <<endl;
	_rlm = _rlm.Define("one", "1.0");
	if (_isData && !isDefined("evWeight"))
	{
		_rlm = _rlm.Define("evWeight", [](){
				return 1.0;
			}, {} );
	}
	// Store PDF sum of weights
   /*if(!_isData){
            auto storeWeights = [this](floats weights)->floats {

                for (unsigned int i=0; i<weights.size(); i++)
                    PDFWeights[i] += weights[i];

                return PDFWeights;
            };
            try {
                _rlm.Foreach(storeWeights, {"LHEPdfWeight"});
            } catch (exception& e) {
                cout << e.what() << endl;
                cout << "No PDF weight in this root file!" << endl;
            }
	}*/
	

	setupCuts_and_Hists();
	setupTree();
}


bool NanoAODAnalyzerrdframe::readgoodjson(string goodjsonfname)
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

void NanoAODAnalyzerrdframe::selectFatJets()
{
	_rlm = _rlm.Define("fatjetcuts", "FatJet_pt>400.0 && abs(FatJet_eta)<2.4 && FatJet_tau1>0.0 && FatJet_tau2>0.0 && FatJet_tau3>0.0 && FatJet_tau3/FatJet_tau2<0.5")
				.Define("Sel_fatjetpt", "FatJet_pt[fatjetcuts]")
				.Define("Sel_fatjeteta", "FatJet_eta[fatjetcuts]")
				.Define("Sel_fatjetphi", "FatJet_phi[fatjetcuts]")
				.Define("Sel_fatjetmass", "FatJet_mass[fatjetcuts]")
				.Define("nfatjetspass", "int(Sel_fatjetpt.size())")
				.Define("Sel_fatjetweight", "std::vector<double>(nfatjetspass, evWeight)")
				.Define("Sel_fatjet4vecs", ::generate_4vec, {"Sel_fatjetpt", "Sel_fatjeteta", "Sel_fatjetphi", "Sel_fatjetmass"});
}


void NanoAODAnalyzerrdframe::setupJetMETCorrection(string fname, string jettag) //data
{

    cout << "SETUP JETMET correction" << endl;
	// read from file 
	_correction_jerc = correction::CorrectionSet::from_file(fname);//jercfname=json
	assert(_correction_jerc->validate()); //the assert functionality : check if the parameters passed to a function are valid =1:true
	// correction type(jobconfiganalysis.py)
	cout<<"JERC JSON file : " << fname<<endl;
	_jetCorrector = _correction_jerc->compound().at(jettag);//jerctag#JSON (JEC,compound)compoundLevel="L1L2L3Res"
	cout<< "JET tag in JSON : " << jettag << endl;
	_jetCorrectionUnc = _correction_jerc->at(_jercunctag);
	cout<< "JET uncertainity tag in JSON  : " << _jercunctag << endl;
	std::cout<< "================================//=================================" << std::endl;
}

void NanoAODAnalyzerrdframe::applyJetMETCorrections() //data
{
    cout << "apply JETMET correction" << endl;

	auto appcorrlambdaf = [this](floats jetpts, floats jetetas, floats jetAreas, floats jetrawf, float rho)->floats
	{
		floats corrfactors;
		corrfactors.reserve(jetpts.size());
		for (auto i =0; i<int(jetpts.size()); i++)
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
			for (auto i =0; i<int(jetpts.size()); i++)
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
		for (auto i=0; i<int(jetphis.size()); i++)
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
		for (auto i=0; i<int(jetphis.size()); i++)
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

void NanoAODAnalyzerrdframe::applyMuPtCorrection() //data and MC
{
  cout << "apply Muon Pt correction" << endl;
  
  if(_isData){
    
    auto lambdaf_data = [this](const ints mu_charges, const floats mu_pts, const floats mu_etas, const floats mu_phis)->floats
      {
	floats corrMuPts;
	corrMuPts.reserve(mu_pts.size());
	//std::cout<<"Number of muons: "<<mu_pts.size()<<std::endl;
	for (auto i =0; i<int(mu_pts.size()); i++)
	  {
	    float mu_pt_uncorr = mu_pts[i];
	    //std::cout<<"The Muon Uncorrected pt"<< mu_pt_uncorr<<std::endl;	    
	    float corrfactor = 1;
	    corrfactor = _Roch_corr.kScaleDT(mu_charges[i], mu_pt_uncorr, mu_etas[i], mu_phis[i], 0, 0);
	    //std::cout<<"Muon pt correction factor===="<< corrfactor <<std::endl;
	    corrMuPts.emplace_back(mu_pt_uncorr * corrfactor);
	  }    
	return corrMuPts;
	
      };
    
    
    _rlm = _rlm.Define("Muon_pt_corr", lambdaf_data, {"Muon_charge", "Muon_pt", "Muon_eta", "Muon_phi"});
  }
  else{
  
    auto lambdaf_mc = [this](const ints mu_charges, const floats mu_pts, const floats mu_etas, const floats mu_phis, const ints muon_genIdx, const floats gen_pts,  const ints nls)->floats
      {
	floats corrMuPts;
	corrMuPts.reserve(mu_pts.size());
	//std::cout <<"Number of muons: "<<mu_pts.size()<<std::endl;
	for (int i=0; i<int(mu_pts.size()); i++)
	  {
	    float corrfactor = 1;
	    float mu_pt_uncorr = mu_pts[i];
	    //std::cout<<"The Muon Uncorrected pt"<< mu_pt_uncorr<<std::endl;
	    if ( muon_genIdx[i] != -1 ){
	      corrfactor = _Roch_corr.kSpreadMC(mu_charges[i], mu_pt_uncorr, mu_etas[i], mu_phis[i], gen_pts[muon_genIdx[i]], 0, 0);
	    }
	    else{
	      float rand = gRandom->Rndm();
	      corrfactor = _Roch_corr.kSmearMC(mu_charges[i], mu_pt_uncorr, mu_etas[i], mu_phis[i], nls[i], rand, 0, 0);
	    }
	    //std::cout<<"Muon corrected Pt ===="<< corrfactor * mu_pt_uncorr <<std::endl;
	    corrMuPts.emplace_back(corrfactor * mu_pt_uncorr);
	  }
	return corrMuPts;
      };
    _rlm = _rlm.Define("Muon_gen_pt", "GenPart_pt[Muon_genPartIdx]");
    _rlm = _rlm.Define("Muon_pt_corr", lambdaf_mc, {"Muon_charge", "Muon_pt", "Muon_eta", "Muon_phi", "Muon_genPartIdx", "GenPart_pt", "Muon_nTrackerLayers"});
    
  }
}



void NanoAODAnalyzerrdframe::setupCorrections(string goodjsonfname, string pufname, string putag, string btvfname, string btvtype, string fname_btagEff, string hname_btagEff_bcflav, string hname_btagEff_lflav, string muon_roch_fname, string muon_fname, string muonhlttype, string muonrecotype,string muonidtype,string muonisotype,string electron_fname, string electron_reco_type, string electron_id_type, string jercfname, string jerctag, string jercunctag)
//In this function the correction is evaluated for each jet, Muon, Electron and MET. The correction depends on the momentum, pseudorapidity, energy, and cone area of the jet, as well as the value of “rho” (the average momentum per area) and number of interactions in the event. The correction is used to scale the momentum of the jet.
{
    cout << "set up Corrections!" << endl;
	if (_isData) _jsonOK = readgoodjson(goodjsonfname); // read golden json file
	std::cout << "Rochester correction files: " << muon_roch_fname << std::endl;
	_Roch_corr.init(muon_roch_fname);
	if (!_isData) {
	  // using correctionlib
	  //Muon corrections
	  _correction_muon = correction::CorrectionSet::from_file(muon_fname);
	  _muon_hlt_type = muonhlttype;
	  _muon_reco_type = muonrecotype;
	  _muon_id_type = muonidtype;
	  _muon_iso_type = muonisotype;
	  std::cout<< "================================//=================================" << std::endl;
	  cout<< "MUON JSON FILE : " <<  muon_fname << endl;
	  cout<< "MUON HLT type in JSON  : " << _muon_hlt_type << endl;
	  cout<< "MUON RECO type in JSON  : " << _muon_reco_type << endl;
	  cout<< "MUON ID type in JSON  : " << _muon_id_type << endl;
	  cout<< "MUON ISO type in JSON  : " << _muon_iso_type << endl;
	  assert(_correction_muon->validate());
	  
	  //Electron corrections
	  _correction_electron = correction::CorrectionSet::from_file(electron_fname);
	  _electron_reco_type = electron_reco_type;
	  _electron_id_type = electron_id_type;
	  std::cout<< "================================//=================================" << std::endl;
	  cout<< "ELECTRON JSON FILE : " << electron_fname << endl;
	  cout<< "ELECTRON RECO type in JSON  : " << _electron_reco_type << endl;
	  cout<< "ELECTRONID type in JSON  : " << _electron_id_type << endl;
	  assert(_correction_electron->validate());
	  
	  // btag corrections
	  _correction_btag1 = correction::CorrectionSet::from_file(btvfname);
	  _btvtype = btvtype;
	  assert(_correction_btag1->validate());

	  f_btagEff = new TFile(fname_btagEff.c_str(), "READ");
	  hist_btagEff_bcflav = dynamic_cast<TH2D*>(f_btagEff->Get(hname_btagEff_bcflav.c_str()));
	  hist_btagEff_lflav = dynamic_cast<TH2D*>(f_btagEff->Get(hname_btagEff_lflav.c_str()));


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
		  return x*y;
		}, {"genWeight", "puWeight"});
	    }
	}
	_jerctag = jerctag;
	_jercunctag = jercunctag;
	
	setupJetMETCorrection(jercfname, _jerctag);
	applyJetMETCorrections();
	applyMuPtCorrection();
}
double NanoAODAnalyzerrdframe::getBTaggingEff(double hadflav, double eta, double pt){
  double efficiency = 1.0;
  int maxXBin = -1;
  int maxYBin = -1;
  int binX = -1;
  int binY = -1;
  if(hadflav!=0){
    // Get the maximum bin number for x and y axes
    maxXBin = hist_btagEff_bcflav->GetXaxis()->GetNbins();
    maxYBin = hist_btagEff_bcflav->GetYaxis()->GetNbins();

    // Get the bin number corresponding to the provided x and y values
    binX = hist_btagEff_bcflav->GetXaxis()->FindBin(eta);
    binY = hist_btagEff_bcflav->GetYaxis()->FindBin(pt);

    efficiency = hist_btagEff_bcflav->GetBinContent(binX, binY);
  }
  else{
    // Get the maximum bin number for x and y axes
    maxXBin = hist_btagEff_lflav->GetXaxis()->GetNbins();
    maxYBin = hist_btagEff_lflav->GetYaxis()->GetNbins();

    // Get the bin number corresponding to the provided x and y values
    binX = hist_btagEff_lflav->GetXaxis()->FindBin(eta);
    binY = hist_btagEff_lflav->GetYaxis()->FindBin(pt);

    efficiency = hist_btagEff_bcflav->GetBinContent(binX, binY);
  }

  return efficiency;
}

ROOT::RDF::RNode NanoAODAnalyzerrdframe::calculateBTagSF(RNode _rlm, std::vector<std::string> Jets_vars_names, int _case, const double btag_cut, std::string _BTaggingWP, std::string output_var)
{

  //case1 : fixedWP correction with mujets (here medium WP) # evaluate('systematic', 'working_point', 'flavor', 'abseta', 'pt')
  //for case 1  use one of the btvtype = "deepJet_mujets " , deepJet_comb" for b/c , deepJet_incl" for lightjets 
  if(_case==1){

      //======================================================================================================================================
      //>>>> function to calculate event weights for MC events, incorporating fixedWP correction with mujets (here medium WP)and systematics with
      //all variations seperately (up/down/correlated/uncorrelated/)
      //The weight for each variation is stored in separate columns (btag_SF_central,btag_SF_up, btag_SF_down, etc.). 
      // btagWeight_case1_central  is used to recalculate the eventweight. Other variations are intended for systematics calculations.
      //======================================================================================================================================
    auto btagweightgenerator_bcflav_case1 = [this](const ROOT::VecOps::RVec<int>& hadflav, const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts, const ROOT::VecOps::RVec<float>& btag_scores, const double btag_cut, std::string _BTagWP, const std::string& variation) -> float {
      double btagWeight_bcflav = 1.0;
      for (std::size_t i = 0; i < pts.size(); i++) {
	//std::cout<<"The BTag flavor"<< hadflav[i]<< " BTagJet eta:"<< etas[i]<<" BTagJet pt"<< pts[i]<<std::endl;
	if(std::abs(etas[i])>2.4999 || pts[i]<30.000001 || hadflav[i]==0) continue;
	//double selection_cut = _correction_btag1->("deepJet_wp_values")->evaluate(_BTagWP);
	if(btag_scores[i]>=btag_cut){
	    double bcjets_weights = _correction_btag1->at("deepJet_mujets")->evaluate({variation, _BTagWP, hadflav[i], std::fabs(etas[i]), pts[i]});
	    btagWeight_bcflav *= bcjets_weights;
	}
	else{
	    double bcjets_weights = _correction_btag1->at("deepJet_mujets")->evaluate({variation, _BTagWP, hadflav[i], std::fabs(etas[i]), pts[i]});
	    double eff = getBTaggingEff(hadflav[i], etas[i], pts[i]);
	    btagWeight_bcflav *= (1 - bcjets_weights*eff)/(1-eff);
	}
      }
      return btagWeight_bcflav;
    };

    auto btagweightgenerator_lflav_case1 = [this](const ROOT::VecOps::RVec<int>& hadflav, const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts, const ROOT::VecOps::RVec<float>& btag_scores, const double btag_cut, std::string _BTagWP, const std::string& variation) -> float {
      double btagWeight_lflav = 1.0;
      for (std::size_t i = 0; i < pts.size(); i++) {
	//std::cout<<"The BTag flavor"<< hadflav[i]<< " BTagJet eta:"<< etas[i]<<" BTagJet pt"<< pts[i]<<std::endl;
	if(std::abs(etas[i])>2.4999 || pts[i]<30.000001 || hadflav[i]!=0) continue;
	//double selection_cut = _correction_btag1->("deepJet_wp_values")->evaluate(_BTagWP);
	if(btag_scores[i]>=btag_cut){
	  double lightjets_weights = _correction_btag1->at("deepJet_incl")->evaluate({variation, _BTagWP, hadflav[i], std::fabs(etas[i]), pts[i]});
	  btagWeight_lflav *= lightjets_weights;
	}
	else{
	  double lightjets_weights = _correction_btag1->at("deepJet_incl")->evaluate({variation, _BTagWP, hadflav[i], std::fabs(etas[i]), pts[i]});
	  double eff = getBTaggingEff(hadflav[i], etas[i], pts[i]);
	  btagWeight_lflav *= (1 - lightjets_weights*eff)/(1-eff);
	}
      }
      return btagWeight_lflav;
    };
    // btag weight for each variation individually
    std::vector<std::string> variations = {"central", "up", "down", "up_correlated", "down_correlated", "up_uncorrelated", "down_uncorrelated", "up_statistic", "down_statistic"}; 
    for (const std::string& variation : variations) {
      std::string column_name_bcflav = output_var + "bcflav_" +variation;
      _rlm = _rlm.Define(column_name_bcflav, [btagweightgenerator_bcflav_case1, variation, btag_cut, _BTaggingWP](const ROOT::VecOps::RVec<int>& hadflav, const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts, const ROOT::VecOps::RVec<float>& btag_scores) {
	  float weight = btagweightgenerator_bcflav_case1(hadflav, etas, pts, btag_scores, btag_cut, _BTaggingWP, variation);// Get the weight for the corresponding variation
	  return weight;
	}, Jets_vars_names); //after all cuts, remove overlapped

      std::string column_name_lflav = output_var + "lflav_" +variation;
      _rlm = _rlm.Define(column_name_lflav, [btagweightgenerator_lflav_case1, variation, btag_cut, _BTaggingWP](const ROOT::VecOps::RVec<int>& hadflav, const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts, const ROOT::VecOps::RVec<float>& btag_scores) {
	  float weight = btagweightgenerator_lflav_case1(hadflav, etas, pts, btag_scores, btag_cut, _BTaggingWP, variation);// Get the weight for the corresponding variation
	  return weight;
	}, Jets_vars_names); //after all cuts, remove overlapped
      std::cout<< "BJet SF column name: " << column_name_bcflav<< " and " << column_name_lflav  << std::endl;
      if(isDefined("column_name_bcflav")){
	std::cout<< "BJet SF column: " << column_name_bcflav << " is saved in the Node."<< std::endl;
      }
      if(isDefined("column_name_lflav")){
	std::cout<< "BJet SF column: " << column_name_lflav << " is saved in the Node."<< std::endl;
      }
    }
  }
  else if(_case==3){
    //======================================================================================================================================
    //case3 - Shape correction
    //for case 3 : use btvtype': 'deepJet_shape' in jobconfiganalysis.py
    cout<<"case 3 Shape correction B tagging SF for MC "<<endl;
    //======================================================================================================================================
    //>>>> function to calculate event weights for MC events,based on DeepJet algorithm, incorporating shape correction with central variation
    //======================================================================================================================================
    auto btagweightgenerator3= [this](ints &hadflav, floats &etas, floats &pts, floats &btags)->float
      {
	double bweight=1.0;
	
	for (auto i=0; i<int(pts.size()); i++)
	  {
	    if(std::abs(etas[i])>2.5 || pts[i]<30.000001) continue;
	    double w = _correction_btag1->at(_btvtype)->evaluate({"central", int(hadflav[i]), fabs(float(etas[i])), float(pts[i]), float(btags[i])});
	    bweight *= w;
	  }
	return bweight;
      };
    
    cout<<"Generate case3 b-tagging weight"<<endl;
    std::string column_name = output_var + "case3";
    _rlm = _rlm.Define(column_name, btagweightgenerator3, Jets_vars_names);
    //Total event weight after shape correction
    //_rlm = _rlm.Define("evWeight", "pugenWeight*btagWeight_case3");
    std::cout<< "BJet SF column name: " << column_name << std::endl;

  }
  return _rlm;
}


ROOT::RDF::RNode NanoAODAnalyzerrdframe::calculateMuSF(RNode _rlm, std::vector<std::string> Muon_vars, std::string output_var)
{

    //=====================================================Muon SF and eventweight============================================================// 
    //muontype= for thight: NUM_TightID_DEN_genTracks //for medium: NUM_MediumID_DEN_TrackerMuons
    //Muon MediumID ISO UL type: NUM_TightRelIso_DEN_MediumID && thightID:NUM_TightRelIso_DEN_TightIDandIPCut --> the type can be found in json file
    //--> As an example Medium wp is used 
    //===============================================================================================================================================//
    //cout<<"muon HLT SF for MC "<<endl;
  auto muon_weightgenerator = [this](const std::string& muon_type, const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts, const std::string& variation) -> float {
      double muonHLT_w = 1.0;

      for (std::size_t i = 0; i < pts.size(); i++) {
	//std::cout << "Muon abs_eta:" << std::fabs(etas[i]) << " pt: " << pts[i] << std::endl;
	double w = _correction_muon->at(muon_type)->evaluate({std::to_string(_year)+"_"+_runtype, std::fabs(etas[i]), pts[i], variation}); 
	muonHLT_w *= w;
	//std::cout << "Individual HLT weight (muon " << i << "): " << w << std::endl;
	//std::cout << "Cumulative HLT weight after muon " << i << ": " << muonHLT_w << std::endl;
      }
      return muonHLT_w;
    };

    //'sf' is nominal, and 'systup' and 'systdown' are up/down variations with total stat+-syst uncertainties. Individual systs are also available (in these cases syst only, not sf +/- syst
    std::vector<std::string> variations = {"sf", "systup", "systdown","syst"};


    //cout<<"Generate MUONHLT weight"<<endl;
    //muonHLT sf and systematics with up/down variations
    //===========//===========//===========//===========//===========
    // define muon HLT weight sf/systs for each variation individually
    for (const std::string& variation : variations) {
      std::string column_name_hlt = output_var+"hlt_" + variation;
      _rlm = _rlm.Define(column_name_hlt, [this, muon_weightgenerator, variation](const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts) {
	  float weight = muon_weightgenerator(_muon_hlt_type, etas, pts, variation); // Get the weight for the corresponding variation
	  //std::cout << "Muon HLT weight (" << variation << "): " << weight << std::endl;
	  return weight;
	}, Muon_vars);


      std::string column_name_reco = output_var+"reco_" + variation;
      _rlm = _rlm.Define(column_name_reco, [this, muon_weightgenerator, variation](const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts) {
	  float weight = muon_weightgenerator(_muon_reco_type, etas, pts, variation); // Get the weight for the corresponding variation
	  //std::cout << "Muon HLT weight (" << variation << "): " << weight << std::endl;
	  return weight;
	}, Muon_vars);


      std::string column_name_id = output_var+"id_" + variation;
      _rlm = _rlm.Define(column_name_id, [this, muon_weightgenerator, variation](const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts) {
	  float weight = muon_weightgenerator(_muon_id_type, etas, pts, variation); // Get the weight for the corresponding variation
	  //std::cout << "Muon HLT weight (" << variation << "): " << weight << std::endl;
	  return weight;
	}, Muon_vars);


      std::string column_name_iso = output_var+"iso_" + variation;
      _rlm = _rlm.Define(column_name_iso, [this, muon_weightgenerator, variation](const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts) {
	  float weight = muon_weightgenerator(_muon_iso_type, etas, pts, variation); // Get the weight for the corresponding variation
	  //std::cout << "Muon HLT weight (" << variation << "): " << weight << std::endl;
	  return weight;
	}, Muon_vars);

      std::string column_name = output_var;
      if(variation=="sf"){
	column_name += "central";
      }
      else if(variation=="systup"){
	column_name += "up";
      }
      else if(variation=="systdown"){
	column_name += "down";
      }
      else{
	column_name += "syst";
      }

	std::string sf_definition = column_name_hlt+" * "+column_name_reco+" * "+column_name_id+" * "+column_name_iso;
	_rlm = _rlm.Define(column_name, sf_definition);
	std::cout<< "Muon SF column name: " << column_name << std::endl;
    }
    return _rlm;
}


ROOT::RDF::RNode NanoAODAnalyzerrdframe::calculateEleSF(RNode _rlm, std::vector<std::string> Ele_vars, std::string output_var)
{

    //auto cs = correction::CorrectionSet::from_file("electron.json.gz");
    //cout<<"Generate ELECTRONRECO weight"<<endl;
    //electronRECO sf and systematics with up/down variations
    //===========//===========//===========//===========//===========
  auto electron_weightgenerator = [this](const std::string eletype, const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts, const std::string& variation) -> float {
      double electronReco_w = 1.0;

      for (std::size_t i = 0; i < pts.size(); i++) {
	
	double w = _correction_electron->at("UL-Electron-ID-SF")->evaluate({std::to_string(_year), variation, eletype, std::fabs(etas[i]), pts[i]}); 
	electronReco_w *= w;
	//std::cout << "Individual weight (electron " << i << "): " << w << std::endl;
	//std::cout << "Cumulative weight after electron " << i << ": " << electronId_w << std::endl;
      }
      return electronReco_w;
    };

    //'sf' is nominal, and 'systup' and 'systdown' are up/down variations with total stat+-syst uncertainties. Individual systs are also available (in these cases syst only, not sf +/- syst
    std::vector<std::string> variations_elec = {"sf", "sfup", "sfdown"};


    for (const std::string& variation : variations_elec) {

      // define electron RECO weight sf/systs for each variation individually
      std::string column_name_reco = output_var+ "reco_" + variation;
      _rlm = _rlm.Define(column_name_reco, [this, electron_weightgenerator, variation](const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts) {
	  float weight = electron_weightgenerator(_electron_reco_type, etas, pts, variation); // Get the weight for the corresponding variation
	  //std::cout << "Electron RECO weight (" << variation << "): " << weight << std::endl;
	  return weight;
	}, Ele_vars);


      // define electron ID weight sf/systs for each variation individually
      std::string column_name_id = output_var+ "id_" + variation;
      _rlm = _rlm.Define(column_name_id, [this, electron_weightgenerator, variation](const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts) {
	  float weight = electron_weightgenerator(_electron_id_type, etas, pts, variation); // Get the weight for the corresponding variation
	  //std::cout << "Electron RECO weight (" << variation << "): " << weight << std::endl;
	  return weight;
	}, Ele_vars);
      
      std::string column_name = output_var;

      if(variation=="sf"){
	column_name += "central";
      }
      else if(variation=="sfup"){
	column_name += "up";
      }
      else{
	column_name += "down";
      }
      std::cout<< "Electron SF column name: " << column_name << std::endl;
      std::string sf_definition = column_name_reco+" * "+column_name_id;

      _rlm = _rlm.Define(column_name, sf_definition); 

    }
    return _rlm;
}


bool NanoAODAnalyzerrdframe::helper_1DHistCreator(std::string hname, std::string title, const int nbins, const double xlow, const double xhi, std::string rdfvar, std::string evWeight, RNode *anode)
{
	//cout << "1DHistCreator " << hname  << endl;

	RDF1DHist histojets = anode->Histo1D({hname.c_str(), title.c_str(), nbins, xlow, xhi}, rdfvar, evWeight); // Fill with weight given by evWeight
	_th1dhistos[hname] = histojets;
	//histojets.GetPtr()->Print("all");
	return true;
}

//for 2D histograms//
bool NanoAODAnalyzerrdframe::helper_2DHistCreator(std::string hname, std::string title, const int nbinsx, const double xlow, const double xhi, const int nbinsy, const double ylow, const double yhi,std::string rdfvarx,std::string rdfvary, std::string evWeight, RNode *anode)
{
	//cout << "1DHistCreator " << hname  << endl;

	RDF2DHist histojets = anode->Histo2D({hname.c_str(), title.c_str(), nbinsx, xlow, xhi,nbinsy, ylow, yhi}, rdfvarx,rdfvary, evWeight); // Fill with weight given by evWeight
	_th2dhistos[hname] = histojets;
	histojets.GetPtr()->Print("all");
	return true;
}

// Automatically loop to create
void NanoAODAnalyzerrdframe::setupCuts_and_Hists()
{
	cout << "setting up definitions, cuts, and histograms" <<endl;

	for ( auto &c : _varinfovector)
	{
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

	//for 2D histograms
	for (auto &x : _hist2dinfovector)
	{
		std::string hpost = "_nocut";

		if (x.mincutstep.length()==0)
		{
			helper_2DHistCreator(std::string(x.hmodel.fName)+hpost,  std::string(x.hmodel.fTitle)+hpost, x.hmodel.fNbinsX, x.hmodel.fXLow, x.hmodel.fXUp, x.hmodel.fNbinsY, x.hmodel.fYLow, x.hmodel.fYUp, x.varname1, x.varname2, x.weightname, &_rlm);
		}
	}


	_rnt.setRNode(&_rlm);

	for (auto acut : _cutinfovector)
	{
		std::string cutname = "cut"+ acut.idx;
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

			//for 2DHistograms
		for (auto &x : _hist2dinfovector)
		{
			if (acut.idx.compare(0, x.mincutstep.length(), x.mincutstep)==0)
			{
				helper_2DHistCreator(std::string(x.hmodel.fName)+hpost,  std::string(x.hmodel.fTitle)+hpost, x.hmodel.fNbinsX, x.hmodel.fXLow, x.hmodel.fXUp, x.hmodel.fNbinsY, x.hmodel.fYLow, x.hmodel.fYUp, x.varname1, x.varname2, x.weightname, rnext);
			}
		}
		_rnt.addDaughter(rnext, acut.idx);

	}
}

void NanoAODAnalyzerrdframe::add1DHist(TH1DModel histdef, std::string variable, std::string weight,string mincutstep)
{
	_hist1dinfovector.push_back({histdef, variable, weight, mincutstep});
}
//for 2DHistograms
void NanoAODAnalyzerrdframe::add2DHist(TH2DModel histdef, std::string variable1,std::string variable2, std::string weight,string mincutstep)
{
	_hist2dinfovector.push_back({histdef, variable1,variable2, weight, mincutstep});
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
	// if varname=="Muon_.*", then any branch name that starts with "Muon_" string will
	// be saved
	_varstostore.push_back(varname);

}

void NanoAODAnalyzerrdframe::setupTree()
{
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


	vector<RNodeTree *> rntends;
	_rnt.getRNodeLeafs(rntends);
	//_rnt.Print();
    //cout << rntends.size() << endl;


	for (auto arnt: rntends)
	{
		string nodename = arnt->getIndex();
		string outname = _outfilename;
		if (rntends.size()>1) outname.replace(outname.find(".root"), 5, "_"+nodename+".root");
		_outrootfilenames.push_back(outname);
		RNode *arnode = arnt->getRNode();
		std::cout<< "-------------------------------------------------------------------" << std::endl;
                cout<<"cut : " ;
                cout << arnt->getIndex();
		if (saveAll) {
			arnode->Snapshot(outtreename, outname);
		}
		else {
            cout << " --writing branches" << endl;
			std::cout<< "-------------------------------------------------------------------" << std::endl;
			for (auto bname: _varstostorepertree[nodename])
			{
				cout << bname << endl;
			}
			arnode->Snapshot(outtreename, outname, _varstostorepertree[nodename]);
		}
		std::cout<< "-------------------------------------------------------------------" << std::endl;
		cout << "Creating output root file :  " << endl;
		cout << outname << " ";
		cout<<endl;
		std::cout<< "-------------------------------------------------------------------" << std::endl;
		_outrootfile = new TFile(outname.c_str(), "UPDATE");
		cout << "Writing histograms...   " << endl;
		std::cout<< "-------------------------------------------------------------------" << std::endl;
		for (auto &h : _th1dhistos)
		{
			if (h.second.GetPtr() != nullptr) {
				h.second.GetPtr()->Print();
				h.second.GetPtr()->Write();
			}
		}
		//for 2D histograms
		for (auto &h : _th2dhistos)
		{
			if (h.second.GetPtr() != nullptr) {
				h.second.GetPtr()->Print();
				h.second.GetPtr()->Write();
			}
		}


		/*TH1F* hPDFWeights = new TH1F("LHEPdfWeightSum", "LHEPdfWeightSum", 103, 0, 1);
        for (size_t i=0; i<PDFWeights.size(); i++){
            hPDFWeights->SetBinContent(i+1, PDFWeights[i]);
		}*/
		_outrootfile->Write(0, TObject::kOverwrite);
		_outrootfile->Close();
	}
    std::cout<< "-------------------------------------------------------------------" << std::endl;
    std::cout << "END...  :) " << std::endl; 

}

void NanoAODAnalyzerrdframe::setParams(int year, string runtype, int datatype)
{
    /*if(debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }*/
	_year=year;
	_runtype=runtype;
	_datatype=datatype;
	

	if(_year==2016) {
        cout << "Analysing through Run 2016" << endl;
    }else if(_year==2017) {
        cout << "Analysing through Run 2017" << endl;
    }else if(_year==2018){
        cout << "Analysing through Run 2018" << endl;
    }

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


	if (_datatype==0){
		_isData = false;
        std::cout << " MC input files Selected!! "<<std::endl;
        std::cout<< "-------------------------------------------------------------------" << std::endl;

    }else if(_datatype==1){
        _isData = true;
        std::cout << " DATA input files Selected!!" <<std::endl;
        std::cout<< "-------------------------------------------------------------------" << std::endl;
    }if(_datatype==-1){
		std::cout<< "Default root version :checking out gen branches! "<< std::endl;
        std::cout<< "-------------------------------------------------------------------" << std::endl;

		if (_atree->GetBranch("genWeight") == nullptr) {
			_isData = true;
			cout << "input file is DATA" <<endl;
		}
		else
		{
			_isData = false;
			cout << "input file is MC" <<endl;
		}
	}
	TObjArray *allbranches = _atree->GetListOfBranches();
	for (int i =0; i<allbranches->GetSize(); i++)
	{
		TBranch *abranch = dynamic_cast<TBranch *>(allbranches->At(i));
		if (abranch!= nullptr){
			//cout << abranch->GetName() << endl;
			_originalvars.push_back(abranch->GetName());
		}
	}


}
//Checking HLTs in the input root file
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
            if(_year==2016){
                HLTGlobalNames=HLT2016Names;
            }else if (_year==2017){
                HLTGlobalNames=HLT2017Names;
            }else if(_year==2018){
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
//control all branch names using in the addCuts function
std::string NanoAODAnalyzerrdframe::ctrlBranchName(std::string str_Branch){

    if(debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }

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



//cut-based ID Fall17 V2 (0:fail, 1:veto, 2:loose, 3:medium, 4:tight)
std::string NanoAODAnalyzerrdframe::ElectronID(int cutbasedID){

    if(debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }
    //double Electron_eta;
    //double Electron_pt;
    if (cutbasedID==1)std::cout<< " VETO Electron ID requested    == " << cutbasedID <<std::endl;
    if (cutbasedID==2)std::cout<< " LOOSE Electron ID requested   == " << cutbasedID <<std::endl;
    if (cutbasedID==3)std::cout<< " MEDIUM Electron ID requested  == " << cutbasedID <<std::endl;
    if (cutbasedID==4)std::cout<< " TIGHT Electron ID requested   == " << cutbasedID <<std::endl;
    std::cout<< "-------------------------------------------------------------------" << std::endl;

    if (cutbasedID<0 || cutbasedID>4){
        std::cout<< "ERROR!! Wrong Electron ID requested  == " << cutbasedID << "!! Can't be applied" <<std::endl;
        std::cout<< "Please select ElectronID from 1 to 4 " <<std::endl;
        std::cout<< "-------------------------------------------------------------------" << std::endl;
        std::cout<< "EXITING PROGRAM!!" << std::endl;
        exit(1);
    }
	/*if(_year==2018 && _isUL){
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

std::string NanoAODAnalyzerrdframe::MuonID(int cutbasedID){
    
    if(debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }
    
    //  cut-based ID Fall17 V2 (0:fail, 1:veto, 2:loose, 3:medium, 4:tight)
    if (cutbasedID==1)std::cout<< " Veto Muon ID requested   == " << cutbasedID <<std::endl;
    if (cutbasedID==2)std::cout<< " LOOSE Muon ID requested  == " << cutbasedID <<std::endl;
    if (cutbasedID==3)std::cout<< " MEDIUM Muon ID requested == " << cutbasedID <<std::endl;
    if (cutbasedID==4)std::cout<< " TIGHT Muon ID requested as == " << cutbasedID <<std::endl;
    std::cout<< "-------------------------------------------------------------------" << std::endl;

    if (cutbasedID<1 || cutbasedID>4){
        std::cout<< "ERROR!! Wrong Muon ID requested  == " << cutbasedID << "!! Can't be applied" <<std::endl;
        std::cout<< "Please select Muon ID from 2 to 4 " <<std::endl;
        std::cout<< "-------------------------------------------------------------------" << std::endl;
        std::cout<< "EXITING PROGRAM!!" << std::endl;

        exit(1);
    }

    string Muon_cutBased_ID;
    
    if (cutbasedID==1){
        Muon_cutBased_ID = "Muon_looseId";
        std::cout<< " VETO Muon ID requested == " << cutbasedID <<", but it doesn't exist in the nanoAOD branches. It is moved to loose MuonID. " << cutbasedID <<std::endl;
    }
    if (cutbasedID == 2){
        Muon_cutBased_ID = "Muon_looseId";
    
    }else if(cutbasedID == 3){
        Muon_cutBased_ID = "Muon_mediumId";

    }else if(cutbasedID == 4){
        Muon_cutBased_ID = "Muon_tightId";
    }
    string output;
    output = Form ("%s==true",Muon_cutBased_ID.c_str());
    return output;
}


std::string NanoAODAnalyzerrdframe::JetID(int cutbasedID){

    if(debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }

    if(cutbasedID<1 || cutbasedID>7){
        std::cout<< "Error Wrong JET ID requested == " << cutbasedID << "!! Can't be applied" <<std::endl;
        std::cout<< "Please select number from 1 to 7 " <<std::endl;

    }else{
	    std::cout<< " JET ID requested  == " << cutbasedID <<std::endl;
    }

    string output;
    output = Form ("Jet_jetId==%d",cutbasedID);
    return output;
}
