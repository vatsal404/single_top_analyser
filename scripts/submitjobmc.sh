#!/bin/bash
version="skim_LFV_4_1_0"
skimmed16mc="/data1/common/skimmed_NanoAOD/$version/mc/2016"
skimmed17mc="/data1/common/skimmed_NanoAOD/$version/mc/2017"
skimmed18mc="/data1/common/skimmed_NanoAOD/$version/mc/2018"
skimmed16lfv="/data1/common/skimmed_NanoAOD/$version/lfv/2016"
skimmed17lfv="/data1/common/skimmed_NanoAOD/$version/lfv/2017"
skimmed18lfv="/data1/common/skimmed_NanoAOD/$version/lfv/2018"

#2018
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/DYJetsToLL_M-10to50 DYJetsToLL_M-10to50_18.root &> DYJetsToLL_M-10to50_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/DYJetsToLL_M-50-amcatnloFXFX DYJetsToLL_M-50-amcatnloFXFX_18.root &> DYJetsToLL_M-50-amcatnloFXFX_18.out &

./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/TTTo2L2Nu TTTo2L2Nu_18.root &> TTTo2L2Nu_18.out &
#./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/TTToSemiLeptonic TTToSemiLeptonic_18.root &> TTToSemiLeptonic_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/TTToSemiLeptonic/000 TTToSemiLeptonic1_18.root &> TTToSemiLeptonic1_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/TTToSemiLeptonic/070 TTToSemiLeptonic2_18.root &> TTToSemiLeptonic2_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/TTToSemiLeptonic/140 TTToSemiLeptonic3_18.root &> TTToSemiLeptonic3_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/TTToSemiLeptonic/210 TTToSemiLeptonic4_18.root &> TTToSemiLeptonic4_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/TTToSemiLeptonic/280 TTToSemiLeptonic5_18.root &> TTToSemiLeptonic5_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/TTToSemiLeptonic/350 TTToSemiLeptonic6_18.root &> TTToSemiLeptonic6_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/TTToHadronic TTToHadronic_18.root &> TTToHadronic_18.out &

./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-15to20_MuEnrichedPt5 QCD_Pt-15to20_MuEnrichedPt5_18.root &> QCD_Pt-15to20_MuEnrichedPt5_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-20to30_MuEnrichedPt5 QCD_Pt-20to30_MuEnrichedPt5_18.root &> QCD_Pt-20to30_MuEnrichedPt5_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-30to50_MuEnrichedPt5 QCD_Pt-30to50_MuEnrichedPt5_18.root &> QCD_Pt-30to50_MuEnrichedPt5_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-50to80_MuEnrichedPt5 QCD_Pt-50to80_MuEnrichedPt5_18.root &> QCD_Pt-50to80_MuEnrichedPt5_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-80to120_MuEnrichedPt5 QCD_Pt-80to120_MuEnrichedPt5_18.root &> QCD_Pt-80to120_MuEnrichedPt5_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-120to170_MuEnrichedPt5 QCD_Pt-120to170_MuEnrichedPt5_18.root &> QCD_Pt-120to170_MuEnrichedPt5_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-170to300_MuEnrichedPt5 QCD_Pt-170to300_MuEnrichedPt5_18.root &> QCD_Pt-170to300_MuEnrichedPt5_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-300to470_MuEnrichedPt5 QCD_Pt-300to470_MuEnrichedPt5_18.root &> QCD_Pt-300to470_MuEnrichedPt5_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-470to600_MuEnrichedPt5 QCD_Pt-470to600_MuEnrichedPt5_18.root &> QCD_Pt-470to600_MuEnrichedPt5_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-600to800_MuEnrichedPt5 QCD_Pt-600to800_MuEnrichedPt5_18.root &> QCD_Pt-600to800_MuEnrichedPt5_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-800to1000_MuEnrichedPt5 QCD_Pt-800to1000_MuEnrichedPt5_18.root &> QCD_Pt-800to1000_MuEnrichedPt5_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-1000toInf_MuEnrichedPt5 QCD_Pt-1000toInf_MuEnrichedPt5_18.root &> QCD_Pt-1000toInf_MuEnrichedPt5_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/ST_t-channel_top ST_t-channel_top_18.root &> ST_t-channel_top_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/ST_t-channel_antitop ST_t-channel_antitop_18.root &> ST_t-channel_antitop_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/ST_tW_top ST_tW_top_18.root &> ST_tW_top_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/ST_tW_antitop ST_tW_antitop_18.root &> ST_tW_antitop_18.out &

./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/TTWJetsToLNu TTWJetsToLNu_18.root &> TTWJetsToLNu_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/TTWJetsToQQ TTWJetsToQQ_18.root &> TTWJetsToQQ_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/TTZToLLNuNu_M-10 TTZToLLNuNu_M-10_18.root &> TTZToLLNuNu_M-10_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/TTZToQQ_TuneCP5_13TeV TTZToQQ_18.root &> TTZToQQ_18.out &

#./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu WJetsToLNu_18.root &> WJetsToLNu_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_inclHT100 WJetsToLNu_inclHT100_18.root &> WJetsToLNu_inclHT100_18.out &
#./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_inclHT-0to100 WJetsToLNu_inclHT-0to100_18.root &> WJetsToLNu_inclHT-0to100_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_HT-100To200 WJetsToLNu_HT-100To200_18.root &> WJetsToLNu_HT-100To200_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_HT-200To400 WJetsToLNu_HT-200To400_18.root &> WJetsToLNu_HT-200To400_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_HT-400To600 WJetsToLNu_HT-400To600_18.root &> WJetsToLNu_HT-400To600_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_HT-600To800 WJetsToLNu_HT-600To800_18.root &> WJetsToLNu_HT-600To800_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_HT-800To1200 WJetsToLNu_HT-800To1200_18.root &> WJetsToLNu_HT-800To1200_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_HT-1200To2500 WJetsToLNu_HT-1200To2500_18.root &> WJetsToLNu_HT-1200To2500_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_HT-2500ToInf WJetsToLNu_HT-2500ToInf_18.root &> WJetsToLNu_HT-2500ToInf_18.out &

./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/WWTo2L2Nu WWTo2L2Nu_18.root &> WWTo2L2Nu_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/WWToLNuQQ WWToLNuQQ_18.root &> WWToLNuQQ_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/WZTo2L2Q WZTo2L2Q_18.root &> WZTo2L2Q_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/WZTo3LNu WZTo3LNu_18.root &> WZTo3LNu_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18mc}/ZZTo2L2Q ZZTo2L2Q_18.root &> ZZTo2L2Q_18.out &
sleep 30m
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18lfv}/LFV_TT_TToCMuTau_Scalar LFV_TT_TToCMuTau_Scalar_18.root &> LFV_TT_TToCMuTau_Scalar_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18lfv}/LFV_TT_TToCMuTau_Vector LFV_TT_TToCMuTau_Vector_18.root &> LFV_TT_TToCMuTau_Vector_18.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_V19 ${skimmed18lfv}/LFV_TT_TToCMuTau_Tensor LFV_TT_TToCMuTau_Tensor_18.root &> LFV_TT_TToCMuTau_Tensor_18.out &

# 2017
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/TTTo2L2Nu TTTo2L2Nu_17.root &> TTTo2L2Nu_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/TTToHadronic TTToHadronic_17.root &> TTToHadronic_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/TTToSemiLeptonic TTToSemiLeptonic_17.root &> TTToSemiLeptonic_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_HT-100To200 WJetsToLNu_HT-100To200_17.root &> WJetsToLNu_HT-100To200_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_HT-200To400 WJetsToLNu_HT-200To400_17.root &> WJetsToLNu_HT-200To400_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_HT-400To600 WJetsToLNu_HT-400To600_17.root &> WJetsToLNu_HT-400To600_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_HT-600To800 WJetsToLNu_HT-600To800_17.root &> WJetsToLNu_HT-600To800_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_HT-800To1200 WJetsToLNu_HT-800To1200_17.root &> WJetsToLNu_HT-800To1200_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_HT-1200To2500 WJetsToLNu_HT-1200To2500_17.root &> WJetsToLNu_HT-1200To2500_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_HT-2500ToInf WJetsToLNu_HT-2500ToInf_17.root &> WJetsToLNu_HT-2500ToInf_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_inclHT100 WJetsToLNu_inclHT100_17.root &> WJetsToLNu_inclHT100_17.out &

./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/DYJetsToLL_M-10to50 DYJetsToLL_M-10to50_17.root &> DYJetsToLL_M-10to50_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/DYJetsToLL_M-50-amcatnloFXFX DYJetsToLL_M-50-amcatnloFXFX_17.root &> DYJetsToLL_M-50-amcatnloFXFX_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/ST_t-channel_top ST_t-channel_top_17.root &> ST_t-channel_top_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/ST_t-channel_antitop ST_t-channel_antitop_17.root &> ST_t-channel_antitop_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/ST_tW_top ST_tW_top_17.root &> ST_tW_top_17.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/ST_tW_antitop ST_tW_antitop_17.root &> ST_tW_antitop_17.out &

# 2016
./processnanoaod.py --allinone -Y 2016CP5 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/TTTo2L2Nu TTTo2L2Nu_16.root &> TTTo2L2Nu_16.out &
./processnanoaod.py --allinone -Y 2016CP5 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/TTToHadronic TTToHadronic_16.root &> TTToHadronic_16.out &
./processnanoaod.py --allinone -Y 2016CP5 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/TTToSemiLeptonic TTToSemiLeptonic_16.root &> TTToSemiLeptonic_16.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_HT-100To200 WJetsToLNu_HT-100To200_16.root &> WJetsToLNu_HT-100To200_16.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_HT-200To400 WJetsToLNu_HT-200To400_16.root &> WJetsToLNu_HT-200To400_16.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_HT-400To600 WJetsToLNu_HT-400To600_16.root &> WJetsToLNu_HT-400To600_16.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_HT-600To800 WJetsToLNu_HT-600To800_16.root &> WJetsToLNu_HT-600To800_16.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_HT-800To1200 WJetsToLNu_HT-800To1200_16.root &> WJetsToLNu_HT-800To1200_16.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_HT-1200To2500 WJetsToLNu_HT-1200To2500_16.root &> WJetsToLNu_HT-1200To2500_16.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_HT-2500ToInf WJetsToLNu_HT-2500ToInf_16.root &> WJetsToLNu_HT-2500ToInf_16.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_inclHT100 WJetsToLNu_inclHT100_16.root &> WJetsToLNu_inclHT100_16.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/DYJetsToLL_M-10to50 DYJetsToLL_M-10to50_16.root &> DYJetsToLL_M-10to50_16.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/DYJetsToLL_M-50-amcatnloFXFX DYJetsToLL_M-50-amcatnloFXFX_16.root &> DYJetsToLL_M-50-amcatnloFXFX_16.out &
./processnanoaod.py --allinone -Y 2016CP5 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/ST_t-channel_top ST_t-channel_top_16.root &> ST_t-channel_top_16.out &
./processnanoaod.py --allinone -Y 2016CP5 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/ST_t-channel_antitop ST_t-channel_antitop_16.root &> ST_t-channel_antitop_16.out &
./processnanoaod.py --allinone -Y 2016CP5 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/ST_tW_top ST_tW_top_16.root &> ST_tW_top_16.out &
./processnanoaod.py --allinone -Y 2016CP5 --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/ST_tW_antitop ST_tW_antitop_16.root &> ST_tW_antitop_16.out &
