#!/bin/bash
version="skim_LFVv6"
skimmed_data="/data1/common/skimmed_NanoAOD/${version}/data"
skimmed_16premc="/data1/common/skimmed_NanoAOD/${version}/mc/16pre"
skimmed_16postmc="/data1/common/skimmed_NanoAOD/${version}/mc/16post"
skimmed_17mc="/data1/common/skimmed_NanoAOD/${version}/mc/17"
skimmed_18mc="/data1/common/skimmed_NanoAOD/${version}/mc/18"

#2018
syst=norm
#./processnanoaod.py --allinone -Y 16pre -S ${syst} --globaltag Summer19UL16APV_RunBCD_V7 --json=data/Pileup/JSON/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016Bv1 Run2016Bv1_${syst}.root &> Run2016Bv1_${syst}.out &
./processnanoaod.py --allinone -Y 16pre -S ${syst} --globaltag Summer19UL16APV_RunBCD_V7 --json=data/Pileup/JSON/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016Bv2 Run2016Bv2_${syst}.root &> Run2016Bv2_${syst}.out &
#./processnanoaod.py --allinone -Y 18 -S ${syst} --globaltag Summer19UL18_V5 ${skimmed_18mc}/DYJetsToLL_M-10to50 DYJetsToLL_M-10to50_18_${syst}.root &> DYJetsToLL_M-10to50_18_${syst}.out &
