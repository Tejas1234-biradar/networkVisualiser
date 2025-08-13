import sys
import json
import threading
import time
from collections import defaultdict
from flask import Flask
from flask_socketio import SocketIO

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='threading')

# Persistent storage with timestamps
node_data = {}  # node -> {"first_seen": timestamp, "last_seen": timestamp, "packet_count": count}
edge_data = {}  # (src, dst) -> {"first_seen": timestamp, "last_seen": timestamp, "packet_count": count}
connected_clients = 0

# Configuration
NODE_TIMEOUT = 300  # Keep nodes for 5 minutes after last seen
CLEANUP_INTERVAL = 60  # Clean up old nodes every minute

def cleanup_old_data():
    """Remove nodes and edges that haven't been seen recently"""
    current_time = time.time()
    
    # Clean up old nodes
    old_nodes = [node for node, data in node_data.items() 
                 if current_time - data["last_seen"] > NODE_TIMEOUT]
    
    for node in old_nodes:
        del node_data[node]
        print(f"[DEBUG] Removed old node: {node}")
    
    # Clean up old edges
    old_edges = [edge for edge, data in edge_data.items() 
                 if current_time - data["last_seen"] > NODE_TIMEOUT]
    
    for edge in old_edges:
        del edge_data[edge]
        print(f"[DEBUG] Removed old edge: {edge}")
    
    if old_nodes or old_edges:
        # Send updated full graph if we removed anything
        send_full_update()

def send_full_update():
    """Send complete graph state to all clients"""
    if connected_clients > 0:
        update_data = {
            "type": "full",
            "nodes": list(node_data.keys()),
            "edges": [{"source": s, "target": t} for s, t in edge_data.keys()],
            "stats": {
                "total_nodes": len(node_data),
                "total_edges": len(edge_data),
                "unique_ips": len(set(node.split(':')[0] for node in node_data.keys()))
            }
        }
        socketio.emit("graph_update", update_data)
        print(f"[DEBUG] Sent full update: {len(node_data)} nodes, {len(edge_data)} edges")

def process_packet(packet):
    global connected_clients
    current_time = time.time()
    
    src_ip = packet.get('src_ip', 'unknown')
    dst_ip = packet.get('dst_ip', 'unknown')
    src_port = packet.get('src_port')
    dst_port = packet.get('dst_port')
    
    # Create node identifiers
    if src_port is not None:
        src = f"{src_ip}:{src_port}"
    else:
        src = src_ip
        
    if dst_port is not None:
        dst = f"{dst_ip}:{dst_port}"
    else:
        dst = dst_ip
    
    new_nodes = []
    
    # Track source node
    if src not in node_data:
        node_data[src] = {
            "first_seen": current_time,
            "last_seen": current_time,
            "packet_count": 1
        }
        new_nodes.append(src)
    else:
        node_data[src]["last_seen"] = current_time
        node_data[src]["packet_count"] += 1
    
    # Track destination node
    if dst not in node_data:
        node_data[dst] = {
            "first_seen": current_time,
            "last_seen": current_time,
            "packet_count": 1
        }
        new_nodes.append(dst)
    else:
        node_data[dst]["last_seen"] = current_time
        node_data[dst]["packet_count"] += 1
    
    # Track edge
    edge_tuple = (src, dst)
    new_edge = None
    
    if edge_tuple not in edge_data:
        edge_data[edge_tuple] = {
            "first_seen": current_time,
            "last_seen": current_time,
            "packet_count": 1
        }
        new_edge = {"source": src, "target": dst}
    else:
        edge_data[edge_tuple]["last_seen"] = current_time
        edge_data[edge_tuple]["packet_count"] += 1
    
    # Emit updates if we have connected clients
    if connected_clients > 0:
        if new_nodes:
            send_full_update()
        elif new_edge:
            update_data = {
                "type": "edge", 
                "edge": new_edge
            }
            socketio.emit("graph_update", update_data)
            print(f"[DEBUG] Emitted incremental edge: {new_edge}")

def periodic_cleanup():
    """Periodically clean up old data"""
    while True:
        time.sleep(CLEANUP_INTERVAL)
        cleanup_old_data()

def stdin_listener():
    print("[DEBUG] Starting stdin listener...")
    try:
        for line in sys.stdin:
            line = line.strip()
            if not line:
                continue
            try:
                packet = json.loads(line)
                process_packet(packet)
            except json.JSONDecodeError as e:
                print(f"[DEBUG] JSON decode error: {e} for line: {line}")
    except EOFError:
        print("[DEBUG] EOF reached, stdin listener stopping")
    except Exception as e:
        print(f"[DEBUG] stdin listener error: {e}")

@app.route("/")
def index():
    return "WebSocket graph backend running with persistent tracking."

@app.route("/api/graph")
def get_graph():
    """Get all current nodes and edges"""
    unique_ips = set()
    for node in node_data.keys():
        ip = node.split(':')[0] if ':' in node else node
        unique_ips.add(ip)
    
    return {
        "nodes": list(node_data.keys()),
        "edges": [{"source": s, "target": t} for s, t in edge_data.keys()],
        "stats": {
            "total_nodes": len(node_data),
            "total_edges": len(edge_data),
            "unique_ips": len(unique_ips)
        }
    }

@app.route("/api/graph/detailed")
def get_detailed_graph():
    """Get all nodes and edges with detailed information"""
    # Prepare detailed node information
    detailed_nodes = []
    for node, data in node_data.items():
        ip = node.split(':')[0] if ':' in node else node
        port = node.split(':')[1] if ':' in node else None
        
        detailed_nodes.append({
            "id": node,
            "ip": ip,
            "port": port,
            "first_seen": data["first_seen"],
            "last_seen": data["last_seen"],
            "packet_count": data["packet_count"],
            "age_seconds": time.time() - data["first_seen"]
        })
    
    # Prepare detailed edge information
    detailed_edges = []
    for (src, dst), data in edge_data.items():
        detailed_edges.append({
            "source": src,
            "target": dst,
            "first_seen": data["first_seen"],
            "last_seen": data["last_seen"],
            "packet_count": data["packet_count"],
            "age_seconds": time.time() - data["first_seen"]
        })
    
    unique_ips = set()
    for node in node_data.keys():
        ip = node.split(':')[0] if ':' in node else node
        unique_ips.add(ip)
    
    return {
        "nodes": detailed_nodes,
        "edges": detailed_edges,
        "stats": {
            "total_nodes": len(node_data),
            "total_edges": len(edge_data),
            "unique_ips": len(unique_ips),
            "unique_ip_list": sorted(list(unique_ips))
        },
        "config": {
            "node_timeout_seconds": NODE_TIMEOUT,
            "cleanup_interval_seconds": CLEANUP_INTERVAL
        }
    }

@app.route("/stats")
def stats():
    """Legacy stats endpoint - redirects to detailed graph API"""
    unique_ips = set()
    for node in node_data.keys():
        ip = node.split(':')[0] if ':' in node else node
        unique_ips.add(ip)
    
    return {
        "total_nodes": len(node_data),
        "total_edges": len(edge_data),
        "unique_ips": len(unique_ips),
        "unique_ip_list": sorted(list(unique_ips)),
        "connected_clients": connected_clients,
        "node_timeout_seconds": NODE_TIMEOUT,
        "api_endpoints": {
            "simple_graph": "/api/graph",
            "detailed_graph": "/api/graph/detailed"
        }
    }

@socketio.on('connect')
def handle_connect():
    global connected_clients
    connected_clients += 1
    print(f"[DEBUG] Client connected (total: {connected_clients})")
    
    # Send current state to new client
    send_full_update()

@socketio.on('disconnect')
def handle_disconnect():
    global connected_clients
    connected_clients -= 1
    print(f"[DEBUG] Client disconnected (remaining: {connected_clients})")

if __name__ == "__main__":
    print("[DEBUG] Starting Flask-SocketIO server with persistent tracking...")
    
    # Start background threads
    stdin_thread = threading.Thread(target=stdin_listener, daemon=True)
    cleanup_thread = threading.Thread(target=periodic_cleanup, daemon=True)
    
    stdin_thread.start()
    cleanup_thread.start()
    
    # Run the SocketIO server
    socketio.run(app, host="0.0.0.0", port=5000, debug=False,allow_unsafe_werkzeug=True )