from flask import Blueprint, request, jsonify
from malicious_predictor import predict_malicious_packet, load_model

# Create a Blueprint for prediction routes
prediction_bp = Blueprint('prediction', __name__)

# Global variable to track model loading status
malicious_model_loaded = False

def initialize_model():
    """Initialize the malicious packet detection model"""
    global malicious_model_loaded
    malicious_model_loaded = load_model()
    if not malicious_model_loaded:
        print("[ERROR] Failed to load malicious packet detection model")
    else:
        print("[SUCCESS] Malicious packet detection model loaded successfully")
    return malicious_model_loaded

@prediction_bp.route('/predict/malicious-packet', methods=['POST'])
def predict_malicious_packet_endpoint():
    """
    Predict if a network packet is malicious
    
    Expected JSON payload:
    {
        "packet_size": 500,      # Size of packet in bytes (required)
        "duration": 2.5,         # Connection duration in seconds (required)
        "dst_port": 80,          # Destination port (required, 1-65535)
        "protocol": 6,           # Protocol: 1=ICMP, 6=TCP, 17=UDP (required)
        "syn_flag": 0            # SYN flag: 0 or 1 (optional, default 0)
    }
    
    Returns:
    {
        "status": "success",
        "is_malicious": false,
        "anomaly_score": -0.123456,
        "risk_level": "LOW",
        "attack_type": "None",
        "confidence": "High"
    }
    """
    # Check if request is JSON
    if not request.is_json:
        return jsonify({
            "status": "error",
            "error": "Content-Type must be application/json"
        }), 400
    
    # Check if model is loaded
    if not malicious_model_loaded:
        return jsonify({
            "status": "error",
            "error": "Malicious packet detection model not available",
            "message": "Please ensure the model file exists at models/malicious_packet_model.pkl"
        }), 503
    
    try:
        data = request.get_json()
        
        # Validate required fields
        required_fields = ['packet_size', 'duration', 'dst_port', 'protocol']
        missing_fields = [field for field in required_fields if field not in data]
        
        if missing_fields:
            return jsonify({
                "status": "error",
                "error": f"Missing required fields: {', '.join(missing_fields)}",
                "required_fields": required_fields
            }), 400
        
        # Validate field types and ranges
        try:
            packet_size = float(data['packet_size'])
            duration = float(data['duration'])
            dst_port = int(data['dst_port'])
            protocol = int(data['protocol'])
            
            if packet_size <= 0:
                return jsonify({
                    "status": "error",
                    "error": "packet_size must be positive"
                }), 400
                
            if duration <= 0:
                return jsonify({
                    "status": "error",
                    "error": "duration must be positive"
                }), 400
                
            if not 1 <= dst_port <= 65535:
                return jsonify({
                    "status": "error",
                    "error": "dst_port must be between 1 and 65535"
                }), 400
                
            if protocol not in [1, 6, 17]:
                return jsonify({
                    "status": "error",
                    "error": "protocol must be 1 (ICMP), 6 (TCP), or 17 (UDP)"
                }), 400
                
        except ValueError as ve:
            return jsonify({
                "status": "error",
                "error": f"Invalid field type: {str(ve)}"
            }), 400
        
        # Set default for optional fields
        if 'syn_flag' not in data:
            data['syn_flag'] = 0
        else:
            syn_flag = int(data['syn_flag'])
            if syn_flag not in [0, 1]:
                return jsonify({
                    "status": "error",
                    "error": "syn_flag must be 0 or 1"
                }), 400
        
        # Call the prediction function
        result = predict_malicious_packet(data)
        
        # Convert numpy/bool types to Python native types for JSON serialization
        if result.get('status') == 'success':
            serializable_result = {
                'status': str(result.get('status', 'success')),
                'is_malicious': bool(result.get('is_malicious', False)),
                'anomaly_score': float(result.get('anomaly_score', 0.0)),
                'risk_level': str(result.get('risk_level', 'UNKNOWN')),
                'attack_type': str(result.get('attack_type', 'None')),
                'prediction_confidence': str(result.get('prediction_confidence', 'Low'))
            }
        else:
            serializable_result = result
        
        # Log the prediction
        print(f"[PREDICTION] Packet: size={packet_size}, duration={duration}, "
              f"port={dst_port}, protocol={protocol} -> "
              f"Malicious={serializable_result.get('is_malicious')}, "
              f"Risk={serializable_result.get('risk_level')}")
        
        return jsonify(serializable_result), 200
        
    except Exception as e:
        print(f"[ERROR] Prediction failed: {str(e)}")
        return jsonify({
            "status": "error", 
            "error": "Prediction failed",
            "message": str(e)
        }), 500


@prediction_bp.route('/predict/health', methods=['GET'])
def health_check():
    """Health check endpoint for prediction service"""
    return jsonify({
        "status": "healthy",
        "service": "Malicious Packet Prediction",
        "model_loaded": malicious_model_loaded,
        "endpoints": {
            "/predict/malicious-packet": "POST - Detect malicious network packets",
            "/predict/health": "GET - Check prediction service health"
        }
    }), 200


@prediction_bp.route('/predict/test', methods=['GET'])
def test_prediction():
    """Test endpoint with sample data"""
    if not malicious_model_loaded:
        return jsonify({
            "status": "error",
            "error": "Model not loaded"
        }), 503
    
    # Test with a normal packet
    test_data = {
        'packet_size': 500,
        'duration': 2.5,
        'dst_port': 80,
        'protocol': 6,
        'syn_flag': 0
    }
    
    result = predict_malicious_packet(test_data)
    
    return jsonify({
        "status": "success",
        "message": "Test prediction completed",
        "test_data": test_data,
        "prediction": result
    }), 200


# Helper function to get prediction statistics (optional)
@prediction_bp.route('/predict/stats', methods=['GET'])
def prediction_stats():
    """Get prediction service statistics"""
    return jsonify({
        "model_loaded": malicious_model_loaded,
        "supported_protocols": {
            "1": "ICMP",
            "6": "TCP",
            "17": "UDP"
        },
        "risk_levels": ["LOW", "MEDIUM", "HIGH", "CRITICAL"],
        "attack_types": ["DDoS", "Port Scan", "Brute Force", "Protocol Anomaly", "Unknown Attack", "None"]
    }), 200