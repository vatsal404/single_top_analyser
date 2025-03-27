import uproot
import numpy as np
import pandas as pd
from sklearn.metrics import roc_auc_score, roc_curve
import matplotlib.pyplot as plt

def read_bdt_variable(root_file, tree_name, bdt_variable):
    """Read a BDT variable from a ROOT file and replace NaN values with -999."""
    with uproot.open(root_file) as file:
        tree = file[tree_name]
        values = tree[bdt_variable].array(library="np")
        values = np.nan_to_num(values, nan=-999)  # Replace NaN with -999
        return values

def compute_auc(file_signal, file_background, tree_name, bdt_variable):
    """Calculate AUC between two files for a given BDT variable."""
    # Read BDT variable from both files
    signal_values = read_bdt_variable(file_signal, tree_name, bdt_variable)
    background_values = read_bdt_variable(file_background, tree_name, bdt_variable)

    # Create labels (1 for signal, 0 for background)
    y_true = np.concatenate([np.ones(len(signal_values)), np.zeros(len(background_values))])
    y_scores = np.concatenate([signal_values, background_values])

    # Compute AUC
    auc_score = roc_auc_score(y_true, y_scores)

    # Plot ROC Curve
    fpr, tpr, _ = roc_curve(y_true, y_scores)
    plt.figure()
    plt.plot(fpr, tpr, label=f"AUC = {auc_score:.3f}")
    plt.plot([0, 1], [0, 1], linestyle="--", color="gray")
    plt.xlabel("False Positive Rate")
    plt.ylabel("True Positive Rate")
    plt.title(f"ROC Curve for {bdt_variable}")
    plt.legend()
    plt.show()

    return auc_score

# Example usage
file_signal = "signal.root"
file_background = "TTBAR.root"
tree_name = "outputTree"  # Change this if needed
bdt_variable = "bdt_eventShape"  # Replace with your BDT variable name

auc = compute_auc(file_signal, file_background, tree_name, bdt_variable)
print(f"AUC for {bdt_variable}: {auc:.3f}")

