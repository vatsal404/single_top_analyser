#!/bin/bash
version=skim_LFVv6
mc16pre=/data1/common/skimmed_NanoAOD/$version/mc/16pre
mc16post=/data1/common/skimmed_NanoAOD/$version/mc/16post
mc17=/data1/common/skimmed_NanoAOD/$version/mc/17
mc18=/data1/common/skimmed_NanoAOD/$version/mc/18
sys=norm
target=sep_01
mkdir -p ${target}
while read line
do
    sample=${line}
    ./processnanoaod.py -A -Y 18 -S ${sys} --globaltag Summer19UL18_V5 ${mc18}/${sample} ${target}/${sample}_18_${sys}.root &> ${target}/${sample}_18_${sys}.out &
done < $1
