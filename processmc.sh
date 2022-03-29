#!/bin/bash
version=skim_LFVv7
mc16pre=/data1/common/skimmed_NanoAOD/$version/mc/16pre
mc16post=/data1/common/skimmed_NanoAOD/$version/mc/16post
mc17=/data1/common/skimmed_NanoAOD/$version/mc/17
mc18=/data1/common/skimmed_NanoAOD/$version/mc/18
sys=norm
target=nov_01
mkdir -p ${target}
# 16pre
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/DYJetsToLL_M-10to50 ${target}/DYJetsToLL_M-10to50_16pre_${sys}.root &> ${target}/DYJetsToLL_M-10to50_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/DYJetsToLL_M-50_amcatnlo ${target}/DYJetsToLL_M-50_amcatnlo_16pre_${sys}.root &> ${target}/DYJetsToLL_M-50_amcatnlo_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/DYJetsToLL_M-50_madgraph ${target}/DYJetsToLL_M-50_madgraph_16pre_${sys}.root &> ${target}/DYJetsToLL_M-50_madgraph_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/ST_t-channel_antitop ${target}/ST_t-channel_antitop_16pre_${sys}.root &> ${target}/ST_t-channel_antitop_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/ST_t-channel_top ${target}/ST_t-channel_top_16pre_${sys}.root &> ${target}/ST_t-channel_top_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/ST_tW_antitop ${target}/ST_tW_antitop_16pre_${sys}.root &> ${target}/ST_tW_antitop_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/ST_tW_top ${target}/ST_tW_top_16pre_${sys}.root &> ${target}/ST_tW_top_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/TTTo2L2Nu ${target}/TTTo2L2Nu_16pre_${sys}.root &> ${target}/TTTo2L2Nu_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/TTToHadronic ${target}/TTToHadronic_16pre_${sys}.root &> ${target}/TTToHadronic_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/TTToSemiLeptonic ${target}/TTToSemiLeptonic_16pre_${sys}.root &> ${target}/TTToSemiLeptonic_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/TTWJetsToLNu ${target}/TTWJetsToLNu_16pre_${sys}.root &> ${target}/TTWJetsToLNu_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/TTWJetsToQQ ${target}/TTWJetsToQQ_16pre_${sys}.root &> ${target}/TTWJetsToQQ_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/TTZToLLNuNu ${target}/TTZToLLNuNu_16pre_${sys}.root &> ${target}/TTZToLLNuNu_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/TTZToQQ ${target}/TTZToQQ_16pre_${sys}.root &> ${target}/TTZToQQ_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/WJetsToLNu_HT-100To200 ${target}/WJetsToLNu_HT-100To200_16pre_${sys}.root &> ${target}/WJetsToLNu_HT-100To200_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/WJetsToLNu_HT-1200To2500 ${target}/WJetsToLNu_HT-1200To2500_16pre_${sys}.root &> ${target}/WJetsToLNu_HT-1200To2500_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/WJetsToLNu_HT-200To400 ${target}/WJetsToLNu_HT-200To400_16pre_${sys}.root &> ${target}/WJetsToLNu_HT-200To400_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/WJetsToLNu_HT-2500ToInf ${target}/WJetsToLNu_HT-2500ToInf_16pre_${sys}.root &> ${target}/WJetsToLNu_HT-2500ToInf_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/WJetsToLNu_HT-400To600 ${target}/WJetsToLNu_HT-400To600_16pre_${sys}.root &> ${target}/WJetsToLNu_HT-400To600_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/WJetsToLNu_HT-600To800 ${target}/WJetsToLNu_HT-600To800_16pre_${sys}.root &> ${target}/WJetsToLNu_HT-600To800_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/WJetsToLNu_HT-800To1200 ${target}/WJetsToLNu_HT-800To1200_16pre_${sys}.root &> ${target}/WJetsToLNu_HT-800To1200_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/WJetsToLNu_inclHT100 ${target}/WJetsToLNu_inclHT100_16pre_${sys}.root &> ${target}/WJetsToLNu_inclHT100_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/WW ${target}/WW_16pre_${sys}.root &> ${target}/WW_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/WZ ${target}/WZ_16pre_${sys}.root &> ${target}/WZ_16pre_${sys}.out &
#./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/ZZ ${target}/ZZ_16pre_${sys}.root &> ${target}/ZZ_16pre_${sys}.out &

# 16post
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/DYJetsToLL_M-10to50 ${target}/DYJetsToLL_M-10to50_16post_${sys}.root &> ${target}/DYJetsToLL_M-10to50_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/DYJetsToLL_M-50_amcatnlo ${target}/DYJetsToLL_M-50_amcatnlo_16post_${sys}.root &> ${target}/DYJetsToLL_M-50_amcatnlo_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/DYJetsToLL_M-50_madgraph ${target}/DYJetsToLL_M-50_madgraph_16post_${sys}.root &> ${target}/DYJetsToLL_M-50_madgraph_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/ST_t-channel_antitop ${target}/ST_t-channel_antitop_16post_${sys}.root &> ${target}/ST_t-channel_antitop_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/ST_t-channel_top ${target}/ST_t-channel_top_16post_${sys}.root &> ${target}/ST_t-channel_top_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/ST_tW_antitop ${target}/ST_tW_antitop_16post_${sys}.root &> ${target}/ST_tW_antitop_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/ST_tW_top ${target}/ST_tW_top_16post_${sys}.root &> ${target}/ST_tW_top_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/TTTo2L2Nu ${target}/TTTo2L2Nu_16post_${sys}.root &> ${target}/TTTo2L2Nu_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/TTToHadronic ${target}/TTToHadronic_16post_${sys}.root &> ${target}/TTToHadronic_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/TTToSemiLeptonic ${target}/TTToSemiLeptonic_16post_${sys}.root &> ${target}/TTToSemiLeptonic_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/TTWJetsToLNu ${target}/TTWJetsToLNu_16post_${sys}.root &> ${target}/TTWJetsToLNu_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/TTWJetsToQQ ${target}/TTWJetsToQQ_16post_${sys}.root &> ${target}/TTWJetsToQQ_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/TTZToLLNuNu ${target}/TTZToLLNuNu_16post_${sys}.root &> ${target}/TTZToLLNuNu_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/TTZToQQ ${target}/TTZToQQ_16post_${sys}.root &> ${target}/TTZToQQ_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/WJetsToLNu_HT-100To200 ${target}/WJetsToLNu_HT-100To200_16post_${sys}.root &> ${target}/WJetsToLNu_HT-100To200_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/WJetsToLNu_HT-1200To2500 ${target}/WJetsToLNu_HT-1200To2500_16post_${sys}.root &> ${target}/WJetsToLNu_HT-1200To2500_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/WJetsToLNu_HT-200To400 ${target}/WJetsToLNu_HT-200To400_16post_${sys}.root &> ${target}/WJetsToLNu_HT-200To400_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/WJetsToLNu_HT-2500ToInf ${target}/WJetsToLNu_HT-2500ToInf_16post_${sys}.root &> ${target}/WJetsToLNu_HT-2500ToInf_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/WJetsToLNu_HT-400To600 ${target}/WJetsToLNu_HT-400To600_16post_${sys}.root &> ${target}/WJetsToLNu_HT-400To600_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/WJetsToLNu_HT-600To800 ${target}/WJetsToLNu_HT-600To800_16post_${sys}.root &> ${target}/WJetsToLNu_HT-600To800_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/WJetsToLNu_HT-800To1200 ${target}/WJetsToLNu_HT-800To1200_16post_${sys}.root &> ${target}/WJetsToLNu_HT-800To1200_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/WJetsToLNu_inclHT100 ${target}/WJetsToLNu_inclHT100_16post_${sys}.root &> ${target}/WJetsToLNu_inclHT100_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/WW ${target}/WW_16post_${sys}.root &> ${target}/WW_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/WZ ${target}/WZ_16post_${sys}.root &> ${target}/WZ_16post_${sys}.out &
#./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/ZZ ${target}/ZZ_16post_${sys}.root &> ${target}/ZZ_16post_${sys}.out &

# 17
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/DYJetsToLL_M-10to50 ${target}/DYJetsToLL_M-10to50_17_${sys}.root &> ${target}/DYJetsToLL_M-10to50_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/DYJetsToLL_M-50_amcatnlo ${target}/DYJetsToLL_M-50_amcatnlo_17_${sys}.root &> ${target}/DYJetsToLL_M-50_amcatnlo_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/DYJetsToLL_M-50_madgraph ${target}/DYJetsToLL_M-50_madgraph_17_${sys}.root &> ${target}/DYJetsToLL_M-50_madgraph_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/ST_t-channel_antitop ${target}/ST_t-channel_antitop_17_${sys}.root &> ${target}/ST_t-channel_antitop_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/ST_t-channel_top ${target}/ST_t-channel_top_17_${sys}.root &> ${target}/ST_t-channel_top_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/ST_tW_antitop ${target}/ST_tW_antitop_17_${sys}.root &> ${target}/ST_tW_antitop_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/ST_tW_top ${target}/ST_tW_top_17_${sys}.root &> ${target}/ST_tW_top_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/TTTo2L2Nu ${target}/TTTo2L2Nu_17_${sys}.root &> ${target}/TTTo2L2Nu_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/TTToHadronic ${target}/TTToHadronic_17_${sys}.root &> ${target}/TTToHadronic_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/TTToSemiLeptonic ${target}/TTToSemiLeptonic_17_${sys}.root &> ${target}/TTToSemiLeptonic_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/TTWJetsToLNu ${target}/TTWJetsToLNu_17_${sys}.root &> ${target}/TTWJetsToLNu_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/TTWJetsToQQ ${target}/TTWJetsToQQ_17_${sys}.root &> ${target}/TTWJetsToQQ_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/TTZToLLNuNu ${target}/TTZToLLNuNu_17_${sys}.root &> ${target}/TTZToLLNuNu_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/TTZToQQ ${target}/TTZToQQ_17_${sys}.root &> ${target}/TTZToQQ_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/WJetsToLNu_HT-100To200 ${target}/WJetsToLNu_HT-100To200_17_${sys}.root &> ${target}/WJetsToLNu_HT-100To200_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/WJetsToLNu_HT-1200To2500 ${target}/WJetsToLNu_HT-1200To2500_17_${sys}.root &> ${target}/WJetsToLNu_HT-1200To2500_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/WJetsToLNu_HT-200To400 ${target}/WJetsToLNu_HT-200To400_17_${sys}.root &> ${target}/WJetsToLNu_HT-200To400_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/WJetsToLNu_HT-2500ToInf ${target}/WJetsToLNu_HT-2500ToInf_17_${sys}.root &> ${target}/WJetsToLNu_HT-2500ToInf_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/WJetsToLNu_HT-400To600 ${target}/WJetsToLNu_HT-400To600_17_${sys}.root &> ${target}/WJetsToLNu_HT-400To600_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/WJetsToLNu_HT-600To800 ${target}/WJetsToLNu_HT-600To800_17_${sys}.root &> ${target}/WJetsToLNu_HT-600To800_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/WJetsToLNu_HT-800To1200 ${target}/WJetsToLNu_HT-800To1200_17_${sys}.root &> ${target}/WJetsToLNu_HT-800To1200_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/WJetsToLNu_inclHT100 ${target}/WJetsToLNu_inclHT100_17_${sys}.root &> ${target}/WJetsToLNu_inclHT100_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/WW ${target}/WW_17_${sys}.root &> ${target}/WW_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/WZ ${target}/WZ_17_${sys}.root &> ${target}/WZ_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/ZZ ${target}/ZZ_17_${sys}.root &> ${target}/ZZ_17_${sys}.out &

# 18
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/DYJetsToLL_M-10to50 ${target}/DYJetsToLL_M-10to50_18_${sys}.root &> ${target}/DYJetsToLL_M-10to50_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/DYJetsToLL_M-50_amcatnlo ${target}/DYJetsToLL_M-50_amcatnlo_18_${sys}.root &> ${target}/DYJetsToLL_M-50_amcatnlo_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/DYJetsToLL_M-50_madgraph ${target}/DYJetsToLL_M-50_madgraph_18_${sys}.root &> ${target}/DYJetsToLL_M-50_madgraph_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/ST_t-channel_antitop ${target}/ST_t-channel_antitop_18_${sys}.root &> ${target}/ST_t-channel_antitop_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/ST_t-channel_top ${target}/ST_t-channel_top_18_${sys}.root &> ${target}/ST_t-channel_top_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/ST_tW_antitop ${target}/ST_tW_antitop_18_${sys}.root &> ${target}/ST_tW_antitop_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/ST_tW_top ${target}/ST_tW_top_18_${sys}.root &> ${target}/ST_tW_top_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/TTTo2L2Nu ${target}/TTTo2L2Nu_18_${sys}.root &> ${target}/TTTo2L2Nu_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/TTToHadronic ${target}/TTToHadronic_18_${sys}.root &> ${target}/TTToHadronic_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/TTToSemiLeptonic ${target}/TTToSemiLeptonic_18_${sys}.root &> ${target}/TTToSemiLeptonic_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/TTWJetsToLNu ${target}/TTWJetsToLNu_18_${sys}.root &> ${target}/TTWJetsToLNu_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/TTWJetsToQQ ${target}/TTWJetsToQQ_18_${sys}.root &> ${target}/TTWJetsToQQ_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/TTZToLLNuNu ${target}/TTZToLLNuNu_18_${sys}.root &> ${target}/TTZToLLNuNu_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/TTZToQQ ${target}/TTZToQQ_18_${sys}.root &> ${target}/TTZToQQ_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/WJetsToLNu_HT-100To200 ${target}/WJetsToLNu_HT-100To200_18_${sys}.root &> ${target}/WJetsToLNu_HT-100To200_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/WJetsToLNu_HT-1200To2500 ${target}/WJetsToLNu_HT-1200To2500_18_${sys}.root &> ${target}/WJetsToLNu_HT-1200To2500_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/WJetsToLNu_HT-200To400 ${target}/WJetsToLNu_HT-200To400_18_${sys}.root &> ${target}/WJetsToLNu_HT-200To400_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/WJetsToLNu_HT-2500ToInf ${target}/WJetsToLNu_HT-2500ToInf_18_${sys}.root &> ${target}/WJetsToLNu_HT-2500ToInf_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/WJetsToLNu_HT-400To600 ${target}/WJetsToLNu_HT-400To600_18_${sys}.root &> ${target}/WJetsToLNu_HT-400To600_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/WJetsToLNu_HT-600To800 ${target}/WJetsToLNu_HT-600To800_18_${sys}.root &> ${target}/WJetsToLNu_HT-600To800_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/WJetsToLNu_HT-800To1200 ${target}/WJetsToLNu_HT-800To1200_18_${sys}.root &> ${target}/WJetsToLNu_HT-800To1200_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/WJetsToLNu_inclHT100 ${target}/WJetsToLNu_inclHT100_18_${sys}.root &> ${target}/WJetsToLNu_inclHT100_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/WW ${target}/WW_18_${sys}.root &> ${target}/WW_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/WZ ${target}/WZ_18_${sys}.root &> ${target}/WZ_18_${sys}.out &
#./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/ZZ ${target}/ZZ_18_${sys}.root &> ${target}/ZZ_18_${sys}.out &
