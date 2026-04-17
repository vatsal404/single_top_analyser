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


void NanoAODAnalyzerrdframe::setupJetMETCorrection(string fname, string jettag,string jettagMC,string JER_tag,string JER_tag_res) //data
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
    for (const auto& tag : _jercunctag){
        _jetCorrectionUnc.emplace_back(tag, _correction_jerc->at(tag));
    }
    for (const auto& tag : _jercunctag) {
    cout << "JET uncertainty tag in JSON : " << tag << endl;
    }
    cout<< "JER tag in json: " << JER_tag << endl;
    _jer_corrector = _correction_jerc->at(JER_tag);

    _jer_resolution = _correction_jerc->at(JER_tag_res);

	std::cout<< "================================//=================================" << std::endl;
}

void NanoAODAnalyzerrdframe::applyJetMETCorrections()
{
    std::cout << "Applying JET/MET corrections" << std::endl;

    using ROOT::VecOps::RVec;
    using floats = RVec<float>;

    //------------------------------------------------------------------
    // MET Propagation (Type-1 PUPPI MET correction)
    //------------------------------------------------------------------
    auto propagatePuppiMET = [](
            floats jetpt_corr,
            floats jeteta,
            floats jetphi,
            floats jetrawf,
            floats jetpt_nano,
            float  met_pt,
            float  met_phi) -> floats
    {
        float met_x = met_pt * std::cos(met_phi);
        float met_y = met_pt * std::sin(met_phi);

        for (size_t i = 0; i < jetpt_corr.size(); i++)
        {
            float rawpt   = jetpt_nano[i] * (1.f - jetrawf[i]);
            float corr_pt = jetpt_corr[i];

            if (corr_pt < 15.f || std::abs(jeteta[i]) > 5.2f) continue;

            float dpx = (corr_pt - rawpt) * std::cos(jetphi[i]);
            float dpy = (corr_pt - rawpt) * std::sin(jetphi[i]);

            met_x -= dpx;
            met_y -= dpy;
        }

        return floats{ std::sqrt(met_x*met_x + met_y*met_y),
                       std::atan2(met_y, met_x) };
    };

    //------------------------------------------------------------------
    // Helper to define a MET variant column pair from a jet pT column
    //------------------------------------------------------------------
    auto defineMETVariant = [&](const std::string& jetCol,
                                const std::string& metPtCol,
                                const std::string& metPhiCol)
    {
        std::string vecCol = metPtCol + "_vec";

        _rlm = _rlm.Define(vecCol,
            [propagatePuppiMET](floats jpt_corr, floats jeta, floats jphi,
                                floats jrawf,    floats jpt,
                                float  met_pt,   float  met_phi)
            {
                return propagatePuppiMET(jpt_corr, jeta, jphi, jrawf, jpt, met_pt, met_phi);
            },
            {jetCol, "Jet_eta", "Jet_phi", "Jet_rawFactor",
             "Jet_pt", "PuppiMET_pt", "PuppiMET_phi"});

        _rlm = _rlm.Define(metPtCol,  vecCol + "[0]");
        _rlm = _rlm.Define(metPhiCol, vecCol + "[1]");

        std::cout << "Defined MET columns: " << metPtCol
                  << ", " << metPhiCol << std::endl;
    };

    if (_jetCorrector == nullptr) return;

    //------------------------------------------------------------------
    // DATA path
    //------------------------------------------------------------------
    if (_isData)
    {
        // Vectorised run branch needed by the corrector
        _rlm = _rlm.Define("run_f",
            [](unsigned int run, const floats& jetpts) {
                return floats(jetpts.size(), float(run));
            },
            {"run", "Jet_pt"});

        auto jetCorrLambda_Data =
            [this](floats jetpts,
                   floats jetetas,
                   floats jetAreas,
                   floats jetrawf,
                   float  rho,
                   floats jetphis,
                   floats run_f) -> floats
        {
            floats out;
            out.reserve(jetpts.size());

            for (size_t i = 0; i < jetpts.size(); i++)
            {
                float rawpt = jetpts[i] * (1.f - jetrawf[i]);
                float corr;

                if (_year == "2023")
                    corr = _jetCorrector->evaluate({jetAreas[i], jetetas[i], rawpt, rho, run_f[i]});
                else if (_year == "2023BPix" || _year == "2024")
                    corr = _jetCorrector->evaluate({jetAreas[i], jetetas[i], rawpt, rho, jetphis[i], run_f[i]});
                else
                    corr = _jetCorrector->evaluate({jetAreas[i], jetetas[i], rawpt, rho});
                out.emplace_back(rawpt * corr);
            }
            return out;
        };

        _rlm = _rlm.Define("Jet_pt_corr",
            jetCorrLambda_Data,
            {"Jet_pt", "Jet_eta", "Jet_area", "Jet_rawFactor",
             "Rho_fixedGridRhoFastjetAll", "Jet_phi", "run_f"});

        // Nominal MET propagation only — no JER/JEC systematics for Data
        defineMETVariant("Jet_pt_corr", "PuppiMET_pt_corr", "PuppiMET_phi_corr");
    }
    //------------------------------------------------------------------
    // MC path
    //------------------------------------------------------------------
    else
    {
        // ------------------------------------------
        // Step 1: JEC
        // ------------------------------------------
        auto jetCorrLambda_MC =
            [this](floats jetpts,
                   floats jetetas,
                   floats jetAreas,
                   floats jetrawf,
                   floats jetphis,
                   float  rho) -> floats
        {
            floats out;
            out.reserve(jetpts.size());

            for (size_t i = 0; i < jetpts.size(); i++)
            {
                float rawpt = jetpts[i] * (1.f - jetrawf[i]);
                float corr  = (_year == "2023BPix" && _year == "2024")
                    ? _jetCorrector->evaluate({jetAreas[i], jetetas[i], rawpt, rho, jetphis[i]})
                    : _jetCorrector->evaluate({jetAreas[i], jetetas[i], rawpt, rho});

                out.emplace_back(rawpt * corr);
            }
            return out;
        };

        _rlm = _rlm.Define("Jet_pt_JEC",
            jetCorrLambda_MC,
            {"Jet_pt", "Jet_eta", "Jet_area", "Jet_rawFactor",
             "Jet_phi", "Rho_fixedGridRhoFastjetAll"});

        // ------------------------------------------
        // Step 2: JER smearing (nom / up / down)
        // ------------------------------------------
        auto jerSmearLambda =
            [this](floats jetpts,
                   floats jetetas,
                   floats jetgenpt,
                   float  rho,
                   unsigned int       run_n,
                   unsigned int       lumi,
                   unsigned long long event,
                   std::string        variation) -> floats
        {
            floats out;
            out.reserve(jetpts.size());

            std::normal_distribution<float> gauss(0.f, 1.f);

            for (size_t i = 0; i < jetpts.size(); i++)
            {
                float pt    = jetpts[i];
                float eta   = jetetas[i];
                float genpt = jetgenpt[i];

                uint64_t seed =
                    (uint64_t(run_n) << 32) ^
                    (uint64_t(lumi)  << 16) ^
                    (uint64_t(event))       ^
                    uint64_t(i);

                seed ^= (seed >> 33);
                seed *= 0xff51afd7ed558ccdULL;
                seed ^= (seed >> 33);
                seed *= 0xc4ceb9fe1a85ec53ULL;
                seed ^= (seed >> 33);

                std::mt19937_64 gen(seed);

                float resolution = _jer_resolution->evaluate({eta, pt, rho});
                float sf         = _jer_corrector->evaluate({eta, pt, variation});
                float smeared_pt = pt;

                if (genpt > 0)
                {
                    smeared_pt = std::max(0.f, genpt + sf * (pt - genpt));
                }
                else
                {
                    float sigma    = resolution * std::sqrt(std::max(sf*sf - 1.f, 0.f));
                    float gaus_val = gauss(gen);
                    smeared_pt     = pt * (1.f + sigma * gaus_val);
                }

                out.emplace_back(smeared_pt);
            }
            return out;
        };

        // GenJet matching
        _rlm = _rlm.Define("Jet_genJetPt",
            [](const floats& GenJet_pt,
               const ROOT::VecOps::RVec<short>& Jet_genJetIdx)
            {
                floats out;
                out.reserve(Jet_genJetIdx.size());
                for (size_t i = 0; i < Jet_genJetIdx.size(); i++)
                {
                    int idx = Jet_genJetIdx[i];
                    out.emplace_back((idx >= 0 && idx < (int)GenJet_pt.size())
                                     ? GenJet_pt[idx] : -1.f);
                }
                return out;
            },
            {"GenJet_pt", "Jet_genJetIdx"});

        // Shared column list for all JER defines
        const std::vector<std::string> jerCols = {
            "Jet_pt_JEC", "Jet_eta", "Jet_genJetPt",
            "Rho_fixedGridRhoFastjetAll", "run", "luminosityBlock", "event"
        };

        for (const auto& var : std::vector<std::pair<std::string,std::string>>{
                {"nom",  "Jet_pt_corr"},
                {"up",   "Jet_pt_corr_jer_up"},
                {"down", "Jet_pt_corr_jer_down"}})
        {
            const std::string variation = var.first;
            const std::string colName   = var.second;

            _rlm = _rlm.Define(colName,
                [jerSmearLambda, variation](
                    floats pt, floats eta, floats genpt, float rho,
                    unsigned int run_n, unsigned int lumi, unsigned long long event)
                {
                    return jerSmearLambda(pt, eta, genpt, rho, run_n, lumi, event, variation);
                },
                jerCols);
        }

        // ------------------------------------------
        // Step 3: JEC uncertainties
        // ------------------------------------------
        for (const auto& [tag, unc] : _jetCorrectionUnc)
        {
            std::string colBase;
            size_t mc_pos  = tag.find("MC_");
            size_t ak4_pos = tag.find("_AK4");

            if (mc_pos != std::string::npos && ak4_pos != std::string::npos)
                colBase = tag.substr(mc_pos + 3, ak4_pos - (mc_pos + 3));
            else
                colBase = tag;

            std::replace_if(colBase.begin(), colBase.end(),
                [](char c){ return !std::isalnum(c); }, '_');

            const std::string colUp   = "Jet_pt_corr_" + colBase + "_up";
            const std::string colDown = "Jet_pt_corr_" + colBase + "_down";
            auto unc_copy = unc;

            _rlm = _rlm.Define(colUp,
                [unc_copy](floats jetpts, floats jetetas) -> floats {
                    floats out;
                    out.reserve(jetpts.size());
                    for (size_t i = 0; i < jetpts.size(); i++)
                        out.emplace_back(jetpts[i] * (1.f + unc_copy->evaluate({jetetas[i], jetpts[i]})));
                    return out;
                },
                {"Jet_pt_JEC", "Jet_eta"});

            _rlm = _rlm.Define(colDown,
                [unc_copy](floats jetpts, floats jetetas) -> floats {
                    floats out;
                    out.reserve(jetpts.size());
                    for (size_t i = 0; i < jetpts.size(); i++)
                        out.emplace_back(jetpts[i] * (1.f - unc_copy->evaluate({jetetas[i], jetpts[i]})));
                    return out;
                },
                {"Jet_pt_JEC", "Jet_eta"});

            std::cout << "Defined uncertainty columns: "
                      << colUp << ", " << colDown << std::endl;
        }

        // ------------------------------------------
        // Step 4: Propagate all MC variants to MET
        // ------------------------------------------
        // Nominal + JER
        for (const auto& var : std::vector<std::string>{"", "_jer_up", "_jer_down"})
        {
            defineMETVariant(
                "Jet_pt_corr" + var,
                "PuppiMET_pt_corr"  + var,
                "PuppiMET_phi_corr" + var);
        }

        // JEC uncertainty variants
        for (const auto& [tag, unc] : _jetCorrectionUnc)
        {
            std::string colBase;
            size_t mc_pos  = tag.find("MC_");
            size_t ak4_pos = tag.find("_AK4");

            if (mc_pos != std::string::npos && ak4_pos != std::string::npos)
                colBase = tag.substr(mc_pos + 3, ak4_pos - (mc_pos + 3));
            else
                colBase = tag;

            std::replace_if(colBase.begin(), colBase.end(),
                [](char c){ return !std::isalnum(c); }, '_');

            for (const auto& ud : std::vector<std::string>{"_up", "_down"})
            {
                defineMETVariant(
                    "Jet_pt_corr_" + colBase + ud,
                    "PuppiMET_pt_corr_"  + colBase + ud,
                    "PuppiMET_phi_corr_" + colBase + ud);
            }
        }
    } // end MC path
}
// Add to your NanoAODAnalyzerrdframe.cpp
// Include at the top:
// #include "MuonScaRe_RDF.cc"

// Add to your NanoAODAnalyzerrdframe.cpp
// Iclude at the top:
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
    auto smear_corr = _correction_electronss->at("SmearAndSyst");

    auto scale_corr = _correction_electronss->compound().at("Scale");

    // Define supercluster eta once (used by both DATA and MC)

    // =====================================================
    // DATA - Scale corrections
    // =====================================================
    _rlm = _rlm.Define("Electron_eta_supercluster",
                   "Electron_eta + Electron_deltaEtaSC");


    if (_isData) {
        auto scale_corr = _correction_electronss->compound().at("Scale");

        // Create a std::function with explicit signature
        std::function<floats(const floats&, const floats&, const floats&,
                             const ROOT::VecOps::RVec<UChar_t>&, unsigned int, 
                             const std::string&)> scale_lambda =
            [scale_corr](const floats &pt,
                         const floats &scEta,
                         const floats &r9,
                         const ROOT::VecOps::RVec<UChar_t> &seedGain,
                         unsigned int run,
                         const std::string &variation) -> floats
        {
            floats out;
            out.reserve(pt.size());

            for (size_t i = 0; i < pt.size(); ++i) {
                const double pt_v    = pt[i];
                const double scEta_v = scEta[i];
                const double r9_v    = r9[i];

                // Validation checks (fixed the bug from original line 24)
                if (pt_v < 20 ||
                    !std::isfinite(scEta_v) ||
                    !std::isfinite(r9_v) ||
                    std::abs(scEta_v) >= 2.5)
                {
                    out.emplace_back(pt[i]);
                    continue;
                }

                const double scale = scale_corr->evaluate({
                    variation,  // "scale", "scale_up", or "scale_down"
                    static_cast<double>(run),
                    scEta_v,
                    r9_v,
                    pt_v,
                    static_cast<double>(seedGain[i])
                });

                out.emplace_back(pt[i] * scale);
            }
            return out;
        };

        // Define all scale variations with explicit return types
        _rlm = _rlm
            .Define("Electron_pt_corr",
                    [scale_lambda](const floats &pt, const floats &scEta, 
                                  const floats &r9,
                                  const ROOT::VecOps::RVec<UChar_t> &seedGain, 
                                  unsigned int run) -> floats {
                        return scale_lambda(pt, scEta, r9, seedGain, run, "scale");
                    },
                    {"Electron_pt", "Electron_eta_supercluster", "Electron_r9",
                     "Electron_seedGain", "run"})
            .Define("Electron_pt_corr_scaleUp",
                    [scale_lambda](const floats &pt, const floats &scEta, 
                                  const floats &r9,
                                  const ROOT::VecOps::RVec<UChar_t> &seedGain, 
                                  unsigned int run) -> floats {
                        return scale_lambda(pt, scEta, r9, seedGain, run, "scale_up");
                    },
                    {"Electron_pt", "Electron_eta_supercluster", "Electron_r9",
                     "Electron_seedGain", "run"})
            .Define("Electron_pt_corr_scaleDown",
                    [scale_lambda](const floats &pt, const floats &scEta, 
                                  const floats &r9,
                                  const ROOT::VecOps::RVec<UChar_t> &seedGain, 
                                  unsigned int run) -> floats {
                        return scale_lambda(pt, scEta, r9, seedGain, run, "scale_down");
                    },
                    {"Electron_pt", "Electron_eta_supercluster", "Electron_r9",
                     "Electron_seedGain", "run"});
    }

    // =====================================================
    // MC - Smear and Scale corrections
    // =====================================================
    else {
        // For MC: apply Smearing corrections
        auto smear_lambda =
            [smear_corr](const floats &pt,
                    const floats &scEta,
                    const floats &r9,
                    const UInt_t run,
                    const UInt_t lumi,
                    const ULong64_t event)
            -> std::tuple<floats, floats, floats>
            {
                floats nominal, smear_up, smear_down;
                size_t N = pt.size();

                nominal.reserve(N);
                smear_up.reserve(N);
                smear_down.reserve(N);

                std::normal_distribution<float> gauss(0.0, 1.0);

                for (size_t i = 0; i < N; ++i) {

                    /* -----------------------------------------
                       Deterministic seed per electron
                       ----------------------------------------- */
                    uint64_t seed =
                        (uint64_t(run)  << 32) ^
                        (uint64_t(lumi) << 16) ^
                        (uint64_t(event)) ^
                        uint64_t(i);   // electron index

                    std::mt19937 gen(seed);
                    float rand = gauss(gen);

                    try {
                        float smear_val = smear_corr->evaluate({
                                "smear",
                                static_cast<double>(pt[i]),
                                static_cast<double>(r9[i]),
                                static_cast<double>(scEta[i])
                                });

                        float smear_unc_up = smear_corr->evaluate({
                                "smear_up",
                                static_cast<double>(pt[i]),
                                static_cast<double>(r9[i]),
                                static_cast<double>(scEta[i])
                                });

                        float smear_unc_down = smear_corr->evaluate({
                                "smear_down",
                                static_cast<double>(pt[i]),
                                static_cast<double>(r9[i]),
                                static_cast<double>(scEta[i])
                                });

                        nominal.emplace_back(pt[i] * (1.0f + smear_val * rand));
                        smear_up.emplace_back(pt[i] * (1.0f + smear_unc_up * rand));
                        smear_down.emplace_back(pt[i] * (1.0f + smear_unc_down * rand));
                    }
                    catch (const std::exception &e) {
                        std::cerr << "Smearing error at index " << i
                            << ": " << e.what() << std::endl;
                        nominal.emplace_back(pt[i]);
                        smear_up.emplace_back(pt[i]);
                        smear_down.emplace_back(pt[i]);
                    }
                }

                return std::make_tuple(nominal, smear_up, smear_down);
            };


_rlm = _rlm.Define(
            "Electron_pt_corr_triple",
            smear_lambda,
            {
                "Electron_pt",
                "Electron_eta_supercluster",
                "Electron_r9",
                "run",
                "luminosityBlock",
                "event"
            })
        .Define("Electron_pt_corr",
                "std::get<0>(Electron_pt_corr_triple)")
        .Define("Electron_pt_corr_smearUp",
                "std::get<1>(Electron_pt_corr_triple)")
        .Define("Electron_pt_corr_smearDown",
                "std::get<2>(Electron_pt_corr_triple)");

    }
}
/*
void NanoAODAnalyzerrdframe::applyMETPtPhiCorrection()
{
    std::cout << "apply MET Pt and Phi correction" << std::endl;

    if (!_correction_MET_pt_corrector) {
        throw std::runtime_error("MET correction JSON not loaded");
    }

    auto corr = _correction_MET_pt_corrector->at("met_xy_corrections");

    using pairf = std::pair<float, float>;

    // =====================
    // Reusable eval lambda
    // =====================
    auto xyCorrect = [this, corr](float met_pt, float met_phi,
                                   float npvGood,
                                   const std::string& var) -> pairf
    {
        float pt_corr  = corr->evaluate({
            "pt",  "PuppiMET", _year,
            _isData ? "DATA" : "MC",
            var, met_pt, met_phi, npvGood
        });
        float phi_corr = corr->evaluate({
            "phi", "PuppiMET", _year,
            _isData ? "DATA" : "MC",
            var, met_pt, met_phi, npvGood
        });
        return {pt_corr, phi_corr};
    };

    // =====================
    // DATA (nominal only)
    // =====================
    if (_isData)
    {
        _rlm = _rlm
            .Define("MET_pt_phi_corr",
                [xyCorrect](float met_pt, float met_phi, unsigned char npvGood) -> pairf {
                    return xyCorrect(met_pt, met_phi, float(npvGood), "nom");
                },
                {"PuppiMET_pt_corr", "PuppiMET_phi_corr", "PV_npvsGood"})
            .Define("MET_pt_corr",  "MET_pt_phi_corr.first")
            .Define("MET_phi_corr", "MET_pt_phi_corr.second");
    }

    // =====================
    // MC
    // =====================
    else
    {
        // --------------------------------------------------
        // 1. Nominal + PU variations (on nominal Type-1 MET)
        // --------------------------------------------------
        _rlm = _rlm
            .Define("MET_pt_phi_corr_triple",
                [xyCorrect](float met_pt, float met_phi, unsigned char npvGood)
                -> std::tuple<pairf, pairf, pairf>
                {
                    float npv = float(npvGood);
                    return {
                        xyCorrect(met_pt, met_phi, npv, "nom"),
                        xyCorrect(met_pt, met_phi, npv, "pu_up"),
                        xyCorrect(met_pt, met_phi, npv, "pu_dn")
                    };
                },
                {"PuppiMET_pt_corr", "PuppiMET_phi_corr", "PV_npvsGood"})

            // nominal
            .Define("MET_pt_corr",
                    "std::get<0>(MET_pt_phi_corr_triple).first")
            .Define("MET_phi_corr",
                    "std::get<0>(MET_pt_phi_corr_triple).second")

            // PU up  — consistent naming
            .Define("MET_pt_corr_puUp",
                    "std::get<1>(MET_pt_phi_corr_triple).first")
            .Define("MET_phi_corr_puUp",
                    "std::get<1>(MET_pt_phi_corr_triple).second")

            // PU down — consistent naming
            .Define("MET_pt_corr_puDown",
                    "std::get<2>(MET_pt_phi_corr_triple).first")
            .Define("MET_phi_corr_puDown",
                    "std::get<2>(MET_pt_phi_corr_triple).second");

        // --------------------------------------------------
        // 2. JEC/JER systematics — apply XY correction on
        //    each Type-1 corrected MET variation
        // --------------------------------------------------

        // Build list of (inputPtCol, inputPhiCol, outPtCol, outPhiCol)
        std::vector<std::tuple<std::string,std::string,std::string,std::string>> jecVariants =
        {
            // JER
            {"PuppiMET_pt_corr_jer_up",   "PuppiMET_phi_corr_jer_up",
             "MET_pt_corr_jer_up",        "MET_phi_corr_jer_up"},

            {"PuppiMET_pt_corr_jer_down", "PuppiMET_phi_corr_jer_down",
             "MET_pt_corr_jer_down",      "MET_phi_corr_jer_down"},
        };

        // Dynamically add all JEC uncertainty variants
        // (mirrors the same loop in applyJetMETCorrections)
        for (const auto& [tag, unc] : _jetCorrectionUnc)
        {
            std::string colBase;
            size_t mc_pos  = tag.find("MC_");
            size_t ak4_pos = tag.find("_AK4");

            if (mc_pos != std::string::npos && ak4_pos != std::string::npos)
                colBase = tag.substr(mc_pos + 3, ak4_pos - (mc_pos + 3));
            else
                colBase = tag;

            std::replace_if(colBase.begin(), colBase.end(),
                            [](char c){ return !std::isalnum(c); }, '_');

            for (const std::string& dir : {"up", "down"})
            {
                std::string inPt   = "PuppiMET_pt_corr_"  + colBase + "_" + dir;
                std::string inPhi  = "PuppiMET_phi_corr_" + colBase + "_" + dir;
                std::string outPt  = "MET_pt_corr_"       + colBase + "_" + dir;
                std::string outPhi = "MET_phi_corr_"      + colBase + "_" + dir;

                jecVariants.emplace_back(inPt, inPhi, outPt, outPhi);
            }
        }

        // Define XY-corrected MET for every JEC/JER variant
        for (const auto& [inPtCol, inPhiCol, outPtCol, outPhiCol] : jecVariants)
        {
            std::string pairCol = outPtCol + "_pair";

            _rlm = _rlm
                .Define(pairCol,
                    [xyCorrect](float met_pt, float met_phi,
                                unsigned char npvGood) -> pairf {
                        return xyCorrect(met_pt, met_phi, float(npvGood), "nom");
                    },
                    {inPtCol, inPhiCol, "PV_npvsGood"})
                .Define(outPtCol,  pairCol + ".first")
                .Define(outPhiCol, pairCol + ".second");

            std::cout << "Defined XY-corrected MET: "
                      << outPtCol << ", " << outPhiCol << std::endl;
        }
    }
}*/
void NanoAODAnalyzerrdframe::setupCorrections(string goodjsonfname, string pufname, string putag, string btvfname, string btvtype, string fname_btagEff, string hname_btagEff_bcflav, string hname_btagEff_lflav, string muon_roch_fname, string muon_fname, string muonhlttype,string muonidtype,string muonisotype,string electron_fname,string Hlt_fname,string electron_reco_type1,string electron_reco_type2, string electron_id_type, string jercfname, string jerctag,string jerctagMC, vector<string> jercunctag,string jet_veto_f_name,string jet_veto_tag,string electron_SSF,string metpt_fname,string JER_tag,string JER_tag_res)
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


//	  assert(_correction_muon->validate());
	  
	  cout<< "works" << endl;
	  //Electron corrections
	  _correction_electron = correction::CorrectionSet::from_file(electron_fname);

	  cout<< "works" << _muon_iso_type << endl;
	  _electron_reco_type1=electron_reco_type1;
	  _electron_reco_type2=electron_reco_type2;
	  _electron_id_type = electron_id_type;
	  std::cout<< "================================//=================================" << std::endl;
	  cout<< "ELECTRON JSON FILE : " << electron_fname << endl;

      cout<< "ELECTRON RECO type in JSON  : " << _electron_reco_type1 << endl;
	  cout<< "ELECTRONID type in JSON  : " << _electron_id_type << endl;
      assert(_correction_electron->validate());
      assert(_correction_Hlt->validate());
	  //electron scale and smearing correction
 
	  // btag corrections
	  _correction_btag1 = correction::CorrectionSet::from_file(btvfname);
	  cout<< "btv correction filename: " << btvfname << endl;

	  _btvtype = btvtype;
	  assert(_correction_btag1->validate());
/*
	  f_btagEff = new TFile(fname_btagEff.c_str(), "READ");
	  hist_btagEff_bcflav = dynamic_cast<TH2D*>(f_btagEff->Get(hname_btagEff_bcflav.c_str()));
	  hist_btagEff_lflav = dynamic_cast<TH2D*>(f_btagEff->Get(hname_btagEff_lflav.c_str()));
*/

// Replace your existing 3 lines with this comprehensive validation code:
// This version does NOT exit the function on error, allowing other corrections to continue

std::cout << "=== B-Tagging Efficiency Histogram Loading ===" << std::endl;
std::cout << "File path: " << fname_btagEff << std::endl;
std::cout << "bcflav histogram name: " << hname_btagEff_bcflav << std::endl;
std::cout << "lflav histogram name: " << hname_btagEff_lflav << std::endl;

// Open the file
f_btagEff = new TFile(fname_btagEff.c_str(), "READ");

// Check if file opened successfully
if (!f_btagEff) {
    std::cerr << "ERROR: f_btagEff is NULL - file could not be opened!" << std::endl;
    hist_btagEff_bcflav = nullptr;
    hist_btagEff_lflav = nullptr;
    // Don't return - let other corrections continue
}
else if (f_btagEff->IsZombie()) {
    std::cerr << "ERROR: File is a Zombie (corrupt or doesn't exist): " << fname_btagEff << std::endl;
    hist_btagEff_bcflav = nullptr;
    hist_btagEff_lflav = nullptr;
    f_btagEff->Close();
    delete f_btagEff;
    f_btagEff = nullptr;
    // Don't return - let other corrections continue
}
else {
    std::cout << "SUCCESS: File opened successfully" << std::endl;

    // List all objects in the file for debugging
    std::cout << "\n=== Contents of " << fname_btagEff << " ===" << std::endl;
    f_btagEff->ls(); // Simple method to list contents
    std::cout << "==========================================\n" << std::endl;

    // Load bcflav histogram
    std::cout << "Loading bcflav histogram: " << hname_btagEff_bcflav << std::endl;
    hist_btagEff_bcflav = dynamic_cast<TH2D*>(f_btagEff->Get(hname_btagEff_bcflav.c_str()));

    if (!hist_btagEff_bcflav) {
        std::cerr << "ERROR: hist_btagEff_bcflav is NULL!" << std::endl;
        std::cerr << "  Histogram '" << hname_btagEff_bcflav << "' not found in file" << std::endl;
        std::cerr << "  Check histogram name spelling and file contents above" << std::endl;
    } else {
        std::cout << "SUCCESS: hist_btagEff_bcflav loaded" << std::endl;
        std::cout << "  X-axis (eta): " << hist_btagEff_bcflav->GetXaxis()->GetNbins()
                  << " bins from " << hist_btagEff_bcflav->GetXaxis()->GetXmin()
                  << " to " << hist_btagEff_bcflav->GetXaxis()->GetXmax() << std::endl;
        std::cout << "  Y-axis (pt): " << hist_btagEff_bcflav->GetYaxis()->GetNbins()
                  << " bins from " << hist_btagEff_bcflav->GetYaxis()->GetXmin()
                  << " to " << hist_btagEff_bcflav->GetYaxis()->GetXmax() << std::endl;
        std::cout << "  Total entries: " << hist_btagEff_bcflav->GetEntries() << std::endl;

        // Detach from file so it persists
        hist_btagEff_bcflav->SetDirectory(0);
    }

    // Load lflav histogram
    std::cout << "\nLoading lflav histogram: " << hname_btagEff_lflav << std::endl;
    hist_btagEff_lflav = dynamic_cast<TH2D*>(f_btagEff->Get(hname_btagEff_lflav.c_str()));

    if (!hist_btagEff_lflav) {
        std::cerr << "ERROR: hist_btagEff_lflav is NULL!" << std::endl;
        std::cerr << "  Histogram '" << hname_btagEff_lflav << "' not found in file" << std::endl;
        std::cerr << "  Check histogram name spelling and file contents above" << std::endl;
    } else {
        std::cout << "SUCCESS: hist_btagEff_lflav loaded" << std::endl;
        std::cout << "  X-axis (eta): " << hist_btagEff_lflav->GetXaxis()->GetNbins()
                  << " bins from " << hist_btagEff_lflav->GetXaxis()->GetXmin()
                  << " to " << hist_btagEff_lflav->GetXaxis()->GetXmax() << std::endl;
        std::cout << "  Y-axis (pt): " << hist_btagEff_lflav->GetYaxis()->GetNbins()
                  << " bins from " << hist_btagEff_lflav->GetYaxis()->GetXmin()
                  << " to " << hist_btagEff_lflav->GetYaxis()->GetXmax() << std::endl;
        std::cout << "  Total entries: " << hist_btagEff_lflav->GetEntries() << std::endl;

        // Detach from file so it persists
        hist_btagEff_lflav->SetDirectory(0);
    }
}

std::cout << "\n=== Histogram Loading Complete ===" << std::endl;

// Summary
if (hist_btagEff_bcflav && hist_btagEff_lflav) {
    std::cout << "STATUS: Both histograms loaded successfully!" << std::endl;
} else {
    std::cerr << "WARNING: One or both histograms failed to load!" << std::endl;
    std::cerr << "  bcflav: " << (hist_btagEff_bcflav ? "OK" : "FAILED") << std::endl;
    std::cerr << "  lflav: " << (hist_btagEff_lflav ? "OK" : "FAILED") << std::endl;
    std::cerr << "  Continuing with other corrections..." << std::endl;
}
std::cout << "======================================\n" << std::endl;

	  // pile up weights
	  _correction_pu = correction::CorrectionSet::from_file(pufname);
	  cout<< "Pileup correction filename  : " << pufname << endl;

	  assert(_correction_pu->validate());
	  _putag = putag;
	  auto punominal = [this](float x) { return pucorrection(_correction_pu, _putag, "nominal", x); };
	  auto puplus = [this](float x) { return pucorrection(_correction_pu, _putag, "up", x); };
	  auto puminus = [this](float x) { return pucorrection(_correction_pu, _putag, "down", x); };
	  
	  if (!isDefined("puWeight")) _rlm = _rlm.Define("puWeight", punominal, {"Pileup_nTrueInt"});
	  if (!isDefined("puWeight_up")) _rlm = _rlm.Define("puWeight_up", puplus, {"Pileup_nTrueInt"});
	  if (!isDefined("puWeight_down")) _rlm = _rlm.Define("puWeight_down", puminus, {"Pileup_nTrueInt"});
	  
	  
	}
    hltSFFile_ = TFile::Open(Hlt_fname.c_str(), "READ");
    if (!hltSFFile_ || hltSFFile_->IsZombie()) {
        throw std::runtime_error("Cannot open trigger_scale_factors.root");
    }

    hltSFHist_ = dynamic_cast<TH2*>(hltSFFile_->Get("scale_factor"));
    if (!hltSFHist_) {
        throw std::runtime_error("HLT scale_factor histogram not found");
    }

    hltSF_unc_Hist_ = dynamic_cast<TH2*>(hltSFFile_->Get("scale_factor_total"));
    if (!hltSF_unc_Hist_) {
        throw std::runtime_error("HLT scale_factor_total histogram not found");
    }

    // ✅ detach histograms
    hltSFHist_->SetDirectory(nullptr);
    hltSF_unc_Hist_->SetDirectory(nullptr);

    // 🔥 critical fix
    hltSFFile_->Close();
    delete hltSFFile_;
    hltSFFile_ = nullptr;
	_jerctag = jerctag;
    _jerctagMC=jerctagMC;
	_jercunctag = jercunctag;
    _JER_tag = JER_tag;
    _JER_tag_res=JER_tag_res;
	
	setupJetMETCorrection(jercfname, _jerctag,_jerctagMC,_JER_tag,_JER_tag_res);
	applyJetMETCorrections();
	applyMuPtCorrection();
    applyElectronPtCorrection();
    applyGoodJetId();
//     applyMETPtPhiCorrection();

}
HLTSF NanoAODAnalyzerrdframe::getHLTSF(double ele_pt, double mu_pt) const
{
    if (!hltSFHist_) return {1.0, 1.0, 1.0};

    int xbin = hltSFHist_->GetXaxis()->FindBin(ele_pt);
    int ybin = hltSFHist_->GetYaxis()->FindBin(mu_pt);

    // Clamp bins
    xbin = std::max(1, std::min(xbin, hltSFHist_->GetNbinsX()));
    ybin = std::max(1, std::min(ybin, hltSFHist_->GetNbinsY()));

    double central = hltSFHist_->GetBinContent(xbin, ybin);

    // Get uncertainty from the separate histogram (absolute uncertainty)
    double unc = 0.0;
    if (hltSF_unc_Hist_) {
        int xbin_unc = hltSF_unc_Hist_->GetXaxis()->FindBin(ele_pt);
        int ybin_unc = hltSF_unc_Hist_->GetYaxis()->FindBin(mu_pt);
        xbin_unc = std::max(1, std::min(xbin_unc, hltSF_unc_Hist_->GetNbinsX()));
        ybin_unc = std::max(1, std::min(ybin_unc, hltSF_unc_Hist_->GetNbinsY()));
        unc = hltSF_unc_Hist_->GetBinContent(xbin_unc, ybin_unc);
    }

    return {central, central + unc, central - unc};
}
double NanoAODAnalyzerrdframe::getTopPtWeight(
    const ROOT::VecOps::RVec<float>& GenPart_pt,
    const ROOT::VecOps::RVec<int>&   GenPart_pdgId,
    const ROOT::VecOps::RVec<int>&   GenPart_status) const
{
    float pt_top     = -1.0f;
    float pt_antitop = -1.0f;

    for (unsigned int i = 0; i < GenPart_pt.size(); ++i) {
        if (GenPart_status[i] != 62) continue;

        if (GenPart_pdgId[i] ==  6 && pt_top     < 0) pt_top     = GenPart_pt[i];
        if (GenPart_pdgId[i] == -6 && pt_antitop < 0) pt_antitop = GenPart_pt[i];

        // Early exit once both are found
        if (pt_top >= 0 && pt_antitop >= 0) break;
    }

    // Not a ttbar event (or status 62 tops not found) -> no reweighting
    if (pt_top < 0 || pt_antitop < 0) return 1.0;

    // POWHEG SF eq. (3): 0.103*exp(-0.0118*pT) - 0.000134*pT + 0.973
    auto sf_powheg = [](float pt) -> double {
        return 0.103 * std::exp(-0.0118 * pt) - 0.000134 * pt + 0.973;
    };

    // Extrapolation 13 -> 13.6 TeV eq. (4): 0.991 + 0.000075*pT
    auto sf_extrap = [](float pt) -> double {
        return 0.986 + 0.000111 * pt;
    };

    double sf_top     = sf_powheg(pt_top)     * sf_extrap(pt_top);
    double sf_antitop = sf_powheg(pt_antitop) * sf_extrap(pt_antitop);

    return sf_top * sf_antitop;



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
 
     efficiency = hist_btagEff_lflav->GetBinContent(binX, binY);
   }
 
   return efficiency;
 }

ROOT::RDF::RNode NanoAODAnalyzerrdframe::applyTopPtWeight(
    ROOT::RDF::RNode _rlm,
    std::string output_var)
{
    return _rlm.Define(
        output_var,
        [this](const ROOT::VecOps::RVec<float>& pt,
               const ROOT::VecOps::RVec<int>&   pdgId,
               const ROOT::VecOps::RVec<int>&   status) -> double {
            return this->getTopPtWeight(pt, pdgId, status);
        },
        {"GenPart_pt", "GenPart_pdgId", "GenPart_status"}
    );
}

ROOT::RDF::RNode NanoAODAnalyzerrdframe::calculateHLTSF(
    ROOT::RDF::RNode _rlm,
    std::string output_var)
{
    // Define central value
    _rlm = _rlm.Define(
        output_var,
        [this](double electron_pt, double muon_pt) {
            return this->getHLTSF(electron_pt, muon_pt).central;
        },
        {"goodElectrons_leading_pt", "goodmuons_leading_pt"}
    );

    // Define up variation
    _rlm = _rlm.Define(
        output_var + "_up",
        [this](double electron_pt, double muon_pt) {
            return this->getHLTSF(electron_pt, muon_pt).up;
        },
        {"goodElectrons_leading_pt", "goodmuons_leading_pt"}
    );

    // Define down variation
    _rlm = _rlm.Define(
        output_var + "_down",
        [this](double electron_pt, double muon_pt) {
            return this->getHLTSF(electron_pt, muon_pt).down;
        },
        {"goodElectrons_leading_pt", "goodmuons_leading_pt"}
    );

    return _rlm;
}

ROOT::RDF::RNode NanoAODAnalyzerrdframe::calculateBTagSF(
    RNode _rlm,
    std::vector<std::string> Jets_vars_names,
    int _case,
    const double btag_cut,
    std::string _BTaggingWP,
    std::string output_var)
{
    if (_case == 1) {
        
        // Add btag_cut as a constant column so it's properly available
        std::string btag_cut_col = "btag_cut_value_temp";
        _rlm = _rlm.Define(btag_cut_col, [btag_cut]() { return btag_cut; });
        
        // Add WP as a constant column
        std::string wp_col = "btag_wp_temp";
        _rlm = _rlm.Define(wp_col, [_BTaggingWP]() { return _BTaggingWP; });
        
        // Modify Jets_vars_names to include our new columns
        std::vector<std::string> extended_vars = Jets_vars_names;
        extended_vars.push_back(btag_cut_col);
        extended_vars.push_back(wp_col);

        struct BTagWeightCalculator {
            NanoAODAnalyzerrdframe* analyzer;
            std::string var;
            int max_debug;
            
            BTagWeightCalculator(NanoAODAnalyzerrdframe* a, std::string v, int md)
                : analyzer(a), var(v), max_debug(md) {}
            
            float operator()(const ROOT::VecOps::RVec<unsigned char> &hadflav,
                           const ROOT::VecOps::RVec<float> &etas,
                           const ROOT::VecOps::RVec<float> &pts,
                           const ROOT::VecOps::RVec<float> &btag_scores,
                           double btag_cut_val,
                           std::string btag_wp,
                           bool is_bcflav) const
            {
                double btagWeight = 1.0;
                
                static std::atomic<int> counter_bc(0);
                static std::atomic<int> counter_l(0);
                int current_entry = is_bcflav ? counter_bc.fetch_add(1) : counter_l.fetch_add(1);
                bool do_debug = (current_entry < max_debug);
                
                if (do_debug) {
                    std::cout << "\n========== " << (is_bcflav ? "BC" : "L") << "-FLAVOR DEBUG (Entry " 
                              << current_entry << ", Variation: " << var << ") ==========" << std::endl;
                    std::cout << "B-tag cut: " << btag_cut_val << std::endl;
                    std::cout << "Working Point: " << btag_wp << std::endl;
                    std::cout << "Number of jets: " << pts.size() << std::endl;
                }

                for (std::size_t i = 0; i < pts.size(); i++) {
                    
                    bool skip = false;
                    if (is_bcflav) {
                        skip = (std::abs(etas[i]) > 2.4999 || pts[i] < 20.0 || hadflav[i] == 0);
                    } else {
                        skip = (std::abs(etas[i]) > 2.4999 || pts[i] < 20.0 || hadflav[i] != 0);
                    }
                    
                    if (skip) {
                        if (do_debug) {
                            std::cout << "  Jet " << i << ": SKIPPED (eta=" << etas[i] 
                                      << ", pt=" << pts[i] << ", hadflav=" << (int)hadflav[i] << ")" << std::endl;
                        }
                        continue;
                    }

                    double sf;
                    try {
                        if (is_bcflav) {
                            sf = analyzer->_correction_btag1->at("robustParticleTransformer_comb")
                                     ->evaluate({var, btag_wp, hadflav[i],
                                                std::fabs(etas[i]), pts[i]});
                        } else {
                            sf = analyzer->_correction_btag1->at("robustParticleTransformer_light")
                                     ->evaluate({var, btag_wp, hadflav[i],
                                                std::fabs(etas[i]), pts[i]});
                        }
                    } catch (...) {
                        throw;
                    }

                    if (do_debug) {
                        std::cout << "  Jet " << i << ":" << std::endl;
                        std::cout << "    Hadron flavor: " << (int)hadflav[i] << std::endl;
                        std::cout << "    eta: " << etas[i] << ", pt: " << pts[i] << std::endl;
                        std::cout << "    btag_score: " << btag_scores[i] << " (cut: " << btag_cut_val << ")" << std::endl;
                        std::cout << "    SF: " << sf << std::endl;
                    }

                    if (btag_scores[i] >= btag_cut_val) {
                        btagWeight *= sf;
                        if (do_debug) {
                            std::cout << "    TAGGED: weight *= " << sf 
                                      << " -> weight = " << btagWeight << std::endl;
                        }
                    } else {
                        double eff;
                        try {
                            eff = analyzer->getBTaggingEff(hadflav[i], etas[i], pts[i]);

                            if (std::isnan(eff) || eff < 0 || eff > 1)
                                eff = 0.0;

                            if (std::abs(1 - eff) < 1e-10) {
                                if (do_debug) {
                                    std::cout << "    NOT TAGGED: eff = " << eff 
                                              << " (efficiency ~1, skipping)" << std::endl;
                                }
                                continue;
                            }

                            double weight_factor = (1 - sf * eff) / (1 - eff);
                            btagWeight *= weight_factor;
                            
                            if (do_debug) {
                                std::cout << "    NOT TAGGED: eff = " << eff << std::endl;
                                std::cout << "    weight_factor = (1 - " << sf << " * " << eff 
                                          << ") / (1 - " << eff << ") = " << weight_factor << std::endl;
                                std::cout << "    weight = " << btagWeight << std::endl;
                            }

                        } catch (...) {
                            throw;
                        }
                    }
                }
                
                if (do_debug) {
                    std::cout << "  FINAL " << (is_bcflav ? "BC" : "L") << "-FLAVOR WEIGHT: " 
                              << btagWeight << std::endl;
                    std::cout << "========================================\n" << std::endl;
                }

                return btagWeight;
            }
        };

        std::vector<std::string> variations = {
            "central", "up_correlated","up_uncorrelated", "down_uncorrelated","down_correlated"
        };

        for (const std::string &variation : variations) {

            {
                std::string col_bc = output_var + "bcflav_" + variation;
                auto calculator_bc = BTagWeightCalculator(this, variation, 2);

                _rlm = _rlm.Define(
                    col_bc,
                    [calculator_bc](
                        const ROOT::VecOps::RVec<unsigned char> &hadflav,
                        const ROOT::VecOps::RVec<float> &etas,
                        const ROOT::VecOps::RVec<float> &pts,
                        const ROOT::VecOps::RVec<float> &btag_scores,
                        double btag_cut_val,
                        std::string btag_wp) -> float
                    {
                        return calculator_bc(hadflav, etas, pts, btag_scores, btag_cut_val, btag_wp, true);
                    },
                    extended_vars);
            }

            {
                std::string col_l = output_var + "lflav_" + variation;
                auto calculator_l = BTagWeightCalculator(this, variation, 2);

                _rlm = _rlm.Define(
                    col_l,
                    [calculator_l](
                        const ROOT::VecOps::RVec<unsigned char> &hadflav,
                        const ROOT::VecOps::RVec<float> &etas,
                        const ROOT::VecOps::RVec<float> &pts,
                        const ROOT::VecOps::RVec<float> &btag_scores,
                        double btag_cut_val,
                        std::string btag_wp) -> float
                    {
                        return calculator_l(hadflav, etas, pts, btag_scores, btag_cut_val, btag_wp, false);
                    },
                    extended_vars);
            }
        }
    }

    return _rlm;
}

// =====================================================================
    // CASE 3: DeepJet shape correction
    // =====================================================================
/*    else if (_case == 3) {
        
        std::cout << "DEBUG: Entering CASE 3" << std::endl;
        std::cout << "Case 3 Shape correction B-tagging SF for MC\n";
        
        // Define shape-correction lambda
        auto btagweightgenerator3 =
            [this](const ROOT::VecOps::RVec<int> &hadflav,
                   const ROOT::VecOps::RVec<float> &etas,
                   const ROOT::VecOps::RVec<float> &pts,
                   const ROOT::VecOps::RVec<float> &btags) -> float {
                
                std::cout << "DEBUG case3: Input vector sizes - hadflav: " << hadflav.size() 
                         << ", etas: " << etas.size() << ", pts: " << pts.size() 
                         << ", btags: " << btags.size() << std::endl;
                
                // Print first few elements
                int n_print = std::min(3, (int)pts.size());
                for (int idx = 0; idx < n_print; idx++) {
                    std::cout << "DEBUG case3 jet[" << idx << "]: hadflav=" << hadflav[idx]
                             << ", eta=" << etas[idx] << ", pt=" << pts[idx] 
                             << ", btag=" << btags[idx] << std::endl;
                }
                
                double bweight = 1.0;
                
                for (int i = 0; i < (int)pts.size(); i++) {
                    if (std::abs(etas[i]) > 2.5 || pts[i] < 30.000001) {
                        if (i < 3) std::cout << "DEBUG case3 jet[" << i << "]: SKIPPED (eta/pt cut)" << std::endl;
                        continue;
                    }
                    
                    if (i < 3) std::cout << "DEBUG case3: Evaluating correction for jet " << i 
                                        << " with _btvtype = " << _btvtype << std::endl;
                    
                    double w = _correction_btag1->at(_btvtype)
                        ->evaluate({"central",
                                   (int)hadflav[i],
                                   float(std::fabs(etas[i])),
                                   float(pts[i]),
                                   float(btags[i])});
                    
                    if (i < 3) std::cout << "DEBUG case3 jet[" << i << "]: weight = " << w << std::endl;
                    
                    bweight *= w;
                    
                    if (i < 3) std::cout << "DEBUG case3 jet[" << i << "]: Running bweight = " << bweight << std::endl;
                }
                std::cout << "DEBUG case3: FINAL bweight = " << bweight << std::endl;
                return bweight;
            };
        
        std::string col = output_var + "case3";
        
        std::cout << "DEBUG: Defining column: " << col << std::endl;
        
        _rlm = _rlm.Define(col, btagweightgenerator3, Jets_vars_names);
        
        std::cout << "Created column: " << col << std::endl;
        
        std::cout << "DEBUG: CASE 3 complete" << std::endl;
    }
    
    std::cout << "=== calculateBTagSF EXIT ===" << std::endl;
    return _rlm;
}    */
    /*
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

*/
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
    std::vector<std::string> variations = {"nominal", "systup", "systdown","stat"};


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
	column_name += "stat";
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
        for (size_t i = 0; i < pts.size(); i++)
        {

            double w = 1.0;

            if (_year == "2023")
            {
                w = _correction_electron
                    ->at("Electron-ID-SF")
                    ->evaluate({"2023PromptC", variation, eletype,
                            std::fabs(etas[i]), pts[i], phis[i]});
            }
            else if (_year == "2022")
            {
                w = _correction_electron
                    ->at("Electron-ID-SF")
                    ->evaluate({"2022Re-recoBCD", variation, eletype,
                            std::fabs(etas[i]), pts[i]});
            }
            else if (_year == "2022EE")
            {
                w = _correction_electron
                    ->at("Electron-ID-SF")
                    ->evaluate({"2022Re-recoE+PromptFG", variation, eletype,
                            std::fabs(etas[i]), pts[i]});
            }
            else if (_year == "2023BPix")
            {
                w = _correction_electron
                    ->at("Electron-ID-SF")
                    ->evaluate({"2023PromptD", variation, eletype,
                            std::fabs(etas[i]), pts[i], phis[i]});
            }

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
        auto veto_corr = _correction_jetveto->at(_jet_veto_tag);
        std::string veto_type = "jetvetomap";

        for (size_t i = 0; i < etas.size(); ++i) {
            // DEBUG: print suspicious values
            if (std::abs(etas[i]) > 5.2 || std::abs(phis[i]) > M_PI + 0.01) {
                std::cout << "OUT OF RANGE: eta=" << etas[i] 
                    << " phi=" << phis[i] << std::endl;
            }
            if (std::isnan(etas[i]) || std::isnan(phis[i]) ||
                    std::isinf(etas[i]) || std::isinf(phis[i])) {
                std::cout << "NaN/Inf found: eta=" << etas[i] 
                    << " phi=" << phis[i] << std::endl;
            }
        }
        return false; // temporarily disable veto to just see the prints
    };

    // Define a new column with a single boolean per event
    return _rlm.Define(output_var, is_vetoed_event, {eta_var, phi_var});

}

void NanoAODAnalyzerrdframe::applyGoodJetId()
{
    std::cout << "Applying Good Jet ID (year=" << _year << ")..." << std::endl;

    if (_year == "2024") {
        // ── NanoV15: full kinematic recipe ──────────────────────────────────

        auto jet_pass_tight_v15 = [](const ROOT::VecOps::RVec<float>&         etas,
                                     const ROOT::VecOps::RVec<float>&         neHEFs,
                                     const ROOT::VecOps::RVec<float>&         neEmEFs,
                                     const ROOT::VecOps::RVec<float>&         chHEFs,
                                     const ROOT::VecOps::RVec<unsigned char>& chMults,
                                     const ROOT::VecOps::RVec<unsigned char>& neMults)
            -> ROOT::VecOps::RVec<bool> {
            ROOT::VecOps::RVec<bool> result(etas.size(), false);
            for (size_t i = 0; i < etas.size(); ++i) {
                float absEta = std::abs(etas[i]);
                bool  passId = false;
                if (absEta <= 2.6)
                    passId = (neHEFs[i]  < 0.99) &&
                             (neEmEFs[i] < 0.90) &&
                             (static_cast<int>(chMults[i]) + static_cast<int>(neMults[i]) > 1) &&
                             (chHEFs[i]  > 0.01) &&
                             (chMults[i] > 0);
                else if (absEta > 2.6 && absEta <= 2.7)
                    passId = (neHEFs[i]  < 0.90) &&
                             (neEmEFs[i] < 0.99);
                else if (absEta > 2.7 && absEta <= 3.0)
                    passId = (neHEFs[i] < 0.99);
                else if (absEta > 3.0)
                    passId = (static_cast<int>(neMults[i]) >= 2) &&
                             (neEmEFs[i] < 0.4);
                result[i] = passId;
            }
            return result;
        };

        auto jet_pass_tightlv_v15 = [](const ROOT::VecOps::RVec<float>& etas,
                                       const ROOT::VecOps::RVec<bool>&  tightPass,
                                       const ROOT::VecOps::RVec<float>& muEFs,
                                       const ROOT::VecOps::RVec<float>& chEmEFs)
            -> ROOT::VecOps::RVec<bool> {
            ROOT::VecOps::RVec<bool> result(etas.size(), false);
            for (size_t i = 0; i < etas.size(); ++i) {
                if (std::abs(etas[i]) <= 2.7)
                    result[i] = tightPass[i] && (muEFs[i] < 0.8) && (chEmEFs[i] < 0.8);
                else
                    result[i] = tightPass[i];
            }
            return result;
        };

        _rlm = _rlm.Define("Jet_passJetIdTight",
                           jet_pass_tight_v15,
                           {"Jet_eta", "Jet_neHEF", "Jet_neEmEF",
                            "Jet_chHEF", "Jet_chMultiplicity", "Jet_neMultiplicity"});

        _rlm = _rlm.Define("Jet_passJetIdTightLepVeto",
                           jet_pass_tightlv_v15,
                           {"Jet_eta", "Jet_passJetIdTight", "Jet_muEF", "Jet_chEmEF"});

    } else {

        auto jet_pass_tight_v12 = [](const ROOT::VecOps::RVec<float>&        etas,
                                     const ROOT::VecOps::RVec<unsigned char>& jetids,
                                     const ROOT::VecOps::RVec<float>&         neHEFs,
                                     const ROOT::VecOps::RVec<float>&         neEmEFs)
            -> ROOT::VecOps::RVec<bool> {
            ROOT::VecOps::RVec<bool> result(etas.size(), false);
            for (size_t i = 0; i < etas.size(); ++i) {
                float absEta = std::abs(etas[i]);
                bool  passId = false;
                if (absEta <= 2.7)
                    passId = jetids[i] & (1 << 1);
                else if (absEta > 2.7 && absEta <= 3.0)
                    passId = (jetids[i] & (1 << 1)) && (neHEFs[i] < 0.99);
                else if (absEta > 3.0)
                    passId = (jetids[i] & (1 << 1)) && (neEmEFs[i] < 0.4);
                result[i] = passId;
            }
            return result;
        };

        auto jet_pass_tightlv_v12 = [](const ROOT::VecOps::RVec<float>&  etas,
                                       const ROOT::VecOps::RVec<bool>&   tightPass,
                                       const ROOT::VecOps::RVec<float>&  muEFs,
                                       const ROOT::VecOps::RVec<float>&  chEmEFs)
            -> ROOT::VecOps::RVec<bool> {
            ROOT::VecOps::RVec<bool> result(etas.size(), false);
            for (size_t i = 0; i < etas.size(); ++i) {
                if (std::abs(etas[i]) <= 2.7)
                    result[i] = tightPass[i] && (muEFs[i] < 0.8) && (chEmEFs[i] < 0.8);
                else
                    result[i] = tightPass[i];
            }
            return result;
        };

        _rlm = _rlm.Define("Jet_passJetIdTight",
                           jet_pass_tight_v12,
                           {"Jet_eta", "Jet_jetId", "Jet_neHEF", "Jet_neEmEF"});

        _rlm = _rlm.Define("Jet_passJetIdTightLepVeto",
                           jet_pass_tightlv_v12,
                           {"Jet_eta", "Jet_passJetIdTight", "Jet_muEF", "Jet_chEmEF"});
    }
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

void NanoAODAnalyzerrdframe::run(bool saveAll, std::string outtreename){
    std::vector<RNodeTree *> rntends;
    _rnt.getRNodeLeafs(rntends);
    _rnt.Print();

    std::cout << rntends.size() << std::endl;

    for (auto arnt : rntends)
    {
        std::string nodename = arnt->getIndex();
        std::string outname = _outfilename;

        if (rntends.size() > 1)
            outname.replace(outname.find(".root"), 5, "_" + nodename + ".root");

        _outrootfilenames.push_back(outname);

        RNode *arnode = arnt->getRNode();

        std::cout << "-------------------------------------------------------------------" << std::endl;
        std::cout << "cut : " << nodename << std::endl;

        if (saveAll)
        {
            arnode->Snapshot(outtreename, outname);
        }
        else
        {
            std::cout << " --writing branches" << std::endl;
            std::cout << "-------------------------------------------------------------------" << std::endl;

            for (auto bname : _varstostorepertree[nodename])
            {
                std::cout << bname << std::endl;
                std::cout << "-----branch stored" << std::endl;
            }


            arnode->Snapshot(outtreename, outname, _varstostorepertree[nodename]);
        }

        std::cout << "-------------------------------------------------------------------" << std::endl;
        std::cout << "Creating output root file :  " << std::endl;
        std::cout << outname << std::endl;
        std::cout << "-------------------------------------------------------------------" << std::endl;

        _outrootfile = new TFile(outname.c_str(), "UPDATE");

        std::cout << "Writing histograms...   " << std::endl;
        std::cout << "-------------------------------------------------------------------" << std::endl;

        // === 1D histograms ===
        for (auto &h : _th1dhistos)
        {
            auto hist = h.second.GetPtr();
            if (hist != nullptr)
            {
                hist->SetDirectory(0);   // 🔥 critical fix
                hist->Print();
                hist->Write();
            }
        }

        // === 2D histograms ===
        for (auto &h : _th2dhistos)
        {
            auto hist = h.second.GetPtr();
            if (hist != nullptr)
            {
                hist->SetDirectory(0);   // 🔥 critical fix
                hist->Print();
                hist->Write();
            }
        }

        _outrootfile->Write(0, TObject::kOverwrite);
        _outrootfile->Close();

        // 🔥 Proper cleanup
        delete _outrootfile;
        _outrootfile = nullptr;
    }

    std::cout << "-------------------------------------------------------------------" << std::endl;
    std::cout << "END...  :) " << std::endl;
}

void NanoAODAnalyzerrdframe::setParams(string year, string runtype, int datatype)
{
    /*if(debug){
        std::cout<< "================================//=================================" << std::endl;
        std::cout<< "Line : "<< __LINE__ << " Function : " << __FUNCTION__ << std::endl;
        std::cout<< "================================//=================================" << std::endl;
    }*/
	_year=year;
	_runtype=runtype;
	_datatype=datatype;
std::cout << "[DEBUG setParams] year = " << _year
          << ", runtype = " << _runtype
          << ", datatype = " << _datatype << std::endl;
	

	if(_year=="2022") {
        cout << "Analysing through Run 2022" << endl;
    }else if(_year=="2023") {
        cout << "Analysing through Run 2023" << endl;
    }else if(_year=="2024"){
        cout << "Analysing through Run 2024" << endl;
    }else if(_year=="2022EE") {
        cout << "Analysing through Run 2022EE" << endl;
    }else if(_year=="2023BPix") {
        cout << "Analysing through Run 2023BPix" << endl;
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
    else if(_runtype.find("PreBPIX") != std::string::npos){
        _isPostEE = true;
        cout << "PreBPIX  Selected!" << endl;
        std::cout<< "-------------------------------------------------------------------" << std::endl;
    }
    else if(_runtype.find("PostBPIX") != std::string::npos){
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
            if(_year=="2022"){
                HLTGlobalNames=HLT2022Names;
            }else if (_year=="2023"){
                HLTGlobalNames=HLT2023Names;
            }else if(_year=="2022EE"){
                HLTGlobalNames=HLT2024Names;
            }else if(_year=="2023BPix"){
                HLTGlobalNames=HLT2024Names;
            }else if(_year=="2024"){
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
