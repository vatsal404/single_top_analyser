#!/bin/bash
centraldata_dir="/data1/common/NanoAOD/data"
centralmc16_dir="/data1/common/NanoAOD/mc"
centralmc17_dir="/data1/common/NanoAOD/mc"
centralmc18_dir="/data1/common/NanoAOD/mc/RunIIAutumn18NanoAODv6"

version="skim_LFV_4_2_0"
targetmc18_dir="/data1/common/skimmed_NanoAOD/$version/mc/2018"
targetdata_dir="/data1/common/skimmed_NanoAOD/$version/data/"

#2018
./skimnanoaod.py -F --split 50 -Y 2018 $centraldata_dir/Run2018A/SingleMuon/NANOAOD/UL2018_MiniAODv1_NanoAODv2-v1 $targetdata_dir/SingleMuonUL2018A > SingleMuonUL2018A.out
./skimnanoaod.py -F --split 50 -Y 2018 $centraldata_dir/Run2018B/SingleMuon/NANOAOD/UL2018_MiniAODv1_NanoAODv2-v1 $targetdata_dir/SingleMuonUL2018B > SingleMuonUL2018B.out
./skimnanoaod.py -F --split 50 -Y 2018 $centraldata_dir/Run2018C/SingleMuon/NANOAOD/UL2018_MiniAODv1_NanoAODv2-v1 $targetdata_dir/SingleMuonUL2018C > SingleMuonUL2018C.out
./skimnanoaod.py -F --split 50 -Y 2018 $centraldata_dir/Run2018D/SingleMuon/NANOAOD/UL2018_MiniAODv1_NanoAODv2-v1 $targetdata_dir/SingleMuonUL2018D > SingleMuonUL2018D.out

#2017
./skimnanoaod.py -F --split 50 -Y 2017 $centraldata_dir/Run2017B/SingleMuon/NANOAOD/Nano25Oct2019-v1 $targetdata_dir/SingleMuon2017B > SingleMuon2017B.out
./skimnanoaod.py -F --split 50 -Y 2017 $centraldata_dir/Run2017C/SingleMuon/NANOAOD/Nano25Oct2019-v1 $targetdata_dir/SingleMuon2017C > SingleMuon2017C.out
./skimnanoaod.py -F --split 50 -Y 2017 $centraldata_dir/Run2017D/SingleMuon/NANOAOD/Nano25Oct2019-v1 $targetdata_dir/SingleMuon2017D > SingleMuon2017D.out
./skimnanoaod.py -F --split 50 -Y 2017 $centraldata_dir/Run2017E/SingleMuon/NANOAOD/Nano25Oct2019-v1 $targetdata_dir/SingleMuon2017E > SingleMuon2017E.out
./skimnanoaod.py -F --split 50 -Y 2017 $centraldata_dir/Run2017F/SingleMuon/NANOAOD/Nano25Oct2019-v1 $targetdata_dir/SingleMuon2017F > SingleMuon2017F.out

#2016
./skimnanoaod.py -F --split 50 -Y 2016 $centraldata_dir/Run2016B_ver2/SingleMuon/NANOAOD/Nano25Oct2019-v1 $targetdata_dir/SingleMuon2016B_ver2 > SingleMuon2016B_ver2.out
./skimnanoaod.py -F --split 50 -Y 2016 $centraldata_dir/Run2016C/SingleMuon/NANOAOD/Nano25Oct2019-v1 $targetdata_dir/SingleMuon2016C > SingleMuon2016C.out
./skimnanoaod.py -F --split 50 -Y 2016 $centraldata_dir/Run2016D/SingleMuon/NANOAOD/Nano25Oct2019-v1 $targetdata_dir/SingleMuon2016D > SingleMuon2016D.out
./skimnanoaod.py -F --split 50 -Y 2016 $centraldata_dir/Run2016E/SingleMuon/NANOAOD/Nano25Oct2019-v1 $targetdata_dir/SingleMuon2016E > SingleMuon2016E.out
./skimnanoaod.py -F --split 50 -Y 2016 $centraldata_dir/Run2016F/SingleMuon/NANOAOD/Nano25Oct2019-v1 $targetdata_dir/SingleMuon2016F > SingleMuon2016F.out
./skimnanoaod.py -F --split 50 -Y 2016 $centraldata_dir/Run2016G/SingleMuon/NANOAOD/Nano25Oct2019-v1 $targetdata_dir/SingleMuon2016G > SingleMuon2016G.out
./skimnanoaod.py -F --split 50 -Y 2016 $centraldata_dir/Run2016H/SingleMuon/NANOAOD/Nano25Oct2019-v1 $targetdata_dir/SingleMuon2016H > SingleMuon2016H.out
