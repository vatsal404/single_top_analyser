#!/bin/bash
version="skim_LFV_5_0_0"
skimmed16mc="/data1/common/skimmed_NanoAOD/$version/mc/2016v6"
skimmed17mc="/data1/common/skimmed_NanoAOD/$version/mc/2017v6"
skimmed18mc="/data1/common/skimmed_NanoAOD/$version/mc/2018v6"
skimmedUL16mc="/data1/common/skimmed_NanoAOD/$version/mc/2016UL"
skimmedUL17mc="/data1/common/skimmed_NanoAOD/$version/mc/2017UL"
skimmedUL18mc="/data1/common/skimmed_NanoAOD/$version/mc/2018UL"
sys=norm

#2017
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/TTTo2L2Nu TTTo2L2Nu_17_${sys}.root &> TTTo2L2Nu_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/TTToHadronic TTToHadronic_17_${sys}.root &> TTToHadronic_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/TTToSemiLeptonic/000 TTToSemiLeptonic1_17_${sys}.root &> TTToSemiLeptonic1_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/TTToSemiLeptonic/063 TTToSemiLeptonic2_17_${sys}.root &> TTToSemiLeptonic2_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/TTToSemiLeptonic/126 TTToSemiLeptonic3_17_${sys}.root &> TTToSemiLeptonic3_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/QCD_Pt-15to20_MuEnrichedPt5 QCD_Pt-15to20_MuEnrichedPt5_17_${sys}.root &> QCD_Pt-15to20_MuEnrichedPt5_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/QCD_Pt-20to30_MuEnrichedPt5 QCD_Pt-20to30_MuEnrichedPt5_17_${sys}.root &> QCD_Pt-20to30_MuEnrichedPt5_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/QCD_Pt-30to50_MuEnrichedPt5 QCD_Pt-30to50_MuEnrichedPt5_17_${sys}.root &> QCD_Pt-30to50_MuEnrichedPt5_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/QCD_Pt-50to80_MuEnrichedPt5 QCD_Pt-50to80_MuEnrichedPt5_17_${sys}.root &> QCD_Pt-50to80_MuEnrichedPt5_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/QCD_Pt-80to120_MuEnrichedPt5 QCD_Pt-80to120_MuEnrichedPt5_17_${sys}.root &> QCD_Pt-80to120_MuEnrichedPt5_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/QCD_Pt-120to170_MuEnrichedPt5 QCD_Pt-120to170_MuEnrichedPt5_17_${sys}.root &> QCD_Pt-120to170_MuEnrichedPt5_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/QCD_Pt-170to300_MuEnrichedPt5 QCD_Pt-170to300_MuEnrichedPt5_17_${sys}.root &> QCD_Pt-170to300_MuEnrichedPt5_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/QCD_Pt-300to470_MuEnrichedPt5 QCD_Pt-300to470_MuEnrichedPt5_17_${sys}.root &> QCD_Pt-300to470_MuEnrichedPt5_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/QCD_Pt-470to600_MuEnrichedPt5 QCD_Pt-470to600_MuEnrichedPt5_17_${sys}.root &> QCD_Pt-470to600_MuEnrichedPt5_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/QCD_Pt-600to800_MuEnrichedPt5 QCD_Pt-600to800_MuEnrichedPt5_17_${sys}.root &> QCD_Pt-600to800_MuEnrichedPt5_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/QCD_Pt-800to1000_MuEnrichedPt5 QCD_Pt-800to1000_MuEnrichedPt5_17_${sys}.root &> QCD_Pt-800to1000_MuEnrichedPt5_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/QCD_Pt-1000toInf_MuEnrichedPt5 QCD_Pt-1000toInf_MuEnrichedPt5_17_${sys}.root &> QCD_Pt-1000toInf_MuEnrichedPt5_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_HT-100To200 WJetsToLNu_HT-100To200_17_${sys}.root &> WJetsToLNu_HT-100To200_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_HT-200To400 WJetsToLNu_HT-200To400_17_${sys}.root &> WJetsToLNu_HT-200To400_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_HT-400To600 WJetsToLNu_HT-400To600_17_${sys}.root &> WJetsToLNu_HT-400To600_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_HT-600To800 WJetsToLNu_HT-600To800_17_${sys}.root &> WJetsToLNu_HT-600To800_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_HT-800To1200 WJetsToLNu_HT-800To1200_17_${sys}.root &> WJetsToLNu_HT-800To1200_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_HT-1200To2500 WJetsToLNu_HT-1200To2500_17_${sys}.root &> WJetsToLNu_HT-1200To2500_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_HT-2500ToInf WJetsToLNu_HT-2500ToInf_17_${sys}.root &> WJetsToLNu_HT-2500ToInf_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WJetsToLNu_inclHT100 WJetsToLNu_inclHT100_17_${sys}.root &> WJetsToLNu_inclHT100_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/DYJetsToLL_M-10to50 DYJetsToLL_M-10to50_17_${sys}.root &> DYJetsToLL_M-10to50_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/DYJetsToLL_M-50-amcatnloFXFX DYJetsToLL_M-50-amcatnloFXFX_17_${sys}.root &> DYJetsToLL_M-50-amcatnloFXFX_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/ST_t-channel_top ST_t-channel_top_17_${sys}.root &> ST_t-channel_top_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/ST_t-channel_antitop ST_t-channel_antitop_17_${sys}.root &> ST_t-channel_antitop_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/ST_tW_top ST_tW_top_17_${sys}.root &> ST_tW_top_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/ST_tW_antitop ST_tW_antitop_17_${sys}.root &> ST_tW_antitop_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/TTWJetsToLNu TTWJetsToLNu_17_${sys}.root &> TTWJetsToLNu_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/TTWJetsToQQ TTWJetsToQQ_17_${sys}.root &> TTWJetsToQQ_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/TTZToLLNuNu_M-10 TTZToLLNuNu_M-10_17_${sys}.root &> TTZToLLNuNu_M-10_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/TTZToQQ TTZToQQ_17_${sys}.root &> TTZToQQ_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WWTo2L2Nu WWTo2L2Nu_17_${sys}.root &> WWTo2L2Nu_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WWToLNuQQ WWToLNuQQ_17_${sys}.root &> WWToLNuQQ_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WZTo2L2Q WZTo2L2Q_17_${sys}.root &> WZTo2L2Q_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/WZTo3LNu WZTo3LNu_17_${sys}.root &> WZTo3LNu_17_${sys}.out &
./processnanoaod.py --allinone -Y 2017 -S ${sys} --globaltag Fall17_17Nov2017_V32 ${skimmed17mc}/ZZTo2L2Q ZZTo2L2Q_17_${sys}.root &> ZZTo2L2Q_17_${sys}.out &

#2016
./processnanoaod.py --allinone -Y 2016CP5 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/TTTo2L2Nu TTTo2L2Nu_16_${sys}.root &> TTTo2L2Nu_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016CP5 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/TTToHadronic TTToHadronic_16_${sys}.root &> TTToHadronic_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016CP5 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/TTToSemiLeptonic TTToSemiLeptonic_16_${sys}.root &> TTToSemiLeptonic_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/QCD_Pt-15to20_MuEnrichedPt5 QCD_Pt-15to20_MuEnrichedPt5_16_${sys}.root &> QCD_Pt-15to20_MuEnrichedPt5_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/QCD_Pt-20to30_MuEnrichedPt5 QCD_Pt-20to30_MuEnrichedPt5_16_${sys}.root &> QCD_Pt-20to30_MuEnrichedPt5_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/QCD_Pt-30to50_MuEnrichedPt5 QCD_Pt-30to50_MuEnrichedPt5_16_${sys}.root &> QCD_Pt-30to50_MuEnrichedPt5_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/QCD_Pt-50to80_MuEnrichedPt5 QCD_Pt-50to80_MuEnrichedPt5_16_${sys}.root &> QCD_Pt-50to80_MuEnrichedPt5_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/QCD_Pt-80to120_MuEnrichedPt5 QCD_Pt-80to120_MuEnrichedPt5_16_${sys}.root &> QCD_Pt-80to120_MuEnrichedPt5_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/QCD_Pt-120to170_MuEnrichedPt5 QCD_Pt-120to170_MuEnrichedPt5_16_${sys}.root &> QCD_Pt-120to170_MuEnrichedPt5_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/QCD_Pt-170to300_MuEnrichedPt5 QCD_Pt-170to300_MuEnrichedPt5_16_${sys}.root &> QCD_Pt-170to300_MuEnrichedPt5_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/QCD_Pt-300to470_MuEnrichedPt5 QCD_Pt-300to470_MuEnrichedPt5_16_${sys}.root &> QCD_Pt-300to470_MuEnrichedPt5_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/QCD_Pt-470to600_MuEnrichedPt5 QCD_Pt-470to600_MuEnrichedPt5_16_${sys}.root &> QCD_Pt-470to600_MuEnrichedPt5_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/QCD_Pt-600to800_MuEnrichedPt5 QCD_Pt-600to800_MuEnrichedPt5_16_${sys}.root &> QCD_Pt-600to800_MuEnrichedPt5_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/QCD_Pt-800to1000_MuEnrichedPt5 QCD_Pt-800to1000_MuEnrichedPt5_16_${sys}.root &> QCD_Pt-800to1000_MuEnrichedPt5_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/QCD_Pt-1000toInf_MuEnrichedPt5 QCD_Pt-1000toInf_MuEnrichedPt5_16_${sys}.root &> QCD_Pt-1000toInf_MuEnrichedPt5_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_HT-100To200 WJetsToLNu_HT-100To200_16_${sys}.root &> WJetsToLNu_HT-100To200_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_HT-200To400 WJetsToLNu_HT-200To400_16_${sys}.root &> WJetsToLNu_HT-200To400_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_HT-400To600 WJetsToLNu_HT-400To600_16_${sys}.root &> WJetsToLNu_HT-400To600_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_HT-600To800 WJetsToLNu_HT-600To800_16_${sys}.root &> WJetsToLNu_HT-600To800_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_HT-800To1200 WJetsToLNu_HT-800To1200_16_${sys}.root &> WJetsToLNu_HT-800To1200_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_HT-1200To2500 WJetsToLNu_HT-1200To2500_16_${sys}.root &> WJetsToLNu_HT-1200To2500_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_HT-2500ToInf WJetsToLNu_HT-2500ToInf_16_${sys}.root &> WJetsToLNu_HT-2500ToInf_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WJetsToLNu_inclHT100 WJetsToLNu_inclHT100_16_${sys}.root &> WJetsToLNu_inclHT100_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/DYJetsToLL_M-10to50 DYJetsToLL_M-10to50_16_${sys}.root &> DYJetsToLL_M-10to50_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/DYJetsToLL_M-50-amcatnloFXFX DYJetsToLL_M-50-amcatnloFXFX_16_${sys}.root &> DYJetsToLL_M-50-amcatnloFXFX_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/TTWJetsToLNu TTWJetsToLNu_16_${sys}.root &> TTWJetsToLNu_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/TTWJetsToQQ TTWJetsToQQ_16_${sys}.root &> TTWJetsToQQ_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/TTZToLLNuNu_M-10 TTZToLLNuNu_M-10_16_${sys}.root &> TTZToLLNuNu_M-10_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/TTZToQQ TTZToQQ_16_${sys}.root &> TTZToQQ_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016CP5 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/ST_t-channel_top ST_t-channel_top_16_${sys}.root &> ST_t-channel_top_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016CP5 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/ST_t-channel_antitop ST_t-channel_antitop_16_${sys}.root &> ST_t-channel_antitop_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016CP5 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/ST_tW_top ST_tW_top_16_${sys}.root &> ST_tW_top_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016CP5 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/ST_tW_antitop ST_tW_antitop_16_${sys}.root &> ST_tW_antitop_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WWTo2L2Nu WWTo2L2Nu_16_${sys}.root &> WWTo2L2Nu_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WWToLNuQQ WWToLNuQQ_16_${sys}.root &> WWToLNuQQ_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WZTo2L2Q WZTo2L2Q_16_${sys}.root &> WZTo2L2Q_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/WZTo3LNu WZTo3LNu_16_${sys}.root &> WZTo3LNu_16_${sys}.out &
./processnanoaod.py --allinone -Y 2016 -S ${sys} --globaltag Summer16_07Aug2017_V11 ${skimmed16mc}/ZZTo2L2Q ZZTo2L2Q_16_${sys}.root &> ZZTo2L2Q_16_${sys}.out &
