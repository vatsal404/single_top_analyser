#!/bin/bash
centraldata_dir="/data1/common/NanoAOD/data"
centralmc16_dir="/data1/common/NanoAOD/mc/RunIISummer16NanoAODv6"
centralmc17_dir="/data1/common/NanoAOD/mc/RunIIFall17NanoAODv6"
centralmc18_dir="/data1/common/NanoAOD/mc/RunIIAutumn18NanoAODv6"

version="skim_LFV_4_2_0"
targetmc16_dir="/data1/common/skimmed_NanoAOD/$version/mc/2016v6"
targetmc17_dir="/data1/common/skimmed_NanoAOD/$version/mc/2017v6"
targetmc18_dir="/data1/common/skimmed_NanoAOD/$version/mc/2018v6"
targetmcUL16_dir="/data1/common/skimmed_NanoAOD/$version/mc/2016UL"
targetmcUL17_dir="/data1/common/skimmed_NanoAOD/$version/mc/2017UL"
targetmcUL18_dir="/data1/common/skimmed_NanoAOD/$version/mc/2018UL"

# WJet HT < 100 skim
#./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_inclHT100  > WJetsToLNu_inclHT100_skim18.out
#./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_inclHT100  > WJetsToLNu_inclHT100_skim17.out
#./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/WJetsToLNu_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_inclHT100  > WJetsToLNu_inclHT100_skim16.out

#2018
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/DYJetsToLL_M-10to50_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/DYJetsToLL_M-10to50  > DYJetsToLL_M-10to50_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8 $targetmc18_dir/DYJetsToLL_M-50-amcatnloFXFX  > DYJetsToLL_M-50-amcatnloFXFX_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/QCD_Pt-15to20_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-15to20_MuEnrichedPt5  > QCD_Pt-15to20_MuEnrichedPt5_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/QCD_Pt-20to30_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-20to30_MuEnrichedPt5  > QCD_Pt-20to30_MuEnrichedPt5_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/QCD_Pt-30to50_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-30to50_MuEnrichedPt5  > QCD_Pt-30to50_MuEnrichedPt5_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/QCD_Pt-50to80_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-50to80_MuEnrichedPt5  > QCD_Pt-50to80_MuEnrichedPt5_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/QCD_Pt-80to120_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-80to120_MuEnrichedPt5  > QCD_Pt-80to120_MuEnrichedPt5_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/QCD_Pt-120to170_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-120to170_MuEnrichedPt5  > QCD_Pt-120to170_MuEnrichedPt5_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/QCD_Pt-170to300_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-170to300_MuEnrichedPt5  > QCD_Pt-170to300_MuEnrichedPt5_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/QCD_Pt-300to470_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-300to470_MuEnrichedPt5  > CD_Pt-300to470_MuEnrichedPt5_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/QCD_Pt-470to600_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-470to600_MuEnrichedPt5  > QCD_Pt-470to600_MuEnrichedPt5_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/QCD_Pt-600to800_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-600to800_MuEnrichedPt5  > QCD_Pt-600to800_MuEnrichedPt5_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/QCD_Pt-800to1000_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-800to1000_MuEnrichedPt5  > QCD_Pt-800to1000_MuEnrichedPt5_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/QCD_Pt-1000toInf_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-1000toInf_MuEnrichedPt5  > QCD_Pt-1000toInf_MuEnrichedPt5_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8 $targetmc18_dir/ST_t-channel_antitop  > ST_t-channel_antitop_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8 $targetmc18_dir/ST_t-channel_top  > ST_t-channel_top_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/ST_tW_antitop_5f_inclusiveDecays_TuneCP5_13TeV-powheg-pythia8 $targetmc18_dir/ST_tW_antitop  > ST_tW_antitop_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/ST_tW_top_5f_inclusiveDecays_TuneCP5_13TeV-powheg-pythia8 $targetmc18_dir/ST_tW_top  > ST_tW_top_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8 $targetmc18_dir/TTTo2L2Nu  > TTTo2L2Nu_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8 $targetmc18_dir/TTWJetsToLNu  > TTWJetsToLNu_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/TTWJetsToQQ_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8 $targetmc18_dir/TTWJetsToQQ  > TTWJetsToQQ_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8 $targetmc18_dir/TTZToLLNuNu_M-10  > TTZToLLNuNu_M-10_skim18.out
./skimnanoaod.py -F --split 25 -Y 2018 $centralmc18_dir/TTZToQQ_TuneCP5_13TeV-amcatnlo-pythia8 $targetmc18_dir/TTZToQQ_TuneCP5_13TeV  > TTZToQQ_TuneCP5_13TeV_skim18.out

./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/TTToHadronic_TuneCP5_13TeV-powheg-pythia8 $targetmc18_dir/TTToHadronic  > TTToHadronic_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8 $targetmc18_dir/TTToSemiLeptonic  > TTToSemiLeptonic_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/WJetsToLNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_HT-100To200  > WJetsToLNu_HT-100To200_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/WJetsToLNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_HT-200To400  > WJetsToLNu_HT-200To400_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/WJetsToLNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_HT-400To600  > WJetsToLNu_HT-400To600_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/WJetsToLNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_HT-600To800  > WJetsToLNu_HT-600To800_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/WJetsToLNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_HT-800To1200  > WJetsToLNu_HT-800To1200_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/WJetsToLNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_HT-1200To2500  > WJetsToLNu_HT-1200To2500_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/WJetsToLNu_HT-2500ToInf_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_HT-2500ToInf  > WJetsToLNu_HT-2500ToInf_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/WWTo2L2Nu_NNPDF31_TuneCP5_13TeV-powheg-pythia8 $targetmc18_dir/WWTo2L2Nu  > WWTo2L2Nu_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/WWToLNuQQ_NNPDF31_TuneCP5_13TeV-powheg-pythia8 $targetmc18_dir/WWToLNuQQ  > WWToLNuQQ_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/WZTo2L2Q_13TeV_amcatnloFXFX_madspin_pythia8 $targetmc18_dir/WZTo2L2Q  > WZTo2L2Q_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/WZTo3LNu_TuneCP5_13TeV-amcatnloFXFX-pythia8 $targetmc18_dir/WZTo3LNu  > WZTo3LNu_skim18.out
./skimnanoaod.py -F --split 27 -Y 2018 $centralmc18_dir/ZZTo2L2Q_13TeV_amcatnloFXFX_madspin_pythia8 $targetmc18_dir/ZZTo2L2Q  > ZZTo2L2Q_skim18.out

#2017
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8 $targetmc17_dir/TTTo2L2Nu > TTTo2L2Nu_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/TTToHadronic_TuneCP5_13TeV-powheg-pythia8 $targetmc17_dir/TTToHadronic > TTToHadronic_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8 $targetmc17_dir/TTToSemiLeptonic > TTToSemiLeptonic_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/WJetsToLNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_HT-100To200 > WJetsToLNu_HT-100To200_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/WJetsToLNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_HT-200To400 > WJetsToLNu_HT-200To400_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/WJetsToLNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_HT-400To600 > WJetsToLNu_HT-400To600_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/WJetsToLNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_HT-600To800 > WJetsToLNu_HT-600To800_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/WJetsToLNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_HT-800To1200 > WJetsToLNu_HT-800To1200_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/WJetsToLNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_HT-1200To2500 > WJetsToLNu_HT-1200To2500_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/WJetsToLNu_HT-2500ToInf_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_HT-2500ToInf > WJetsToLNu_HT-2500ToInf_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/DYJetsToLL_M-10to50_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/DYJetsToLL_M-10to50 > DYJetsToLL_M-10to50_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8 $targetmc17_dir/DYJetsToLL_M-50-amcatnloFXFX > DYJetsToLL_M-50-amcatnloFXFX_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc17_dir/ST_t-channel_antitop > ST_t-channel_antitop_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc17_dir/ST_t-channel_top > ST_t-channel_top_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/ST_tW_antitop_5f_inclusiveDecays_TuneCP5_13TeV-powheg-pythia8 $targetmc17_dir/ST_tW_antitop > ST_tW_antitop_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/ST_tW_top_5f_inclusiveDecays_TuneCP5_13TeV-powheg-pythia8 $targetmc17_dir/ST_tW_top > ST_tW_top_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/QCD_Pt-15to20_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-15to20_MuEnrichedPt5  > QCD_Pt-15to20_MuEnrichedPt5_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/QCD_Pt-20to30_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-20to30_MuEnrichedPt5  > QCD_Pt-20to30_MuEnrichedPt5_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/QCD_Pt-30to50_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-30to50_MuEnrichedPt5  > QCD_Pt-30to50_MuEnrichedPt5_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/QCD_Pt-50to80_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-50to80_MuEnrichedPt5  > QCD_Pt-50to80_MuEnrichedPt5_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/QCD_Pt-80to120_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-80to120_MuEnrichedPt5  > QCD_Pt-80to120_MuEnrichedPt5_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/QCD_Pt-120to170_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-120to170_MuEnrichedPt5  > QCD_Pt-120to170_MuEnrichedPt5_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/QCD_Pt-170to300_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-170to300_MuEnrichedPt5  > QCD_Pt-170to300_MuEnrichedPt5_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/QCD_Pt-300to470_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-300to470_MuEnrichedPt5  > CD_Pt-300to470_MuEnrichedPt5_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/QCD_Pt-470to600_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-470to600_MuEnrichedPt5  > QCD_Pt-470to600_MuEnrichedPt5_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/QCD_Pt-600to800_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-600to800_MuEnrichedPt5  > QCD_Pt-600to800_MuEnrichedPt5_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/QCD_Pt-800to1000_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-800to1000_MuEnrichedPt5  > QCD_Pt-800to1000_MuEnrichedPt5_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/QCD_Pt-1000toInf_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-1000toInf_MuEnrichedPt5  > QCD_Pt-1000toInf_MuEnrichedPt5_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/WWTo2L2Nu_NNPDF31_TuneCP5_13TeV-powheg-pythia8 $targetmc17_dir/WWTo2L2Nu  > WWTo2L2Nu_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/WWToLNuQQ_NNPDF31_TuneCP5_13TeV-powheg-pythia8 $targetmc17_dir/WWToLNuQQ  > WWToLNuQQ_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/WZTo2L2Q_13TeV_amcatnloFXFX_madspin_pythia8 $targetmc17_dir/WZTo2L2Q  > WZTo2L2Q_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/WZTo3LNu_TuneCP5_13TeV-amcatnloFXFX-pythia8 $targetmc17_dir/WZTo3LNu  > WZTo3LNu_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/ZZTo2L2Q_13TeV_amcatnloFXFX_madspin_pythia8 $targetmc17_dir/ZZTo2L2Q  > ZZTo2L2Q_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8 $targetmc17_dir/TTWJetsToLNu  > TTWJetsToLNu_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/TTWJetsToQQ_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8 $targetmc17_dir/TTWJetsToQQ  > TTWJetsToQQ_skim17.out
./skimnanoaod.py -F --split 27 -Y 2017 $centralmc17_dir/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8 $targetmc17_dir/TTZToLLNuNu_M-10  > TTZToLLNuNu_M-10_skim17.out
./skimnanoaod.py -F --split 25 -Y 2017 $centralmc17_dir/TTZToQQ_TuneCP5_13TeV-amcatnlo-pythia8 $targetmc17_dir/TTZToQQ_TuneCP5_13TeV  > TTZToQQ_TuneCP5_13TeV_skim17.out


#2016
./skimnanoaod.py -F --split 27 -Y 2016CP5 $centralmc16_dir/TTTo2L2Nu_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc16_dir/TTTo2L2Nu > TTTo2L2Nu_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016CP5 $centralmc16_dir/TTToHadronic_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc16_dir/TTToHadronic > TTToHadronic_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016CP5 $centralmc16_dir/TTToSemiLeptonic_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc16_dir/TTToSemiLeptonic > TTToSemiLeptonic_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/WJetsToLNu_HT-100To200_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_HT-100To200 > WJetsToLNu_HT-100To200_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/WJetsToLNu_HT-200To400_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_HT-200To400 > WJetsToLNu_HT-200To400_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/WJetsToLNu_HT-400To600_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_HT-400To600 > WJetsToLNu_HT-400To600_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/WJetsToLNu_HT-600To800_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_HT-600To800 > WJetsToLNu_HT-600To800_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/WJetsToLNu_HT-800To1200_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_HT-800To1200 > WJetsToLNu_HT-800To1200_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/WJetsToLNu_HT-1200To2500_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_HT-1200To2500 > WJetsToLNu_HT-1200To2500_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/WJetsToLNu_HT-2500ToInf_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_HT-2500ToInf > WJetsToLNu_HT-2500ToInf_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/DYJetsToLL_M-10to50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8 $targetmc16_dir/DYJetsToLL_M-10to50 > DYJetsToLL_M-10to50_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8 $targetmc16_dir/DYJetsToLL_M-50-amcatnloFXFX > DYJetsToLL_M-50-amcatnloFXFX_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016CP5 $centralmc16_dir/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc16_dir/ST_t-channel_antitop > ST_t-channel_antitop_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016CP5 $centralmc16_dir/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc16_dir/ST_t-channel_top > ST_t-channel_top_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016CP5 $centralmc16_dir/ST_tW_antitop_5f_inclusiveDecays_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc16_dir/ST_tW_antitop > ST_tW_antitop_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016CP5 $centralmc16_dir/ST_tW_top_5f_inclusiveDecays_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc16_dir/ST_tW_top > ST_tW_top_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/QCD_Pt-15to20_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc16_dir/QCD_Pt-15to20_MuEnrichedPt5  > QCD_Pt-15to20_MuEnrichedPt5_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/QCD_Pt-20to30_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-20to30_MuEnrichedPt5  > QCD_Pt-20to30_MuEnrichedPt5_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/QCD_Pt-30to50_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-30to50_MuEnrichedPt5  > QCD_Pt-30to50_MuEnrichedPt5_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/QCD_Pt-50to80_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-50to80_MuEnrichedPt5  > QCD_Pt-50to80_MuEnrichedPt5_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/QCD_Pt-80to120_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-80to120_MuEnrichedPt5  > QCD_Pt-80to120_MuEnrichedPt5_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/QCD_Pt-120to170_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-120to170_MuEnrichedPt5  > QCD_Pt-120to170_MuEnrichedPt5_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/QCD_Pt-160to300_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-160to300_MuEnrichedPt5  > QCD_Pt-160to300_MuEnrichedPt5_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/QCD_Pt-300to470_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-300to470_MuEnrichedPt5  > CD_Pt-300to470_MuEnrichedPt5_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/QCD_Pt-470to600_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-470to600_MuEnrichedPt5  > QCD_Pt-470to600_MuEnrichedPt5_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/QCD_Pt-600to800_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-600to800_MuEnrichedPt5  > QCD_Pt-600to800_MuEnrichedPt5_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/QCD_Pt-800to1000_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-800to1000_MuEnrichedPt5  > QCD_Pt-800to1000_MuEnrichedPt5_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/QCD_Pt-1000toInf_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-1000toInf_MuEnrichedPt5  > QCD_Pt-1000toInf_MuEnrichedPt5_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/TTWJetsToLNu_TuneCUETP8M1_13TeV-amcatnloFXFX-madspin-pythia8 $targetmc16_dir/TTWJetsToLNu  > TTWJetsToLNu_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/TTWJetsToQQ_TuneCUETP8M1_13TeV-amcatnloFXFX-madspin-pythia8 $targetmc16_dir/TTWJetsToQQ  > TTWJetsToQQ_skim16.out
./skimnanoaod.py -F --split 27 -Y 2016 $centralmc16_dir/TTZToLLNuNu_M-10_TuneCUETP8M1_13TeV-amcatnlo-pythia8 $targetmc16_dir/TTZToLLNuNu_M-10  > TTZToLLNuNu_M-10_skim16.out
./skimnanoaod.py -F --split 25 -Y 2016 $centralmc16_dir/TTZToQQ_TuneCUETP8M1_13TeV-amcatnlo-pythia8 $targetmc16_dir/TTZToQQ_TuneCUETP8M1_13TeV  > TTZToQQ_TuneCUETP8M1_13TeV_skim16.out
