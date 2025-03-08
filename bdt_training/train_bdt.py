import uproot
import pandas as pd
import numpy as np
import xgboost as xgb
from sklearn.model_selection import train_test_split
from sklearn.metrics import roc_auc_score, classification_report, confusion_matrix

# === Configuration ===
signal_files = ["/uscms/home/vsinha/nobackup/CMSSW_12_3_4/src/fly/signal.root"]   # Add all your signal files
background_files = ["/uscms/home/vsinha/nobackup/CMSSW_12_3_4/src/fly/tt_bar.root"]

tree_name = "outputTree"  # Update with correct TTree name
variables = [

         "bdt_wboson_muon_2j1t",
          "bdt_top_muon_2j1t",
           "bdt_specJet_2j1t_leading_eta_muon",
            "bdt_delR_muon_2j1t",
             "bdt_deltaEta_muon_2j1t",
               "bdt_eventShape_muon_2j1t",
                "bdt_WHelicity_muon_2j1t",
                 "bdt_MET_pt_muon_2j1t"
        ]

# Function to read ROOT files into pandas DataFrame
def read_root_files(files, tree_name, variables):
    df_list = []
    for file in files:
        with uproot.open(file) as f:
            tree = f[tree_name]
            df = tree.arrays(variables, library="pd")
            df_list.append(df)
    return pd.concat(df_list, ignore_index=True)

# === Load Signal & Background ===
signal_df = read_root_files(signal_files, tree_name, variables)
background_df = read_root_files(background_files, tree_name, variables)

# === Drop Rows Containing -999 ===
#signal_df = signal_df.replace(-999, np.nan).dropna()  # Remove all events where -999 was present
#background_df = background_df.replace(-999, np.nan).dropna()

mask = (signal_df[variables] == -999).any(axis=1)
signal_df = signal_df[~mask]

mask_bg = (background_df[variables] == -999).any(axis=1)
background_df = background_df[~mask_bg]



# === Add Labels ===
signal_df["label"] = 1
background_df["label"] = 0

# === Combine and Split ===
df = pd.concat([signal_df, background_df], ignore_index=True)

print(df.isna().sum())  # This will show you how many NaNs are there in each column

# Split features and labels
X = df[variables]
y = df["label"]

# === Train-Test Split ===
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=42)

# === XGBoost DMatrix (with NaN handling) ===
dtrain = xgb.DMatrix(X_train, label=y_train, missing=np.nan)
dtest = xgb.DMatrix(X_test, label=y_test, missing=np.nan)

# === Train XGBoost BDT ===
params = {
    "objective": "binary:logistic",
    "eval_metric": "auc",
    "max_depth": 4,
    "learning_rate": 0.1,
#    "num_boost_round": 100
}

bst = xgb.train(params, dtrain, num_boost_round=100)

# === Predict & Evaluate ===
y_pred_proba = bst.predict(dtest)
y_pred = (y_pred_proba > 0.5).astype(int)

print(f"AUC Score: {roc_auc_score(y_test, y_pred_proba):.4f}")
print("\nClassification Report:\n", classification_report(y_test, y_pred))
print("\nConfusion Matrix:\n", confusion_matrix(y_test, y_pred))

# Optionally, save the model
bst.save_model("bdt_model.json")

