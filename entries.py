import os
import re

def extract_genweight_sum_from_file(file_path):
    """
    Extract the 'Sum of genWeights' from a given file.
    Returns the genWeight sum as a float if found, else returns None.
    """
    try:
        with open(file_path, 'r') as file:
            for line in file:
                # Look for the specific line containing 'Sum of genWeights'
                if "Sum of genWeights" in line:
                    # Use regular expression to extract the numerical value
                    match = re.search(r"Sum of genWeights = ([\d\.]+)", line)
                    if match:
                        return float(match.group(1))
    except Exception as e:
        print(f"Error reading file {file_path}: {e}")
    return None

def process_directory(directory_path):
    """
    Process all .out files in the given directory and calculate the sum of genWeights.
    """
    total_genweight_sum = 0.0
    file_count = 0

    # Loop through all files in the directory
    for filename in os.listdir(directory_path):
        if filename.endswith('.out') or filename.endswith('.txt'):
            file_path = os.path.join(directory_path, filename)
            genweight_sum = extract_genweight_sum_from_file(file_path)
            if genweight_sum is not None:
                total_genweight_sum += genweight_sum
                file_count += 1
                print(f"File: {filename}, Sum of genWeights: {genweight_sum}")

    if file_count > 0:
        print(f"\nTotal Sum of genWeights across {file_count} files: {total_genweight_sum}")
    else:
        print("No genWeights found in the .out files.")

# Example usage (use the current directory '.'):
process_directory("new_bdt_logs/")
