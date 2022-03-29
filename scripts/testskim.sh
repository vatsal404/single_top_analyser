#!/bin/bash
centraldata_dir="/data1/common/NanoAOD/data"
centralmc16_dir="/data1/common/NanoAOD/mc/RunIISummer16NanoAODv6"
centralmc17_dir="/data1/common/NanoAOD/mc/RunIIFall17NanoAODv6"
centralmc18_dir="/data1/common/NanoAOD/mc/RunIIAutumn18NanoAODv6"
centralULmc16_dir="/data1/common/NanoAOD/mc/RunIISummer20UL16NanoAODv2"
centralULmc17_dir="/data1/common/NanoAOD/mc/RunIISummer20UL17NanoAODv2"
centralULmc18_dir="/data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2"
sys=norm
#./skimonefile.py -Y skim18 /data1/common/NanoAOD/mc/RunIISummer20UL18NanoAODv2/LFV_ST_TCMuTau_Scalar_NANO/NANOAODSIM/210510_093245/0000/LFV_ST_TCMuTau_Scalar_NANO_Apr2021_A_1.root testmc.root Events outputTree &> testmc.out &
#./skimonefile.py -Y skim18 /data1/common/NanoAOD/data/Run2016B/SingleMuon/NANOAOD/ver1_HIPM_UL2016_MiniAODv1_NanoAODv2-v1/230000/1F84AAA7-1B6E-D24C-BF97-38495AF84FD4.root testdata.root Events outputTree &> testdata.out &
#./skimnanoaod.py -F --split 56 -Y skim18 $centralULmc18_dir/LFV_ST_TCMuTau_Scalar_NANO ./LFV_ST_TCMuTau_Scalar &> test_LFV_skim18.out 
./processnanoaod.py --allinone -Y 18 -S ${sys} --globaltag Autumn18_V19 LFV_ST_TCMuTau_Scalar test_LFV_process18.root &> test_LFV_process18.out &
