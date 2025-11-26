#ifndef MUONSCARE_H
#define MUONSCARE_H

#include <cmath>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <TRandom3.h>
#include "correction.h" // from correctionlib

// Forward declaration
extern std::shared_ptr<correction::CorrectionSet> cset;

// Muon pt corrections (scale + resolution)
double pt_resol_var(double pt_woresol, double pt_wresol, double eta, std::string updn);
double pt_scale_var(double pt, double eta, double phi, int charge, std::string updn);
double pt_resol(double pt, double eta, double phi, float nL, int evtNumber, int lumiNumber, double low_pt_threshold);
double pt_scale(bool is_data, double pt, double eta, double phi, int charge, double low_pt_threshold);

#endif // MUONSCARE_H

