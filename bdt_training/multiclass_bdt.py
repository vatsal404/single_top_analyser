import uproot
import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from xgboost import XGBClassifier
from sklearn.metrics import classification_report, confusion_matrix

# === Configuration ===
# Replace with your actual ROOT file paths and tree names
files = {
    "signal": "signal_8_input.root",
    "ttbar": "ttbar_pure_input.root",
    "wjets": "wjets_pure_input.root"
}
tree_name = "outputTree"  # or whatever your tree is called

# List of variables to use for classification
features = ["top_mass", "specJet_leading_eta", "bdt_delR", "bdt_deltaEta", "Wboson_transversMass","bdt_WHelicity","bdt_eventShape", "MET_pt_corr"] 

# === Load Data ===
def load_root_file(file_path, label):
    with uproot.open(file_path)[tree_name] as tree:
        data = tree.arrays(features, library="pd")
        data["label"] = label
    return data

# Load all classes
df_signal = load_root_file(files["signal"], label=0)
df_ttbar  = load_root_file(files["ttbar"], label=1)
df_wjets  = load_root_file(files["wjets"], label=2)

# Concatenate and shuffle
df = pd.concat([df_signal, df_ttbar, df_wjets], ignore_index=True).sample(frac=1).reset_index(drop=True)

# Handle missing or awkward values (optional)
df = df.dropna()

# === Prepare Data ===
X = df[features]
y = df["label"]

# Train-test split
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
model = XGBClassifier(
    objective='multi:softprob',
    num_class=5,
    eval_metric='mlogloss',
    learning_rate=0.05,
    max_depth=5,
    subsample=0.8,
    colsample_bytree=0.8,
    gamma=0.1,
    reg_alpha=0.1,
    reg_lambda=1,
    n_estimators=800,
    early_stopping_rounds=50,
    use_label_encoder=False
)
model.fit(X_train, y_train, eval_set=[(X_test, y_test)], verbose=True)
# === Train BDT ===

# === Evaluate ===
y_pred = model.predict(X_test)
print("Classification Report:")
print(classification_report(y_test, y_pred, target_names=["signal", "ttbar", "wjets"]))
print("Confusion Matrix:")
print(confusion_matrix(y_test, y_pred))

# === Optional: Save model ===
model.save_model("multiclass_bdt_model.json")

