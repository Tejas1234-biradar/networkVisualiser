#!/usr/bin/env python3
import json
import sys
import time
import threading
from flask import Flask, jsonify
from flask_socketio import SocketIO, emit

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

# Configuration
NODE_TIMEOUT = 300  # 5 minutes
CLEANUP_INTERVAL = 60  # 1 minute
connected_clients = 0

# Data structures for network topology
node_data = {}  # IP -> node info
edge_data = {}  # (src_ip, dst_ip) -> edge info
traceroute_paths = {}  # dst_ip -> list of hop IPs

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
    
    # Clean up old traceroute paths
    old_paths = [dst for dst, data in traceroute_paths.items()
                 if current_time - data["last_seen"] > NODE_TIMEOUT]
    
    for dst in old_paths:
        del traceroute_paths[dst]
        print(f"[DEBUG] Removed old traceroute path: {dst}")
    
    if old_nodes or old_edges or old_paths:
        send_full_update()

def send_full_update():
    """Send complete graph state to all clients"""
    if connected_clients > 0:
        # Prepare nodes with metadata
        nodes = []
        for ip, data in node_data.items():
            node = {
                "id": ip,
                "ip": ip,
                "type": data.get("type", "host"),
                "packet_count": data["packet_count"],
                "first_seen": data["first_seen"],
                "last_seen": data["last_seen"],
                "is_local": data.get("is_local", False)
            }
            nodes.append(node)
        
        # Prepare edges
        edges = []
        for (src, dst), data in edge_data.items():
            edge = {
                "source": src,
                "target": dst,
                "type": data["type"],
                "packet_count": data["packet_count"],
                "protocols": list(data["protocols"]),
                "first_seen": data["first_seen"],
                "last_seen": data["last_seen"]
            }
            edges.append(edge)
        
        update_data = {
            "type": "full",
            "nodes": nodes,
            "edges": edges,
            "traceroute_paths": traceroute_paths,
            "stats": {
                "total_nodes": len(node_data),
                "total_edges": len(edge_data),
                "total_paths": len(traceroute_paths)
            }
        }
        socketio.emit("graph_update", update_data)
        print(f"[DEBUG] Sent full update: {len(node_data)} nodes, {len(edge_data)} edges, {len(traceroute_paths)} paths")

def get_local_ip():
    """Get the local machine's IP address (you may need to adjust this)"""
    import socket
    try:
        # Connect to a dummy address to find local IP
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.connect(("8.8.8.8", 80))
            return s.getsockname()[0]
    except:
        return "127.0.0.1"

LOCAL_IP = get_local_ip()

def process_regular_packet(packet):
    """Process regular network packet data"""
    current_time = time.time()
    
    src_ip = packet.get('src_ip', 'unknown')
    dst_ip = packet.get('dst_ip', 'unknown')
    protocol = packet.get('protocol', 'UNKNOWN')
    
    # Skip invalid IPs
    if src_ip in ['unknown', '0.0.0.0'] or dst_ip in ['unknown', '0.0.0.0']:
        return
    
    new_nodes = []
    
    # Process source node
    if src_ip not in node_data:
        node_data[src_ip] = {
            "first_seen": current_time,
            "last_seen": current_time,
            "packet_count": 1,
            "type": "local" if src_ip == LOCAL_IP else "remote",
            "is_local": src_ip == LOCAL_IP,
            "protocols": {protocol}
        }
        new_nodes.append(src_ip)
    else:
        node_data[src_ip]["last_seen"] = current_time
        node_data[src_ip]["packet_count"] += 1
        node_data[src_ip]["protocols"].add(protocol)
    
    # Process destination node
    if dst_ip not in node_data:
        node_data[dst_ip] = {
            "first_seen": current_time,
            "last_seen": current_time,
            "packet_count": 1,
            "type": "local" if dst_ip == LOCAL_IP else "remote",
            "is_local": dst_ip == LOCAL_IP,
            "protocols": {protocol}
        }
        new_nodes.append(dst_ip)
    else:
        node_data[dst_ip]["last_seen"] = current_time
        node_data[dst_ip]["packet_count"] += 1
        node_data[dst_ip]["protocols"].add(protocol)
    
    # Process edge
    edge_tuple = (src_ip, dst_ip)
    new_edge = None
    
    if edge_tuple not in edge_data:
        edge_data[edge_tuple] = {
            "first_seen": current_time,
            "last_seen": current_time,
            "packet_count": 1,
            "type": "direct",
            "protocols": {protocol}
        }
        new_edge = {"source": src_ip, "target": dst_ip}
    else:
        edge_data[edge_tuple]["last_seen"] = current_time
        edge_data[edge_tuple]["packet_count"] += 1
        edge_data[edge_tuple]["protocols"].add(protocol)
    
    # Send updates to connected clients
    if connected_clients > 0:
        if new_nodes:
            send_full_update()
        elif new_edge:
            update_data = {
                "type": "edge",
                "edge": {
                    "source": src_ip,
                    "target": dst_ip,
                    "type": "direct",
                    "packet_count": edge_data[edge_tuple]["packet_count"],
                    "protocols": list(edge_data[edge_tuple]["protocols"])
                }
            }
            socketio.emit("graph_update", update_data)

def process_traceroute_packet(packet):
    """Process traceroute data to build network topology"""
    current_time = time.time()
    
    dst_ip = packet.get('dst_ip')
    hops = packet.get('hops', [])
    
    if not dst_ip or not hops:
        return
    
    # Update traceroute path data
    traceroute_paths[dst_ip] = {
        "last_seen": current_time,
        "hops": hops,
        "hop_count": len(hops)
    }
    
    new_nodes = []
    new_edges = []
    
    # Process each hop in the traceroute
    previous_ip = LOCAL_IP  # Start from local machine
    
    for hop in hops:
        hop_ttl = hop.get('ttl')
        responses = hop.get('responses', [])
        
        for response in responses:
            hop_ip = response.get('ip')
            rtts = response.get('rtts', [])
            
            # Skip invalid or timeout responses
            if not hop_ip or hop_ip == "*":
                continue
            
            # Add router node
            if hop_ip not in node_data:
                node_data[hop_ip] = {
                    "first_seen": current_time,
                    "last_seen": current_time,
                    "packet_count": 1,
                    "type": "router",
                    "is_local": False,
                    "protocols": {"ICMP"}
                }
                new_nodes.append(hop_ip)
            else:
                node_data[hop_ip]["last_seen"] = current_time
                node_data[hop_ip]["packet_count"] += 1
                node_data[hop_ip]["protocols"].add("ICMP")
            
            # Add traceroute edge from previous hop to current hop
            edge_tuple = (previous_ip, hop_ip)
            
            if edge_tuple not in edge_data:
                avg_rtt = sum(rtt for rtt in rtts if rtt > 0) / len([rtt for rtt in rtts if rtt > 0]) if rtts else 0
                
                edge_data[edge_tuple] = {
                    "first_seen": current_time,
                    "last_seen": current_time,
                    "packet_count": 1,
                    "type": "traceroute",
                    "protocols": {"ICMP"},
                    "ttl": hop_ttl,
                    "avg_rtt": avg_rtt
                }
                new_edges.append({"source": previous_ip, "target": hop_ip})
            else:
                edge_data[edge_tuple]["last_seen"] = current_time
                edge_data[edge_tuple]["packet_count"] += 1
                edge_data[edge_tuple]["protocols"].add("ICMP")
            
            # Update previous_ip for next iteration
            previous_ip = hop_ip
    
    # Add final edge to destination if we reached it
    if previous_ip != dst_ip and previous_ip != LOCAL_IP:
        # Ensure destination node exists
        if dst_ip not in node_data:
            node_data[dst_ip] = {
                "first_seen": current_time,
                "last_seen": current_time,
                "packet_count": 1,
                "type": "destination",
                "is_local": False,
                "protocols": {"ICMP"}
            }
            new_nodes.append(dst_ip)
        
        # Add final edge
        final_edge = (previous_ip, dst_ip)
        if final_edge not in edge_data:
            edge_data[final_edge] = {
                "first_seen": current_time,
                "last_seen": current_time,
                "packet_count": 1,
                "type": "traceroute",
                "protocols": {"ICMP"}
            }
            new_edges.append({"source": previous_ip, "target": dst_ip})
    
    # Send updates
    if connected_clients > 0:
        if new_nodes or new_edges:
            send_full_update()
        else:
            # Send incremental traceroute update
            update_data = {
                "type": "traceroute_update",
                "dst_ip": dst_ip,
                "path": [LOCAL_IP] + [hop.get('responses', [{}])[0].get('ip', '*') 
                        for hop in hops if hop.get('responses') and hop['responses'][0].get('ip') != '*']
            }
            socketio.emit("graph_update", update_data)

def process_packet(packet):
    """Main packet processing function"""
    protocol = packet.get('protocol', '')
    
    if protocol == 'TRACEROUTE':
        process_traceroute_packet(packet)
    else:
        process_regular_packet(packet)

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
            
            # Skip non-JSON lines - be more aggressive about filtering
            # Check for emoji characters, status messages, etc.
            if (not line.startswith('{')) or (not line.endswith('}')) or ('🔍' in line) or ('🚀' in line) or ('📁' in line) or ('Press Ctrl+C' in line):
                # Don't print every skipped line to reduce noise
                if any(char in line for char in ['🔍', '🚀', '📁']) or 'Press Ctrl+C' in line or 'Listening on' in line:
                    print(f"[DEBUG] Skipping status message: {line[:50]}...")
                continue
                
            try:
                packet = json.loads(line)
                # Validate that this looks like our packet structure
                if isinstance(packet, dict) and ('src_ip' in packet or 'dst_ip' in packet or packet.get('protocol') == 'TRACEROUTE'):
                    process_packet(packet)
                else:
                    print(f"[DEBUG] Skipping invalid packet structure: {line[:50]}...")
            except json.JSONDecodeError as e:
                # Only log decode errors for lines that look like they should be JSON
                if line.startswith('{') and line.endswith('}'):
                    print(f"[DEBUG] JSON decode error: {e} for line: {line[:50]}...")
    except EOFError:
        print("[DEBUG] EOF reached, stdin listener stopping")
    except Exception as e:
        print(f"[DEBUG] stdin listener error: {e}")

@app.route("/")
def index():
    return "Network Topology Graph Server - Tracking packets and traceroute paths"

@app.route("/api/graph")
def get_graph():
    """Get simplified graph structure"""
    return {
        "nodes": [{"id": ip, "type": data["type"], "is_local": data["is_local"]} 
                 for ip, data in node_data.items()],
        "edges": [{"source": s, "target": t, "type": data["type"]} 
                 for (s, t), data in edge_data.items()],
        "stats": {
            "total_nodes": len(node_data),
            "total_edges": len(edge_data),
            "traceroute_paths": len(traceroute_paths),
            "local_ip": LOCAL_IP
        }
    }

@app.route("/api/graph/detailed")
def get_detailed_graph():
    """Get detailed graph with all metadata"""
    current_time = time.time()
    
    # Detailed nodes
    detailed_nodes = []
    for ip, data in node_data.items():
        node = {
            "id": ip,
            "ip": ip,
            "type": data["type"],
            "packet_count": data["packet_count"],
            "first_seen": data["first_seen"],
            "last_seen": data["last_seen"],
            "age_seconds": current_time - data["first_seen"],
            "is_local": data["is_local"],
            "protocols": list(data["protocols"])
        }
        detailed_nodes.append(node)
    
    # Detailed edges
    detailed_edges = []
    for (src, dst), data in edge_data.items():
        edge = {
            "source": src,
            "target": dst,
            "type": data["type"],
            "packet_count": data["packet_count"],
            "protocols": list(data["protocols"]),
            "first_seen": data["first_seen"],
            "last_seen": data["last_seen"],
            "age_seconds": current_time - data["first_seen"]
        }
        
        # Add traceroute-specific data
        if data["type"] == "traceroute":
            edge["ttl"] = data.get("ttl")
            edge["avg_rtt"] = data.get("avg_rtt")
        
        detailed_edges.append(edge)
    
    # Traceroute path information
    paths_info = {}
    for dst_ip, path_data in traceroute_paths.items():
        paths_info[dst_ip] = {
            "hop_count": path_data["hop_count"],
            "last_traced": path_data["last_seen"],
            "path": [LOCAL_IP] + [hop.get('responses', [{}])[0].get('ip', '*') 
                    for hop in path_data["hops"] 
                    if hop.get('responses') and hop['responses'][0].get('ip') != '*']
        }
    
    return {
        "nodes": detailed_nodes,
        "edges": detailed_edges,
        "traceroute_paths": paths_info,
        "stats": {
            "total_nodes": len(node_data),
            "total_edges": len(edge_data),
            "direct_connections": len([e for e in edge_data.values() if e["type"] == "direct"]),
            "traceroute_hops": len([e for e in edge_data.values() if e["type"] == "traceroute"]),
            "local_ip": LOCAL_IP
        },
        "config": {
            "node_timeout_seconds": NODE_TIMEOUT,
            "cleanup_interval_seconds": CLEANUP_INTERVAL
        }
    }

@app.route("/api/topology")
def get_topology():
    """Get network topology with path analysis"""
    # Find all paths from local machine
    paths_from_local = {}
    
    for dst_ip, path_data in traceroute_paths.items():
        if dst_ip in node_data:
            hops = path_data["hops"]
            path = [LOCAL_IP]
            
            for hop in hops:
                responses = hop.get('responses', [])
                if responses and responses[0].get('ip') != '*':
                    path.append(responses[0]['ip'])
            
            if len(path) > 1:
                paths_from_local[dst_ip] = {
                    "path": path,
                    "hop_count": len(path) - 1,
                    "last_traced": path_data["last_seen"]
                }
    
    return {
        "local_ip": LOCAL_IP,
        "paths_from_local": paths_from_local,
        "network_layers": {
            "local": [ip for ip, data in node_data.items() if data["is_local"]],
            "routers": [ip for ip, data in node_data.items() if data["type"] == "router"],
            "destinations": [ip for ip, data in node_data.items() if data["type"] in ["remote", "destination"]]
        }
    }

@app.route("/stats")
def stats():
    """Statistics endpoint"""
    router_count = len([ip for ip, data in node_data.items() if data["type"] == "router"])
    local_count = len([ip for ip, data in node_data.items() if data["is_local"]])
    remote_count = len([ip for ip, data in node_data.items() if not data["is_local"]])
    
    return {
        "nodes": {
            "total": len(node_data),
            "local": local_count,
            "routers": router_count,
            "remote": remote_count
        },
        "edges": {
            "total": len(edge_data),
            "direct": len([e for e in edge_data.values() if e["type"] == "direct"]),
            "traceroute": len([e for e in edge_data.values() if e["type"] == "traceroute"])
        },
        "traceroute_paths": len(traceroute_paths),
        "connected_clients": connected_clients,
        "local_ip": LOCAL_IP
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

@socketio.on('request_topology')
def handle_topology_request():
    """Handle client request for topology data"""
    topology_data = get_topology()
    emit("topology_data", topology_data)

if __name__ == "__main__":
    print(f"[DEBUG] Starting Network Topology Server...")
    print(f"[DEBUG] Local IP detected as: {LOCAL_IP}")
    
    # Start background threads
    stdin_thread = threading.Thread(target=stdin_listener, daemon=True)
    cleanup_thread = threading.Thread(target=periodic_cleanup, daemon=True)
    
    stdin_thread.start()
    cleanup_thread.start()
    
    # Run the SocketIO server
    socketio.run(app, host="0.0.0.0", port=5000, debug=False, allow_unsafe_werkzeug=True)
