/*
 * BaseAnalyser.h
 *
 *  Created on: 14 December, 2021
 *      Author: cdozen 
 */

#ifndef BASEANALYSER_H_
#define BASEANALYSER_H_

#include "NanoAODAnalyzerrdframe.h"

class BaseAnalyser: public NanoAODAnalyzerrdframe
{
    public:
        BaseAnalyser(TTree *t, std::string outfilename, std::string year="",std::string runtype="",std::string syst="",  std::string jsonfname="", string globaltag="", int nthreads=1,int isDATA=-1 );
        void defineCuts();
        void defineMoreVars();
        void bookHists();
        void setTree(TTree *t, std::string outfilename);
        void setupAnalysis();
        void selectElectrons();
        void selectMuons();
        void selectJets();
        void removeOverlaps();
        void selectFatJets();
        void calculateEvWeight();
        bool debug = true;

        private:
            std::string year;
            std::string runtype;
            std::string syst;

};



#endif /* BASEANALYSER_H_ */
