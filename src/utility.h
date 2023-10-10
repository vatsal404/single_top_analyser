/*
 * utility.h
 *
 *  Created on: Dec 4, 2018
 *      Author: suyong
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "Math/Vector4D.h"
#include "correction.h"
#include <string>
#include <TLorentzVector.h>

using floats =  ROOT::VecOps::RVec<float>;
using doubles =  ROOT::VecOps::RVec<double>;
using ints =  ROOT::VecOps::RVec<int>;
using bools = ROOT::VecOps::RVec<bool>;
using uchars = ROOT::VecOps::RVec<unsigned char>;

using FourVector = ROOT::Math::PtEtaPhiMVector;
using FourVectorVec = std::vector<FourVector>;
using FourVectorRVec = ROOT::VecOps::RVec<FourVector>;

ints pTcounter(floats vec);

struct hist1dinfo
{
	ROOT::RDF::TH1DModel hmodel;
	std::string varname;
	std::string weightname;
	std::string mincutstep;
} ;

//for 2D histograms
struct hist2dinfo
{
  ROOT::RDF::TH2DModel hmodel;
  std::string varname1;
  std::string varname2;
  std::string weightname;
  std::string mincutstep;
} ;



struct varinfo
{
	std::string varname;
	std::string vardefinition;
	std::string mincutstep;
};

struct cutinfo
{
	std::string cutdefinition;
	std::string idx;
};


// generates vectors of 4 vectors given vectors of pt, eta, phi, mass
FourVectorVec generate_4vec(floats &pt, floats &eta, floats &phi, floats &mass);

// return a vector size equal to length of x all filled with evWeight value
floats weightv(floats &x, float evWeight);

floats sphericity(FourVectorVec &p);

double foxwolframmoment(int l, FourVectorVec &p, int minj=0, int maxj=-1);

float pucorrection(std::unique_ptr<correction::CorrectionSet> &cset, std::string name, std::string syst, float ntruepileup);

ints good_idx(ints good); 
float calculate_deltaEta( FourVector &p1, FourVector &p2);
float calculate_deltaPhi( FourVector &p1, FourVector &p2);
float calculate_deltaPhi_scalars(double &phi1, double &phi2);
float calculate_deltaR( FourVector &p1, FourVector &p2);
float calculate_invMass( FourVector &p1, FourVector &p2);
FourVector sum_4vec( FourVector &p1, FourVector &p2);
floats sort_discriminant( floats discr, floats obj );
FourVector select_leadingvec( FourVectorVec &v );
void PrintVector(floats myvector);
floats w_reconstruction (FourVectorVec &jets);
floats compute_DR (FourVectorVec &muons, ints goodMuons_charge);
float calculate_MT( FourVectorVec &muons, float met, float metphi);

FourVectorVec addVecFourVec(FourVector &A, FourVector &B, FourVector &C); // 
FourVector generate_single_4vec(double &pt, double &eta, double &phi, double &mass);
doubles calculateDeltaR_group(FourVectorVec &jets, FourVector &lepton);
TLorentzVector generate_TLorentzVector(double &pt, double &eta, double &phi, double &mass) ;

//floats btvcorrection(std::unique_ptr<correction::CorrectionSet> &cset, std::string name, std::string syst, floats &pts, floats &etas, ints &hadflav, floats &btags);


/*=====================================BJets SF============================================*/
floats btvcorrection(std::unique_ptr<correction::CorrectionSet> &cset, std::string name, std::string syst, ints &hadflav, floats &etas,floats &pts,  floats &btags);

floats btv_case1(std::unique_ptr<correction::CorrectionSet> &cset, std::string type, std::string sys,std::string wp, ints &hadflav, floats &etas, floats &pts );
floats btv_case2(std::unique_ptr<correction::CorrectionSet> &cset, std::string type, std::string sys,std::string wp, ints &hadflav, floats &etas, floats &pts );

floats btv_casetest(std::unique_ptr<correction::CorrectionSet>& cset, std::string type1, std::string sys, std::string wp, ints& hadflav, floats& etas, floats& pts);


/*=====================================Muons SF============================================*/
//floats muoncorrection(std::unique_ptr<correction::CorrectionSet> &cset, std::string type, std::string year, floats &etas, floats &pts, std::string sys);
floats muoncorrection(std::unique_ptr<correction::CorrectionSet> &cset, std::string type, std::string year, std::string runtype, floats &etas, floats &pts, std::string sys);

/* --------------- neutrino Reconstruction for SingleTop t-channel ------------------------*/

float calculateLambda(TLorentzVector &lepton, float met_pt, float met_phi);
float calculateDelta(TLorentzVector &lepton, float met_pt, float lambda);

// Reco Pz of neutrino
float calculate_nu_z(TLorentzVector &lepton, float lambda, float delta, float met_pt, float met_phi);


// Real Solutions
float calculate_nu_z_plus(TLorentzVector &lepton, float lambda, float delta);
float calculate_nu_z_minus(TLorentzVector &lepton, float lambda, float delta);

// Complex Solutions
float calculate_nu_z_complex(TLorentzVector &lepton, float met_pt, float met_phi);


float calculate_nu_energy(float met_pt, float met_phi, float met_pz);
TLorentzVector get_neutrino_TL4vec(float met_pt, float met_phi, float met_pz, float met_energy);

/* -------------- W boson Reconstruction for SingleTop t-channel ------------------------*/

TLorentzVector reconstructWboson_TL4vec(TLorentzVector &lepton, TLorentzVector &neutrino);


#endif /* UTILITY_H_ */
