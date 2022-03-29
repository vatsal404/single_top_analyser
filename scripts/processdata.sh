#!/bin/bash
version=skim_LFVv6
skimmed_data=/data1/common/skimmed_NanoAOD/${version}/data
syst=norm
# 16 pre
./processnanoaod.py --allinone -Y 16 -S ${syst} --globaltag Summer19UL16APV_RunBCD_V7 --json=data/Pileup/JSON/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016Bv1   Run2016Bv1_${syst}.root
./processnanoaod.py --allinone -Y 16 -S ${syst} --globaltag Summer19UL16APV_RunBCD_V7 --json=data/Pileup/JSON/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016Bv2   Run2016Bv2_${syst}.root
./processnanoaod.py --allinone -Y 16 -S ${syst} --globaltag Summer19UL16APV_RunBCD_V7 --json=data/Pileup/JSON/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016C     Run2016C_${syst}.root
./processnanoaod.py --allinone -Y 16 -S ${syst} --globaltag Summer19UL16APV_RunBCD_V7 --json=data/Pileup/JSON/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016D     Run2016D_${syst}.root
./processnanoaod.py --allinone -Y 16 -S ${syst} --globaltag Summer19UL16APV_RunEF_V7 --json=data/Pileup/JSON/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016E      Run2016E_${syst}.root
./processnanoaod.py --allinone -Y 16 -S ${syst} --globaltag Summer19UL16APV_RunEF_V7 --json=data/Pileup/JSON/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016F_HIPM Run2016Fpre_${syst}.root
# 16 post
./processnanoaod.py --allinone -Y 16 -S ${syst} --globaltag Summer19UL16_RunFGH_V7 --json=data/Pileup/JSON/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016F Run2016Fpost_${syst}.root
./processnanoaod.py --allinone -Y 16 -S ${syst} --globaltag Summer19UL16_RunFGH_V7 --json=data/Pileup/JSON/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016G Run2016G_${syst}.root
./processnanoaod.py --allinone -Y 16 -S ${syst} --globaltag Summer19UL16_RunFGH_V7 --json=data/Pileup/JSON/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016H Run2016H_${syst}.root
# 17
./processnanoaod.py --allinone -Y 17 -S ${syst} --globaltag Summer19UL17_RunB_V5 --json=data/Pileup/JSON/Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON.txt ${skimmed_data}/SingleMuon2017B Run2017B_${syst}.root
./processnanoaod.py --allinone -Y 17 -S ${syst} --globaltag Summer19UL17_RunC_V5 --json=data/Pileup/JSON/Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON.txt ${skimmed_data}/SingleMuon2017C Run2017C_${syst}.root
./processnanoaod.py --allinone -Y 17 -S ${syst} --globaltag Summer19UL17_RunD_V5 --json=data/Pileup/JSON/Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON.txt ${skimmed_data}/SingleMuon2017D Run2017D_${syst}.root
./processnanoaod.py --allinone -Y 17 -S ${syst} --globaltag Summer19UL17_RunE_V5 --json=data/Pileup/JSON/Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON.txt ${skimmed_data}/SingleMuon2017E Run2017E_${syst}.root
./processnanoaod.py --allinone -Y 17 -S ${syst} --globaltag Summer19UL17_RunF_V5 --json=data/Pileup/JSON/Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON.txt ${skimmed_data}/SingleMuon2017F Run2017F_${syst}.root
# 18
./processnanoaod.py --allinone -Y 18 -S ${syst} --globaltag Summer19UL18_RunA_V5 --json=data/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2018A Run2018A_${syst}.root
./processnanoaod.py --allinone -Y 18 -S ${syst} --globaltag Summer19UL18_RunB_V5 --json=data/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2018B Run2018B_${syst}.root
./processnanoaod.py --allinone -Y 18 -S ${syst} --globaltag Summer19UL18_RunC_V5 --json=data/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2018C Run2018C_${syst}.root
./processnanoaod.py --allinone -Y 18 -S ${syst} --globaltag Summer19UL18_RunD_V5 --json=data/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2018D Run2018D_${syst}.root
