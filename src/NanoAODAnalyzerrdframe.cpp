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
#include <TRandom3.h>
correction::CorrectionSet* muon_scalsmear_corrector = nullptr;
#include "MuonScaRe.cc"
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
	_hist1dinfovector.clear();
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


void NanoAODAnalyzerrdframe::setupJetMETCorrection(string fname, string jettag,string jettagMC) //data
{

    cout << "SETUP JETMET correction" << endl;
	// read from file 
	_correction_jerc = correction::CorrectionSet::from_file(fname);//jercfname=json
	assert(_correction_jerc->validate()); //the assert functionality : check if the parameters passed to a function are valid =1:true
	// correction type(jobconfiganalysis.py)
	cout<<"JERC JSON file : " << fname<<endl;
    if (_isData){
        _jetCorrector = _correction_jerc->compound().at(jettag);//jerctag#JSON (JEC,compound)compoundLevel="L1L2L3Res"
    }
    else {
        cout<<"JERC JSON file : " << fname<<endl;
        _jetCorrector = _correction_jerc->compound().at(jettagMC);
    }
	cout<< "JET tag in JSON : " << jettag << endl;
	_jetCorrectionUnc = _correction_jerc->at(_jercunctag);
	cout<< "JET uncertainity tag in JSON  : " << _jercunctag << endl;
	std::cout<< "================================//=================================" << std::endl;
}
/*
void NanoAODAnalyzerrdframe::applyJetMETCorrections() //data
{
    cout << "apply JETMET correction" << endl;
if (!_isData){

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
            float corrfactor = _jetCorrector->evaluate({jetAreas[i], jetetas[i], rawjetpt, rho });
            //std::cout<<"correction factor===="<< corrfactor <<std::endl;
			corrfactors.emplace_back(rawjetpt * corrfactor);
			//std::cout<<"rawjetpt* corrfactor ===="<< rawjetpt * corrfactor <<std::endl;

		}
        //std::cout<<"Facsss===="<< corrfactors <<std::endl;
		return corrfactors;
		
	};
}
else {
	auto appcorrlambdaf = [this](floats jetpts, floats jetetas, floats jetAreas, floats jetrawf, float rho, float run)->floats
	{
		floats corrfactors;
		corrfactors.reserve(jetpts.size());
		for (auto i =0; i<int(jetpts.size()); i++)
		{
			float rawjetpt = jetpts[i]*(1.0-jetrawf[i]);
			//std::cout<<"jetpt===="<< jetpts[i] <<std::endl;
			//float jet_rawmass = jet_mass * (1 - jet.rawFactor)
			//std::cout<<"rawjetpt===="<< rawjetpt <<std::endl;
            float corrfactor = _jetCorrector->evaluate({jetAreas[i], jetetas[i], rawjetpt, rho , run});
            //std::cout<<"correction factor===="<< corrfactor <<std::endl;
			corrfactors.emplace_back(rawjetpt * corrfactor);
			//std::cout<<"rawjetpt* corrfactor ===="<< rawjetpt * corrfactor <<std::endl;

		}
        //std::cout<<"Facsss===="<< corrfactors <<std::endl;
		return corrfactors;
		
	};

}
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


	if (_jetCorrector != 0)
	{
        cout << "jetcorrector==" <<_jetCorrector << endl;

		_rlm = _rlm.Define("Jet_pt_corr", appcorrlambdaf, {"Jet_pt", "Jet_eta", "Jet_area", "Jet_rawFactor", "Rho_fixedGridRhoFastjetAll"});
		_rlm = _rlm.Define("Jet_pt_relerror", jecuncertaintylambdaf, {"Jet_pt", "Jet_eta", "Jet_area", "Jet_rawFactor", "Rho_fixedGridRhoFastjetAll"});
		_rlm = _rlm.Define("Jet_pt_corr_up", "Jet_pt_corr*(1.0f + Jet_pt_relerror)");
		_rlm = _rlm.Define("Jet_pt_corr_down", "Jet_pt_corr*(1.0f - Jet_pt_relerror)");

	}

}*/

void NanoAODAnalyzerrdframe::applyJetMETCorrections()
{
    std::cout << "Applying JET/MET corrections" << std::endl;

    using ROOT::VecOps::RVec;
    using floats = RVec<float>;

    //------------------------------------------------------------------
    // 1. Create a vectorized run branch (needed only for Data)
    //------------------------------------------------------------------
    if (_isData)
    {
        _rlm = _rlm.Define("run_f",
            [](unsigned int run, const floats &jetpts) {
                return floats(jetpts.size(), float(run));
            },
            {"run", "Jet_pt"}
        );
    }

    //------------------------------------------------------------------
    // 2. Define branches in RDF
    //------------------------------------------------------------------
    if (_jetCorrector != nullptr)
    {
        if (_isData)
        {
            // Lambda for Data (with run)
            auto jetCorrLambda_Data =
                [this](floats jetpts,
                       floats jetetas,
                       floats jetAreas,
                       floats jetrawf,
                       float rho,
                       floats run_f) -> floats
            {
                floats out;
                out.reserve(jetpts.size());

                for (size_t i = 0; i < jetpts.size(); i++)
                {
                    float rawpt = jetpts[i] * (1.f - jetrawf[i]);
                    float corr = _jetCorrector->evaluate({jetAreas[i], jetetas[i], rawpt, rho, run_f[i]});
                    out.emplace_back(rawpt * corr);
                }
                return out;
            };

            _rlm = _rlm.Define("Jet_pt_corr",
                jetCorrLambda_Data,
                {"Jet_pt", "Jet_eta", "Jet_area", "Jet_rawFactor",
                 "Rho_fixedGridRhoFastjetAll", "run_f"});
        }
        else
        {
            // Lambda for MC (without run)
            auto jetCorrLambda_MC =
                [this](floats jetpts,
                       floats jetetas,
                       floats jetAreas,
                       floats jetrawf,
                       float rho) -> floats
            {
                floats out;
                out.reserve(jetpts.size());

                for (size_t i = 0; i < jetpts.size(); i++)
                {
                    float rawpt = jetpts[i] * (1.f - jetrawf[i]);
                    float corr = _jetCorrector->evaluate({jetAreas[i], jetetas[i], rawpt, rho});
                    out.emplace_back(rawpt * corr);
                }
                return out;
            };

            _rlm = _rlm.Define("Jet_pt_corr",
                jetCorrLambda_MC,
                {"Jet_pt", "Jet_eta", "Jet_area", "Jet_rawFactor",
                 "Rho_fixedGridRhoFastjetAll"});
        }
    }
}
// Add to your NanoAODAnalyzerrdframe.cpp
// Include at the top:
// #include "MuonScaRe_RDF.cc"

// Add to your NanoAODAnalyzerrdframe.cpp
// Include at the top:
// #include "MuonScaRe_RDF.cc"

void NanoAODAnalyzerrdframe::applyMuPtCorrection()
{
    cout << "Applying Muon Pt correction using correctionlib" << endl;

    // Create helper object that captures the CorrectionSet
    auto muonHelper = std::make_shared<MuonCorrectionHelper>(_muon_scalsmear_corrector.get());

    if (_isData) {
        // Data: Only apply scale corrections
        auto lambdaf_data = [muonHelper](const ROOT::VecOps::RVec<int>& mu_charges,
                                         const ROOT::VecOps::RVec<float>& mu_pts,
                                         const ROOT::VecOps::RVec<float>& mu_etas,
                                         const ROOT::VecOps::RVec<float>& mu_phis)
        {
            ROOT::VecOps::RVec<float> corrMuPts;
            corrMuPts.reserve(mu_pts.size());
            
            for (size_t i = 0; i < mu_pts.size(); i++) {
                float corrected_pt = muonHelper->pt_scale(true, mu_pts[i], mu_etas[i], 
                                                         mu_phis[i], mu_charges[i]);
                corrMuPts.emplace_back(corrected_pt);
            }
            return corrMuPts;
        };

        _rlm = _rlm.Define("Muon_pt_corr", lambdaf_data, 
                          {"Muon_charge", "Muon_pt", "Muon_eta", "Muon_phi"});
    }
    else {
        // MC: Apply both scale and resolution corrections
        auto lambdaf_mc = [muonHelper](const ROOT::VecOps::RVec<int>& mu_charges,
                                       const ROOT::VecOps::RVec<float>& mu_pts,
                                       const ROOT::VecOps::RVec<float>& mu_etas,
                                       const ROOT::VecOps::RVec<float>& mu_phis,
                                       const ROOT::VecOps::RVec<UChar_t>& nls,
                                       ULong64_t event,
                                       UInt_t lumi)
        {
            ROOT::VecOps::RVec<float> corrMuPts;
            corrMuPts.reserve(mu_pts.size());
            
            for (size_t i = 0; i < mu_pts.size(); i++) {
                float nTrackerLayers = static_cast<float>(nls[i]);
                
                // Step 1: Apply scale correction
                float pt_scaled = muonHelper->pt_scale(false, mu_pts[i], mu_etas[i], 
                                                      mu_phis[i], mu_charges[i]);
                
                // Step 2: Apply resolution smearing
                float corrected_pt = muonHelper->pt_resol(pt_scaled, mu_etas[i], mu_phis[i],
                                                         nTrackerLayers, 
                                                         static_cast<int>(event), 
                                                         static_cast<int>(lumi));
                
                corrMuPts.emplace_back(corrected_pt);
            }
            return corrMuPts;
        };
        
        _rlm = _rlm.Define("Muon_pt_corr", lambdaf_mc, 
                          {"Muon_charge", "Muon_pt", "Muon_eta", "Muon_phi",
                           "Muon_nTrackerLayers", "event", "luminosityBlock"});
        
        // --- MC ONLY: Add systematic uncertainty variations ---
        cout << "Adding Muon Pt correction uncertainties for MC" << endl;
        
        // First, create the intermediate scaled pt (before resolution)
        auto lambdaf_scaled = [muonHelper](const ROOT::VecOps::RVec<int>& mu_charges,
                                           const ROOT::VecOps::RVec<float>& mu_pts,
                                           const ROOT::VecOps::RVec<float>& mu_etas,
                                           const ROOT::VecOps::RVec<float>& mu_phis)
        {
            ROOT::VecOps::RVec<float> scaledMuPts;
            scaledMuPts.reserve(mu_pts.size());
            
            for (size_t i = 0; i < mu_pts.size(); i++) {
                float pt_scaled = muonHelper->pt_scale(false, mu_pts[i], mu_etas[i], 
                                                      mu_phis[i], mu_charges[i]);
                scaledMuPts.emplace_back(pt_scaled);
            }
            return scaledMuPts;
        };

        _rlm = _rlm.Define("Muon_pt_scaled", lambdaf_scaled,
                          {"Muon_charge", "Muon_pt", "Muon_eta", "Muon_phi"});

        // Scale variations (Up)
        auto lambdaf_scale_up = [muonHelper](const ROOT::VecOps::RVec<float>& mu_pts_corr,
                                             const ROOT::VecOps::RVec<float>& mu_etas,
                                             const ROOT::VecOps::RVec<float>& mu_phis,
                                             const ROOT::VecOps::RVec<int>& mu_charges)
        {
            ROOT::VecOps::RVec<float> varMuPts;
            varMuPts.reserve(mu_pts_corr.size());
            
            for (size_t i = 0; i < mu_pts_corr.size(); i++) {
                float pt_var = muonHelper->pt_scale_var(mu_pts_corr[i], mu_etas[i], 
                                                       mu_phis[i], mu_charges[i], "up");
                varMuPts.emplace_back(pt_var);
            }
            return varMuPts;
        };

        _rlm = _rlm.Define("Muon_pt_corr_scaleUp", lambdaf_scale_up,
                          {"Muon_pt_corr", "Muon_eta", "Muon_phi", "Muon_charge"});

        // Scale variations (Down)
        auto lambdaf_scale_dn = [muonHelper](const ROOT::VecOps::RVec<float>& mu_pts_corr,
                                             const ROOT::VecOps::RVec<float>& mu_etas,
                                             const ROOT::VecOps::RVec<float>& mu_phis,
                                             const ROOT::VecOps::RVec<int>& mu_charges)
        {
            ROOT::VecOps::RVec<float> varMuPts;
            varMuPts.reserve(mu_pts_corr.size());
            
            for (size_t i = 0; i < mu_pts_corr.size(); i++) {
                float pt_var = muonHelper->pt_scale_var(mu_pts_corr[i], mu_etas[i], 
                                                       mu_phis[i], mu_charges[i], "dn");
                varMuPts.emplace_back(pt_var);
            }
            return varMuPts;
        };

        _rlm = _rlm.Define("Muon_pt_corr_scaleDn", lambdaf_scale_dn,
                          {"Muon_pt_corr", "Muon_eta", "Muon_phi", "Muon_charge"});

        // Resolution variations (Up)
        auto lambdaf_resol_up = [muonHelper](const ROOT::VecOps::RVec<float>& pt_scaled,
                                             const ROOT::VecOps::RVec<float>& pt_corr,
                                             const ROOT::VecOps::RVec<float>& mu_etas)
        {
            ROOT::VecOps::RVec<float> varMuPts;
            varMuPts.reserve(pt_corr.size());
            
            for (size_t i = 0; i < pt_corr.size(); i++) {
                float pt_var = muonHelper->pt_resol_var(pt_scaled[i], pt_corr[i], 
                                                       mu_etas[i], "up");
                varMuPts.emplace_back(pt_var);
            }
            return varMuPts;
        };

        _rlm = _rlm.Define("Muon_pt_corr_resolUp", lambdaf_resol_up,
                          {"Muon_pt_scaled", "Muon_pt_corr", "Muon_eta"});

        // Resolution variations (Down)
        auto lambdaf_resol_dn = [muonHelper](const ROOT::VecOps::RVec<float>& pt_scaled,
                                             const ROOT::VecOps::RVec<float>& pt_corr,
                                             const ROOT::VecOps::RVec<float>& mu_etas)
        {
            ROOT::VecOps::RVec<float> varMuPts;
            varMuPts.reserve(pt_corr.size());
            
            for (size_t i = 0; i < pt_corr.size(); i++) {
                float pt_var = muonHelper->pt_resol_var(pt_scaled[i], pt_corr[i], 
                                                       mu_etas[i], "dn");
                varMuPts.emplace_back(pt_var);
            }
            return varMuPts;
        };

        _rlm = _rlm.Define("Muon_pt_corr_resolDn", lambdaf_resol_dn,
                          {"Muon_pt_scaled", "Muon_pt_corr", "Muon_eta"});
    }
    
    cout << "Muon Pt correction applied successfully" << endl;
}

void NanoAODAnalyzerrdframe::applyElectronPtCorrection()
{
    std::cout << "Apply Electron Pt correction" << std::endl;

    if (!_correction_electronss) {
        std::cerr << "Electron corrections file not loaded!" << std::endl;
        return;
    }

    using ROOT::VecOps::RVec;
    using floats = RVec<float>;
      cout << "Works fine till her" << endl;
   auto smear_corr = _correction_electronss->at("SmearAndSyst");
 cout << "Works fine till her" << endl;

    auto scale_corr = _correction_electronss->compound().at("Scale");
    cout << "Works fine till her" << endl;

    if (_isData) {
        // For data: apply Scale corrections
        auto scale_lambda = [scale_corr](const ROOT::VecOps::RVec<float> &pt,
                                  const ROOT::VecOps::RVec<float> &scEta,
                                  const ROOT::VecOps::RVec<float> &r9,
                                  const ROOT::VecOps::RVec<UChar_t> &seedGain,
                                  unsigned int run) -> ROOT::VecOps::RVec<float>
        {
            ROOT::VecOps::RVec<float> result;
            result.reserve(pt.size());

            for (size_t i = 0; i < pt.size(); ++i) {
                try {
                    // Scale correction expects: syst, run, ScEta, r9, pt, seedGain
                    float factor = scale_corr->evaluate({
                        "scale",                    // syst (string)
                        static_cast<double>(run),              // run (real)
                        static_cast<double>(scEta[i]),         // ScEta (real) - NO abs()
                        static_cast<double>(r9[i]),            // r9 (real)
                        static_cast<double>(pt[i]),            // pt (real)
                        static_cast<double>(seedGain[i])       // seedGain (real)
                    });

                    result.emplace_back(pt[i] * factor);
                } catch (const std::exception &e) {
                    std::cerr << "Error evaluating scale correction at index " << i << ": " << e.what() << std::endl;
                    result.emplace_back(pt[i]);  // fallback to uncorrected
                }
            }

            return result;
        };

        _rlm = _rlm.Define("Electron_eta_supercluster", "Electron_eta + Electron_deltaEtaSC");
        _rlm = _rlm.Define("Electron_pt_corr", scale_lambda,
                           {"Electron_pt", "Electron_eta_supercluster", "Electron_r9", "Electron_seedGain", "run"});
    }
    else {
        // For MC: apply Smearing corrections
        auto smear_lambda = [smear_corr](const floats &pt,
                                          const floats &scEta,
                                          const floats &r9) -> std::tuple<floats, floats, floats>
        {
            floats nominal, smear_up, smear_down;
            size_t N = pt.size();
            nominal.reserve(N);
            smear_up.reserve(N);
            smear_down.reserve(N);

            std::random_device rd;
            std::mt19937 gen(rd());
            std::normal_distribution<float> gauss(0.0, 1.0);

            for (size_t i = 0; i < N; ++i) {
                try {
                    // SmearAndSyst expects: syst, pt, r9, ScEta
                    float smear_val = smear_corr->evaluate({
                        "smear",                              // syst (string)
                        static_cast<double>(pt[i]),           // pt (real)
                        static_cast<double>(r9[i]),           // r9 (real)
                        static_cast<double>(scEta[i])         // ScEta (real) - NO abs()
                    });

                    float smear_unc_up = smear_corr->evaluate({
                        "smear_up",                           // syst (string)
                        static_cast<double>(pt[i]),           // pt (real)
                        static_cast<double>(r9[i]),           // r9 (real)
                        static_cast<double>(scEta[i])         // ScEta (real)
                    });

                    float smear_unc_down = smear_corr->evaluate({
                        "smear_down",                         // syst (string)
                        static_cast<double>(pt[i]),           // pt (real)
                        static_cast<double>(r9[i]),           // r9 (real)
                        static_cast<double>(scEta[i])         // ScEta (real)
                    });

                    float rand = gauss(gen);

                    nominal.emplace_back(pt[i] * (1.0 + smear_val * rand));
                    smear_up.emplace_back(pt[i] * (1.0 + smear_unc_up * rand));
                    smear_down.emplace_back(pt[i] * (1.0 + smear_unc_down * rand));
                } catch (const std::exception &e) {
                    std::cerr << "Error evaluating smear correction at index " << i << ": " << e.what() << std::endl;
                    nominal.emplace_back(pt[i]);
                    smear_up.emplace_back(pt[i]);
                    smear_down.emplace_back(pt[i]);
                }
            }

            return std::make_tuple(nominal, smear_up, smear_down);
        };

        _rlm = _rlm.Define("Electron_eta_supercluster", "Electron_eta + Electron_deltaEtaSC");
        _rlm = _rlm.Define("Electron_pt_corr_triple", smear_lambda,
                           {"Electron_pt", "Electron_eta_supercluster", "Electron_r9"})
                   .Define("Electron_pt_corr", "std::get<0>(Electron_pt_corr_triple)")
                   .Define("Electron_pt_corr_smearUp", "std::get<1>(Electron_pt_corr_triple)")
                   .Define("Electron_pt_corr_smearDown", "std::get<2>(Electron_pt_corr_triple)");
    }
}

void NanoAODAnalyzerrdframe::applyMETPtPhiCorrection() //data and MC
{
  cout << "apply MET Pt and Phi correction" << endl;
  
  if(_isData){
    
    auto lambdaf_met_data = [this](float met_pt, float met_phi, unsigned char npvGood)->std::pair<float, float>
      {
        // Get corrected pt
        float met_pt_corr = _correction_MET_pt_corrector->at("met_xy_corrections")->evaluate({"pt", "PuppiMET", "2023", "DATA", "nom", 
                                                       met_pt, met_phi, static_cast<float>(npvGood)});
        
        // Get corrected phi
        float met_phi_corr = _correction_MET_pt_corrector->at("met_xy_corrections")->evaluate({"phi", "PuppiMET", "2023", "DATA", "nom", 
                                                        met_pt, met_phi, static_cast<float>(npvGood)});
        
        return std::make_pair(met_pt_corr, met_phi_corr);
      };
    
    _rlm = _rlm.Define("MET_pt_phi_corr", lambdaf_met_data, {"PuppiMET_pt", "PuppiMET_phi", "PV_npvsGood"});
    _rlm = _rlm.Define("PuppiMET_pt_corr", "MET_pt_phi_corr.first");
    _rlm = _rlm.Define("PuppiMET_phi_corr", "MET_pt_phi_corr.second");
  }
  else{
    
    auto lambdaf_met_mc = [this](float met_pt, float met_phi, unsigned char npvGood)->std::pair<float, float>
      {
        // Get corrected pt
        float met_pt_corr = _correction_MET_pt_corrector->at("met_xy_corrections")->evaluate({"pt", "PuppiMET", "2023", "MC", "nom", 
                                                       met_pt, met_phi, static_cast<float>(npvGood)});
        
        // Get corrected phi
        float met_phi_corr = _correction_MET_pt_corrector->at("met_xy_corrections")->evaluate({"phi", "PuppiMET", "2023", "MC", "nom", 
                                                        met_pt, met_phi, static_cast<float>(npvGood)});
        
        return std::make_pair(met_pt_corr, met_phi_corr);
      };
    
    _rlm = _rlm.Define("MET_pt_phi_corr", lambdaf_met_mc, {"PuppiMET_pt", "PuppiMET_phi", "PV_npvsGood"});
    _rlm = _rlm.Define("PuppiMET_pt_corr", "MET_pt_phi_corr.first");
    _rlm = _rlm.Define("PuppiMET_phi_corr", "MET_pt_phi_corr.second");
  }
}
void NanoAODAnalyzerrdframe::setupCorrections(string goodjsonfname, string pufname, string putag, string btvfname, string btvtype, /*, string fname_btagEff, string hname_btagEff_bcflav, string hname_btagEff_lflav,i*/ string muon_roch_fname, string muon_fname, string muonhlttype,string muonidtype,string muonisotype,string electron_fname,string electronHlt_fname,string electronHlt_type,string electron_reco_type1,string electron_reco_type2, string electron_id_type, string jercfname, string jerctag,string jerctagMC, string jercunctag,string jet_veto_f_name,string jet_veto_tag,string electron_SSF,string metpt_fname)
//In this function the correction is evaluated for each jet, Muon, Electron and MET. The correction depends on the momentum, pseudorapidity, energy, and cone area of the jet, as well as the value of “rho” (the average momentum per area) and number of interactions in the event. The correction is used to scale the momentum of the jet.
{
    cout << "set up Corrections!" << endl;
         _correction_electronss = correction::CorrectionSet::from_file(electron_SSF);
	 cout<< "Electron scaling and smearing filename   : " << electron_SSF << endl;
     assert(_correction_electronss->validate());


      _electron_SSF=electron_SSF;
     _correction_MET_pt_corrector = correction::CorrectionSet::from_file(metpt_fname);
	 cout<< "met pt correction file name    : " << metpt_fname<< endl;
     _metpt_fname=metpt_fname;
     assert(_correction_MET_pt_corrector->validate());

    _muon_scalsmear_corrector = correction::CorrectionSet::from_file(muon_roch_fname);
    cout<<"muon scaling and smearing filename :"<< muon_roch_fname<<endl;
    _muon_roch_fname=muon_roch_fname;
    assert(_muon_scalsmear_corrector ->validate());

	if (_isData) _jsonOK = readgoodjson(goodjsonfname); // read golden json file
	std::cout << "Rochester correction files: " << muon_roch_fname << std::endl;
         _correction_jetveto = correction::CorrectionSet::from_file(jet_veto_f_name);
	 cout<< "Jrt veto JSON FILE : " <<  jet_veto_f_name << endl;
         assert(_correction_jetveto->validate());
         _jet_veto_tag = jet_veto_tag;
	if (!_isData) {
	  // using correctionlib
	  //Muon corrections
	  _correction_muon = correction::CorrectionSet::from_file(muon_fname);
	  _muon_hlt_type = muonhlttype;
	  //_muon_reco_type = muonrecotype;
	  _muon_id_type = muonidtype;
	  _muon_iso_type = muonisotype;
	  std::cout<< "================================//=================================" << std::endl;
	  cout<< "MUON JSON FILE : " <<  muon_fname << endl;
	  cout<< "MUON HLT type in JSON  : " << _muon_hlt_type << endl;
	 // cout<< "MUON RECO type in JSON  : " << _muon_reco_type << endl;
	  cout<< "MUON ID type in JSON  : " << _muon_id_type << endl;
	  cout<< "MUON ISO type in JSON  : " << _muon_iso_type << endl;
	  assert(_correction_muon->validate());
	  
	  //Electron corrections
	  _correction_electron = correction::CorrectionSet::from_file(electron_fname);
      _correction_electronHlt = correction::CorrectionSet::from_file(electronHlt_fname);

	  _electron_reco_type1=electron_reco_type1;
	  _electron_reco_type2=electron_reco_type2;
	  _electron_id_type = electron_id_type;
      _electronHlt_type =electronHlt_type;
	  std::cout<< "================================//=================================" << std::endl;
	  cout<< "ELECTRON JSON FILE : " << electron_fname << endl;
	  cout<< "ELECTRON HLT JSON FILE : " << electronHlt_fname << endl;

      cout<< "ELECTRON RECO type in JSON  : " << _electron_reco_type1 << endl;
	  cout<< "ELECTRONID type in JSON  : " << _electron_id_type << endl;
	  cout<< "ELECTRON HLT type in JSON  : " << _electronHlt_type << endl;
      assert(_correction_electron->validate());
      assert(_correction_electronHlt->validate());
	  //electron scale and smearing correction
 
	  // btag corrections
	  _correction_btag1 = correction::CorrectionSet::from_file(btvfname);
	  cout<< "btv correction filename: " << btvfname << endl;

	  _btvtype = btvtype;
	  assert(_correction_btag1->validate());
/*
	  fname_btagEff = new TFile(fname_btagEff.c_str(), "READ");
	  hname_btagEff_bcflav = dynamic_cast<TH2D*>(f_btagEff->Get(hname_btagEff_bcflav.c_str()));
	  hname_btagEff_lflav = dynamic_cast<TH2D*>(f_btagEff->Get(hname_btagEff_lflav.c_str()));
*/

	  // pile up weights
	  _correction_pu = correction::CorrectionSet::from_file(pufname);
	  cout<< "Pileup correction filename  : " << pufname << endl;

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
    _jerctagMC=jerctagMC;
	_jercunctag = jercunctag;
	
	setupJetMETCorrection(jercfname, _jerctag,_jerctagMC);
	applyJetMETCorrections();
	applyMuPtCorrection();
    applyElectronPtCorrection();
     applyMETPtPhiCorrection();

}
/*double NanoAODAnalyzerrdframe::getBTaggingEff(double hadflav, double eta, double pt){
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

    efficiency = hist_btagEff_lflav->GetBinContent(binX, binY);
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
    ;
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
*/
ROOT::RDF::RNode NanoAODAnalyzerrdframe::calculateBTagSF(
    RNode _rlm, std::vector<std::string> Jets_vars_names, int _case, std::string output_var)
{
    // ================================================================
    // Case 1 : FixedWP / shape correction for MC
    // ================================================================
    if (_case == 1)
    {
        std::cout << "Case 1: FixedWP/shape correction for MC" << std::endl;

        auto btagweightgenerator_case1 =
            [this](const ROOT::VecOps::RVec<unsigned char> &hadflav,
                    const ROOT::VecOps::RVec<float> &etas,
                    const ROOT::VecOps::RVec<float> &pts,
                    const std::string &variation) -> float
        {
            double btagWeight = 1.0;

            // Debug print for the first few entries only for central variation
            static int debug_counter = 0;
            bool do_debug = (variation == "central" && debug_counter < 5);



            for (std::size_t i = 0; i < pts.size(); ++i)
            {
                if (std::abs(etas[i]) > 2.5 || pts[i] < 30.0)
                    continue;

                double weight = _correction_btag1->at("particleNet_shape")
                                    ->evaluate({variation, static_cast<int>(hadflav[i]),
                                                std::fabs(etas[i]), pts[i], 0.1917});
                btagWeight *= weight;
            }


            return btagWeight;
        };

        std::vector<std::string> variations = {
            "central", "up", "down", "up_correlated", "down_correlated", "uncorrelated"};

        for (const auto &variation_name : variations)
        {
            const std::string varcopy = variation_name;
            std::string column_name = output_var + varcopy;

            _rlm = _rlm.Define(column_name,
                               [btagweightgenerator_case1, varcopy](const ROOT::VecOps::RVec<unsigned char> &hadflav,
                                                                    const ROOT::VecOps::RVec<float> &etas,
                                                                    const ROOT::VecOps::RVec<float> &pts)
                               {
                                   return btagweightgenerator_case1(hadflav, etas, pts, varcopy);
                               },
                               Jets_vars_names);

            std::cout << "Defined column: " << column_name << std::endl;
        }
    }



    // ================================================================
    // Case 3 : DeepJet shape correction for MC
    // ================================================================
    else if (_case == 3)
    {
        std::cout << "Case 3: Shape correction (DeepJet) for MC" << std::endl;

        auto btagweightgenerator_case3 =
            [this](const ROOT::VecOps::RVec<unsigned char> &hadflav,
                    const ROOT::VecOps::RVec<float> &etas,
                    const ROOT::VecOps::RVec<float> &pts,
                    const ROOT::VecOps::RVec<float> &btags) -> float
        {
            double bweight = 1.0;

            for (std::size_t i = 0; i < pts.size(); ++i)
            {
                if (std::abs(etas[i]) > 2.5 || pts[i] < 30.0)
                    continue;

                double w = _correction_btag1->at(_btvtype)->evaluate(
                    {"central", static_cast<int>(hadflav[i]),
                     std::fabs(etas[i]), pts[i], btags[i]});

                bweight *= w;
            }
            return bweight;
        };

        std::string column_name = output_var + "_case3";
        _rlm = _rlm.Define(column_name, btagweightgenerator_case3, Jets_vars_names);

        std::cout << "BJet SF column name: " << column_name << std::endl;
    }

    // ================================================================
    // Return the modified RDataFrame
    // ================================================================
    return _rlm;
}


ROOT::RDF::RNode NanoAODAnalyzerrdframe::calculateMuSF(RNode _rlm, std::vector<std::string> Muon_vars, std::string output_var)
{

    //=====================================================Muon SF and eventweight============================================================// 
    //muontype= for thight: NUM_TightID_DEN_genTracks //for medium: NUM_MediumID_DEN_TrackerMuons
    //Muon MediumID ISO UL type: NUM_TightRelIso_DEN_MediumID && thightID:NUM_TightRelIso_DEN_TightIDandIPCut --> the type can be found in json file
    //--> As an example Medium wp is used 
    //===============================================================================================================================================//
    cout<<"muon HLT SF for MC "<<endl;
  auto muon_weightgenerator = [this](const std::string& muon_type, const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts, const std::string& variation) -> float {
      double muonHLT_w = 1.0;

      for (std::size_t i = 0; i < pts.size(); i++) {
	//std::cout << "Muon abs_eta:" << std::fabs(etas[i]) << " pt: " << pts[i] << std::endl;
	double w = _correction_muon->at(muon_type)->evaluate({std::fabs(etas[i]), pts[i], variation}); 
	muonHLT_w *= w;
	//std::cout << "Individual HLT weight (muon " << i << "): " << w << std::endl;
	//std::cout << "Cumulative HLT weight after muon " << i << ": " << muonHLT_w << std::endl;
      }
      return muonHLT_w;
    };

    //'sf' is nominal, and 'systup' and 'systdown' are up/down variations with total stat+-syst uncertainties. Individual systs are also available (in these cases syst only, not sf +/- syst
    std::vector<std::string> variations = {"nominal", "systup", "systdown","syst"};


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


/*      std::string column_name_reco = output_var+"reco_" + variation;
      _rlm = _rlm.Define(column_name_reco, [this, muon_weightgenerator, variation](const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts) {
	  float weight = muon_weightgenerator(_muon_reco_type, etas, pts, variation); // Get the weight for the corresponding variation
	  //std::cout << "Muon HLT weight (" << variation << "): " << weight << std::endl;
	  return weight;
	}, Muon_vars);
*/

      std::string column_name_id = output_var+"id_" + variation;
      _rlm = _rlm.Define(column_name_id, [this, muon_weightgenerator, variation](const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts) {
	  float weight = muon_weightgenerator(_muon_id_type, etas, pts, variation); // Get the weight for the corresponding variation
	  //std::cout << "Muon id  weight (" << variation << "): " << weight << std::endl;
	  return weight;
	}, Muon_vars);


      std::string column_name_iso = output_var+"iso_" + variation;
      _rlm = _rlm.Define(column_name_iso, [this, muon_weightgenerator, variation](const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts) {
	  float weight = muon_weightgenerator(_muon_iso_type, etas, pts, variation); // Get the weight for the corresponding variation
	  //std::cout << "Muon iso weight (" << variation << "): " << weight << std::endl;
	  return weight;
	}, Muon_vars);

      std::string column_name = output_var;
      if(variation=="nominal"){
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

	std::string sf_definition = column_name_id+" * "+column_name_iso;
	_rlm = _rlm.Define(column_name, sf_definition);
	std::cout<< "Muon SF column name: " << column_name << std::endl;
    }
    return _rlm;
}

/*
ROOT::RDF::RNode NanoAODAnalyzerrdframe::calculateEleSF(RNode _rlm, std::vector<std::string> Ele_vars, std::string output_var)
{

    //auto cs = correction::CorrectionSet::from_file("electron.json.gz");
    //cout<<"Generate ELECTRONRECO weight"<<endl;
    //electronRECO sf and systematics with up/down variations
    //===========//===========//===========//===========//===========
    auto electron_weightgenerator = [this](const std::string eletype, const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts, const std::string& variation) -> float {
        double electronReco_w = 1.0;

        for (std::size_t i = 0; i < pts.size(); i++) {

            double w = _correction_electron->at("Electron-ID-SF")->evaluate({"2022Re-recoBCD", variation, eletype, std::fabs(etas[i]), pts[i]}); 
            electronReco_w *= w;
            //std::cout << "Individual weight (electron " << i << "): " << w << std::endl;
            //std::cout << "Cumulative weight after electron " << i << ": " << electronId_w << std::endl;
        }
        return electronReco_w;
    };
    auto electronHlt_weightgenerator = [this](const std::string eletype, const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts, const std::string& variation) -> float {
        double electronHlt_w = 1.0;

        for (std::size_t i = 0; i < pts.size(); i++) {

            double w = _correction_electron->at("Electron-HLT-SF")->evaluate({"2022Re-recoBCD", variation, eletype, std::fabs(etas[i]), pts[i]}); 
            electronHlt_w *= w;
            //std::cout << "Individual weight (electron " << i << "): " << w << std::endl;
            //std::cout << "Cumulative weight after electron " << i << ": " << electronId_w << std::endl;
        }
        return electronHlt_w;
    };
    //'sf' is nominal, and 'systup' and 'systdown' are up/down variations with total stat+-syst uncertainties. Individual systs are also available (in these cases syst only, not sf +/- syst
    std::vector<std::string> variations_elec = {"sf", "sfup", "sfdown"};


    for (const std::string& variation : variations_elec) {
*/
        // define electron RECO weight sf/systs for each variation individually
        /*      std::string column_name_reco = output_var+ "reco_" + variation;
                _rlm = _rlm.Define(column_name_reco, [this, electron_weightgenerator, variation](const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts) {
                float weight = electron_weightgenerator(_electron_reco_type, etas, pts, variation); // Get the weight for the corresponding variation
        //std::cout << "Electron RECO weight (" << variation << "): " << weight << std::endl;
        return weight;
        }, Ele_vars);*/
/*
        std::string column_name_reco = output_var + "reco_" + variation;
        _rlm = _rlm.Define(column_name_reco,
                [this, electron_weightgenerator, variation](const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts) {
                ROOT::VecOps::RVec<float> weights(pts.size());

                for (size_t i = 0; i < pts.size(); ++i) {
                std::string reco_type = (pts[i] < 75.0) ? _electron_reco_type1 : _electron_reco_type2;

                // Wrap each eta and pt into RVec of size 1 for individual eval
                ROOT::VecOps::RVec<float> eta_single = { etas[i] };
                ROOT::VecOps::RVec<float> pt_single = { pts[i] };

                weights[i] = electron_weightgenerator(reco_type, eta_single, pt_single, variation);
                }
                return std::accumulate(weights.begin(), weights.end(), 1.0f, std::multiplies<float>());
                }, Ele_vars);



        // define electron ID weight sf/systs for each variation individually
        std::string column_name_id = output_var+ "id_" + variation;
        _rlm = _rlm.Define(column_name_id, [this, electron_weightgenerator, variation](const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts) {
                float weight = electron_weightgenerator(_electron_id_type, etas, pts, variation); // Get the weight for the corresponding variation
                //std::cout << "Electron RECO weight (" << variation << "): " << weight << std::endl;
                return weight;
                }, Ele_vars);
        std::string column_name_Hlt = output_var+ "Hlt_" + variation;
        _rlm = _rlm.Define(column_name_Hlt, [this, electronHlt_weightgenerator, variation](const ROOT::VecOps::RVec<float>& etas, const ROOT::VecOps::RVec<float>& pts) {
                float weight = electronHlt_weightgenerator(_electron_id_type, etas, pts, variation); // Get the weight for the corresponding variation
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
        std::string sf_definition = column_name_reco+" * "+column_name_id+"*"+ column_name_Hlt;

        _rlm = _rlm.Define(column_name, sf_definition); 

    }
    return _rlm;
}
*/
ROOT::RDF::RNode NanoAODAnalyzerrdframe::calculateEleSF(
    RNode _rlm,
    std::vector<std::string> Ele_vars,
    std::string output_var)
{
    // electron RECO & ID scale factor generator
    auto electron_weightgenerator = [this](
        const std::string eletype,
        const ROOT::VecOps::RVec<float>& etas,
        const ROOT::VecOps::RVec<float>& pts,
        const ROOT::VecOps::RVec<float>& phis,
        const std::string& variation) -> float 
    {
        double w_tot = 1.0;

        for (size_t i = 0; i < pts.size(); i++) {
            double w = _correction_electron
                ->at("Electron-ID-SF")
                ->evaluate({"2023PromptC", variation, eletype,
                            std::fabs(etas[i]), pts[i], phis[i]});
            w_tot *= w;
        }
        return w_tot;
    };

    // electron HLT scale factor generator
    auto electronHlt_weightgenerator = [this](
        const std::string eletype,
        const ROOT::VecOps::RVec<float>& etas,
        const ROOT::VecOps::RVec<float>& pts,
        const std::string& variation) -> float
    {
        double w_tot = 1.0;

        for (size_t i = 0; i < pts.size(); i++) {

            if (pts[i] < 25.0) continue; // HLT threshold

            double w = _correction_electronHlt
                ->at("Electron-HLT-SF")
                ->evaluate({"2023PromptC", variation, eletype,
                            etas[i], pts[i]});
            w_tot *= w;
        }
        return w_tot;
    };


    // variations
    std::vector<std::string> variations_elec = {"sf", "sfup", "sfdown"};

    for (const std::string& variation : variations_elec)
    {
        // ======================================================
        // 1) ELECTRON RECO SCALE FACTOR
        // ======================================================
        std::string column_name_reco = output_var + "reco_" + variation;

        _rlm = _rlm.Define(
            column_name_reco,
            [this, electron_weightgenerator, variation](
                const ROOT::VecOps::RVec<float>& etas,
                const ROOT::VecOps::RVec<float>& pts,
                const ROOT::VecOps::RVec<float>& phis)
            {
                ROOT::VecOps::RVec<float> weights(pts.size());

                for (size_t i = 0; i < pts.size(); ++i) {

                    std::string reco_type =
                        (pts[i] < 75.0) ? _electron_reco_type1 : _electron_reco_type2;

                    ROOT::VecOps::RVec<float> eta1 = {etas[i]};
                    ROOT::VecOps::RVec<float> pt1  = {pts[i]};
                    ROOT::VecOps::RVec<float> phi1 = {phis[i]};

                    weights[i] = electron_weightgenerator(
                        reco_type, eta1, pt1, phi1, variation);
                }

                return std::accumulate(weights.begin(),
                                       weights.end(),
                                       1.0f,
                                       std::multiplies<float>());
            },
            Ele_vars  // MUST have 3 vars: eta, pt, phi
        );

        // ======================================================
        // 2) ELECTRON ID SCALE FACTOR
        // ======================================================
        std::string column_name_id = output_var + "id_" + variation;

        _rlm = _rlm.Define(
            column_name_id,
            [this, electron_weightgenerator, variation](
                const ROOT::VecOps::RVec<float>& etas,
                const ROOT::VecOps::RVec<float>& pts,
                const ROOT::VecOps::RVec<float>& phis)
            {
                return electron_weightgenerator(
                    _electron_id_type, etas, pts, phis, variation);
            },
            Ele_vars
        );

        // ======================================================
        // 3) ELECTRON HLT SCALE FACTOR
        // ======================================================
        std::string column_name_Hlt = output_var + "Hlt_" + variation;

        _rlm = _rlm.Define(
            column_name_Hlt,
            [this, electronHlt_weightgenerator, variation](
                const ROOT::VecOps::RVec<float>& etas,
                const ROOT::VecOps::RVec<float>& pts,
                const ROOT::VecOps::RVec<float>& phis)   // phi included for consistency
            {
                return electronHlt_weightgenerator(
                    _electronHlt_type, etas, pts, variation);
            },
            Ele_vars
        );

        // ======================================================
        // 4) COMBINE: RECO * ID * HLT
        // ======================================================
        std::string column_name = output_var;

        if (variation == "sf")
            column_name += "central";
        else if (variation == "sfup")
            column_name += "up";
        else
            column_name += "down";

        std::cout << "Electron SF column name: " << column_name << std::endl;

        _rlm = _rlm.Define(
            column_name,
            column_name_reco + " * " + column_name_id 
        );
    }

    return _rlm;
}

ROOT::RDF::RNode NanoAODAnalyzerrdframe::applyJetVetoMap(ROOT::RDF::RNode _rlm,
                                                          const std::string& eta_var,
                                                          const std::string& phi_var,
                                                          const std::string& output_var) {
    std::cout << "Applying Jet veto map..." << std::endl;

    auto is_vetoed_event = [this](const ROOT::VecOps::RVec<float>& etas,
                                  const ROOT::VecOps::RVec<float>& phis) -> bool {
        // Get the correction object
        auto veto_corr = _correction_jetveto->at(_jet_veto_tag);
        std::string veto_type = "jetvetomap";

        for (size_t i = 0; i < etas.size(); ++i) {
            double veto_val = veto_corr->evaluate({veto_type, etas[i], phis[i]});
            if (veto_val != 0) {
                return true;  // At least one jet in vetoed region
            }
        }
        return false;  // No jet in vetoed region
    };

    // Define a new column with a single boolean per event
    return _rlm.Define(output_var, is_vetoed_event, {eta_var, phi_var});

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
/*
void NanoAODAnalyzerrdframe::run(bool saveAll, string outtreename)
{
    cout << "\n===================== DEBUG: Starting run() =====================\n";

    vector<RNodeTree *> rntends;
    _rnt.getRNodeLeafs(rntends);

    for (auto arnt : rntends)
    {
        string nodename = arnt->getIndex();
        string outname = _outfilename;

        if (rntends.size() > 1)
            outname.replace(outname.find(".root"), 5, "_" + nodename + ".root");

        RNode *arnode = arnt->getRNode();
        auto &vars = _varstostorepertree[nodename];

        cout << "\n===================== NODE " << nodename << " =====================\n";
        cout << "[DEBUG] Checking branch TYPE and existence...\n";

        bool hasError = false;

        for (auto &v : vars)
        {
            cout << " → " << v << " : ";

            // check existence
            if (!arnode->HasColumn(v))
            {
                cout << "❌ MISSING\n";
                hasError = true;
                continue;
            }

            // check type
            try
            {
                auto t = arnode->GetColumnType(v);
                cout << "type = " << t << " ✔\n";
            }
            catch (const std::exception &e)
            {
                cout << "❌ TYPE ERROR! " << e.what() << endl;
                hasError = true;
            }
        }

        if (hasError)
        {
            cout << "\n❌ FATAL: Errors found above. Fix missing/type-mismatch columns.\n";
            return;
        }

        // Test evaluation of all snapshot columns
        cout << "\n[DEBUG] Forcing evaluation of ALL columns...\n";
        try
        {
            for (auto &v : vars)
            {
                cout << "[EVAL] " << v << endl;
                arnode->Take<std::string>(v);   // try to fetch raw data
            }
        }
        catch (const std::exception &e)
        {
            cout << "\n❌ COLUMN EVALUATION FAILED in " << endl;
            cout << "   → " << e.what() << endl;
            cout << "This is the broken branch." << endl;
            return;
        }

        cout << "\n[DEBUG] Count() OK, evaluating...\n";
        cout << " → Events: " << arnode->Count().GetValue() << endl;

        cout << "\n[DEBUG] RUNNING SNAPSHOT...\n";

        try
        {
            if (saveAll)
                arnode->Snapshot(outtreename, outname);
            else
                arnode->Snapshot(outtreename, outname, vars);
        }
        catch (const std::exception &e)
        {
            cout << "\n❌ SNAPSHOT CRASH!\n";
            cout << "Reason: " << e.what() << endl;
            return;
        }

        cout << "✔ Snapshot succeeded.\n";
    }

    cout << "\n===================== END =====================\n";
}

*/

void NanoAODAnalyzerrdframe::run(bool saveAll, string outtreename){
	vector<RNodeTree *> rntends;
	_rnt.getRNodeLeafs(rntends);
	_rnt.Print();
	 cout << rntends.size() << endl;
	for (auto arnt : rntends)
	{
		string nodename = arnt->getIndex();
		string outname = _outfilename;
		if (rntends.size() > 1)
			outname.replace(outname.find(".root"), 5, "_" + nodename + ".root");
		_outrootfilenames.push_back(outname);
		RNode *arnode = arnt->getRNode();
		std::cout << "-------------------------------------------------------------------" << std::endl;
		cout << "cut : ";
		cout << arnt->getIndex();
		if (saveAll)
		{
			arnode->Snapshot(outtreename, outname);
		}
		else
		{
		
			
			cout << " --writing branches" << endl;
			std::cout << "-------------------------------------------------------------------" << std::endl;

			for (auto bname : _varstostorepertree[nodename])
			{

			       
			   	cout << bname << endl;
			        cout << "-----branch stored" << endl;
			}

			arnode->Snapshot(outtreename, outname, _varstostorepertree[nodename]);


		}
		std::cout << "-------------------------------------------------------------------" << std::endl;
		cout << "Creating output root file :  " << endl;
		cout << outname << " ";
		cout << endl;
		std::cout << "-------------------------------------------------------------------" << std::endl;
		_outrootfile = new TFile(outname.c_str(), "UPDATE");
		cout << "Writing histograms...   " << endl;
		std::cout << "-------------------------------------------------------------------" << std::endl;
		for (auto &h : _th1dhistos)
		{
			if (h.second.GetPtr() != nullptr)
			{
				h.second.GetPtr()->Print();
				h.second.GetPtr()->Write();
			}
		}
		// for 2D histograms
		for (auto &h : _th2dhistos)
		{
			if (h.second.GetPtr() != nullptr)
			{
				h.second.GetPtr()->Print();
				h.second.GetPtr()->Write();
			}
		}
		_outrootfile->Write(0, TObject::kOverwrite);
		_outrootfile->Close();
	}
	std::cout << "-------------------------------------------------------------------" << std::endl;
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
	

	if(_year==2022) {
        cout << "Analysing through Run 2022" << endl;
    }else if(_year==2023) {
        cout << "Analysing through Run 2017" << endl;
    }else if(_year==2024){
        cout << "Analysing through Run 2018" << endl;
    }

	if(_runtype.find("PreEE") != std::string::npos){
        _isPreEE = true;
        cout << "PreEE Selected " << endl;
        std::cout<< "-------------------------------------------------------------------" << std::endl;
    }else if(_runtype.find("PostEE") != std::string::npos){
        _isPostEE = true;
        cout << "PostEE  Selected!" << endl;
        std::cout<< "-------------------------------------------------------------------" << std::endl;
    }
    if (!_isPreEE && !_isPostEE){
        std::cout<< "Default run version :PreEE or PostEE is not selected! "<< std::endl;
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
            if(_year==2022){
                HLTGlobalNames=HLT2022Names;
            }else if (_year==2023){
                HLTGlobalNames=HLT2023Names;
            }else if(_year==2024){
                HLTGlobalNames=HLT2024Names;
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
