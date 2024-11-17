import subprocess

def is_valid_das_path(indir):
    """
    Check if the indir is a valid DAS path by querying dasgoclient.
    """
    command = f"dasgoclient --query='dataset={indir}'"
    try:
        output = subprocess.check_output(command, shell=True, universal_newlines=True)
        return bool(output.strip())
    except subprocess.CalledProcessError:
        return False

def get_root_file_paths(indir, xrootd_prefix="root://cmsxrootd.fnal.gov/"):
    """
    Function to retrieve ROOT file paths using dasgoclient, after checking if the DAS path is valid.
    """
    # Check if the DAS path is valid
    if not is_valid_das_path(indir):
        print(f"Invalid DAS path: {indir}")
        return []

    # Run dasgoclient command to get the list of files
    dataset_query = f"file dataset={indir}"
    command = f"dasgoclient --query='{dataset_query}'"
    try:
        output = subprocess.check_output(command, shell=True, universal_newlines=True)
    except subprocess.CalledProcessError as e:
        print(f"Error running dasgoclient: {e.output}")
        return []

    # Process the output and prepend the xrootd prefix
    files = output.strip().split('\n')
    root_files = [xrootd_prefix + f for f in files]

    return root_files

# Example usage
dataset_path = "/ST_t-channel_top_4f_inclusiveDecays_13TeV-powhegV2-madspin-pythia8_TuneCUETP8M1/RunIISummer16NanoAODv7-PUMoriond17_Nano02Apr2020_102X_mcRun2_asymptotic_v8-v2/NANOAODSIM"

l=get_root_file_paths(dataset_path)
for i in l:
    print(i,'/n')

