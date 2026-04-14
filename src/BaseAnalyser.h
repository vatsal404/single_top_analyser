#ifndef BASEANALYSER_H_
#define BASEANALYSER_H_

#include "NanoAODAnalyzerrdframe.h"

enum class SystType {

    // Nominal
    Nominal,

    // Electron systematics
    EleSmearUp,
    EleSmearDown,
    EleScaleUp,
    EleScaleDown,

    // MET PU
    Met_PUUp,
    Met_PUDown,

    // Muon systematics
    muon_scaleup,
    muon_scaledn,
    muon_resoup,
    muon_resodn,

    // ================= JEC systematics ================= //

    // ================= JEC systematics (REGROUPED) ================= //

    // -------- Global (correlated across years) -------- //

    // FlavorQCD
    JEC_Regrouped_FlavorQCDUp,
    JEC_Regrouped_FlavorQCDDown,

    // RelativeBal
    JEC_Regrouped_RelativeBalUp,
    JEC_Regrouped_RelativeBalDown,

    // HF
    JEC_Regrouped_HFUp,
    JEC_Regrouped_HFDown,

    // BBEC1
    JEC_Regrouped_BBEC1Up,
    JEC_Regrouped_BBEC1Down,

    // EC2
    JEC_Regrouped_EC2Up,
    JEC_Regrouped_EC2Down,

    // Absolute
    JEC_Regrouped_AbsoluteUp,
    JEC_Regrouped_AbsoluteDown,

    // -------- Year-dependent (decorrelated) -------- //

    // Absolute_YEAR
    JEC_Regrouped_Absolute_YearUp,
    JEC_Regrouped_Absolute_YearDown,

    // HF_YEAR
    JEC_Regrouped_HF_YearUp,
    JEC_Regrouped_HF_YearDown,

    // EC2_YEAR
    JEC_Regrouped_EC2_YearUp,
    JEC_Regrouped_EC2_YearDown,

    // RelativeSample_YEAR
    JEC_Regrouped_RelativeSample_YearUp,
    JEC_Regrouped_RelativeSample_YearDown,

    // BBEC1_YEAR
    JEC_Regrouped_BBEC1_YearUp,
    JEC_Regrouped_BBEC1_YearDown,
     //JER variation
    JER_Up,
    JER_Down,

};
struct BranchSet {
    std::string ele_pt;
    std::string met_pt;
    std::string met_phi;
    std::string muon_pt;
    std::string jet_pt;

    void setNominal() {
        ele_pt  = "Electron_pt_corr";
        met_pt  = "MET_pt_corr";
        met_phi = "MET_phi_corr";
        muon_pt = "Muon_pt_corr";
        jet_pt = "Jet_pt_corr";
    }
};

class BaseAnalyser: public NanoAODAnalyzerrdframe
{
    public:
        BaseAnalyser(TTree *t,
                     std::string outfilename,
                     float crossection,
                     float sumgenWeight,
                     SystType syst = SystType::Nominal);   // NEW (default)

        float _crossection;
        float _sumgenWeight;

        void defineCuts();
        void defineMoreVars();
        void bookHists();

        // UPDATED signature
        void setTree(TTree *t,
                     std::string outfilename,
                     float crossection,
                     float sumgenWeight,
                     SystType syst = SystType::Nominal);   // NEW

        void setupObjects();
        void setupAnalysis();

        // object selectors
        void selectElectrons();
        void spectatorJets();
        void bdt_variables();
        void selectChannel();
        void reconstructWboson();
        void selectMuons();
        void selectJets();
        void calculateEvWeight();
        void selectMET();
        void plotWBosonMass();
        void removeOverlaps();
        void reconstructTop();
        void defineRegion();

        bool isData() const { return _isData; }
        bool debug = true;
        bool _jsonOK;
        double btag_cut_value;
        std::string _outfilename;

        TFile *_outrootfile;
        std::vector<std::string> _outrootfilenames;

        int _redefine;

    private:
        // ================= Systematics ================= //
        SystType _syst;               // NEW
        void defineSystematics();     // NEW
};

#endif /* BASEANALYSER_H_ */

