#!/bin/bash

INPUT_DIR="/eos/uscms/store/user/vsinha/Result_2023/cutflow"
OUTPUT_DIR="$(pwd)/merged"

# choose which index to merge (0,1,2,3,4...)
INDEX=1

mkdir -p "$OUTPUT_DIR"
cd "$INPUT_DIR" || exit

# Extract process names by removing _bXXXX_INDEX.root
processes=$(ls *_${INDEX}.root | sed -E "s/_b[0-9]+_${INDEX}\.root$//" | sort | uniq)

for proc in $processes; do
    echo "Merging ${proc} for index ${INDEX}"
    hadd -f "${OUTPUT_DIR}/${proc}.root" ${proc}_b*_${INDEX}.root
done

echo "Done. Merged files are in $OUTPUT_DIR"

