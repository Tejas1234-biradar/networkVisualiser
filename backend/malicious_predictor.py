import os
import joblib
import numpy as np

# Global model variables
_detector = None
_model_loaded = False

class SimpleMaliciousDetector:
    """Lightweight detector class for loading and using the trained model"""
    
    def __init__(self):
        self.isolation_forest = None
        self.scaler = None
        self.feature_names = None
        self.training_stats = {}
        self.is_trained = False
    
    def load_model(self, filepath):
        """Load a trained model from a file"""
        try:
            model_data = joblib.load(filepath)
            
            self.isolation_forest = model_data['isolation_forest']
            self.scaler = model_data['scaler']
            self.feature_names = model_data['feature_names']
            self.training_stats = model_data.get('training_stats', {})
            self.is_trained = model_data['is_trained']
            
            print(f"[INFO] Model loaded from {filepath}")
            print(f"[INFO] Features: {len(self.feature_names)}")
            return True
        except Exception as e:
            print(f"[ERROR] Failed to load model: {e}")
            return False
    
    def _generate_comprehensive_features(self, packet_size, duration, dst_port, protocol, syn_flag):
        """Generate all features from 5 input parameters"""
        features = {}
        
        # Direct mappings
        features['packet_size'] = packet_size
        features['duration'] = duration  
        features['dst_port'] = dst_port
        features['protocol'] = protocol
        features['syn_flag'] = syn_flag
        
        # Size & Volume Features
        features['avg_packet_size'] = packet_size
        features['bytes_sent'] = packet_size
        features['packets'] = 1
        
        avg_size = self.training_stats.get('packet_size', {}).get('mean', 500)
        features['packet_size_ratio'] = packet_size / (avg_size + 1e-6)
        features['small_packet_ratio'] = 1 if packet_size < 64 else 0
        features['large_packet_ratio'] = 1 if packet_size > 1400 else 0
        
        # Rate calculations
        features['packet_rate'] = 1.0 / (duration + 1e-6)
        features['packet_rate_anomaly'] = 1 if features['packet_rate'] > 100 else 0
        features['bytes_per_packet'] = packet_size
        features['packets_per_second'] = features['packet_rate']
        features['bytes_per_second'] = packet_size / (duration + 1e-6)
        
        # Payload analysis
        header_size = 40 if protocol == 6 else (28 if protocol == 17 else 20)
        payload_size = max(0, packet_size - header_size)
        features['payload_size_anomaly'] = 1 if payload_size > 1200 or payload_size == 0 else 0
        
        # Statistical features (Z-scores and outliers)
        for base_feature in ['packet_size', 'duration', 'packets', 'bytes_sent']:
            if base_feature in features and base_feature in self.training_stats:
                value = features[base_feature]
                mean = self.training_stats[base_feature]['mean']
                std = self.training_stats[base_feature]['std']
                zscore = abs((value - mean) / (std + 1e-6))
                features[f'{base_feature}_zscore'] = zscore
                features[f'{base_feature}_outlier'] = 1 if zscore > 3 else 0
        
        # TCP/UDP features
        if protocol == 6:  # TCP
            features['tcp_window_size'] = np.random.randint(1024, 65535)
            features['tcp_window_anomaly'] = 1 if features['tcp_window_size'] < 1024 or features['tcp_window_size'] > 32768 else 0
            features['udp_length_check'] = 0
        else:
            features['tcp_window_size'] = 0
            features['tcp_window_anomaly'] = 0
            features['udp_length_check'] = abs(packet_size - payload_size) if protocol == 17 else 0
        
        # Rate & Timing Features
        features['connection_frequency'] = 1
        
        # Protocol & Flags Features
        features['protocol_tcp'] = 1 if protocol == 6 else 0
        features['protocol_udp'] = 1 if protocol == 17 else 0  
        features['protocol_icmp'] = 1 if protocol == 1 else 0
        features['protocol_other'] = 1 if protocol not in [1, 6, 17] else 0
        
        # TCP flags
        features['tcp_flag_syn'] = syn_flag
        features['ack_flag'] = 0 if syn_flag == 1 else 1
        features['fin_flag'] = 0
        features['rst_flag'] = 0
        features['psh_flag'] = 1 if protocol == 6 and syn_flag == 0 else 0
        features['urg_flag'] = 0
        
        features['tcp_flag_syn_without_ack'] = 1 if syn_flag == 1 and features['ack_flag'] == 0 else 0
        features['tcp_flag_syn_without_ack_ratio'] = features['tcp_flag_syn_without_ack']
        
        # TCP flag combinations
        features['tcp_flags'] = syn_flag + features['ack_flag'] + features['fin_flag'] + features['rst_flag'] + features['psh_flag'] + features['urg_flag']
        features['tcp_flag_combinations'] = features['tcp_flags']
        features['suspicious_flag_combo'] = 1 if syn_flag == 1 and features['fin_flag'] == 1 else 0
        
        # Protocol analysis
        features['tcp_protocol'] = features['protocol_tcp']
        features['udp_protocol'] = features['protocol_udp'] 
        features['icmp_protocol'] = features['protocol_icmp']
        features['other_protocol'] = features['protocol_other']
        
        # Entropy calculations
        features['protocol_entropy'] = 0
        
        # ICMP features
        if protocol == 1:
            features['icmp_type'] = np.random.randint(0, 18)
            features['icmp_flood'] = 0
        else:
            features['icmp_type'] = -1
            features['icmp_flood'] = 0
        
        # Port & Connection Features
        features['unique_src_ports'] = 1
        features['src_port'] = np.random.randint(1024, 65535)
        
        features['common_port_access'] = 1 if dst_port in [22, 23, 80, 443, 21, 25, 53, 135, 139, 445] else 0
        features['high_port_access'] = 1 if dst_port > 1024 else 0
        features['sequential_port_pattern'] = 0
        features['repeated_connection_attempts'] = 1
        features['authentication_port'] = 1 if dst_port in [22, 23, 21, 993, 995] else 0
        
        # Scan detection
        features['port_scan_signals'] = 1 if syn_flag == 1 and features['ack_flag'] == 0 and dst_port > 1024 else 0
        features['null_scan'] = 1 if all(f == 0 for f in [syn_flag, features['ack_flag'], features['fin_flag'], features['rst_flag']]) else 0
        features['xmas_scan'] = 1 if features['fin_flag'] == 1 and features['psh_flag'] == 1 and features['urg_flag'] == 1 else 0
        
        # Connection patterns
        features['short_connection'] = 1 if duration < 0.1 else 0
        features['long_connection'] = 1 if duration > 300 else 0
        
        # Entropy for ports
        features['src_port_entropy'] = 0
        features['dst_port_entropy'] = 0
        
        # Statistical Features
        features['ttl'] = np.random.choice([64, 128, 255])
        features['ttl_anomaly'] = 1 if features['ttl'] not in [64, 128, 255] else 0
        
        # Other Features
        features['unique_src_ips'] = 1
        features['unique_dst_ips'] = 1
        features['syn_flood_indicator'] = 1 if syn_flag == 1 and features['ack_flag'] == 0 and duration < 0.1 else 0
        features['zero_payload'] = 1 if payload_size == 0 else 0
        features['low_ttl'] = 1 if features['ttl'] < 32 else 0
        
        # Additional derived features
        features['packet_rate_anomaly'] = 1 if features['packet_rate'] > self.training_stats.get('packet_rate', {}).get('q75', 50) * 2 else 0
        
        return features
    
    def _classify_attack_type(self, features):
        """Classify attack type based on generated features"""
        
        # DDoS indicators
        if (features.get('packet_rate_anomaly', 0) == 1 or 
            features.get('small_packet_ratio', 0) == 1 or
            features.get('short_connection', 0) == 1):
            return "DDoS"
        
        # Port scan indicators  
        if (features.get('port_scan_signals', 0) == 1 or
            features.get('syn_flag', 0) == 1 and features.get('ack_flag', 0) == 0 or
            features.get('null_scan', 0) == 1 or features.get('xmas_scan', 0) == 1):
            return "Port Scan"
        
        # Brute force indicators
        if (features.get('authentication_port', 0) == 1 and
            features.get('long_connection', 0) == 1):
            return "Brute Force"
            
        # Protocol anomalies
        if (features.get('suspicious_flag_combo', 0) == 1 or
            features.get('other_protocol', 0) == 1 or
            features.get('ttl_anomaly', 0) == 1):
            return "Protocol Anomaly"
        
        return "Unknown Attack"
    
    def predict_packet(self, packet_size, duration, dst_port, protocol, syn_flag=0):
        """Predict if a packet is malicious"""
        if not self.is_trained:
            raise ValueError("Model not trained! Please load a trained model first.")
        
        # Generate all features
        generated_features = self._generate_comprehensive_features(
            packet_size, duration, dst_port, protocol, syn_flag
        )
        
        # Create feature vector
        feature_vector = np.zeros(len(self.feature_names))
        
        for i, feature_name in enumerate(self.feature_names):
            if feature_name in generated_features:
                feature_vector[i] = generated_features[feature_name]
            else:
                if feature_name in self.training_stats:
                    feature_vector[i] = self.training_stats[feature_name]['median']
                else:
                    feature_vector[i] = 0
        
        # Scale and predict
        feature_vector_scaled = self.scaler.transform(feature_vector.reshape(1, -1))
        prediction = self.isolation_forest.predict(feature_vector_scaled)[0]
        anomaly_score = self.isolation_forest.score_samples(feature_vector_scaled)[0]
        
        is_malicious = prediction == -1
        
        # Determine attack type if malicious
        attack_type = "None"
        if is_malicious:
            attack_type = self._classify_attack_type(generated_features)
        
        # Risk level assessment
        if abs(anomaly_score) > 0.3:
            risk_level = "CRITICAL"
        elif abs(anomaly_score) > 0.15:
            risk_level = "HIGH"
        elif abs(anomaly_score) > 0.05:
            risk_level = "MEDIUM"
        else:
            risk_level = "LOW"
        
        return {
            'is_malicious': bool(is_malicious),
            'anomaly_score': float(anomaly_score),
            'risk_level': risk_level,
            'attack_type': attack_type,
            'prediction_confidence': 'High' if abs(anomaly_score) > 0.1 else 'Medium'
        }


def load_model(model_path='models/malicious_packet_model.pkl'):
    """Load the malicious packet detection model"""
    global _detector, _model_loaded
    
    # If relative path, make it relative to this script's directory
    if not os.path.isabs(model_path):
        script_dir = os.path.dirname(os.path.abspath(__file__))
        model_path = os.path.join(script_dir, model_path)
    
    if not os.path.exists(model_path):
        print(f"[WARNING] Model file not found at {model_path}")
        print(f"[INFO] Please train the model first using train_model.py")
        return False
    
    try:
        _detector = SimpleMaliciousDetector()
        success = _detector.load_model(model_path)
        _model_loaded = success
        return success
    except Exception as e:
        print(f"[ERROR] Failed to load model: {e}")
        _model_loaded = False
        return False


def predict_malicious_packet(packet_data):
    """
    Predict if a network packet is malicious
    
    Args:
        packet_data (dict): Dictionary containing:
            - packet_size (float): Packet size in bytes
            - duration (float): Connection duration in seconds
            - dst_port (int): Destination port
            - protocol (int): Protocol (1=ICMP, 6=TCP, 17=UDP)
            - syn_flag (int, optional): SYN flag (0 or 1)
    
    Returns:
        dict: Prediction results
    """
    global _detector, _model_loaded
    
    if not _model_loaded or _detector is None:
        return {
            "status": "error",
            "message": "Model not loaded. Please ensure the model file exists."
        }
    
    try:
        # Extract parameters
        packet_size = float(packet_data.get('packet_size', 0))
        duration = float(packet_data.get('duration', 0))
        dst_port = int(packet_data.get('dst_port', 0))
        protocol = int(packet_data.get('protocol', 0))
        syn_flag = int(packet_data.get('syn_flag', 0))
        
        # Validate inputs
        if packet_size <= 0:
            return {"status": "error", "message": "packet_size must be positive"}
        if duration <= 0:
            return {"status": "error", "message": "duration must be positive"}
        if not 1 <= dst_port <= 65535:
            return {"status": "error", "message": "dst_port must be between 1 and 65535"}
        if protocol not in [1, 6, 17]:
            return {"status": "error", "message": "protocol must be 1 (ICMP), 6 (TCP), or 17 (UDP)"}
        if syn_flag not in [0, 1]:
            return {"status": "error", "message": "syn_flag must be 0 or 1"}
        
        # Make prediction
        result = _detector.predict_packet(packet_size, duration, dst_port, protocol, syn_flag)
        result['status'] = 'success'
        
        return result
        
    except Exception as e:
        return {
            "status": "error",
            "message": f"Prediction failed: {str(e)}"
        }


# For testing this module independently
if __name__ == "__main__":
    print("Testing malicious_predictor module...")
    
    # Try to load model
    if load_model():
        print("\n[SUCCESS] Model loaded successfully!")
        
        # Test prediction
        test_packet = {
            'packet_size': 500,
            'duration': 2.5,
            'dst_port': 80,
            'protocol': 6,
            'syn_flag': 0
        }
        
        print(f"\nTesting with packet: {test_packet}")
        result = predict_malicious_packet(test_packet)
        print(f"\nResult: {result}")
    else:
        print("\n[FAILED] Could not load model. Please train the model first.")