import socketio

sio = socketio.Client()

@sio.event
def connect():
    print("Connected to server")

@sio.event
def disconnect():
    print("Disconnected from server")

@sio.on("graph_update")
def on_graph_update(data):
    print("Received graph update:", data)

sio.connect('http://localhost:5000')
sio.wait()
