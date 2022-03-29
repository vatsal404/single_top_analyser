#!/bin/bash
version="skim_LFVv6"
mc16pre="/data1/common/skimmed_NanoAOD/$version/mc/16pre"
mc16post="/data1/common/skimmed_NanoAOD/$version/mc/16post"
mc17="/data1/common/skimmed_NanoAOD/$version/mc/17"
mc18="/data1/common/skimmed_NanoAOD/$version/mc/18"
sys=norm
./processnanoaod.py --allinone -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/QCD_Pt-15to20 QCD_Pt-15to20_18_${sys}.root &> QCD_Pt-15to20_18_${sys}.out &
