import uproot
import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split, GridSearchCV
from xgboost import XGBClassifier
from sklearn.metrics import classification_report, confusion_matrix

# === Configuration ===
files = {
    "signal": "signal_8_input.root",
    "ttbar": "ttbar_input.root",
    "wjets": "wjets_input.root"
}
tree_name = "outputTree"
features = ["top_mass", "specJet_leading_eta", "bdt_delR", "bdt_deltaEta", 
            "Wboson_transversMass", "bdt_WHelicity", "bdt_eventShape", "MET_pt_corr"]

# === Load Data ===
def load_root_file(file_path, label):
    print(f"Loading file: {file_path} with label {label}")
    with uproot.open(file_path)[tree_name] as tree:
        data = tree.arrays(features, library="pd")
        data["label"] = label
    print(f"Loaded {len(data)} events from {file_path}")
    return data

print("=== Starting Data Loading ===")
df_signal = load_root_file(files["signal"], label=0)
df_ttbar  = load_root_file(files["ttbar"], label=1)
df_wjets  = load_root_file(files["wjets"], label=2)

print("Concatenating and shuffling data...")
df = pd.concat([df_signal, df_ttbar, df_wjets], ignore_index=True).sample(frac=1).reset_index(drop=True)
print(f"Total events after combining: {len(df)}")

# DO NOT drop NaNs — XGBoost can handle them
print("Preview of NaN values per feature:")
print(df[features].isna().sum())

# === Prepare Data ===
print("Splitting into train/test sets...")
X = df[features]
y = df["label"]
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
print(f"Training size: {len(X_train)}, Testing size: {len(X_test)}")

# === Grid Search for BDT ===
print("Setting up parameter grid for GridSearchCV...")
param_grid = {
    'n_estimators': [100, 200],
    'max_depth': [3, 5, 7],
    'learning_rate': [0.01, 0.1, 0.2],
    'subsample': [0.8, 1.0],
    'colsample_bytree': [0.8, 1.0]
}

xgb = XGBClassifier(
    objective='multi:softprob',
    num_class=3,
    eval_metric='mlogloss',
    use_label_encoder=False,
    missing=np.nan  # optional, just being explicit
)

print("Starting Grid Search...")
grid_search = GridSearchCV(
    estimator=xgb,
    param_grid=param_grid,
    scoring='accuracy',
    cv=3,
    verbose=1,  # XGBoost internal progress
    n_jobs=-1
)

grid_search.fit(X_train, y_train)
print("Grid search completed!")

# === Best Model Evaluation ===
print("Evaluating best model...")
best_model = grid_search.best_estimator_
y_pred = best_model.predict(X_test)

print("\n=== Best Parameters Found ===")
print(grid_search.best_params_)

print("\n=== Classification Report ===")
print(classification_report(y_test, y_pred, target_names=["signal", "ttbar", "wjets"]))

print("\n=== Confusion Matrix ===")
print(confusion_matrix(y_test, y_pred))

# === Save Best Model ===
model_path = "best_multiclass_bdt_model.json"
best_model.save_model(model_path)
print(f"\nBest model saved to: {model_path}")

