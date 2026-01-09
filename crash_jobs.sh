#!/bin/bash

JOB_LIST="sample_list_split.txt"

CRASH_LIST="crashed_jobs.txt"
MISSING_LIST="missing_jobs.txt"

# Clean old outputs
> "$CRASH_LIST"
> "$MISSING_LIST"

############################
# 1) Find crashed jobs
############################
for outfile in *.out; do
    if grep -q "There was a crash." "$outfile"; then
        match=$(grep -F "$outfile" "$JOB_LIST")
        if [[ -n "$match" ]]; then
            echo "$match" >> "$CRASH_LIST"
        fi
    fi
done

############################
# 2) Find missing jobs
############################
while read -r line; do
    # extract .out filename from the job list line
    out_file=$(echo "$line" | grep -o '[^ ]*\.out')

    # if .out file is listed but does not exist locally → missing
    if [[ -n "$out_file" && ! -f "$out_file" ]]; then
        echo "$line" >> "$MISSING_LIST"
    fi
done < "$JOB_LIST"

############################
echo "Crashed jobs saved to  : $CRASH_LIST"
echo "Missing jobs saved to  : $MISSING_LIST"

