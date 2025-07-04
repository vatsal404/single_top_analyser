import numpy as np
import pandas as pd
import uproot
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.metrics import (roc_auc_score, roc_curve, precision_score, 
                            recall_score, f1_score, confusion_matrix)
from scipy.stats import ks_2samp
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, Dropout, BatchNormalization
from tensorflow.keras.callbacks import EarlyStopping
from tensorflow.keras.optimizers import Adam
from tensorflow.keras.regularizers import l1_l2
from sklearn.preprocessing import StandardScaler

# Load and prepare data (same as before)
signal_file = uproot.open('ttbar_muon.root')
background_file = uproot.open('wjets_muon.root')
signal_tree = signal_file['outputTree']
background_tree = background_file['outputTree']

signal_data = signal_tree.arrays(library='pd')
background_data = background_tree.arrays(library='pd')

signal_data['label'] = 1
background_data['label'] = 0

data = pd.concat([signal_data, background_data], ignore_index=True)
print("Class distribution:", data['label'].value_counts())

# Handle missing values
data.fillna(data.mean(), inplace=True)

X = data.drop('label', axis=1)
y = data['label'].values

# Standardize features
scaler = StandardScaler()
X_scaled = scaler.fit_transform(X)

# Split data
X_train, X_test, y_train, y_test = train_test_split(
    X_scaled, y, test_size=0.2, random_state=42, stratify=y
)

# Calculate class weights for imbalanced data
class_weight = {
    0: len(y_train)/(2*np.bincount(y_train)[0]),
    1: len(y_train)/(2*np.bincount(y_train)[1])
}

# Build DNN model
model = Sequential([
    Dense(64, activation='relu', input_shape=(X_train.shape[1],)),  # <-- Add missing closing parenthesis
    BatchNormalization(),
    Dropout(0.3),
    Dense(32, activation='relu', kernel_regularizer=l1_l2(l1=0.01, l2=0.01)),
    BatchNormalization(),
    Dropout(0.3),
    Dense(16, activation='relu'),
    Dense(1, activation='sigmoid')
])

optimizer = Adam(learning_rate=0.001)
model.compile(optimizer=optimizer,
              loss='binary_crossentropy',
              metrics=['accuracy'])

# Train with early stopping
early_stop = EarlyStopping(monitor='val_loss', patience=20, restore_best_weights=True)
history = model.fit(
    X_train, y_train,
    validation_data=(X_test, y_test),
    epochs=200,
    batch_size=256,
    class_weight=class_weight,
    callbacks=[early_stop],
    verbose=1
)

# Plot training history
plt.figure(figsize=(12, 5))
plt.subplot(1, 2, 1)
plt.plot(history.history['loss'], label='Train Loss')
plt.plot(history.history['val_loss'], label='Validation Loss')
plt.xlabel('Epoch')
plt.ylabel('Loss')
plt.legend()

plt.subplot(1, 2, 2)
plt.plot(history.history['accuracy'], label='Train Accuracy')
plt.plot(history.history['val_accuracy'], label='Validation Accuracy')
plt.xlabel('Epoch')
plt.ylabel('Accuracy')
plt.legend()
plt.tight_layout()
plt.savefig('dnn_training_history.png')
plt.show()

# Make predictions
y_train_pred = model.predict(X_train).flatten()
y_pred = model.predict(X_test).flatten()

# Plot discriminant distributions
plt.figure(figsize=(12, 8))

plt.subplot(2, 1, 1)
plt.hist(y_train_pred[y_train == 1], bins=50, alpha=0.5, label='Signal (Train)', density=True)
plt.hist(y_train_pred[y_train == 0], bins=50, alpha=0.5, label='Background (Train)', density=True)
plt.xlabel('DNN Output')
plt.ylabel('Density')
plt.title('Training Set DNN Discriminant Distribution')
plt.legend()
plt.grid(True)

plt.subplot(2, 1, 2)
plt.hist(y_pred[y_test == 1], bins=50, alpha=0.5, label='Signal (Test)', density=True)
plt.hist(y_pred[y_test == 0], bins=50, alpha=0.5, label='Background (Test)', density=True)
plt.xlabel('DNN Output')
plt.ylabel('Density')
plt.title('Test Set DNN Discriminant Distribution')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('dnn_discriminant_distributions.png')
plt.show()

# Calculate metrics
y_pred_binary = (y_pred >= 0.5).astype(int)

print("\nPerformance Metrics:")
print(f"Precision: {precision_score(y_test, y_pred_binary):.4f}")
print(f"Recall: {recall_score(y_test, y_pred_binary):.4f}")
print(f"F1 Score: {f1_score(y_test, y_pred_binary):.4f}")

if len(np.unique(y_test)) > 1:
    roc_auc = roc_auc_score(y_test, y_pred)
    print(f"ROC AUC: {roc_auc:.4f}")
    
    fpr, tpr, _ = roc_curve(y_test, y_pred)
    ks_statistic = ks_2samp(tpr, fpr).statistic
    print(f"KS Statistic: {ks_statistic:.4f}")

# KS test between signal and background
signal_pred = model.predict(scaler.transform(signal_data.drop('label', axis=1))).flatten()
background_pred = model.predict(scaler.transform(background_data.drop('label', axis=1))).flatten()
ks_test = ks_2samp(signal_pred, background_pred)
print(f"\nKS Test for Signal and Background: {ks_test}")

# Save model
model.save('dnn_model.h5')
