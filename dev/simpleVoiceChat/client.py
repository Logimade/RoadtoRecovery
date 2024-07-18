import socket
import pyaudio
import threading

# Server configuration
SERVER_IP = "188.37.225.70"  # Change to your server's public IP address
SERVER_PORT = 9001

# Audio configuration
CHUNK = 512  # Reduce chunk size
FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 44100

# Create UDP socket
try:
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client_socket.settimeout(1.0)  # Set timeout for socket operations
except socket.error as e:
    print(f"Failed to create socket: {e}")
    exit()

# Initialize PyAudio
try:
    p = pyaudio.PyAudio()
except Exception as e:
    print(f"Failed to initialize PyAudio: {e}")
    exit()

def send_audio():
    try:
        stream = p.open(format=FORMAT,
                        channels=CHANNELS,
                        rate=RATE,
                        input=True,
                        frames_per_buffer=CHUNK)
    except Exception as e:
        print(f"Failed to open input stream: {e}")
        return

    while True:
        try:
            data = stream.read(CHUNK)
            client_socket.sendto(data, (SERVER_IP, SERVER_PORT))
        except Exception as e:
            print(f"Error sending audio data: {e}")
            break

def receive_audio():
    try:
        stream = p.open(format=FORMAT,
                        channels=CHANNELS,
                        rate=RATE,
                        output=True,
                        frames_per_buffer=CHUNK)
    except Exception as e:
        print(f"Failed to open output stream: {e}")
        return

    print("Connected to the server.")

    while True:
        try:
            data, addr = client_socket.recvfrom(1024)
            stream.write(data)
        except socket.timeout:
            continue
        except Exception as e:
            print(f"Error receiving audio data: {e}")
            break

# Connect to the server
try:
    client_socket.sendto(b"Connected to the server.", (SERVER_IP, SERVER_PORT))

    send_thread = threading.Thread(target=send_audio)
    receive_thread = threading.Thread(target=receive_audio)

    send_thread.start()
    receive_thread.start()

    send_thread.join()
    receive_thread.join()
except Exception as e:
    print(f"Failed to start threads: {e}")
finally:
    client_socket.close()
    p.terminate()
