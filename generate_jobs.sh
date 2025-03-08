# Python dictionary of input-output pairs
input_output_pairs = {
    "/home/user/input1": "/home/user/output1",
    "/home/user/input2": "/home/user/output2",
}

jobconfig = "jobconfiganalysis_17.py"  # Replace with your job configuration
logfile_template = "processnanoaod_log_{}.txt"  # Log file for each job

# Generate the commands
with open("processnanoaod.py", "w") as f:
    f.write("#!/bin/bash\n")
    f.write("set -e\n")
    f.write("set -x\n\n")
    for i, (indir, outdir) in enumerate(input_output_pairs.items()):
        logfile = logfile_template.format(i)
        command = f"./processnanoaod {indir} {outdir} {jobconfig} {logfile} &\n"
        f.write(command)
    f.write("\nwait\n")

