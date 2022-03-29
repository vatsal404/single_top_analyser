#!/bin/bash
#mc for 2018
#./skimnanoaod.py --batch /cms/scratch/tjkim/nanoaod/LQ_2018NANO/200517_045835 /cms/scratch/tjkim/nanoaod-processed/2018/MC/LQ_signal

#./skimnanoaod.py --batch /xrootd/store/mc/RunIIAutumn18NanoAODv6/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20-v1 /cms/scratch/tjkim/nanoaod-processed/2018/MC/TTTo2L2Nu  
#./skimnanoaod.py --batch /xrootd/store/mc/RunIIAutumn18NanoAODv6/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20_ext3-v1 /cms/scratch/tjkim/nanoaod-processed/2018/MC/TTToSemiLeptonic 
#./skimnanoaod.py --batch /xrootd/store/mc/RunIIAutumn18NanoAODv6/TTToHadronic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20-v3 /cms/scratch/tjkim/nanoaod-processed/2018/MC/TTToHadronic 
#./skimnanoaod.py --batch /xrootd/store/mc/RunIIAutumn18NanoAODv6/WJetsToLNu_0J_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20-v1 /cms/scratch/tjkim/nanoaod-processed/2018/MC/WJetsToLNu_0J 
#./skimnanoaod.py --batch /xrootd/store/mc/RunIIAutumn18NanoAODv6/WJetsToLNu_1J_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20-v1 /cms/scratch/tjkim/nanoaod-processed/2018/MC/WJetsToLNu_1J 
#./skimnanoaod.py --batch /xrootd/store/mc/RunIIAutumn18NanoAODv6/WJetsToLNu_2J_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20-v1 /cms/scratch/tjkim/nanoaod-processed/2018/MC/WJetsToLNu_2J 
#./skimnanoaod.py --batch /xrootd/store/mc/RunIIAutumn18NanoAODv6/DYJetsToLL_M-50_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20-v1 /cms/scratch/tjkim/nanoaod-processed/2018/MC/DYJetsToLL_M-50 
#./skimnanoaod.py --batch /xrootd/store/mc/RunIIAutumn18NanoAODv6/DYJetsToLL_M-10to50_TuneCP5_13TeV-madgraphMLM-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20_ext1-v1 /cms/scratch/tjkim/nanoaod-processed/2018/MC/DYJetsToLL_M-10to50 

#./skimnanoaod.py --batch /xrootd/store/mc/RunIIAutumn18NanoAODv6/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20-v1 /cms/scratch/tjkim/nanoaod-processed/2018/MC/ST_t-channel_top
#./skimnanoaod.py --batch /xrootd/store/mc/RunIIAutumn18NanoAODv6/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20-v1 /cms/scratch/tjkim/nanoaod-processed/2018/MC/ST_t-channel_antitop
#./skimnanoaod.py --batch /xrootd/store/mc/RunIIAutumn18NanoAODv6/ST_tW_top_5f_inclusiveDecays_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20_ext1-v1 /cms/scratch/tjkim/nanoaod-processed/2018/MC/ST_tW_top  

#data
#./skimnanoaod.py --batch /xrootd/store/data/Run2018A/SingleMuon/NANOAOD/Nano25Oct2019-v1 /cms/scratch/tjkim/nanoaod-processed/2018/RD/Run2018A
#./skimnanoaod.py --batch /xrootd/store/data/Run2018B/SingleMuon/NANOAOD/Nano25Oct2019-v1 /cms/scratch/tjkim/nanoaod-processed/2018/RD/Run2018B
#./skimnanoaod.py --batch /xrootd/store/data/Run2018C/SingleMuon/NANOAOD/Nano25Oct2019-v1 /cms/scratch/tjkim/nanoaod-processed/2018/RD/Run2018C
#./skimnanoaod.py --batch /xrootd/store/data/Run2018D/SingleMuon/NANOAOD/Nano25Oct2019-v1 /cms/scratch/tjkim/nanoaod-processed/2018/RD/Run2018D

# Splitted Run (No batch)
#./skimnanoaod.py --split 10 /data/common/NanoAOD/mc/RunIIAutumn18NanoAODv6/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20-v1 /data/common/skimmed_NanoAOD/tmp_oneMu_genWeight/TTTo2L2Nu &> TTTo2L2Nu.out
./skimnanoaod.py --split 10 /data/common/NanoAOD/mc/RunIIAutumn18NanoAODv6/TTToHadronic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20_ext2-v1 /data/common/skimmed_NanoAOD/tmp_oneMu_genWeight/TTToHadronic_ext &> TTToHadronic_ext.out
./skimnanoaod.py --split 10 /data/common/NanoAOD/mc/RunIIAutumn18NanoAODv6/TTToHadronic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20-v3 /data/common/skimmed_NanoAOD/tmp_oneMu_genWeight/TTToHadronic &> TTToHadronic.out
./skimnanoaod.py --split 10 /data/common/NanoAOD/mc/RunIIAutumn18NanoAODv6/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20_ext3-v1 /data/common/skimmed_NanoAOD/tmp_oneMu_genWeight/TTToSemiLeptonic &> TTToSemiLeptonic.out
./skimnanoaod.py --split 10 /data/common/NanoAOD/mc/RunIIAutumn18NanoAODv6/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20-v1 /data/common/skimmed_NanoAOD/tmp_oneMu_genWeight/TTToSemiLeptonic_ext &> TTToSemiLeptonic_ext.out




#./skimnanoaod.py --split 25 /data/common/NanoAOD/lq/LFV_TT_To_CMuTau_Vec_NANO/ ./testskim1 &> LFV_TT_To_CMuTau_Vec.out &
#./skimnanoaod.py --split 15 /data/common/NanoAOD/mc/RunIIAutumn18NanoAODv6/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/Nano25Oct2019_102X_upgrade2018_realistic_v20-v1/ ./ > TTToSemiLeptonic.out &
