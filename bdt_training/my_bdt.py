import numpy as np
import pandas as pd
import xgboost as xgb
from sklearn.model_selection import train_test_split
from sklearn.metrics import roc_auc_score, roc_curve
from scipy.stats import ks_2samp
import uproot  # For reading ROOT files
import matplotlib.pyplot as plt  # For plotting

# Load signal and background data from ROOT files
signal_file = uproot.open('signal_muon.root')  # Replace with your signal file path
background_file = uproot.open('background_muon.root')  # Replace with your background file path

# Assuming the data is stored in a TTree named 'tree'
signal_tree = signal_file['outputTree']  # Replace 'tree' with the actual TTree name
background_tree = background_file['outputTree']  # Replace 'tree' with the actual TTree name

# Convert the TTrees to Pandas DataFrames
signal_data = signal_tree.arrays(library='pd')
background_data = background_tree.arrays(library='pd')

# Add labels
signal_data['label'] = 1
background_data['label'] = 0

# Combine the datasets
data = pd.concat([signal_data, background_data], ignore_index=True)

# Check class distribution in the combined dataset
print("Class distribution in the combined dataset:")
print(data['label'].value_counts())

# Check for NaN or infinite values in features
print("Checking for NaN or infinite values in features:")
print(data.isnull().sum())  # Count of NaN values in each column
print(np.isinf(data.select_dtypes(include=[np.number])).sum())  # Count of infinite values in numeric columns

# Ensure the label column is of integer type
data['label'] = data['label'].astype(int)

# Split the data into features and labels
X = data.drop('label', axis=1)
y = data['label']

# Split the data into training and testing sets with stratification
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42, stratify=y)

# Check class distribution in the test set
print("Class distribution in the test set:")
print(y_test.value_counts())

# Create DMatrix for XGBoost
dtrain = xgb.DMatrix(X_train, label=y_train, missing=np.nan)
dtest = xgb.DMatrix(X_test, label=y_test, missing=np.nan)

# Set XGBoost parameters
params = {
    'objective': 'binary:logistic',
    'eval_metric': 'logloss',
    'max_depth': 3,
    'eta': 0.1,
    'subsample': 0.7,
    'colsample_bytree': 0.5,
    'seed': 42,
    'missing': np.nan,  # Handle NaN values
}

# Train the model with evaluation metrics
evals_result = {}  # To store evaluation results
num_round = 1000
bst = xgb.train(params, dtrain, num_round, evals=[(dtrain, 'train'), (dtest, 'test')], evals_result=evals_result, early_stopping_rounds=10, verbose_eval=10)

# Extract training and test loss
train_loss = evals_result['train']['logloss']
test_loss = evals_result['test']['logloss']

# Plot training and test loss
plt.figure(figsize=(10, 6))
plt.plot(train_loss, label='Training Loss')
plt.plot(test_loss, label='Test Loss')
plt.xlabel('Iteration')
plt.ylabel('Log Loss')
plt.title('Training and Test Loss Over Iterations')
plt.legend()
plt.savefig('loss_curve.png')  # Save the plot as an image file
plt.show()

# Make predictions
y_pred = bst.predict(dtest)

# Calculate ROC AUC score (only if both classes are present in y_test)
if len(np.unique(y_test)) > 1:
    roc_auc = roc_auc_score(y_test, y_pred)
    print(f'ROC AUC Score: {roc_auc}')
else:
    print("ROC AUC score is not defined because only one class is present in y_test.")

# Calculate KS statistic
fpr, tpr, _ = roc_curve(y_test, y_pred)
ks_statistic = ks_2samp(tpr, fpr).statistic
print(f'KS Statistic: {ks_statistic}')

# Print the KS test for signal and background
signal_pred = bst.predict(xgb.DMatrix(signal_data.drop('label', axis=1), missing=np.nan))
background_pred = bst.predict(xgb.DMatrix(background_data.drop('label', axis=1), missing=np.nan))

ks_test = ks_2samp(signal_pred, background_pred)
print(f'KS Test for Signal and Background: {ks_test}')

# Save the model if needed
bst.save_model('xgboost_model.model')
