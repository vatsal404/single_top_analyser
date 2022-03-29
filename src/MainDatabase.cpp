#include "MainDatabase.h"
#include "MessageService.h"
///
// * * * MEMBERS Of the class * * *
//---------------------------------------------------------------------------------------------------------------------
// Constructor

MainDatabase::MainDatabase(string file) {

    Md_CardFileName     = file;
//-------------------------------------------------------------
// * * Electron cards * *
//-------------------------------------------------------------
    Md_electron_cuts          	= "-1";
    Md_veto_electron_cuts      	= "-1";
    Md_Electron_pT 		= -9999;
    Md_Electron_eta 		= -9999;
    Md_Electron_deltaEtaSuperCluster = -9999;


//-------------------------------------------------------------
//* * Muon cards * *
//-------------------------------------------------------------

    Md_muoncuts         = "-1";
    //Md_vetomuoncuts     = "-1";
    Md_Muon_pT 		= -9999;
    Md_Muon_eta 		= -9999;
    Md_muoncuts         = "-1";
    Md_vetoMuon_pT = -9999;
    Md_vetoMuon_eta = -9999;


//-------------------------------------------------------------
//* * Jet cards * *
//-------------------------------------------------------------
    Md_jet_cuts          = "-1";
    Md_jet_pT 		= -9999;
    Md_jet_eta 		= -9999;


//-------------------------------------------------------------
//* * b-Jet cards * *
//-------------------------------------------------------------



//-------------------------------------------------------------
//* * base cut cards * *
//-------------------------------------------------------------
    Md_c0          	    = "-1";
    Md_c01          	= "-1";
    Md_c02          	= "-1";
    Md_c00          	= "-1";
    Md_c000          	= "-1";
    Md_c0000          	= "-1";
    Md_c00000          	= "-1";
    Md_hlt          	= "-1";

    Md_Dataset          = "-1";
    Md_MCtype           = "-1";
    Md_Order            = "-1"; 
        err = 0, ok = 0;

    MS_INFO("==============================================");
    cardfile.open(Md_CardFileName.c_str());
    if (cardfile.is_open() == false) {
        MS_ERROR("Cannot open cardfile");
        MS_ERROR(Md_CardFileName.c_str());
        MS_INFO("==============================================");
        MS_EXIT();
    }else{ok++;}

    MS_INFO("Md_CardFileName cardfile");
    MS_INFO(Md_CardFileName.c_str());
    while (true) {
        if (cardfile.eof() == true) break;
        cardfile >> tmp;
        if(CheckIfComment(tmp)) continue;

//-------------------------------------------------------------
// * * Electron checker * *
//-------------------------------------------------------------
        checker("CardFileName",Md_CardFileName);
        checker("electron_cuts",Md_electron_cuts);
        checker("veto_electron_cuts",Md_veto_electron_cuts);
        checker("Electron_pT",Md_Electron_pT);
        checker("Electron_eta", Md_Electron_eta);
        checker("Electron_deltaEtaSuperCluster", Md_Electron_deltaEtaSuperCluster);

//-------------------------------------------------------------
// * * Muon checker * *
//-------------------------------------------------------------
        checker("muoncuts",Md_muoncuts);
        //checker("vetomuoncuts",Md_vetomuoncuts);
        checker("vetoMuon_pT",Md_vetoMuon_pT);
        checker("vetoMuon_eta",Md_vetoMuon_eta);
        checker("Muon_pT",Md_Muon_pT);
        checker("Muon_eta", Md_Muon_eta);




//-------------------------------------------------------------
// * * Jet  checker * *
//-------------------------------------------------------------
	checker("jet_cuts", Md_jet_cuts);
	checker("jet_pT", Md_jet_pT);
	checker("jet_eta", Md_jet_eta);
//-------------------------------------------------------------
// * * b-Jet  checker * *
//-------------------------------------------------------------

//-------------------------------------------------------------
// * * base cuts  checker * *
//-------------------------------------------------------------
        checker("c0",Md_c0);
        checker("c01",Md_c01);
        checker("c02",Md_c02);
        checker("c00",Md_c00);
        checker("c000",Md_c000);
        checker("c0000",Md_c0000);
        checker("c00000",Md_c00000);


//-------------------------------------------------------------
// * * HLT  cards * *
//-------------------------------------------------------------

	checker("hlt",Md_hlt);


//-------------------------------------------------------------
// * * Data/MC  cards * *
//-------------------------------------------------------------

	checker("Dataset",Md_Dataset);
        checker("MCtype",Md_MCtype);
        checker("Order",Md_Order);

    }
    cardfile.close();
    MS_INFO("Cardfile is closed.");
    MS_INFO("==============================================");
    if (err) {
        MS_ERROR("One or more variables are not set properly");
        MS_EXIT();
    }
    if (ok == 0) {
        MS_ERROR("Input data file (name/path) error");
        MS_EXIT();
    }
}

//---------------------------------------------------------------------------------------------------------------------
// Control the string and set the value ///
// ss: tex dosyasindan okunan 
// 2. degisken e bak type a gore kaydet mesela: &MyStringVariable - bu  variable icine kaydecegim
//---------------------------------------------------------------------------------------------------------------------
void MainDatabase::checker(string ss, string &MyStringVariable){
        if (tmp == ss) {
            cardfile >> tmp2;
            cardfile >> tmp2;
            MyStringVariable = tmp2;
            Trim(MyStringVariable);
            MS_INFO2(ss.c_str(), tmp2.c_str());
            err += error(MyStringVariable);
            ok++;
        }

}
void MainDatabase::checker(string ss, int &MyintVariable){
        if (tmp == ss) {
            cardfile >> tmp2;
            cardfile >> tmp2;
            MyintVariable = atoi(tmp2.c_str()); //atoi: string to int
            MS_INFO2(ss.c_str(), tmp2.c_str());
            err += error(MyintVariable);
        }

}

void MainDatabase::checker(string ss, bool &MyboolVariable){
        if (tmp == ss) {
            cardfile >> tmp2;
            cardfile >> tmp2;
            MyboolVariable =(bool) atoi(tmp2.c_str()); //atoi: string to bool(0 ya da 1)
            MS_INFO2(ss.c_str(), tmp2.c_str());
            err += error(MyboolVariable);
        }

}
void MainDatabase::checker(string ss, float &MyfloatVariable){
        if (tmp == ss) {
            cardfile >> tmp2;
            cardfile >> tmp2;
            MyfloatVariable = atof(tmp2.c_str()); //atof: string to float
            MS_INFO2(ss.c_str(), tmp2.c_str());
            err += error(MyfloatVariable);
        }

}
//---------------------------------------------------------------------------------------------------------------------
// removes some characters from the string.
// arda sacma karakterler ya da bosluk varsa trim ediyor. sac sakal duzeltme=)
//---------------------------------------------------------------------------------------------------------------------

void MainDatabase::Trim(string &str) {
    char chars[] = "\"";
    for (unsigned int i = 0; i < strlen(chars); ++i) {
        str.erase(std::remove(str.begin(), str.end(), chars[i]), str.end());
    }
}
//---------------------------------------------------------------------------------------------------------------------
// overloaded error functions
//---------------------------------------------------------------------------------------------------------------------

int MainDatabase::error(string s) {
    if (s == "-1") {
        cout << " [*** This variable was not set properly ***]\a" << endl;
        return 1;
    } else {
        cout << " [OK]" << endl;
        return 0;
    }
}

int MainDatabase::error(int s) {
    if (s == -9999) {
        cout << " [*** This variable was not set properly ***]\a" << endl;
        return 1;
    } else {
        cout << " [OK]" << endl;
        return 0;
    }
}

int MainDatabase::error(double s) {
    if (s == -1 || s == 0) {
        cout << " [*** This variable was not set properly ***]\a" << endl;
        return 1;
    } else {
        cout << " [OK]" << endl;
        return 0;
    }
}

//--------------------------------------------------------------------------------------------------------------
// Check comments
// eger comments satiri varsa bunu atla
//--------------------------------------------------------------------------------------------------------------

bool MainDatabase::CheckIfComment(string line){
    return ( line[0] == '#' );
}

//--------------------------------------------------------------------------------------------------------------
//String to Enum
//liste fonksiyonlari **** ayri bir olay*** important- lazim olursa bakariz
//--------------------------------------------------------------------------------------------------------------
//template <class T> T MainDatabase::str2enum(string c, const string a[], const int size) {
//    for (int i=0; i < size; i++) {
//        if (c == a[i]) {
//            return static_cast<T>(i);
//        }
//    }
//}
//--------------------------------------------------------------------------------------------------------------
//String to Enum
//liste fonksiyonlari **** ayri bir olay*** important- lazim olursa bakariz
//--------------------------------------------------------------------------------------------------------------
//template <class T> T MainDatabase::str2enum(TString c, std::vector<TString> a) {
//    for (int i=0; i < a.size(); i++) {
//        if (c == a[i]) {
//            return static_cast<T>(i);
//        }
//    }
//}

//
