/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   MessageService.h
 * Author: zack
 *
 * Created on May 18, 2018, 2:58 PM
 */

#ifndef MESSAGESERVICE_H
#define MESSAGESERVICE_H

// MessageService.h
// A message service functions for any library.
// There are 6 levels: INFO, WARNING, ERROR, EVENT, FILE and EXIT

#include <iostream>
#include <iomanip>
#include <cmath>
#include <typeinfo>
#include <cstring>
#include <cstdlib>
using namespace std;

#define MS_FILE()     m_file(__PRETTY_FUNCTION__, __FILE__)
#define MS_EXIT()     m_exit(__PRETTY_FUNCTION__)
#define MS_INFO(x)    m_info(__PRETTY_FUNCTION__, (x))
#define MS_VALU(x)    m_valu(__PRETTY_FUNCTION__, (x))
#define MS_INFO2(x,y) m_info2(__PRETTY_FUNCTION__, (x), (y))
#define MS_WARNING(x) m_warn(__PRETTY_FUNCTION__, (x))
#define MS_ERROR(x)   m_eror(__PRETTY_FUNCTION__, (x))
#define MS_EVENT(x,y) m_evnt((x), (y))
#define MAX_STR_LEN  30

void m_info(char const * function, char const * msg){
	if(strlen(function)<MAX_STR_LEN){
		cout << left << setw(MAX_STR_LEN) << function << right << setw(10) <<    " INFO  " << msg << endl;
	}
	else{
		for(int i=0; i<MAX_STR_LEN-3; i++) cout << left << function[i];
		cout <<"..."<< right << setw(10) <<    " INFO  " << msg << endl;
	}
	cout << left;
}

void m_info2(char const * function, char const * msg, char const * val){
	if(strlen(function)<MAX_STR_LEN){
		cout << left << setw(MAX_STR_LEN) << function << right << setw(10) <<    " INFO  " << msg << val;
	}
	else{
		for(int i=0; i<MAX_STR_LEN-3; i++) cout << left << function[i];
		cout <<"..."<< right << setw(10) <<    " INFO  "<< left <<setw(30) << msg<<"\t"<<left<<setw(2)<<"="<<left<<setw(50)<< val;
	}
	cout << left;
}

void m_warn(char const * function, char const * msg){
	if(strlen(function)<MAX_STR_LEN){
		cout << left << setw(MAX_STR_LEN) << function << right << setw(10) << " WARNING  " << msg << endl;
	}
	else{
		for(int i=0; i<MAX_STR_LEN-3; i++) cout << left << function[i];
		cout <<"..."<< right << setw(10) <<    " WARNING  " << msg << endl;
	}
	cout << left;
}

void m_eror(char const * function, char const * msg){
	if(strlen(function)<MAX_STR_LEN){
		cout << left << setw(MAX_STR_LEN) << function << right << setw(10) <<   " ERROR  " << msg << endl;
	}
	else{
		for(int i=0; i<MAX_STR_LEN-3; i++) cout << left << function[i];
		cout <<"..."<< right << setw(10) <<    " ERROR  " << msg << endl;
	}
	cout << left;
}

void m_file(char const * function, char const * file){
	if(strlen(function)<MAX_STR_LEN){
		cout << left << setw(MAX_STR_LEN) << function << right << setw(10) << " FILE  " << file     << endl;
		cout << left << setw(MAX_STR_LEN) << function << right << setw(10) << " FILE  " << __DATE__ << endl;
		cout << left << setw(MAX_STR_LEN) << function << right << setw(10) << " FILE  " << __TIME__     << endl;
	}
	else{
		for(int i=0; i<MAX_STR_LEN-3; i++) cout << left << function[i];
		cout <<"..."<< right << setw(10) <<    " FILE  " << file << endl;
		for(int i=0; i<MAX_STR_LEN-3; i++) cout << left << function[i];
		cout <<"..."<< right << setw(10) <<    " FILE  " << __DATE__ << endl;
		for(int i=0; i<MAX_STR_LEN-3; i++) cout << left << function[i];
		cout <<"..."<< right << setw(10) <<    " FILE  " << __TIME__ << endl;
	}
	cout << left;
}

void m_exit(char const * function){
	if(strlen(function)<MAX_STR_LEN){
		cout << left << setw(MAX_STR_LEN) << function << right << setw(10) << " EXIT  " << "Return code is 1."     << endl;
	}
	else{
		for(int i=0; i<MAX_STR_LEN-3; i++) cout << left << function[i];
		cout <<"..."<< right << setw(10) <<    " EXIT  " << "Return code is 1." << endl;
	}
	cout << left;
	exit(1);
}

void m_evnt(int evt, int maxEvent){
	if(evt==1 || evt%(maxEvent/20)==0){
		cout << left << setw(MAX_STR_LEN) << "MessageService" << right << setw(10) <<    " INFO  ";
		cout << "::: So far " << right << setw(10)
			  << evt << " / " << maxEvent
			  << " (" << setw(3) << round(100*double(evt)/double(maxEvent)) << "%)" << left << " event(s) has been processed :::" << endl;
	}
	cout << left;
}

void m_valu(char const * function, double val){
	if(strlen(function)<MAX_STR_LEN){
		cout << left << setw(MAX_STR_LEN) << function << right << setw(10) <<    " INFO  " << val << endl;
	}
	else{
		for(int i=0; i<MAX_STR_LEN-3; i++) cout << left << function[i];
		cout <<"..."<< right << setw(10) <<    " INFO  " << val << endl;
	}
	cout << left;
}



#endif /* MESSAGESERVICE_H */

