#!/usr/bin/env python3
"""
Training script for Malicious Packet Detection Model
Run this to generate the malicious_packet_model.pkl file
"""
import pandas as pd
import numpy as np
import joblib
import os
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import RobustScaler
import warnings
warnings.filterwarnings('ignore')

class MaliciousPacketDetector:
    def __init__(self):
        self.scaler = RobustScaler()
        self.isolation_forest = None
        self.feature_names = None
        self.is_trained = False
        self.training_stats = {}
        
    def train_model(self, csv_file_path, contamination=0.05):
        """Train the Isolation Forest model on network packet data"""
        print("="*70)
        print("TRAINING MALICIOUS PACKET DETECTION MODEL")
        print("="*70)
        print(f"\n[1/6] Loading dataset from: {csv_file_path}")
        
        if not os.path.exists(csv_file_path):
            print(f"[ERROR] Dataset file not found: {csv_file_path}")
            print("\nPlease ensure you have the network_features.csv file in the correct location.")
            return None
        
        df = pd.read_csv(csv_file_path)
        
        print(f"[INFO] Dataset shape: {df.shape}")
        print(f"[INFO] Columns: {list(df.columns)[:10]}... (showing first 10)")
        
        # Separate features from labels
        print("\n[2/6] Preprocessing data...")
        label_columns = ['label', 'class', 'attack_type', 'target']
        true_labels = None
        
        for col in label_columns:
            if col in df.columns:
                true_labels = df[col]
                df = df.drop(col, axis=1)
                print(f"[INFO] Found and removed label column: {col}")
                break
        
        # Remove non-feature columns
        non_feature_cols = ['timestamp', 'window_start', 'id', 'flow_id']
        for col in non_feature_cols:
            if col in df.columns:
                df = df.drop(col, axis=1)
                print(f"[INFO] Removed non-feature column: {col}")
        
        # Handle missing values
        df = df.fillna(0)
        
        # Store feature names and training statistics
        self.feature_names = df.columns.tolist()
        
        print(f"\n[3/6] Computing feature statistics...")
        self.training_stats = {}
        for col in df.columns:
            self.training_stats[col] = {
                'mean': df[col].mean(),
                'std': df[col].std(),
                'min': df[col].min(),
                'max': df[col].max(),
                'median': df[col].median(),
                'q25': df[col].quantile(0.25),
                'q75': df[col].quantile(0.75),
                'unique_values': df[col].nunique()
            }
        
        print(f"[INFO] Using {len(self.feature_names)} features for training")
        
        # Scale features
        print(f"\n[4/6] Scaling features...")
        X_scaled = self.scaler.fit_transform(df)
        
        # Train Isolation Forest
        print(f"\n[5/6] Training Isolation Forest model...")
        print(f"[INFO] Contamination rate: {contamination}")
        print(f"[INFO] This may take a few minutes...")
        
        self.isolation_forest = IsolationForest(
            contamination=contamination,
            random_state=42,
            n_estimators=200,
            max_samples='auto',
            max_features=1.0,
            verbose=0
        )
        
        predictions = self.isolation_forest.fit_predict(X_scaled)
        scores = self.isolation_forest.score_samples(X_scaled)
        
        self.is_trained = True
        
        # Calculate results
        print(f"\n[6/6] Training completed! Calculating metrics...")
        malicious_count = np.sum(predictions == -1)
        benign_count = np.sum(predictions == 1)
        total_samples = len(predictions)
        
        results = {
            'total_samples': total_samples,
            'malicious_detected': malicious_count,
            'benign_detected': benign_count,
            'malicious_percentage': (malicious_count / total_samples) * 100,
            'training_completed': True
        }
        
        print("\n" + "="*70)
        print("TRAINING RESULTS")
        print("="*70)
        print(f"Total samples: {total_samples:,}")
        print(f"Malicious detected: {malicious_count:,} ({results['malicious_percentage']:.2f}%)")
        print(f"Benign detected: {benign_count:,}")
        print(f"Features used: {len(self.feature_names)}")
        print("="*70 + "\n")
        
        return results
    
    def save_model(self, filepath):
        """Save the trained model to a file"""
        if not self.is_trained:
            raise ValueError("No trained model to save!")
        
        # Ensure directory exists
        os.makedirs(os.path.dirname(filepath), exist_ok=True)
        
        model_data = {
            'isolation_forest': self.isolation_forest,
            'scaler': self.scaler,
            'feature_names': self.feature_names,
            'training_stats': self.training_stats,
            'is_trained': self.is_trained
        }
        
        joblib.dump(model_data, filepath)
        print(f"[SUCCESS] Model saved to: {filepath}")
        print(f"[INFO] Model size: {os.path.getsize(filepath) / (1024*1024):.2f} MB")


def main():
    """Main training function"""
    print("\n" + "="*70)
    print(" MALICIOUS PACKET DETECTION - MODEL TRAINING SCRIPT ")
    print("="*70 + "\n")
    
    # Configuration - paths relative to backend directory
    SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
    DATASET_PATH = os.path.join(SCRIPT_DIR, 'network_features.csv')
    MODEL_OUTPUT_PATH = os.path.join(SCRIPT_DIR, 'models', 'malicious_packet_model.pkl')
    CONTAMINATION = 0.05  # 5% of data expected to be anomalous
    
    print(f"Configuration:")
    print(f"  Dataset: {DATASET_PATH}")
    print(f"  Output: {MODEL_OUTPUT_PATH}")
    print(f"  Contamination: {CONTAMINATION}")
    print()
    
    # Check if dataset exists
    if not os.path.exists(DATASET_PATH):
        print(f"[ERROR] Dataset not found: {DATASET_PATH}")
        print("\nPlease ensure you have the network_features.csv file.")
        print("You can download sample datasets from:")
        print("  - https://www.kaggle.com/datasets (search for 'network intrusion')")
        print("  - https://www.unb.ca/cic/datasets/")
        return
    
    # Initialize detector
    detector = MaliciousPacketDetector()
    
    # Train the model
    training_results = detector.train_model(DATASET_PATH, contamination=CONTAMINATION)
    
    if training_results is None:
        print("\n[FAILED] Training failed. Please check the error messages above.")
        return
    
    # Save the trained model
    print("\nSaving model...")
    detector.save_model(MODEL_OUTPUT_PATH)
    
    print("\n" + "="*70)
    print(" TRAINING COMPLETE! ")
    print("="*70)
    print(f"\n✓ Model file created: {MODEL_OUTPUT_PATH}")
    print(f"✓ Ready to use for predictions")
    print(f"\nYou can now:")
    print(f"  1. Start your Flask server: python app.py")
    print(f"  2. Test predictions via API: POST to /predict/malicious-packet")
    print(f"  3. Use Postman to test the endpoint")
    print("="*70 + "\n")


if __name__ == "__main__":
    main()