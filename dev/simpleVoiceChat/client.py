import socket
import pyaudio
import threading

# Server configuration
#SERVER_IP = "188.37.225.70"  # Change to your server's public IP address
SERVER_PORT = 9001

file_path = "./VoiceChat Server IP.txt"
ip_default = "roadtorecovery.logimade.com"
ip = None

try:
    with open(file_path, 'r') as file:
        ip = file.read()
        #ip = re.search( r"\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}", o )
except FileNotFoundError:
    with open(file_path, 'w') as file:
        file.write(ip_default)

if ip:
    SERVER_IP = ip
else:
    SERVER_IP = ip_default

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

    connection = False
    flagOneTime = False
    while True:
        try:
            if not connection:
                print("Awaiting connection...")
            elif not flagOneTime:
                print("Connected.")
                flagOneTime = True

            data, addr = client_socket.recvfrom(1024)
            stream.write(data)
            connection = True
        except socket.timeout:
            continue
        except Exception as e:
            print(f"Error receiving audio data: {e}")
            break

# Connect to the server
try:
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
