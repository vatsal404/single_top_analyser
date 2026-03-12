#ifndef BASEANALYSER_H_
#define BASEANALYSER_H_

#include "NanoAODAnalyzerrdframe.h"

// ================= Systematic enum ================= //
enum class SystType {
    Nominal,
    EleSmearUp,
    EleSmearDown,
    EleScaleUp,
    EleScaleDown,
    met_PUUp,
    met_PUDown,
    muon_scaleup,
    muon_scaledn,
    muon_resoup,
    muon_resodn
};
struct BranchSet {
    std::string ele_pt;
    std::string met_pt;
    std::string met_phi;
    std::string muon_pt;

    void setNominal() {
        ele_pt  = "Electron_pt_corr";
        met_pt  = "PuppiMET_pt_corr";
        met_phi = "PuppiMET_phi_corr";
        muon_pt = "Muon_pt_corr";
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

