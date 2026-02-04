#!/bin/bash

BASE_DIR="/eos/uscms/store/user/vsinha/Result_2023/hlt_scale_factor"

for i in {0..9}; do
    echo "Merging TTbar index $i"
    hadd -f TTbar_${i}.root ${BASE_DIR}/TTbar*_${i}.root

    echo "Merging Data index $i"
    hadd -f Data_${i}.root ${BASE_DIR}/Data*_${i}.root
done

