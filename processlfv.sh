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
./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/ST_LFV_TCMuTau_Scalar ${target}/ST_LFV_TCMuTau_Scalar_16pre_${sys}.root &> ${target}/ST_LFV_TCMuTau_Scalar_16pre_${sys}.out &
./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/ST_LFV_TCMuTau_Vector ${target}/ST_LFV_TCMuTau_Vector_16pre_${sys}.root &> ${target}/ST_LFV_TCMuTau_Vector_16pre_${sys}.out &
./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/ST_LFV_TCMuTau_Tensor ${target}/ST_LFV_TCMuTau_Tensor_16pre_${sys}.root &> ${target}/ST_LFV_TCMuTau_Tensor_16pre_${sys}.out &
./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/ST_LFV_TUMuTau_Scalar ${target}/ST_LFV_TUMuTau_Scalar_16pre_${sys}.root &> ${target}/ST_LFV_TUMuTau_Scalar_16pre_${sys}.out &
./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/ST_LFV_TUMuTau_Vector ${target}/ST_LFV_TUMuTau_Vector_16pre_${sys}.root &> ${target}/ST_LFV_TUMuTau_Vector_16pre_${sys}.out &
./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/ST_LFV_TUMuTau_Tensor ${target}/ST_LFV_TUMuTau_Tensor_16pre_${sys}.root &> ${target}/ST_LFV_TUMuTau_Tensor_16pre_${sys}.out &
./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/TT_LFV_TToCMuTau_Scalar ${target}/TT_LFV_TToCMuTau_Scalar_16pre_${sys}.root &> ${target}/TT_LFV_TToCMuTau_Scalar_16pre_${sys}.out &
./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/TT_LFV_TToCMuTau_Vector ${target}/TT_LFV_TToCMuTau_Vector_16pre_${sys}.root &> ${target}/TT_LFV_TToCMuTau_Vector_16pre_${sys}.out &
./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/TT_LFV_TToCMuTau_Tensor ${target}/TT_LFV_TToCMuTau_Tensor_16pre_${sys}.root &> ${target}/TT_LFV_TToCMuTau_Tensor_16pre_${sys}.out &
./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/TT_LFV_TToUMuTau_Scalar ${target}/TT_LFV_TToUMuTau_Scalar_16pre_${sys}.root &> ${target}/TT_LFV_TToUMuTau_Scalar_16pre_${sys}.out &
./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/TT_LFV_TToUMuTau_Vector ${target}/TT_LFV_TToUMuTau_Vector_16pre_${sys}.root &> ${target}/TT_LFV_TToUMuTau_Vector_16pre_${sys}.out &
./processnanoaod.py -A -Y 16pre -S ${sys} --globaltag Summer19UL16APV_V7 ${mc16pre}/TT_LFV_TToUMuTau_Tensor ${target}/TT_LFV_TToUMuTau_Tensor_16pre_${sys}.root &> ${target}/TT_LFV_TToUMuTau_Tensor_16pre_${sys}.out &


# 16post
./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/DYJetsToLL_M-10to50 ${target}/DYJetsToLL_M-10to50_16post_${sys}.root &> ${target}/DYJetsToLL_M-10to50_16post_${sys}.out &
./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/ST_LFV_TCMuTau_Scalar ${target}/ST_LFV_TCMuTau_Scalar_16post_${sys}.root &> ${target}/ST_LFV_TCMuTau_Scalar_16post_${sys}.out &
./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/ST_LFV_TCMuTau_Vector ${target}/ST_LFV_TCMuTau_Vector_16post_${sys}.root &> ${target}/ST_LFV_TCMuTau_Vector_16post_${sys}.out &
./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/ST_LFV_TCMuTau_Tensor ${target}/ST_LFV_TCMuTau_Tensor_16post_${sys}.root &> ${target}/ST_LFV_TCMuTau_Tensor_16post_${sys}.out &
./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/ST_LFV_TUMuTau_Scalar ${target}/ST_LFV_TUMuTau_Scalar_16post_${sys}.root &> ${target}/ST_LFV_TUMuTau_Scalar_16post_${sys}.out &
./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/ST_LFV_TUMuTau_Vector ${target}/ST_LFV_TUMuTau_Vector_16post_${sys}.root &> ${target}/ST_LFV_TUMuTau_Vector_16post_${sys}.out &
./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/ST_LFV_TUMuTau_Tensor ${target}/ST_LFV_TUMuTau_Tensor_16post_${sys}.root &> ${target}/ST_LFV_TUMuTau_Tensor_16post_${sys}.out &
./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/TT_LFV_TToCMuTau_Scalar ${target}/TT_LFV_TToCMuTau_Scalar_16post_${sys}.root &> ${target}/TT_LFV_TToCMuTau_Scalar_16post_${sys}.out &
./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/TT_LFV_TToCMuTau_Vector ${target}/TT_LFV_TToCMuTau_Vector_16post_${sys}.root &> ${target}/TT_LFV_TToCMuTau_Vector_16post_${sys}.out &
./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/TT_LFV_TToCMuTau_Tensor ${target}/TT_LFV_TToCMuTau_Tensor_16post_${sys}.root &> ${target}/TT_LFV_TToCMuTau_Tensor_16post_${sys}.out &
./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/TT_LFV_TToUMuTau_Scalar ${target}/TT_LFV_TToUMuTau_Scalar_16post_${sys}.root &> ${target}/TT_LFV_TToUMuTau_Scalar_16post_${sys}.out &
./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/TT_LFV_TToUMuTau_Vector ${target}/TT_LFV_TToUMuTau_Vector_16post_${sys}.root &> ${target}/TT_LFV_TToUMuTau_Vector_16post_${sys}.out &
./processnanoaod.py -A -Y 16post -S ${sys} --globaltag Summer19UL16_V7 ${mc16post}/TT_LFV_TToUMuTau_Tensor ${target}/TT_LFV_TToUMuTau_Tensor_16post_${sys}.root &> ${target}/TT_LFV_TToUMuTau_Tensor_16post_${sys}.out &
sleep 30m
# 17
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/ST_LFV_TCMuTau_Scalar ${target}/ST_LFV_TCMuTau_Scalar_17_${sys}.root &> ${target}/ST_LFV_TCMuTau_Scalar_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/ST_LFV_TCMuTau_Vector ${target}/ST_LFV_TCMuTau_Vector_17_${sys}.root &> ${target}/ST_LFV_TCMuTau_Vector_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/ST_LFV_TCMuTau_Tensor ${target}/ST_LFV_TCMuTau_Tensor_17_${sys}.root &> ${target}/ST_LFV_TCMuTau_Tensor_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/ST_LFV_TUMuTau_Scalar ${target}/ST_LFV_TUMuTau_Scalar_17_${sys}.root &> ${target}/ST_LFV_TUMuTau_Scalar_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/ST_LFV_TUMuTau_Vector ${target}/ST_LFV_TUMuTau_Vector_17_${sys}.root &> ${target}/ST_LFV_TUMuTau_Vector_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/ST_LFV_TUMuTau_Tensor ${target}/ST_LFV_TUMuTau_Tensor_17_${sys}.root &> ${target}/ST_LFV_TUMuTau_Tensor_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/TT_LFV_TToCMuTau_Scalar ${target}/TT_LFV_TToCMuTau_Scalar_17_${sys}.root &> ${target}/TT_LFV_TToCMuTau_Scalar_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/TT_LFV_TToCMuTau_Vector ${target}/TT_LFV_TToCMuTau_Vector_17_${sys}.root &> ${target}/TT_LFV_TToCMuTau_Vector_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/TT_LFV_TToCMuTau_Tensor ${target}/TT_LFV_TToCMuTau_Tensor_17_${sys}.root &> ${target}/TT_LFV_TToCMuTau_Tensor_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/TT_LFV_TToUMuTau_Scalar ${target}/TT_LFV_TToUMuTau_Scalar_17_${sys}.root &> ${target}/TT_LFV_TToUMuTau_Scalar_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/TT_LFV_TToUMuTau_Vector ${target}/TT_LFV_TToUMuTau_Vector_17_${sys}.root &> ${target}/TT_LFV_TToUMuTau_Vector_17_${sys}.out &
./processnanoaod.py -A -Y 17 -S ${sys} --globaltag Summer19UL17_V5 ${mc17}/TT_LFV_TToUMuTau_Tensor ${target}/TT_LFV_TToUMuTau_Tensor_17_${sys}.root &> ${target}/TT_LFV_TToUMuTau_Tensor_17_${sys}.out &
# 18
./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/ST_LFV_TCMuTau_Scalar ${target}/ST_LFV_TCMuTau_Scalar_18_${sys}.root &> ${target}/ST_LFV_TCMuTau_Scalar_18_${sys}.out &
./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/ST_LFV_TCMuTau_Vector ${target}/ST_LFV_TCMuTau_Vector_18_${sys}.root &> ${target}/ST_LFV_TCMuTau_Vector_18_${sys}.out &
./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/ST_LFV_TCMuTau_Tensor ${target}/ST_LFV_TCMuTau_Tensor_18_${sys}.root &> ${target}/ST_LFV_TCMuTau_Tensor_18_${sys}.out &
./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/ST_LFV_TUMuTau_Scalar ${target}/ST_LFV_TUMuTau_Scalar_18_${sys}.root &> ${target}/ST_LFV_TUMuTau_Scalar_18_${sys}.out &
./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/ST_LFV_TUMuTau_Vector ${target}/ST_LFV_TUMuTau_Vector_18_${sys}.root &> ${target}/ST_LFV_TUMuTau_Vector_18_${sys}.out &
./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/ST_LFV_TUMuTau_Tensor ${target}/ST_LFV_TUMuTau_Tensor_18_${sys}.root &> ${target}/ST_LFV_TUMuTau_Tensor_18_${sys}.out &
./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/TT_LFV_TToCMuTau_Scalar ${target}/TT_LFV_TToCMuTau_Scalar_18_${sys}.root &> ${target}/TT_LFV_TToCMuTau_Scalar_18_${sys}.out &
./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/TT_LFV_TToCMuTau_Vector ${target}/TT_LFV_TToCMuTau_Vector_18_${sys}.root &> ${target}/TT_LFV_TToCMuTau_Vector_18_${sys}.out &
./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/TT_LFV_TToCMuTau_Tensor ${target}/TT_LFV_TToCMuTau_Tensor_18_${sys}.root &> ${target}/TT_LFV_TToCMuTau_Tensor_18_${sys}.out &
./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/TT_LFV_TToUMuTau_Scalar ${target}/TT_LFV_TToUMuTau_Scalar_18_${sys}.root &> ${target}/TT_LFV_TToUMuTau_Scalar_18_${sys}.out &
./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/TT_LFV_TToUMuTau_Vector ${target}/TT_LFV_TToUMuTau_Vector_18_${sys}.root &> ${target}/TT_LFV_TToUMuTau_Vector_18_${sys}.out &
./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/TT_LFV_TToUMuTau_Tensor ${target}/TT_LFV_TToUMuTau_Tensor_18_${sys}.root &> ${target}/TT_LFV_TToUMuTau_Tensor_18_${sys}.out &
