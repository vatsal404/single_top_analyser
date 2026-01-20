#!/bin/bash

INPUT_DIR="/eos/uscms/store/user/vsinha/Result_2022/good_result/"
OUTPUT_DIR="$(pwd)/merged"

# Optional index argument
INDEX="$1"   # empty if not passed

mkdir -p "$OUTPUT_DIR"
cd "$INPUT_DIR" || exit 1

if [[ -z "$INDEX" ]]; then
    echo "No index provided → merging all chunks of each dataset"

    # remove _bXXXX.root
    processes=$(ls *.root | sed -E 's/_b[0-9]+\.root$//' | sort | uniq)

    for proc in $processes; do
        echo "Merging ${proc}"
        hadd -f "${OUTPUT_DIR}/${proc}.root" ${proc}_b*.root
    done

else
    echo "Index provided = ${INDEX} → merging only files with this index"

    # remove _bXXXX_INDEX.root
    processes=$(ls *_${INDEX}.root | sed -E "s/_b[0-9]+_${INDEX}\.root$//" | sort | uniq)

    for proc in $processes; do
        echo "Merging ${proc} (index ${INDEX})"
        hadd -f "${OUTPUT_DIR}/${proc}.root" ${proc}_b*_${INDEX}.root
    done
fi

echo "Done. Merged files are in $OUTPUT_DIR"

