#!/bin/bash

# Directory where your ROOT files are stored
INPUT_DIR="/eos/uscms/store/user/vsinha/Result_2022"

# Save merged outputs in a 'merged' directory inside *current directory*
OUTPUT_DIR="$(pwd)/merged"

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Go to the input directory
cd "$INPUT_DIR" || exit

# Extract unique dataset prefixes:
# Remove suffix: _b<digits>.root
prefixes=$(ls *.root | sed -E 's/_b[0-9]+\.root$//' | sort | uniq)

# Loop through each prefix and merge files
for prefix in $prefixes; do
    echo "Merging files for dataset: $prefix"
    hadd -f "${OUTPUT_DIR}/${prefix}.root" ${prefix}_b*.root
done

# Merge all data files separately (if needed)
echo "Merging Data*.root into data.root"
hadd -f "${OUTPUT_DIR}/data.root" ${OUTPUT_DIR}/Data*.root

echo "Merging complete. All merged files are in: $OUTPUT_DIR"

