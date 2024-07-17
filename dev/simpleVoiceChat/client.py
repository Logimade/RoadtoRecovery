from vidstream import AudioSender, AudioReceiver
import threading
import socket
import time
import atexit

# Function to stop the receiver properly
def stop_receiver():
    print("Stopping receiver...")
    receiver.stop_server()  # This assumes there's a method to stop the server in AudioReceiver
    receive_thread.join()

# Function to stop the sender properly
def stop_sender():
    print("Stopping sender...")
    sender.stop_stream()  # This assumes there's a method to stop the stream in AudioSender
    sender_thread.join()

receiver = AudioReceiver('192.168.1.194', 9999)
receive_thread = threading.Thread(target=receiver.start_server)

sender = AudioSender('192.168.1.77', 5555)
sender_thread = threading.Thread(target=sender.start_stream)

# Register the stop_receiver and stop_sender functions to be called at exit
atexit.register(stop_receiver)
atexit.register(stop_sender)

receive_thread.start()
sender_thread.start()

# To keep the script running and allow receiving and sending audio
try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("Interrupted by user, shutting down.")
finally:
    stop_receiver()
    stop_sender()
