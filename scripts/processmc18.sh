#!/bin/bash
version="skim_LFV_5_0_0"
skimmed16mc="/data1/common/skimmed_NanoAOD/$version/mc/2016v6"
skimmed17mc="/data1/common/skimmed_NanoAOD/$version/mc/2017v6"
skimmed18mc="/data1/common/skimmed_NanoAOD/$version/mc/2018v6"
skimmedUL16mc="/data1/common/skimmed_NanoAOD/$version/mc/2016UL"
skimmedUL17mc="/data1/common/skimmed_NanoAOD/$version/mc/2017UL"
skimmedUL18mc="/data1/common/skimmed_NanoAOD/$version/mc/2018UL"
sys=norm

#2018
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/DYJetsToLL_M-10to50 DYJetsToLL_M-10to50_18_${sys}.root &> DYJetsToLL_M-10to50_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/DYJetsToLL_M-50-amcatnloFXFX DYJetsToLL_M-50-amcatnloFXFX_18_${sys}.root &> DYJetsToLL_M-50-amcatnloFXFX_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/TTTo2L2Nu TTTo2L2Nu_18_${sys}.root &> TTTo2L2Nu_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/TTToSemiLeptonic/000 TTToSemiLeptonic1_18_${sys}.root &> TTToSemiLeptonic1_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/TTToSemiLeptonic/070 TTToSemiLeptonic2_18_${sys}.root &> TTToSemiLeptonic2_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/TTToSemiLeptonic/140 TTToSemiLeptonic3_18_${sys}.root &> TTToSemiLeptonic3_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/TTToSemiLeptonic/210 TTToSemiLeptonic4_18_${sys}.root &> TTToSemiLeptonic4_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/TTToHadronic TTToHadronic_18_${sys}.root &> TTToHadronic_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-15to20_MuEnrichedPt5 QCD_Pt-15to20_MuEnrichedPt5_18_${sys}.root &> QCD_Pt-15to20_MuEnrichedPt5_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-20to30_MuEnrichedPt5 QCD_Pt-20to30_MuEnrichedPt5_18_${sys}.root &> QCD_Pt-20to30_MuEnrichedPt5_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-30to50_MuEnrichedPt5 QCD_Pt-30to50_MuEnrichedPt5_18_${sys}.root &> QCD_Pt-30to50_MuEnrichedPt5_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-50to80_MuEnrichedPt5 QCD_Pt-50to80_MuEnrichedPt5_18_${sys}.root &> QCD_Pt-50to80_MuEnrichedPt5_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-80to120_MuEnrichedPt5 QCD_Pt-80to120_MuEnrichedPt5_18_${sys}.root &> QCD_Pt-80to120_MuEnrichedPt5_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-120to170_MuEnrichedPt5 QCD_Pt-120to170_MuEnrichedPt5_18_${sys}.root &> QCD_Pt-120to170_MuEnrichedPt5_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-170to300_MuEnrichedPt5 QCD_Pt-170to300_MuEnrichedPt5_18_${sys}.root &> QCD_Pt-170to300_MuEnrichedPt5_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-300to470_MuEnrichedPt5 QCD_Pt-300to470_MuEnrichedPt5_18_${sys}.root &> QCD_Pt-300to470_MuEnrichedPt5_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-470to600_MuEnrichedPt5 QCD_Pt-470to600_MuEnrichedPt5_18_${sys}.root &> QCD_Pt-470to600_MuEnrichedPt5_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-600to800_MuEnrichedPt5 QCD_Pt-600to800_MuEnrichedPt5_18_${sys}.root &> QCD_Pt-600to800_MuEnrichedPt5_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-800to1000_MuEnrichedPt5 QCD_Pt-800to1000_MuEnrichedPt5_18_${sys}.root &> QCD_Pt-800to1000_MuEnrichedPt5_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/QCD_Pt-1000toInf_MuEnrichedPt5 QCD_Pt-1000toInf_MuEnrichedPt5_18_${sys}.root &> QCD_Pt-1000toInf_MuEnrichedPt5_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/ST_t-channel_top ST_t-channel_top_18_${sys}.root &> ST_t-channel_top_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/ST_t-channel_antitop ST_t-channel_antitop_18_${sys}.root &> ST_t-channel_antitop_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/ST_tW_top ST_tW_top_18_${sys}.root &> ST_tW_top_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/ST_tW_antitop ST_tW_antitop_18_${sys}.root &> ST_tW_antitop_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/TTWJetsToLNu TTWJetsToLNu_18_${sys}.root &> TTWJetsToLNu_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/TTWJetsToQQ TTWJetsToQQ_18_${sys}.root &> TTWJetsToQQ_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/TTZToLLNuNu_M-10 TTZToLLNuNu_M-10_18_${sys}.root &> TTZToLLNuNu_M-10_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/TTZToQQ_TuneCP5_13TeV TTZToQQ_18_${sys}.root &> TTZToQQ_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_inclHT100 WJetsToLNu_inclHT100_18_${sys}.root &> WJetsToLNu_inclHT100_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_HT-100To200 WJetsToLNu_HT-100To200_18_${sys}.root &> WJetsToLNu_HT-100To200_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_HT-200To400 WJetsToLNu_HT-200To400_18_${sys}.root &> WJetsToLNu_HT-200To400_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_HT-400To600 WJetsToLNu_HT-400To600_18_${sys}.root &> WJetsToLNu_HT-400To600_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_HT-600To800 WJetsToLNu_HT-600To800_18_${sys}.root &> WJetsToLNu_HT-600To800_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_HT-800To1200 WJetsToLNu_HT-800To1200_18_${sys}.root &> WJetsToLNu_HT-800To1200_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_HT-1200To2500 WJetsToLNu_HT-1200To2500_18_${sys}.root &> WJetsToLNu_HT-1200To2500_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/WJetsToLNu_HT-2500ToInf WJetsToLNu_HT-2500ToInf_18_${sys}.root &> WJetsToLNu_HT-2500ToInf_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/WWTo2L2Nu WWTo2L2Nu_18_${sys}.root &> WWTo2L2Nu_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/WWToLNuQQ WWToLNuQQ_18_${sys}.root &> WWToLNuQQ_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/WZTo2L2Q WZTo2L2Q_18_${sys}.root &> WZTo2L2Q_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/WZTo3LNu WZTo3LNu_18_${sys}.root &> WZTo3LNu_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/ZZTo2L2Q ZZTo2L2Q_18_${sys}.root &> ZZTo2L2Q_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmedUL18mc}/LFV_TT_TToCMuTau_Scalar LFV_TT_TToCMuTau_Scalar_18_${sys}.root &> LFV_TT_TToCMuTau_Scalar_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmedUL18mc}/LFV_TT_TToCMuTau_Vector LFV_TT_TToCMuTau_Vector_18_${sys}.root &> LFV_TT_TToCMuTau_Vector_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmedUL18mc}/LFV_TT_TToCMuTau_Tensor LFV_TT_TToCMuTau_Tensor_18_${sys}.root &> LFV_TT_TToCMuTau_Tensor_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmedUL18mc}/LFV_TT_TToUMuTau_Scalar LFV_TT_TToUMuTau_Scalar_18_${sys}.root &> LFV_TT_TToUMuTau_Scalar_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmedUL18mc}/LFV_TT_TToUMuTau_Vector LFV_TT_TToUMuTau_Vector_18_${sys}.root &> LFV_TT_TToUMuTau_Vector_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmedUL18mc}/LFV_TT_TToUMuTau_Tensor LFV_TT_TToUMuTau_Tensor_18_${sys}.root &> LFV_TT_TToUMuTau_Tensor_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmedUL18mc}/LFV_ST_TCMuTau_Scalar LFV_ST_TCMuTau_Scalar_18_${sys}.root &> LFV_ST_TCMuTau_Scalar_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmedUL18mc}/LFV_ST_TCMuTau_Vector LFV_ST_TCMuTau_Vector_18_${sys}.root &> LFV_ST_TCMuTau_Vector_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmedUL18mc}/LFV_ST_TCMuTau_Tensor LFV_ST_TCMuTau_Tensor_18_${sys}.root &> LFV_ST_TCMuTau_Tensor_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmedUL18mc}/LFV_ST_TUMuTau_Scalar LFV_ST_TUMuTau_Scalar_18_${sys}.root &> LFV_ST_TUMuTau_Scalar_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmedUL18mc}/LFV_ST_TUMuTau_Vector LFV_ST_TUMuTau_Vector_18_${sys}.root &> LFV_ST_TUMuTau_Vector_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmedUL18mc}/LFV_ST_TUMuTau_Tensor LFV_ST_TUMuTau_Tensor_18_${sys}.root &> LFV_ST_TUMuTau_Tensor_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/LFV_TT_To_CMuTau_Scalar LFV_TT_To_CMuTau_Scalar_18_${sys}.root &> LFV_TT_To_CMuTau_Scalar_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/LFV_TT_To_CMuTau_Vector LFV_TT_To_CMuTau_Vector_18_${sys}.root &> LFV_TT_To_CMuTau_Vector_18_${sys}.out &
./processnanoaod.py --allinone -Y 2018 -S ${sys} --globaltag Autumn18_V19 ${skimmed18mc}/LFV_TT_To_CMuTau_Tensor LFV_TT_To_CMuTau_Tensor_18_${sys}.root &> LFV_TT_To_CMuTau_Tensor_18_${sys}.out &
