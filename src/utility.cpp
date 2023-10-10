/*
 * utility.cpp
 *
 *  Created on: Dec 4, 2018
 *      Author: suyong
 */
#include "utility.h"
#include "TMatrixDSym.h"
#include "TVectorT.h"
#include "Math/SpecFuncMathMore.h"
#include "correction.h"
#include "Math/GenVector/VectorUtil.h"
#include "Math/GenVector/Rotation3D.h"
#include "Math/Math.h"
#include<cmath>


// Utility function to generate fourvector objects for thigs that pass selections

using namespace std;

FourVectorVec generate_4vec(floats &pt, floats &eta, floats &phi, floats &mass)
{
	const int nsize = pt.size();
	FourVectorVec fourvecs;
	fourvecs.reserve(nsize);
	for (auto i=0; i<nsize; i++)
	{
		fourvecs.emplace_back(pt[i], eta[i], phi[i], fabs(mass[i]));
	}

	return fourvecs;
}

floats weightv(floats &x, float evWeight)
{
	const int nsize = x.size();
	floats weightvector(nsize, evWeight);
	return weightvector;
}

floats sphericity(FourVectorVec &p)
{
	TMatrixDSym NormMomTensor(3);

	NormMomTensor = 0.0;
	double p2sum = 0.0;
	for (auto x: p)
	{
		p2sum += x.P2();
		double mom[3] = {x.Px(), x.Py(), x.Pz()};
		for (int irow=0; irow<3; irow++)
		{
			for (int icol=irow; icol<3; icol++)
			{
				NormMomTensor(irow, icol) += mom[irow] * mom[icol];
			}
		}
	}
	NormMomTensor *= (1.0/p2sum);
	TVectorT<double> Qrev;
	NormMomTensor.EigenVectors(Qrev);
	floats Q(3);
	for (auto i=0; i<3; i++) Q[i] = Qrev[2-i];

	return Q;
}


double foxwolframmoment(int l, FourVectorVec &p, int minj, int maxj)
{   // PRD 87, 073014 (2013)
	double answer = 0.0;

	double ptsum=0.0;

	if (maxj==-1) // process everything
	{
		maxj = p.size();
	}
	//for (auto x: p)
	for (auto i=minj; i<maxj; i++)
	{
		auto x = p[i];
		ptsum += x.Pt();
		//for (auto y: p)
		for (auto j=minj; j<maxj; j++)
		{
			auto y = p[j];
			double wij = x.Pt() * y.Pt();
			double cosdOmega = x.Vect().Dot(y.Vect()) / (x.P() * y.P());
			if (cosdOmega>1.0) cosdOmega=1.0;
			if (cosdOmega<-1.0) cosdOmega=-1.0;
			answer += wij * ROOT::Math::legendre(l, cosdOmega);
		}
	}
	answer /= ptsum*ptsum;
	if (fabs(answer)>1.0) std::cout << "FW>1 " << answer << std::endl;
	return answer;
}


// case 1: Evaluate central SFs 
//fixedWP correction with mujets (here medium WP)
// evaluate('systematic', 'working_point', 'flavor', 'abseta', 'pt')
floats btv_case1(std::unique_ptr<correction::CorrectionSet>& cset, std::string type, std::string sys, std::string wp, ints& hadflav, floats& etas, floats& pts)
{
  floats scalefactors_case1;
  const auto nvecs = pts.size();
  //cout << "NVECS======== " << nvecs << endl;
  scalefactors_case1.reserve(nvecs);
  const auto abs_etas = [etas]() {
    floats res;
    res.reserve(etas.size());
    std::transform(etas.begin(), etas.end(), std::back_inserter(res), [](const auto& e) { return std::fabs(e); });
    return res;
  }();
  const auto cast_pts = [pts]() {
    floats res;
    res.reserve(pts.size());
    std::transform(pts.begin(), pts.end(), std::back_inserter(res), [](const auto& p) { return static_cast<float>(p); });
    return res;
  }();
  float bc_jets = 1.0;
  
  for (auto i = 0; i < int(nvecs); i++) {
    //std::cout << "sys: " << sys << ", wp: " << wp << ", hadflav: " << hadflav[i] << ", etas: " << abs_etas[i] << ", pts: " << cast_pts[i] << '\n';
    if(abs_etas[i]<=2.4){
      if (hadflav[i] != 0) {
	bc_jets *= cset->at("deepJet_mujets")->evaluate({sys, wp, hadflav[i], abs_etas[i], cast_pts[i]});
	//scalefactors_case1.emplace_back(bc_jets);
	//std::cout << "\njet SFs from deepJe_mujets at medium WP\n";
	//std::cout << "SF b/c jets : " << bc_jets << '\n';
      } else{ 
	//const auto bc_jets = cset->at("deepJet_incl")->evaluate({sys, wp, hadflav[i], abs_etas[i], cast_pts[i]});
	bc_jets *= cset->at("deepJet_incl")->evaluate({sys, wp, hadflav[i], abs_etas[i], cast_pts[i]});
      }
    }
    scalefactors_case1.emplace_back(bc_jets);
    //std::cout << "\njet SFs from deepJet_incl at medium WP\n";
    //std::cout << "SF light jets : " << bc_jets << '\n';
    //}
    //
    
  }

  return scalefactors_case1;
}

// case 2: Evaluate varied SFs 
//fixedWP correction uncertainty (here tight WP and comb SF)
// evaluate('systematic', 'working_point', 'flavor', 'abseta', 'pt')
floats btv_case2(std::unique_ptr<correction::CorrectionSet>& cset, std::string type, std::string sys, std::string wp, ints& hadflav, floats& etas, floats& pts)
{
  floats scalefactors_case2;
  const auto nvecs = pts.size();
  //cout << "NVECS======== " << nvecs << endl;
  scalefactors_case2.reserve(nvecs);
  const auto abs_etas = [etas]() {
    floats res;
    res.reserve(etas.size());
    std::transform(etas.begin(), etas.end(), std::back_inserter(res), [](const auto& e) { return std::fabs(e); });
    return res;
  }();
  const auto cast_pts = [pts]() {
    floats res;
    res.reserve(pts.size());
    std::transform(pts.begin(), pts.end(), std::back_inserter(res), [](const auto& p) { return static_cast<float>(p); });
    return res;
  }();
  for (auto i = 0; i < int(nvecs); i++) {

    float bweight = 1.0;
     
    if (hadflav[i] != 0) {
      //std::string type = "deepJet_comb" ;
      const auto bc_jets = cset->at("deepJet_comb")->evaluate({sys, wp, hadflav[i], abs_etas[i], cast_pts[i]});
      bweight = bc_jets;
      //std::cout << "\njet SFs up_correlated for comb at tight WP\n";
      //std::cout << "SF b/c : " << bc_jets << '\n';
    } else{ 
      //std::string type = "depJet_incl" ;
      const auto bc_jets = cset->at("deepJet_incl")->evaluate({sys, wp, hadflav[i], abs_etas[i], cast_pts[i]});
      bweight = bc_jets;
      
      //std::cout << "\njet up_correlated for comb at tight  WP\n";
      //std::cout << "SF light jets : " << bc_jets << '\n';
    }
    scalefactors_case2.emplace_back(bweight);
  }

  return scalefactors_case2;

}


floats btvcorrection(std::unique_ptr<correction::CorrectionSet> &cset, std::string type, std::string sys, ints &hadflav,floats &etas,floats &pts,   floats &btags)
{
  floats scalefactors;
  auto nvecs = pts.size();
  scalefactors.reserve(nvecs);
  for (auto i=0; i<int(nvecs); i++)
    {
      //std::cout << "sys: " << sys << ", hadflav: " << hadflav[i] << ", etas: " << fabs(float(etas[i])) << ", pts: " << float(pts[i]) <<" btag discriminator : " << float(btags[i]) << '\n';
      //for 2018UL working points l: 0.0494, m: 0.2770, t: 0.7264
      //if (btags[i]>0.7264){
      float sfi = cset->at(type)->evaluate({sys, int(hadflav[i]), fabs(float(etas[i])), float(pts[i]), float(btags[i])});
      scalefactors.emplace_back(sfi);
      cout<<" jets central scale factors == "<< sfi << endl;
      //}
    }
  return scalefactors;
}


float pucorrection(std::unique_ptr<correction::CorrectionSet> &cset, std::string name, std::string syst, float ntruepileup)
{
	return float(cset->at(name)->evaluate({ntruepileup, syst.c_str()}));
}

ints good_idx(ints good)
{
	ints out;
	for(int i = 0; i < int(good.size()); i++){
		if( good[i] ){
			out.emplace_back(i);
		}
	}
	return out;

}

float calculate_deltaEta( FourVector &p1, FourVector &p2){
	return p1.Eta() - p2.Eta();
}
float calculate_deltaPhi( FourVector &p1, FourVector &p2){
	return ROOT::Math::VectorUtil::DeltaPhi(p1, p2);
}

float calculate_deltaPhi_scalars(double &phi1, double &phi2){
	return ROOT::VecOps::DeltaPhi(phi1, phi2);
}

float calculate_deltaR( FourVector &p1, FourVector &p2){
	return ROOT::Math::VectorUtil::DeltaR(p1, p2);
}
float calculate_invMass( FourVector &p1, FourVector &p2){
	return ROOT::Math::VectorUtil::InvariantMass(p1, p2);
}
FourVector sum_4vec( FourVector &p1, FourVector &p2){
	return p1+p2;
}
//Get indices that sort the object vectors in descending order
floats sort_discriminant( floats discr, floats obj ){
	auto sorted_discr = Reverse(Argsort(discr));
	floats out;
	for (auto idx : sorted_discr){
		out.emplace_back(obj[idx]);
	}
	return out;
}
FourVector select_leadingvec( FourVectorVec &v ){
	FourVector vout;
	if(v.size() > 0) return v[0];
	else return vout;
}

void PrintVector(floats myvector){
	for (size_t i = 0; i < myvector.size(); i++){
		std::cout<<myvector[i]<<"\n";
	}

}


//======================Muons SF===============================================================================//
//muonID_SF
floats muoncorrection(std::unique_ptr<correction::CorrectionSet> &cset, std::string type, std::string year, std::string runtype,floats &etas, floats &pts, std::string sys)
{
  string Data_Type = year + "_" + runtype;
    floats sf_muon;
    auto nvecs = pts.size();
    //cout<<"NVECS====== == "<< nvecs << endl;
    sf_muon.reserve(nvecs);
    
    if (etas.size() != nvecs) {
        throw std::invalid_argument("etas and pts vectors must have the same size!");
    }
    
    for (auto i=0; i<int(nvecs); i++)
    {
        //std::cout << "year: " << year  << ", etas: " << fabs(float(etas[i])) << ", pts: " << float(pts[i]) << " sys : " << sys << '\n';
        
        if (pts[i] < 0 ) {
            throw std::invalid_argument("Invalid value of pT detected!");
        }
        
        if (fabs(float(etas[i])) > 2.5) {
            throw std::invalid_argument("Invalid value of eta detected!");
        }
		//for muons pt<15 
		/*if (pts[i] < 15 || fabs(float(etas[i])) > 2.4) {
        sf_muon.emplace_back(-1);
        continue;
    	}*/
	
        float sfm = cset->at(type)->evaluate({Data_Type, fabs(float(etas[i])), float(pts[i]), sys});
        sf_muon.emplace_back(sfm);

    }
    return sf_muon;
}


//======================Function to check the number of objects with pT greather than x========================//
ints pTcounter(floats vec)
{
	ints output;
	int counter = 0;
	for(auto pt: vec)
	{
		if (pt>10.0) counter++;
	}
	output.emplace_back(counter);
	return output;
}




//=====================build pair example=============================================//
//W reconstruction : 2 jets
//====================================================================================//
floats w_reconstruction (FourVectorVec &jets){
floats out;
float dijetMass;
float dijetMass_out;
float dijetDR;
const float Mass_W = 80.9; //W mass 
const float Width_W = 10.8; //W width
float X_Min = 99999;
float X_Recwidth_W; 

//Loop on all selected jets
	for(unsigned int j1 = 0; j1<jets.size()-1; j1++){
		for(unsigned int j2 = j1+1; j2<jets.size(); j2++){
			 //select 2 jets
			dijetMass = (jets[j1]+jets[j2]).M();
			  //select a best W candidate in min width in a event
			X_Recwidth_W = std::pow((Mass_W-dijetMass)/Width_W,2);
			if (X_Recwidth_W<X_Min){
				dijetDR = ROOT::Math::VectorUtil::DeltaR(jets[j1],jets[j2]);
				X_Min = X_Recwidth_W;
				dijetMass_out = dijetMass;
			}

		}
	}
	out.push_back(dijetMass_out);   //0:w_mass
	out.push_back(dijetDR);  
	return out;
}


floats compute_DR (FourVectorVec &muons, ints goodMuons_charge){
	floats out;
	float mu_ss_DR;
	float mu_os_DR;
	//std::cout<<"Muonsize: " << muons.size()<<std::endl;
	if(muons.size()>0)
	//Loop on all selected muons
	for(unsigned int mu1 = 0; mu1<muons.size()-1; mu1++){
		for(unsigned int mu2 = mu1+1; mu2<muons.size(); mu2++){
			 //select 2 muons with same sign
			 if (goodMuons_charge[mu1]!=goodMuons_charge[mu2]) continue; //check charge of muons
			mu_ss_DR = ROOT::Math::VectorUtil::DeltaR(muons[mu1],muons[mu2]);
			 //select 2 muons with same sign
			if (goodMuons_charge[mu1]==goodMuons_charge[mu2]) continue;
			mu_os_DR = ROOT::Math::VectorUtil::DeltaR(muons[mu1],muons[mu2]);

		}
	}
	out.push_back(mu_ss_DR);		//0: same sign dimuon DR
	out.push_back(mu_os_DR);        //1: opposite sign dimuon dR
	return out;

}



FourVectorVec addVecFourVec(FourVector &A, FourVector &B, FourVector &C) // 
{
	FourVectorVec all;
	all.push_back(A);
	all.push_back(B);
	all.push_back(C);
	return all;
}



FourVector generate_single_4vec(double &pt, double &eta, double &phi, double &mass)
{
	FourVector fourvecs;

	fourvecs.SetPt(pt);
	fourvecs.SetEta(eta);
	fourvecs.SetPhi(phi);
	fourvecs.SetM(fabs(mass));

	return fourvecs;
}


doubles calculateDeltaR_group(FourVectorVec &jets, FourVector &lepton)
{
	std::vector<double> deltaR(jets.size());
	for(auto ajet: jets)
	{
		deltaR.emplace_back(calculate_deltaR(ajet,lepton));
	}

	return deltaR;
}


TLorentzVector generate_TLorentzVector(double &pt, double &eta, double &phi, double &mass) 
{
   TLorentzVector TLorentzVec;
   TLorentzVec.SetPtEtaPhiM(pt, eta, phi, mass);
   return TLorentzVec;
};


/* -------------- W boson Reconstruction for SingleTop t-channel ------------------------*/


float calculateLambda(TLorentzVector &lepton, float met_pt, float met_phi)
{
   float met_px = met_pt*TMath::Cos(met_phi);
   float met_py = met_pt*TMath::Sin(met_phi);
   float mW = 80.4;

   float lambda = (pow(mW,2))/2 + met_px*lepton.Px() + met_py*lepton.Py();
   return lambda;

}

float calculateDelta(TLorentzVector &lepton, float met_pt, float lambda)
{
    // float delta = 4*lambda*lambda*lepton.Pz()*lepton.Pz() - 4*lepton.Pt()*lepton.Pt()*(lepton.E()*lepton.E()*met_pt*met_pt - lambda*lambda);

	float delta1 = pow(lambda*lepton.Pz(),2);
	float delta2 = pow(lepton.Pt(),2)*(pow(lepton.E(),2)*pow(met_pt,2)-pow(lambda,2));

	float delta = 4*(delta1-delta2);

    return delta;
}


float calculate_nu_z(TLorentzVector &lepton, float lambda, float delta, float met_pt, float met_phi)
{
  float p_nu_z = 0.0;
  if(delta>=0)
    {
      float p_nu_z_plus = calculate_nu_z_plus(lepton, lambda, delta);
      float p_nu_z_minus = calculate_nu_z_minus(lepton, lambda, delta);
      
      p_nu_z = p_nu_z_plus < p_nu_z_minus ? p_nu_z_plus : p_nu_z_minus;
      
    }
  
  if(delta<0)
    {
      p_nu_z = calculate_nu_z_complex(lepton, met_pt, met_phi);
      
    }
  return p_nu_z;
}



float calculate_nu_z_plus(TLorentzVector &lepton, float lambda, float delta)
{
   double p_nu_z_plus = (lambda*lepton.Pz())/(pow(lepton.Pt(),2)) + (sqrt(delta))/(2*pow(lepton.Pt(),2));
   return p_nu_z_plus;
}

float calculate_nu_z_minus(TLorentzVector &lepton, float lambda, float delta)
{
   double p_nu_z_minus = (lambda*lepton.Pz())/(pow(lepton.Pt(),2)) - (sqrt(delta))/(2*pow(lepton.Pt(),2));
   return p_nu_z_minus;
}



float calculate_nu_z_complex(TLorentzVector &lepton, float met_pt, float met_phi)
{
  //float met_px; 
  //float met_py;
  float mW = 80.4;
  
  float reco_nu_pt_up = sqrt(2)*abs(mW + lepton.Pt()/sqrt(2));
  float reco_nu_pt_down = sqrt(2)*abs(mW - lepton.Pt()/sqrt(2));
  
  float reco_nu_pt;
  float reco_nu_phi;
  
  if (reco_nu_pt_down < 0)
    {
      reco_nu_pt = reco_nu_pt_up;
    }
  else if (abs(met_pt - reco_nu_pt_down) < abs(met_pt - reco_nu_pt_up))
    {
      reco_nu_pt = reco_nu_pt_down;
    }
  else if (abs(met_pt - reco_nu_pt_down) > abs(met_pt - reco_nu_pt_up))
    {
      reco_nu_pt = reco_nu_pt_up;
    }
  
  float cosThetaLepNu = (mW*mW - reco_nu_pt*reco_nu_pt - lepton.Pt()*lepton.Pt())/(2*lepton.Pt()*reco_nu_pt);
  
  
  if (abs(cosThetaLepNu)>1 && abs(met_pt - reco_nu_pt_down) < abs(met_pt - reco_nu_pt_up) && reco_nu_pt_up > 0 )
    {
      reco_nu_pt = reco_nu_pt_up;
    }
  if (abs(cosThetaLepNu)>1 && abs(met_pt - reco_nu_pt_down) > abs(met_pt - reco_nu_pt_up) && reco_nu_pt_down > 0)
    {
      reco_nu_pt = reco_nu_pt_down;
    }
  
  cosThetaLepNu = (mW*mW - reco_nu_pt*reco_nu_pt - lepton.Pt()*lepton.Pt())/(2*lepton.Pt()*reco_nu_pt);
  
  if(cosThetaLepNu>=0) reco_nu_phi = lepton.Phi() + acos(cosThetaLepNu);
  if(cosThetaLepNu<0) reco_nu_phi = lepton.Phi() + (2*M_PI - acos(cosThetaLepNu));
  if (isnan(reco_nu_phi)) return -999.9;
  
  //met_px = reco_nu_pt*TMath::Cos(reco_nu_phi); 
  //met_py = reco_nu_pt*TMath::Sin(reco_nu_phi);
  
  float lambda = mW*mW/2 + reco_nu_pt*lepton.Pt()*cosThetaLepNu;
  
  float met_pz = lambda*lepton.Pz()/pow(lepton.Pt(),2);
  
  return met_pz;
}


float calculate_nu_energy(float met_pt, float met_phi, float met_pz)
{
   float met_px = met_pt*TMath::Cos(met_phi); 
   float met_py = met_pt*TMath::Sin(met_phi);

   float nu_energy = TMath::Sqrt(met_px*met_px+met_py*met_py+met_pz*met_pz);

   return nu_energy;
}

TLorentzVector get_neutrino_TL4vec(float met_pt, float met_phi, float met_pz, float met_energy)
{
    float met_px = met_pt*TMath::Cos(met_phi); 
    float met_py = met_pt*TMath::Sin(met_phi);

	TLorentzVector neutrino_TL4vec;

	neutrino_TL4vec.SetPxPyPzE(met_px, met_py, met_pz, met_energy);

	return neutrino_TL4vec;
}


TLorentzVector reconstructWboson_TL4vec(TLorentzVector &lepton, TLorentzVector &neutrino)
{
	TLorentzVector Wboson_TL4vec = lepton + neutrino;
	return Wboson_TL4vec;
}
