#!/bin/bash

# Directory where your ROOT files are stored
INPUT_DIR="."
OUTPUT_DIR="${INPUT_DIR}/merged"

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

cd "$INPUT_DIR" || exit

# Extract unique dataset prefixes
prefixes=$(ls *.root | sed -E 's/_b[0-9]+\.root$//' | sort | uniq)

# Loop through each prefix and merge files
for prefix in $prefixes; do
    echo "Merging files for dataset: $prefix"
    hadd -f "$OUTPUT_DIR/${prefix}.root" ${prefix}_b*.root
done
hadd -f $OUTPUT_DIR/data.root $OUTPUT_DIR/Data*.root
echo "Merging complete. All merged files are in: $OUTPUT_DIR"

