import socket
from threading import Thread

SOCK_IP = '0.0.0.0'  # internal IP of the server
SOCK_PORT = 8765

class ClientHandler(Thread):
    def __init__(self, client_address):
        super().__init__()
        self.client_address = client_address
        self.udp_socket = None

    def run(self):
        try:
            self.udp_socket = create_udp_socket()
            self.receive_names()
        except Exception as e:
            print(f"Error in ClientHandler thread: {e}")
        finally:
            if self.udp_socket:
                self.udp_socket.close()

    def receive_names(self):
        try:
            data, _ = self.udp_socket.recvfrom(1024)
            source_name = data.decode('utf-8').rstrip()
            print(f"Client connected: {source_name}")

            data, _ = self.udp_socket.recvfrom(1024)
            destination_name = data.decode('utf-8').rstrip()
            print(f"{source_name} wants to connect to {destination_name}")

            self.udp_socket.sendto('go'.encode('utf-8'), self.client_address)

            while True:
                data, _ = self.udp_socket.recvfrom(1024)
                print(f"Received from {source_name}: {data.decode('utf-8')}")
        except Exception as e:
            print(f"Error handling client: {e}")

def create_udp_socket():
    return socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def main():
    udp_socket = create_udp_socket()
    print(f"Binding socket on {SOCK_IP}:{SOCK_PORT}")

    try:
        udp_socket.bind((SOCK_IP, SOCK_PORT))

        while True:
            try:
                data, client_address = udp_socket.recvfrom(1024)
                client_handler = ClientHandler(client_address)
                client_handler.start()
            except Exception as e:
                print(f"Error in main loop: {e}")

    except Exception as e:
        print(f"Error in main thread: {e}")
    finally:
        udp_socket.close()

if __name__ == "__main__":
    main()
