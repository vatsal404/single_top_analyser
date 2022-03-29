#!/bin/bash
centraldata="/data1/common/NanoAOD/data"
central19ULmc16pre="/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODAPVv2"
central19ULmc16post="/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2"
central19ULmc17="/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2"
central19ULmc18="/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2"
central20ULmc16pre="/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2"
central20ULmc16post="/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2"
central20ULmc17="/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2"
central20ULmc18="/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2"

version="skim_ULLFV_6_0_0"
targetmcUL16="/data1/common/skimmed_NanoAOD/$version/mc/2016UL"
targetmcUL17="/data1/common/skimmed_NanoAOD/$version/mc/2017UL"
targetmcUL18="/data1/common/skimmed_NanoAOD/$version/mc/2018UL"

# 16 pre
./skimnanoaod.py -F --split 56 -Y skim16pre

$central19ULmc16pre/
$central20ULmc16pre/

# 16 post
$central19ULmc16post/
$central20ULmc16post/


# 17
$central19ULmc17/
$central20ULmc17/



# 18
$central19ULmc18/
$central20ULmc18/

/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/TTToHadronic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/TTZToQQ_TuneCP5_13TeV-amcatnlo-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/WJetsToLNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/WJetsToLNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/WJetsToLNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/WJetsToLNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/WJetsToLNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/WJetsToLNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/TTWJetsToQQ_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/WW_TuneCP5_13TeV-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/WZ_TuneCP5_13TeV-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/ZZ_TuneCP5_13TeV-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2/DYJetsToLL_M-10to50_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/DYJetsToLL_M-10to50_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/TTToHadronic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/TTZToQQ_TuneCP5_13TeV-amcatnlo-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/WJetsToLNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/WJetsToLNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/WJetsToLNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/WJetsToLNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/WJetsToLNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/WJetsToLNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/TTWJetsToQQ_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/WW_TuneCP5_13TeV-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/WZ_TuneCP5_13TeV-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2/ZZ_TuneCP5_13TeV-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1


/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/DYJetsToLL_M-10to50_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/TTToHadronic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/TTZToQQ_TuneCP5_13TeV-amcatnlo-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/WJetsToLNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/WJetsToLNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/WJetsToLNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/WJetsToLNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/WJetsToLNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/WJetsToLNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/LFV_ST_TCMuTau_Scalar_NANO/NANOAODSIM/210510_093245
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/LFV_ST_TCMuTau_Tensor_NANO/NANOAODSIM/210510_093311
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/LFV_ST_TCMuTau_Vector_NANO/NANOAODSIM/210521_064149
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/LFV_ST_TUMuTau_Scalar_NANO/NANOAODSIM/210521_064629
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/LFV_ST_TUMuTau_Tensor_NANO/NANOAODSIM/210510_093406
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/LFV_ST_TUMuTau_Vector_NANO/NANOAODSIM/210510_093338
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/LFV_TT_TToCMuTau_Scalar_NANO/NANOAODSIM/210516_181827
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/LFV_TT_TToCMuTau_Tensor_NANO/NANOAODSIM/210510_093433
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/LFV_TT_TToCMuTau_Vector_NANO/NANOAODSIM/210516_181743
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/LFV_TT_TToUMuTau_Scalar_NANO/NANOAODSIM/210521_065356
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/LFV_TT_TToUMuTau_Tensor_NANO/NANOAODSIM/210516_181902
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/LFV_TT_TToUMuTau_Vector_NANO/NANOAODSIM/210510_093459
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/TTWJetsToQQ_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/WW_TuneCP5_13TeV-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/WZ_TuneCP5_13TeV-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/ZZ_TuneCP5_13TeV-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/DYJetsToLL_M-10to50_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/TTToHadronic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/WJetsToLNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/WJetsToLNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/WJetsToLNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/WJetsToLNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/WJetsToLNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/WW_TuneCP5_13TeV-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/WZ_TuneCP5_13TeV-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/ZZ_TuneCP5_13TeV-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODAPVv2/TTZToQQ_TuneCP5_13TeV-amcatnlo-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODAPVv2/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODAPVv2/QCD_Pt-15to20_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODAPVv2/QCD_Pt-20to30_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODAPVv2/QCD_Pt-30to50_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODAPVv2/QCD_Pt-50to80_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODAPVv2/QCD_Pt-80to120_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v9-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/GENWithPythiaBugFix_106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/QCD_Pt-1000toInf_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/QCD_Pt-120to170_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/QCD_Pt-15to20_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/QCD_Pt-170to300_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/QCD_Pt-20to30_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/QCD_Pt-300to470_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/QCD_Pt-30to50_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/QCD_Pt-470to600_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/QCD_Pt-50to80_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/QCD_Pt-600to800_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/QCD_Pt-800to1000_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/QCD_Pt-80to120_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL17NanoAODv2/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8/NANOAODSIM/106X_mc2017_realistic_v8-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2/QCD_Pt-1000toInf_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2/QCD_Pt-120to170_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2/QCD_Pt-15to20_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2/QCD_Pt-170to300_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2/QCD_Pt-20to30_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2/QCD_Pt-300to470_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2/QCD_Pt-30to50_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2/QCD_Pt-470to600_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2/QCD_Pt-50to80_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2/QCD_Pt-600to800_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2/QCD_Pt-800to1000_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2/QCD_Pt-80to120_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer19UL16NanoAODv2/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_v15-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/QCD_Pt-1000toInf_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/QCD_Pt-120to170_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/QCD_Pt-15to20_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/QCD_Pt-170to300_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/QCD_Pt-20to30_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/QCD_Pt-300to470_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/QCD_Pt-30to50_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/QCD_Pt-470to600_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/QCD_Pt-50to80_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/QCD_Pt-600to800_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/QCD_Pt-800to1000_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/QCD_Pt-80to120_MuEnrichedPt5_TuneCP5_13TeV_pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1
/data1/common/NanoAOD/mc/RunIISummer19UL18NanoAODv2/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v15_L1v1-v1





# WJet HT < 100 skim
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_inclHT100  > WJetsToLNu_inclHT100_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_inclHT100  > WJetsToLNu_inclHT100_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/WJetsToLNu_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_inclHT100  > WJetsToLNu_inclHT100_skim16.out

# LFV
./skimnanoaod.py -F --split 56 -Y 2018 $centralULmc18_dir/LFV_ST_TCMuTau_Scalar_NANO $targetmcUL18_dir/LFV_ST_TCMuTau_Scalar  > LFV_ST_TCMuTau_Scalar_skim18.out & 
./skimnanoaod.py -F --split 56 -Y 2018 $centralULmc18_dir/LFV_ST_TCMuTau_Vector_NANO $targetmcUL18_dir/LFV_ST_TCMuTau_Vector  > LFV_ST_TCMuTau_Vector_skim18.out &
./skimnanoaod.py -F --split 56 -Y 2018 $centralULmc18_dir/LFV_ST_TCMuTau_Tensor_NANO $targetmcUL18_dir/LFV_ST_TCMuTau_Tensor  > LFV_ST_TCMuTau_Tensor_skim18.out 
./skimnanoaod.py -F --split 56 -Y 2018 $centralULmc18_dir/LFV_ST_TUMuTau_Scalar_NANO $targetmcUL18_dir/LFV_ST_TUMuTau_Scalar  > LFV_ST_TUMuTau_Scalar_skim18.out &
./skimnanoaod.py -F --split 56 -Y 2018 $centralULmc18_dir/LFV_ST_TUMuTau_Vector_NANO $targetmcUL18_dir/LFV_ST_TUMuTau_Vector  > LFV_ST_TUMuTau_Vector_skim18.out &
./skimnanoaod.py -F --split 56 -Y 2018 $centralULmc18_dir/LFV_ST_TUMuTau_Tensor_NANO $targetmcUL18_dir/LFV_ST_TUMuTau_Tensor  > LFV_ST_TUMuTau_Tensor_skim18.out 
./skimnanoaod.py -F --split 56 -Y 2018 $centralULmc18_dir/LFV_TT_TToCMuTau_Scalar_NANO $targetmcUL18_dir/LFV_TT_TToCMuTau_Scalar  > LFV_TT_TToCMuTau_Scalar_skim18.out &
./skimnanoaod.py -F --split 56 -Y 2018 $centralULmc18_dir/LFV_TT_TToCMuTau_Vector_NANO $targetmcUL18_dir/LFV_TT_TToCMuTau_Vector  > LFV_TT_TToCMuTau_Vector_skim18.out &
./skimnanoaod.py -F --split 56 -Y 2018 $centralULmc18_dir/LFV_TT_TToCMuTau_Tensor_NANO $targetmcUL18_dir/LFV_TT_TToCMuTau_Tensor  > LFV_TT_TToCMuTau_Tensor_skim18.out 
./skimnanoaod.py -F --split 56 -Y 2018 $centralULmc18_dir/LFV_TT_TToUMuTau_Scalar_NANO $targetmcUL18_dir/LFV_TT_TToUMuTau_Scalar  > LFV_TT_TToUMuTau_Scalar_skim18.out &
./skimnanoaod.py -F --split 56 -Y 2018 $centralULmc18_dir/LFV_TT_TToUMuTau_Vector_NANO $targetmcUL18_dir/LFV_TT_TToUMuTau_Vector  > LFV_TT_TToUMuTau_Vector_skim18.out &
./skimnanoaod.py -F --split 56 -Y 2018 $centralULmc18_dir/LFV_TT_TToUMuTau_Tensor_NANO $targetmcUL18_dir/LFV_TT_TToUMuTau_Tensor  > LFV_TT_TToUMuTau_Tensor_skim18.out 

#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/LFV_TT_To_CMuTau_Clequ1_NANO $targetmc18_dir/LFV_TT_To_CMuTau_Scalar  > LFV_TT_To_CMuTau_Scalar_skim18.out &
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/LFV_TT_To_CMuTau_Vec_NANO $targetmc18_dir/LFV_TT_To_CMuTau_Vector  > LFV_TT_To_CMuTau_Vector_skim18.out &
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/LFV_TT_To_CMuTau_Clequ3_NANO $targetmc18_dir/LFV_TT_To_CMuTau_Tensor  > LFV_TT_To_CMuTau_Tensor_skim18.out &

#2018
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/DYJetsToLL_M-10to50_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/DYJetsToLL_M-10to50  > DYJetsToLL_M-10to50_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8 $targetmc18_dir/DYJetsToLL_M-50-amcatnloFXFX  > DYJetsToLL_M-50-amcatnloFXFX_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/QCD_Pt-15to20_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-15to20_MuEnrichedPt5  > QCD_Pt-15to20_MuEnrichedPt5_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/QCD_Pt-20to30_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-20to30_MuEnrichedPt5  > QCD_Pt-20to30_MuEnrichedPt5_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/QCD_Pt-30to50_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-30to50_MuEnrichedPt5  > QCD_Pt-30to50_MuEnrichedPt5_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/QCD_Pt-50to80_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-50to80_MuEnrichedPt5  > QCD_Pt-50to80_MuEnrichedPt5_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/QCD_Pt-80to120_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-80to120_MuEnrichedPt5  > QCD_Pt-80to120_MuEnrichedPt5_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/QCD_Pt-120to170_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-120to170_MuEnrichedPt5  > QCD_Pt-120to170_MuEnrichedPt5_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/QCD_Pt-170to300_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-170to300_MuEnrichedPt5  > QCD_Pt-170to300_MuEnrichedPt5_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/QCD_Pt-300to470_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-300to470_MuEnrichedPt5  > CD_Pt-300to470_MuEnrichedPt5_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/QCD_Pt-470to600_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-470to600_MuEnrichedPt5  > QCD_Pt-470to600_MuEnrichedPt5_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/QCD_Pt-600to800_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-600to800_MuEnrichedPt5  > QCD_Pt-600to800_MuEnrichedPt5_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/QCD_Pt-800to1000_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-800to1000_MuEnrichedPt5  > QCD_Pt-800to1000_MuEnrichedPt5_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/QCD_Pt-1000toInf_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc18_dir/QCD_Pt-1000toInf_MuEnrichedPt5  > QCD_Pt-1000toInf_MuEnrichedPt5_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8 $targetmc18_dir/ST_t-channel_antitop  > ST_t-channel_antitop_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8 $targetmc18_dir/ST_t-channel_top  > ST_t-channel_top_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/ST_tW_antitop_5f_inclusiveDecays_TuneCP5_13TeV-powheg-pythia8 $targetmc18_dir/ST_tW_antitop  > ST_tW_antitop_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/ST_tW_top_5f_inclusiveDecays_TuneCP5_13TeV-powheg-pythia8 $targetmc18_dir/ST_tW_top  > ST_tW_top_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8 $targetmc18_dir/TTTo2L2Nu  > TTTo2L2Nu_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8 $targetmc18_dir/TTWJetsToLNu  > TTWJetsToLNu_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/TTWJetsToQQ_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8 $targetmc18_dir/TTWJetsToQQ  > TTWJetsToQQ_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8 $targetmc18_dir/TTZToLLNuNu_M-10  > TTZToLLNuNu_M-10_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/TTZToQQ_TuneCP5_13TeV-amcatnlo-pythia8 $targetmc18_dir/TTZToQQ_TuneCP5_13TeV  > TTZToQQ_TuneCP5_13TeV_skim18.out
#
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/TTToHadronic_TuneCP5_13TeV-powheg-pythia8 $targetmc18_dir/TTToHadronic  > TTToHadronic_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8 $targetmc18_dir/TTToSemiLeptonic  > TTToSemiLeptonic_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/WJetsToLNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_HT-100To200  > WJetsToLNu_HT-100To200_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/WJetsToLNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_HT-200To400  > WJetsToLNu_HT-200To400_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/WJetsToLNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_HT-400To600  > WJetsToLNu_HT-400To600_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/WJetsToLNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_HT-600To800  > WJetsToLNu_HT-600To800_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/WJetsToLNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_HT-800To1200  > WJetsToLNu_HT-800To1200_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/WJetsToLNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_HT-1200To2500  > WJetsToLNu_HT-1200To2500_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/WJetsToLNu_HT-2500ToInf_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc18_dir/WJetsToLNu_HT-2500ToInf  > WJetsToLNu_HT-2500ToInf_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/WWTo2L2Nu_NNPDF31_TuneCP5_13TeV-powheg-pythia8 $targetmc18_dir/WWTo2L2Nu  > WWTo2L2Nu_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/WWToLNuQQ_NNPDF31_TuneCP5_13TeV-powheg-pythia8 $targetmc18_dir/WWToLNuQQ  > WWToLNuQQ_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/WZTo2L2Q_13TeV_amcatnloFXFX_madspin_pythia8 $targetmc18_dir/WZTo2L2Q  > WZTo2L2Q_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/WZTo3LNu_TuneCP5_13TeV-amcatnloFXFX-pythia8 $targetmc18_dir/WZTo3LNu  > WZTo3LNu_skim18.out
#./skimnanoaod.py -F --split 56 -Y 2018 $centralmc18_dir/ZZTo2L2Q_13TeV_amcatnloFXFX_madspin_pythia8 $targetmc18_dir/ZZTo2L2Q  > ZZTo2L2Q_skim18.out
#
##2017
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8 $targetmc17_dir/TTTo2L2Nu > TTTo2L2Nu_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/TTToHadronic_TuneCP5_13TeV-powheg-pythia8 $targetmc17_dir/TTToHadronic > TTToHadronic_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8 $targetmc17_dir/TTToSemiLeptonic > TTToSemiLeptonic_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/WJetsToLNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_HT-100To200 > WJetsToLNu_HT-100To200_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/WJetsToLNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_HT-200To400 > WJetsToLNu_HT-200To400_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/WJetsToLNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_HT-400To600 > WJetsToLNu_HT-400To600_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/WJetsToLNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_HT-600To800 > WJetsToLNu_HT-600To800_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/WJetsToLNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_HT-800To1200 > WJetsToLNu_HT-800To1200_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/WJetsToLNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_HT-1200To2500 > WJetsToLNu_HT-1200To2500_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/WJetsToLNu_HT-2500ToInf_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/WJetsToLNu_HT-2500ToInf > WJetsToLNu_HT-2500ToInf_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/DYJetsToLL_M-10to50_TuneCP5_13TeV-madgraphMLM-pythia8 $targetmc17_dir/DYJetsToLL_M-10to50 > DYJetsToLL_M-10to50_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8 $targetmc17_dir/DYJetsToLL_M-50-amcatnloFXFX > DYJetsToLL_M-50-amcatnloFXFX_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc17_dir/ST_t-channel_antitop > ST_t-channel_antitop_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc17_dir/ST_t-channel_top > ST_t-channel_top_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/ST_tW_antitop_5f_inclusiveDecays_TuneCP5_13TeV-powheg-pythia8 $targetmc17_dir/ST_tW_antitop > ST_tW_antitop_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/ST_tW_top_5f_inclusiveDecays_TuneCP5_13TeV-powheg-pythia8 $targetmc17_dir/ST_tW_top > ST_tW_top_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/QCD_Pt-15to20_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-15to20_MuEnrichedPt5  > QCD_Pt-15to20_MuEnrichedPt5_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/QCD_Pt-20to30_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-20to30_MuEnrichedPt5  > QCD_Pt-20to30_MuEnrichedPt5_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/QCD_Pt-30to50_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-30to50_MuEnrichedPt5  > QCD_Pt-30to50_MuEnrichedPt5_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/QCD_Pt-50to80_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-50to80_MuEnrichedPt5  > QCD_Pt-50to80_MuEnrichedPt5_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/QCD_Pt-80to120_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-80to120_MuEnrichedPt5  > QCD_Pt-80to120_MuEnrichedPt5_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/QCD_Pt-120to170_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-120to170_MuEnrichedPt5  > QCD_Pt-120to170_MuEnrichedPt5_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/QCD_Pt-170to300_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-170to300_MuEnrichedPt5  > QCD_Pt-170to300_MuEnrichedPt5_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/QCD_Pt-300to470_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-300to470_MuEnrichedPt5  > CD_Pt-300to470_MuEnrichedPt5_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/QCD_Pt-470to600_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-470to600_MuEnrichedPt5  > QCD_Pt-470to600_MuEnrichedPt5_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/QCD_Pt-600to800_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-600to800_MuEnrichedPt5  > QCD_Pt-600to800_MuEnrichedPt5_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/QCD_Pt-800to1000_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-800to1000_MuEnrichedPt5  > QCD_Pt-800to1000_MuEnrichedPt5_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/QCD_Pt-1000toInf_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc17_dir/QCD_Pt-1000toInf_MuEnrichedPt5  > QCD_Pt-1000toInf_MuEnrichedPt5_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/WWTo2L2Nu_NNPDF31_TuneCP5_13TeV-powheg-pythia8 $targetmc17_dir/WWTo2L2Nu  > WWTo2L2Nu_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/WWToLNuQQ_NNPDF31_TuneCP5_13TeV-powheg-pythia8 $targetmc17_dir/WWToLNuQQ  > WWToLNuQQ_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/WZTo2L2Q_13TeV_amcatnloFXFX_madspin_pythia8 $targetmc17_dir/WZTo2L2Q  > WZTo2L2Q_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/WZTo3LNu_TuneCP5_13TeV-amcatnloFXFX-pythia8 $targetmc17_dir/WZTo3LNu  > WZTo3LNu_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/ZZTo2L2Q_13TeV_amcatnloFXFX_madspin_pythia8 $targetmc17_dir/ZZTo2L2Q  > ZZTo2L2Q_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8 $targetmc17_dir/TTWJetsToLNu  > TTWJetsToLNu_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/TTWJetsToQQ_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8 $targetmc17_dir/TTWJetsToQQ  > TTWJetsToQQ_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8 $targetmc17_dir/TTZToLLNuNu_M-10  > TTZToLLNuNu_M-10_skim17.out
#./skimnanoaod.py -F --split 56 -Y 2017 $centralmc17_dir/TTZToQQ_TuneCP5_13TeV-amcatnlo-pythia8 $targetmc17_dir/TTZToQQ_TuneCP5_13TeV  > TTZToQQ_TuneCP5_13TeV_skim17.out
#
##2016
#./skimnanoaod.py -F --split 56 -Y 2016CP5 $centralmc16_dir/TTTo2L2Nu_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc16_dir/TTTo2L2Nu > TTTo2L2Nu_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016CP5 $centralmc16_dir/TTToHadronic_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc16_dir/TTToHadronic > TTToHadronic_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016CP5 $centralmc16_dir/TTToSemiLeptonic_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc16_dir/TTToSemiLeptonic > TTToSemiLeptonic_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/WJetsToLNu_HT-100To200_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_HT-100To200 > WJetsToLNu_HT-100To200_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/WJetsToLNu_HT-200To400_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_HT-200To400 > WJetsToLNu_HT-200To400_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/WJetsToLNu_HT-400To600_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_HT-400To600 > WJetsToLNu_HT-400To600_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/WJetsToLNu_HT-600To800_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_HT-600To800 > WJetsToLNu_HT-600To800_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/WJetsToLNu_HT-800To1200_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_HT-800To1200 > WJetsToLNu_HT-800To1200_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/WJetsToLNu_HT-1200To2500_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_HT-1200To2500 > WJetsToLNu_HT-1200To2500_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/WJetsToLNu_HT-2500ToInf_TuneCUETP8M1_13TeV-madgraphMLM-pythia8 $targetmc16_dir/WJetsToLNu_HT-2500ToInf > WJetsToLNu_HT-2500ToInf_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/DYJetsToLL_M-10to50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8 $targetmc16_dir/DYJetsToLL_M-10to50 > DYJetsToLL_M-10to50_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8 $targetmc16_dir/DYJetsToLL_M-50-amcatnloFXFX > DYJetsToLL_M-50-amcatnloFXFX_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016CP5 $centralmc16_dir/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc16_dir/ST_t-channel_antitop > ST_t-channel_antitop_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016CP5 $centralmc16_dir/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc16_dir/ST_t-channel_top > ST_t-channel_top_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016CP5 $centralmc16_dir/ST_tW_antitop_5f_inclusiveDecays_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc16_dir/ST_tW_antitop > ST_tW_antitop_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016CP5 $centralmc16_dir/ST_tW_top_5f_inclusiveDecays_TuneCP5_PSweights_13TeV-powheg-pythia8 $targetmc16_dir/ST_tW_top > ST_tW_top_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/QCD_Pt-15to20_MuEnrichedPt5_TuneCP5_13TeV_pythia8 $targetmc16_dir/QCD_Pt-15to20_MuEnrichedPt5  > QCD_Pt-15to20_MuEnrichedPt5_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/QCD_Pt-20to30_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-20to30_MuEnrichedPt5  > QCD_Pt-20to30_MuEnrichedPt5_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/QCD_Pt-30to50_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-30to50_MuEnrichedPt5  > QCD_Pt-30to50_MuEnrichedPt5_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/QCD_Pt-50to80_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-50to80_MuEnrichedPt5  > QCD_Pt-50to80_MuEnrichedPt5_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/QCD_Pt-80to120_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-80to120_MuEnrichedPt5  > QCD_Pt-80to120_MuEnrichedPt5_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/QCD_Pt-120to170_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-120to170_MuEnrichedPt5  > QCD_Pt-120to170_MuEnrichedPt5_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/QCD_Pt-160to300_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-160to300_MuEnrichedPt5  > QCD_Pt-160to300_MuEnrichedPt5_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/QCD_Pt-300to470_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-300to470_MuEnrichedPt5  > CD_Pt-300to470_MuEnrichedPt5_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/QCD_Pt-470to600_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-470to600_MuEnrichedPt5  > QCD_Pt-470to600_MuEnrichedPt5_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/QCD_Pt-600to800_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-600to800_MuEnrichedPt5  > QCD_Pt-600to800_MuEnrichedPt5_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/QCD_Pt-800to1000_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-800to1000_MuEnrichedPt5  > QCD_Pt-800to1000_MuEnrichedPt5_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/QCD_Pt-1000toInf_MuEnrichedPt5_TuneCUETP8M1_13TeV_pythia8 $targetmc16_dir/QCD_Pt-1000toInf_MuEnrichedPt5  > QCD_Pt-1000toInf_MuEnrichedPt5_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/TTWJetsToLNu_TuneCUETP8M1_13TeV-amcatnloFXFX-madspin-pythia8 $targetmc16_dir/TTWJetsToLNu  > TTWJetsToLNu_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/TTWJetsToQQ_TuneCUETP8M1_13TeV-amcatnloFXFX-madspin-pythia8 $targetmc16_dir/TTWJetsToQQ  > TTWJetsToQQ_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/TTZToLLNuNu_M-10_TuneCUETP8M1_13TeV-amcatnlo-pythia8 $targetmc16_dir/TTZToLLNuNu_M-10  > TTZToLLNuNu_M-10_skim16.out
#./skimnanoaod.py -F --split 56 -Y 2016 $centralmc16_dir/TTZToQQ_TuneCUETP8M1_13TeV-amcatnlo-pythia8 $targetmc16_dir/TTZToQQ_TuneCUETP8M1_13TeV  > TTZToQQ_TuneCUETP8M1_13TeV_skim16.out
