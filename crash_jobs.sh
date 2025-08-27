#!/bin/bash

# Input list containing all jobs
JOB_LIST="sample_list_split.txt"
# Output list for crashed jobs
CRASH_LIST="crashed_jobs.txt"

# Remove previous crash list if exists
> "$CRASH_LIST"

# Loop through each .out file in the current directory
for outfile in *.out; do
    if grep -q "There was a crash." "$outfile"; then
        # Extract the line in JOB_LIST that contains this .out file
        match=$(grep "$outfile" "$JOB_LIST")
        if [[ -n "$match" ]]; then
            echo "$match" >> "$CRASH_LIST"
        fi
    fi
done

echo "Crashed jobs have been saved to $CRASH_LIST"

