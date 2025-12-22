import subprocess
import uproot
import json
from datetime import datetime

# === User Config ===
redirector = "root://cmsxrootd.fnal.gov/"  # LPC global redirector

# List of datasets extracted from your document (excluding data samples)
datasets = [
"/DYto2L-2Jets_MLL-10to50_TuneCP5_13p6TeV_amcatnloFXFX-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/DYto2L-2Jets_MLL-50_TuneCP5_13p6TeV_amcatnloFXFX-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/TTG-1Jets_PTG-100to200_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/TTG-1Jets_PTG-10to100_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/TTG-1Jets_PTG-200_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/TTLL_MLL-4to50_TuneCP5_13p6TeV_amcatnlo-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/TTLL_MLL-50_TuneCP5_13p6TeV_amcatnlo-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/TTLNu-1Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/TTZ-ZtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/TTto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/TTtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/TTtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_ext1/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/TWminusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/TbarWplusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/WWto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/WZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/WZto3LNu_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/WtoLNu-2Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/ZZto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/ZZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM",
"/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22EENanoAODv12-130X_mcRun3_2022_realistic_postEE_v6-v2/NANOAODSIM"
]

output_file = "genweight_results.txt"
json_output_file = "genweight_results.json"

def get_das_files(dataset):
    """Query DAS for list of files in the dataset."""
    try:
        cmd = ["dasgoclient", "-query", f"file dataset={dataset}"]
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=60)
        if result.returncode != 0:
            print(f"[Error] DAS query failed for {dataset}: {result.stderr}")
            return []
        files = result.stdout.strip().split("\n")
        return [f for f in files if f.strip()]
    except Exception as e:
        print(f"[Error] Exception during DAS query for {dataset}: {e}")
        return []

def convert_to_xrootd(files, redirector):
    """Convert DAS logical file names to full XRootD paths."""
    return [redirector + f for f in files]

def sum_genEventSumw(files, dataset_name):
    """Sum genEventSumw from the 'Runs' tree of each file."""
    total = 0.0
    successful_files = 0
    failed_files = 0
    
    print(f"\nProcessing {len(files)} files for {dataset_name}...")
    
    for i, file in enumerate(files):
        try:
            with uproot.open(file) as f:
                if "Runs" not in f:
                    print(f"[Warning] Runs tree not found in: {file}")
                    failed_files += 1
                    continue
                runs_tree = f["Runs"]
                if "genEventSumw" not in runs_tree.keys():
                    print(f"[Warning] genEventSumw not found in: {file}")
                    failed_files += 1
                    continue
                sumw = runs_tree["genEventSumw"].array(library="np").sum()
                total += sumw
                successful_files += 1
                if (i + 1) % 10 == 0 or i == len(files) - 1:
                    print(f"  Progress: {i+1}/{len(files)} files processed")
        except Exception as e:
            print(f"[Error] Failed to read {file}: {e}")
            failed_files += 1
    
    print(f"  Completed: {successful_files} successful, {failed_files} failed")
    return total

def get_dataset_short_name(dataset):
    """Extract a short name from the dataset path."""
    # Extract the process name from the dataset path
    parts = dataset.split('/')
    if len(parts) >= 2:
        return parts[1]  # This gets the process name part
    return dataset

def process_all_datasets():
    """Process all datasets and save results."""
    results = {}
    summary_lines = []
    
    print(f"Starting processing of {len(datasets)} datasets...")
    print(f"Timestamp: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print("="*80)
    
    for i, dataset in enumerate(datasets, 1):
        short_name = get_dataset_short_name(dataset)
        print(f"\n[{i}/{len(datasets)}] Processing: {short_name}")
        print(f"Full dataset: {dataset}")
        
        # Get files from DAS
        files = get_das_files(dataset)
        if not files:
            print(f"[Warning] No files found for {dataset}")
            results[short_name] = {
                'dataset': dataset,
                'total_genEventSumw': 0.0,
                'num_files': 0,
                'status': 'failed - no files found'
            }
            continue
            
        # Convert to XRootD paths
        files = convert_to_xrootd(files, redirector)
        print(f"Found {len(files)} files.")
        
        # Calculate total genEventSumw
        total_sumw = sum_genEventSumw(files, short_name)
        
        # Store results
        results[short_name] = {
            'dataset': dataset,
            'total_genEventSumw': total_sumw,
            'num_files': len(files),
            'status': 'success'
        }
        
        summary_line = f"{short_name}: {total_sumw:.2f} (from {len(files)} files)"
        summary_lines.append(summary_line)
        print(f"Result: {summary_line}")
        print("-" * 80)
    
    return results, summary_lines

def save_results(results, summary_lines):
    """Save results to both text and JSON files."""
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    
    # Save text file
    with open(output_file, 'w') as f:
        f.write(f"GenWeight Calculation Results\n")
        f.write(f"Generated on: {timestamp}\n")
        f.write(f"Total datasets processed: {len(results)}\n")
        f.write("="*80 + "\n\n")
        
        f.write("SUMMARY:\n")
        f.write("-"*40 + "\n")
        for line in summary_lines:
            f.write(line + "\n")
        f.write("\n")
        
        f.write("DETAILED RESULTS:\n")
        f.write("-"*40 + "\n")
        for short_name, data in results.items():
            f.write(f"\nDataset: {short_name}\n")
            f.write(f"Full path: {data['dataset']}\n")
            f.write(f"Total genEventSumw: {data['total_genEventSumw']:.6f}\n")
            f.write(f"Number of files: {data['num_files']}\n")
            f.write(f"Status: {data['status']}\n")
            f.write("-" * 60 + "\n")
    
    # Save JSON file
    json_results = {
        'metadata': {
            'timestamp': timestamp,
            'total_datasets': len(results),
            'redirector': redirector
        },
        'results': results
    }
    
    with open(json_output_file, 'w') as f:
        json.dump(json_results, f, indent=2)
    
    print(f"\nResults saved to:")
    print(f"  Text file: {output_file}")
    print(f"  JSON file: {json_output_file}")

# === Main Execution ===
if __name__ == "__main__":
    try:
        results, summary_lines = process_all_datasets()
        save_results(results, summary_lines)
        
        print("\n" + "="*80)
        print("FINAL SUMMARY:")
        print("="*80)
        for line in summary_lines:
            print(line)
        print("\nProcessing completed successfully!")
        
    except KeyboardInterrupt:
        print("\n\nProcessing interrupted by user.")
    except Exception as e:
        print(f"\n\nUnexpected error: {e}")
        import traceback
        traceback.print_exc()
