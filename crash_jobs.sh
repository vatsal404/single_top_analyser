#!/bin/bash

JOB_LIST="sample_list_split.txt"

CRASH_LIST="crashed_jobs.txt"
MISSING_LIST="missing_jobs.txt"

# 🔹 Set the directory containing .out files here
LOG_DIR="/eos/uscms/store/user/vsinha/Result_2023/logs/"

# Clean old outputs
> "$CRASH_LIST"
> "$MISSING_LIST"

echo "Checking .out files inside: $LOG_DIR"
echo

############################
# 1) Find crashed jobs
############################
for outfile in "$LOG_DIR"/*.out; do
    # Skip if no .out files exist
    [ -e "$outfile" ] || continue

    if grep -q "There was a crash." "$outfile"; then
        base_out=$(basename "$outfile")
        match=$(grep -F "$base_out" "$JOB_LIST")
        if [[ -n "$match" ]]; then
            echo "$match" >> "$CRASH_LIST"
        fi
    fi
done

############################
# 2) Find missing jobs
############################
while read -r line; do
    # Extract .out filename from job list line
    out_file=$(echo "$line" | grep -o '[^ ]*\.out')

    # If .out file does not exist inside LOG_DIR → missing
    if [[ -n "$out_file" && ! -f "$LOG_DIR/$out_file" ]]; then
        echo "$line" >> "$MISSING_LIST"
    fi
done < "$JOB_LIST"

############################
echo "Crashed jobs saved to  : $CRASH_LIST"
echo "Missing jobs saved to  : $MISSING_LIST"
