#ifndef BASEANALYSER_H_
#define BASEANALYSER_H_

#include "NanoAODAnalyzerrdframe.h"

// ================= Systematic enum ================= //
enum class SystType {
    Nominal,
    EleSmearUp,
    EleSmearDown
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

