import socketio
import time

# Create a SocketIO client with better configuration
sio = socketio.Client(logger=True, engineio_logger=True)

@sio.event
def connect():
    print("✅ Connected to server")

@sio.event
def connect_error(data):
    print(f"❌ Connection failed: {data}")

@sio.event
def disconnect():
    print("🔌 Disconnected from server")

@sio.on("graph_update")
def on_graph_update(data):
    print("📊 Received graph update:")
    print(f"   Type: {data.get('type', 'unknown')}")
    if data.get('type') == 'full':
        print(f"   Nodes: {len(data.get('nodes', []))}")
        print(f"   Edges: {len(data.get('edges', []))}")
        print(f"   Node list: {data.get('nodes', [])}")
    elif data.get('type') == 'edge':
        edge = data.get('edge', {})
        print(f"   New edge: {edge.get('source', 'unknown')} -> {edge.get('target', 'unknown')}")
    print("---")

def main():
    try:
        print("🔄 Attempting to connect to WebSocket server...")
        sio.connect('http://localhost:5000', wait_timeout=10)
        
        print("⏳ Listening for graph updates... Press Ctrl+C to exit")
        
        # Keep the client alive and listening
        while True:
            time.sleep(1)
            
    except socketio.exceptions.ConnectionError as e:
        print(f"❌ Connection error: {e}")
    except KeyboardInterrupt:
        print("\n🛑 Shutting down...")
    except Exception as e:
        print(f"❌ Unexpected error: {e}")
    finally:
        if sio.connected:
            sio.disconnect()
        print("👋 Goodbye!")

if __name__ == "__main__":
    main()