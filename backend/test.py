#!/usr/bin/env python3
"""
Test runner for network topology server
"""

import subprocess
import time
import requests
import json
import threading
import signal
import sys

def test_api_endpoints():
    """Test all the server API endpoints"""
    base_url = "http://localhost:5000"
    
    endpoints = [
        "/",
        "/api/graph",
        "/api/graph/detailed", 
        "/api/topology",
        "/stats"
    ]
    
    print("🧪 Testing API endpoints...")
    for endpoint in endpoints:
        try:
            response = requests.get(f"{base_url}{endpoint}")
            if response.status_code == 200:
                print(f"✅ {endpoint}: OK")
                if endpoint == "/stats":
                    data = response.json()
                    print(f"   Nodes: {data.get('nodes', {}).get('total', 0)}")
                    print(f"   Edges: {data.get('edges', {}).get('total', 0)}")
            else:
                print(f"❌ {endpoint}: {response.status_code}")
        except Exception as e:
            print(f"❌ {endpoint}: {e}")

def run_server_and_test():
    """Run server with test data and perform tests"""
    print("🚀 Starting integrated test...")
    
    server_process = None
    data_process = None
    
    try:
        # Start server
        print("Starting server...")
        server_process = subprocess.Popen(
            ["python3", "server.py"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        
        # Give server time to start
        time.sleep(2)
        
        # Start data generator and pipe to server
        print("Starting data generator...")
        data_process = subprocess.Popen(
            ["python3", "test.py"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        
        # Pipe data to server
        def pipe_data():
            while True:
                line = data_process.stdout.readline()
                if not line:
                    break
                server_process.stdin.write(line)
                server_process.stdin.flush()
        
        pipe_thread = threading.Thread(target=pipe_data)
        pipe_thread.daemon = True
        pipe_thread.start()
        
        # Wait for some data to be processed
        print("Waiting for data to be processed...")
        time.sleep(10)
        
        # Test API endpoints
        test_api_endpoints()
        
        print("\n📊 Sample data from /api/graph/detailed:")
        try:
            response = requests.get("http://localhost:5000/api/graph/detailed")
            if response.status_code == 200:
                data = response.json()
                print(f"Nodes: {len(data['nodes'])}")
                print(f"Edges: {len(data['edges'])}")
                print("\nSample nodes:")
                for node in data['nodes'][:3]:
                    print(f"  - {node['ip']} ({node['type']})")
                
                print("\nSample edges:")
                for edge in data['edges'][:3]:
                    print(f"  - {edge['source']} -> {edge['target']} ({edge['type']})")
        except Exception as e:
            print(f"Error getting detailed data: {e}")
        
        print("\n⏱️  Test will continue running. Press Ctrl+C to stop...")
        
        # Keep running until interrupted
        while True:
            time.sleep(5)
            # Print periodic stats
            try:
                response = requests.get("http://localhost:5000/stats")
                if response.status_code == 200:
                    stats = response.json()
                    print(f"[{time.strftime('%H:%M:%S')}] Nodes: {stats['nodes']['total']}, Edges: {stats['edges']['total']}, Clients: {stats['connected_clients']}")
            except:
                pass
        
    except KeyboardInterrupt:
        print("\n🛑 Test stopped by user")
    
    finally:
        # Cleanup processes
        if data_process:
            data_process.terminate()
        if server_process:
            server_process.terminate()
        print("✅ Cleanup complete")

def generate_sample_data():
    """Generate just a few sample packets for inspection"""
    print("📝 Generating sample data...")
    subprocess.run(["python3", "test.py", "--batch"])

def show_usage():
    """Show usage information"""
    print("""
🔧 Network Topology Test Suite

Usage:
  python3 run_test.py [command]

Commands:
  test-api     - Test API endpoints (server must be running)
  run-full     - Run server with test data and perform tests  
  sample       - Generate sample data to stdout
  help         - Show this help

Examples:
  # Test with real sniffer:
  sudo ./sniffer wlo1 | python3 server.py

  # Test with simulated data:
  python3 test.py | python3 server.py

  # Run integrated test:
  python3 run_test.py run-full

  # Just test API endpoints:
  python3 run_test.py test-api
""")

if __name__ == "__main__":
    command = sys.argv[1] if len(sys.argv) > 1 else "help"
    
    if command == "test-api":
        test_api_endpoints()
    elif command == "run-full":
        run_server_and_test()
    elif command == "sample":
        generate_sample_data()
    elif command == "help":
        show_usage()
    else:
        print(f"Unknown command: {command}")
        show_usage()