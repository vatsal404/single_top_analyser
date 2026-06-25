#!/bin/bash

############################
# User inputs
############################

# 🔴 IMPORTANT: set full path
JOB_LIST="sample_list/sample_list_split.txt"
LOG_DIR="./2022EE_F_syst/logs/"
PARENT_DIR="${LOG_DIR%/}/../"

CRASH_LIST="crashed_jobs.txt"
MISSING_LIST="missing_jobs.txt"
SMALL_LIST="small_jobs.txt"

############################
# Safety checks
############################

if [[ ! -f "$JOB_LIST" ]]; then
    echo "ERROR: JOB_LIST not found at $JOB_LIST"
    exit 1
fi

############################
# Clean outputs
############################

> "$CRASH_LIST"
> "$MISSING_LIST"
> "$SMALL_LIST"

############################
# Counters
############################

crash_count=0
missing_count=0
small_count=0

############################
# Debug info
############################

echo "Checking .out files inside: $LOG_DIR"
echo "Checking .root files in   : $PARENT_DIR"
echo

############################
# 1) Find crashed jobs
############################

for outfile in "$LOG_DIR"/*.out; do
    [ -e "$outfile" ] || continue
    
    if grep -q "There was a crash." "$outfile" || \
       grep -qi "traceback" "$outfile" || \
       grep -qi "quitting" "$outfile"; then
        base_out=$(basename "$outfile")
        
        match=$(grep -F "$base_out" "$JOB_LIST")
        
        if [[ -n "$match" ]]; then
            echo "$match" >> "$CRASH_LIST"
            ((crash_count++))
        fi
    fi
done

############################
# 2) Find missing jobs
############################

while read -r line; do
    out_file=$(echo "$line" | grep -o '[^ ]*\.out')

    if [[ -n "$out_file" && ! -f "$LOG_DIR/$out_file" ]]; then
        echo "$line" >> "$MISSING_LIST"
        ((missing_count++))
    fi
done < "$JOB_LIST"

############################
# 3) Find small ROOT files (<7 KB)
############################

while read -r line; do
    root_file=$(echo "$line" | grep -o '[^ ]*\.root')

    if [[ -n "$root_file" ]]; then
        full_path="${PARENT_DIR}${root_file}"

        if [[ -f "$full_path" ]]; then
            size=$(stat -c%s "$full_path")

            # 7 KB = 7168 bytes
            if (( size < 7168 )); then
                echo "$line" >> "$SMALL_LIST"
                ((small_count++))
            fi
        fi
    fi
done < "$JOB_LIST"

############################
# Summary
############################

echo "-----------------------------------"
echo "Crashed jobs : $crash_count"
echo "Missing jobs : $missing_count"
echo "Small files  : $small_count"
echo "-----------------------------------"

echo "Crashed jobs saved to : $CRASH_LIST"
echo "Missing jobs saved to : $MISSING_LIST"
echo "Small jobs saved to   : $SMALL_LIST"
