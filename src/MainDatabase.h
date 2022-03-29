/* 
 * File:   MainDatabase.h
 * Author: cdozen 
 *
 * Created on May 18, 2018, 2:15 PM
 */

#ifndef MAINDATABASE_h
#define MAINDATABASE_h
#include <algorithm> // without this you will get error in c++11 for remove function
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#ifndef MessageService_H
#endif


using namespace std;

class MainDatabase {
public:
    MainDatabase(string);
    MainDatabase();
    int error(string);
    int error(int);
    int error(double);
    void Trim(string &);
    void checker(string ,string&);
    void checker(string ,int&);
    void checker(string ,bool&);
    void checker(string ,float&);
    bool CheckIfComment(string line);
    bool CheckIfGroup(string line,string& group);
//template <class T> T str2enum(string c, const string a[], const int size);
//template <class T> T str2enum(TString c,std::vector<TString> a);

//---------------------------------------------------------------------------------------------------------------------
// Select Electrons 
//---------------------------------------------------------------------------------------------------------------------
    string Md_CardFileName;
    string Md_electron_cuts,  
           Md_veto_electron_cuts;
    float  Md_Electron_pT,          
           Md_Electron_eta,              
           Md_Electron_deltaEtaSuperCluster; 
 
           
//---------------------------------------------------------------------------------------------------------------------
// Select Muons
//---------------------------------------------------------------------------------------------------------------------
    string Md_muoncuts;
           //Md_vetomuoncuts;
    float Md_Muon_pT,
	      Md_Muon_eta,
          Md_vetoMuon_pT,
          Md_vetoMuon_eta;

//---------------------------------------------------------------------------------------------------------------------
// Select Jets 
//---------------------------------------------------------------------------------------------------------------------
    string Md_jet_cuts;
    float Md_jet_pT,
	  Md_jet_eta;



//---------------------------------------------------------------------------------------------------------------------
// Select HLT trigger
//---------------------------------------------------------------------------------------------------------------------
    string Md_hlt;
            
//---------------------------------------------------------------------------------------------------------------------
// Select cuts
//---------------------------------------------------------------------------------------------------------------------
    string Md_c0,
           Md_c01,
           Md_c02,
           Md_c00,
           Md_c000,
           Md_c0000,
           Md_c00000;
            
 


//---------------------------------------------------------------------------------------------------------------------
// Program options
//---------------------------------------------------------------------------------------------------------------------
string Md_RunOption  ;
bool Md_CalcAcceptance = true;
bool Md_CalcEfficiency = true;
bool Md_doplots        = true;
bool Md_doybins        = true;
bool Md_enable_PV      = true;
bool Md_enable_Fit     = true;
bool Md_enable_SYS     = true;
int  Md_sigma_pT       = 0;
int  Md_sigma_y        = 0;
int  Md_sigma_Vz       = 0;
int  Md_sigma_PV       = 0;
int  Md_sigma_trg      = 0;
string Md_Dataset;
string Md_MCtype;
string Md_Order;
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
    ifstream cardfile;
//---------------------------------------------------------------------------------------------------------------------
private :
      int err = 0, ok = 0;
        string tmp;
        string tmp2;
//---------------------------------------------------------------------------------------------------------------------
};
//end of the class//


#endif /* MAINDATABASE_h */

