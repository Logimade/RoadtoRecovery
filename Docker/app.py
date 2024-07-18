import socket
import threading
import time

# Server configuration
SERVER_IP = "0.0.0.0"
SERVER_PORT = 9001

# Dictionary to keep track of connected clients and their last active time
clients = {}

# Lock for thread-safe access to clients dictionary
clients_lock = threading.Lock()

# Create UDP socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_socket.bind((SERVER_IP, SERVER_PORT))

print("Server started at {}:{}".format(SERVER_IP, SERVER_PORT))


# Function to periodically check and remove inactive clients
def handle_clients():
    while True:
        with clients_lock:
            current_time = time.time()
            # Identify clients that haven't been active for more than 5 seconds
            inactive_clients = [client for client, last_active in clients.items() if current_time - last_active > 5]
            for client in inactive_clients:
                print(f"Removing inactive client: {client}")
                del clients[client]
        time.sleep(1)


# Start a background thread to handle inactive clients
heartbeat_thread = threading.Thread(target=handle_clients)
heartbeat_thread.daemon = True
heartbeat_thread.start()

try:
    while True:
        try:
            # Receive data from a client
            data, client_address = server_socket.recvfrom(1024)

            # Update the last active time for the client or add a new client
            with clients_lock:
                clients[client_address] = time.time()

                # Forward the data to all clients except the sender
                for client in list(clients.keys()):
                    if client != client_address:
                        server_socket.sendto(data, client)
        except Exception as e:
            print(f"Server error: {e}")
except KeyboardInterrupt:
    print("Server shutting down...")
finally:
    server_socket.close()
