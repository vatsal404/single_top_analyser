#!/bin/bash

INPUT_DIR="/eos/uscms/store/user/vsinha/Result_2022/"
OUTPUT_DIR="$(pwd)/merged"

# Optional index passed as first argument
INDEX="$1"

# Create merged directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# 🔹 Clean old merged ROOT files
echo "Cleaning old merged files in $OUTPUT_DIR"
rm -f "$OUTPUT_DIR"/*.root

cd "$INPUT_DIR" || exit 1

if [[ -n "$INDEX" ]]; then
    echo "Running in INDEXED mode (index = $INDEX)"

    # process_bXXXX_INDEX.root → process
    processes=$(ls *_${INDEX}.root 2>/dev/null \
        | sed -E "s/_b[0-9]+_${INDEX}\.root$//" \
        | sort -u)

    for proc in $processes; do
        echo "Merging ${proc} (index ${INDEX})"
        hadd -f "${OUTPUT_DIR}/${proc}.root" ${proc}_b*_${INDEX}.root
    done

else
    echo "Running in NON-INDEXED mode"

    # process_bXXXX.root → process
    processes=$(ls *.root 2>/dev/null \
        | sed -E 's/_b[0-9]+\.root$//' \
        | sort -u)

    for proc in $processes; do
        echo "Merging ${proc}"
        hadd -f "${OUTPUT_DIR}/${proc}.root" ${proc}_b*.root
    done
fi

# Merge all Data samples (both modes)
echo "Merging data samples"
hadd -f "${OUTPUT_DIR}/data.root" "${OUTPUT_DIR}"/Data*.root 2>/dev/null

echo "Done. Merged files are in $OUTPUT_DIR"
