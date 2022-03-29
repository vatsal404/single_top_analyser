#!/bin/bash
version="skim_LFVv6"
mcdir19ul="/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODAPVv2/"
mcdir20ul="/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/"
tgdir="/data1/common/skimmed_NanoAOD/$version/mc/16pre"
cd ../
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir20ul}/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8 ${tgdir}/TTTo2L2Nu &> TTTo2L2Nu_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir20ul}/TTToHadronic_TuneCP5_13TeV-powheg-pythia8 ${tgdir}/TTToHadronic &> TTToHadronic_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir20ul}/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8 ${tgdir}/TTToSemiLeptonic &> TTToSemiLeptonic_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir20ul}/DYJetsToLL_M-10to50_TuneCP5_13TeV-madgraphMLM-pythia8 ${tgdir}/DYJetsToLL_M-10to50 &> DYJetsToLL_M-10to50_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8 ${tgdir}/DYJetsToLL_M-50 &> DYJetsToLL_M-50_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir20ul}/WJetsToLNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8 ${tgdir}/WJetsToLNu_HT-100To200 &> WJetsToLNu_HT-100To200_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir20ul}/WJetsToLNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8 ${tgdir}/WJetsToLNu_HT-200To400 &> WJetsToLNu_HT-200To400_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir20ul}/WJetsToLNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8 ${tgdir}/WJetsToLNu_HT-400To600 &> WJetsToLNu_HT-400To600_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir20ul}/WJetsToLNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8 ${tgdir}/WJetsToLNu_HT-600To800 &> WJetsToLNu_HT-600To800_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir20ul}/WJetsToLNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8 ${tgdir}/WJetsToLNu_HT-800To1200 &> WJetsToLNu_HT-800To1200_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/QCD_Pt-1000toInf_MuEnrichedPt5_TuneCP5_13TeV_pythia8 ${tgdir}/QCD_Pt-1000toInf_MuEnrichedPt5 &> QCD_Pt-1000toInf_MuEnrichedPt5_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/QCD_Pt-120to170_MuEnrichedPt5_TuneCP5_13TeV_pythia8 ${tgdir}/QCD_Pt-120to170_MuEnrichedPt5 &> QCD_Pt-120to170_MuEnrichedPt5_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/QCD_Pt-15to20_MuEnrichedPt5_TuneCP5_13TeV_pythia8 ${tgdir}/QCD_Pt-15to20_MuEnrichedPt5 &> QCD_Pt-15to20_MuEnrichedPt5_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/QCD_Pt-170to300_MuEnrichedPt5_TuneCP5_13TeV_pythia8 ${tgdir}/QCD_Pt-170to300_MuEnrichedPt5 &> QCD_Pt-170to300_MuEnrichedPt5_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/QCD_Pt-20to30_MuEnrichedPt5_TuneCP5_13TeV_pythia8 ${tgdir}/QCD_Pt-20to30_MuEnrichedPt5 &> QCD_Pt-20to30_MuEnrichedPt5_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/QCD_Pt-300to470_MuEnrichedPt5_TuneCP5_13TeV_pythia8 ${tgdir}/QCD_Pt-300to470_MuEnrichedPt5 &> QCD_Pt-300to470_MuEnrichedPt5_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/QCD_Pt-30to50_MuEnrichedPt5_TuneCP5_13TeV_pythia8 ${tgdir}/QCD_Pt-30to50_MuEnrichedPt5 &> QCD_Pt-30to50_MuEnrichedPt5_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/QCD_Pt-470to600_MuEnrichedPt5_TuneCP5_13TeV_pythia8 ${tgdir}/QCD_Pt-470to600_MuEnrichedPt5 &> QCD_Pt-470to600_MuEnrichedPt5_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/QCD_Pt-50to80_MuEnrichedPt5_TuneCP5_13TeV_pythia8 ${tgdir}/QCD_Pt-50to80_MuEnrichedPt5 &> QCD_Pt-50to80_MuEnrichedPt5_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/QCD_Pt-600to800_MuEnrichedPt5_TuneCP5_13TeV_pythia8 ${tgdir}/QCD_Pt-600to800_MuEnrichedPt5 &> QCD_Pt-600to800_MuEnrichedPt5_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/QCD_Pt-800to1000_MuEnrichedPt5_TuneCP5_13TeV_pythia8 ${tgdir}/QCD_Pt-800to1000_MuEnrichedPt5 &> QCD_Pt-800to1000_MuEnrichedPt5_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/QCD_Pt-80to120_MuEnrichedPt5_TuneCP5_13TeV_pythia8 ${tgdir}/QCD_Pt-80to120_MuEnrichedPt5 &> QCD_Pt-80to120_MuEnrichedPt5_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8 ${tgdir}/ST_t-channel_antitop &> ST_t-channel_antitop_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir19ul}/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8 ${tgdir}/ST_t-channel_top &> ST_t-channel_top_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir20ul}/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8 ${tgdir}/TTZToLLNuNu_M-10 &> TTZToLLNuNu_M-10_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir20ul}/TTZToQQ_TuneCP5_13TeV-amcatnlo-pythia8 ${tgdir}/TTZToQQ &> TTZToQQ_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir20ul}/WW_TuneCP5_13TeV-pythia8 ${tgdir}/WW &> WW_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir20ul}/WZ_TuneCP5_13TeV-pythia8 ${tgdir}/WZ &> WZ_skim16pre.out
./skimnanoaod.py -F --split 80 -Y skim16pre ${mcdir20ul}/ZZ_TuneCP5_13TeV-pythia8 ${tgdir}/ZZ &> ZZ_skim16pre.out
