import errno
from threading import Thread, Lock, Condition
import socket
import sounddevice as sd
from time import sleep
import pickle
import numpy as np
from Crypto.Cipher import AES
from Crypto.Random import get_random_bytes
from socket import timeout
import signal
import sys

MAX_BYTES_SEND = 512  # Must be less than 1024 because of networking limits
MAX_HEADER_LEN = 20  # allocates 20 bytes to store length of data that is transmitted
print("client started")
print("_________________________________________________________________________________")

# client sends self id
# client sends recipient's id
# client sends data

# socket connect to the server

SERVER_IP = '20.90.179.130'  # Public server. Change this to the external IP of the server
#SERVER_IP = '188.37.225.70'  # My server. Change this to the external IP of the server

FONTE = 'Ambulance'
DESTINO = 'Medic'


SERVER_PORT = 9001
BUFMAX = 512
running = True
mutex_t = Lock()
item_available = Condition()
SLEEPTIME = 0.0  # amount of time CPU sleeps between sending recordings to the server
# SLEEPTIME = 0.000001
audio_available = Condition()

sdstream = sd.Stream(samplerate=44100, channels=1, dtype='float32')
sdstream.start()

key = b'thisisthepasswordforAESencryptio'
iv = get_random_bytes(16)
cipher = AES.new(key, AES.MODE_CBC, iv)



"""
def get_iv():
    return get_random_bytes(16)

def decrypt(enc_data):
    cphr = AES.new(key, AES.MODE_CBC, enc_data[:16])
    decoded = cphr.decrypt(enc_data)[16:]
    return decoded.rstrip()


def encrypt(data_string):
    iv = get_iv()
    # cphr = AES.new(key, AES.MODE_CBC, iv)
    d = iv + data_string
    d = (d + (' ' * (len(d) % 16)).encode())
    return cipher.encrypt(d)
"""

def signal_handler(sig, frame):
    # Ctrl+c pressed!
    print('Logging '+FONTE+" out...")

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((SERVER_IP, SERVER_PORT))

    s.send((DESTINO + (' ' * (512 - len(DESTINO)))).encode())
    s.send((FONTE + (' ' * (512 - len(FONTE)))).encode())

    global running
    running = False
    sdstream.stop()
    s.close()

    sys.exit(0)

# Sender function in your client code
def split_send_bytes(s, inp):
    data_len = len(inp)
    if data_len == 0:
        print('ERROR: trying to send 0 bytes')  # should not happen in theory but threads are weird
        return

    # Convert data length to a string and left-pad with zeros to ensure fixed size
    header = str(data_len).zfill(MAX_HEADER_LEN).encode('utf-8')
    s.send(header)

    # send content in small batches. Maximum value of MAX_BYTES_SEND is 1024
    for i in range(data_len // MAX_BYTES_SEND):
        s.send(inp[i * MAX_BYTES_SEND:i * MAX_BYTES_SEND + MAX_BYTES_SEND])

    # send any remaining data
    if data_len % MAX_BYTES_SEND != 0:
        s.send(inp[-(data_len % MAX_BYTES_SEND):])


# Receiver function in your client code
def split_recv_bytes(s):
    dat = b''

    # receive header that specifies number of incoming bytes
    data_len_raw = s.recv(MAX_HEADER_LEN).strip()

    while True:
        try:
            data_len = int(data_len_raw)
            break  # Exit the loop if decoding succeeds
        except ValueError as e:
            print("Exception_ValueError:", e)
            # Handle potential errors or inconsistencies in received header data
            # Adjust as needed based on your application's requirements
            data_len_raw = b'0' * MAX_HEADER_LEN
            sleep(0.000000001)
            continue  # Retry the operation

    # read bytes
    for i in range(data_len // MAX_BYTES_SEND):
        dat += s.recv(MAX_BYTES_SEND)
    if data_len % MAX_BYTES_SEND != 0:
        dat += s.recv(data_len % MAX_BYTES_SEND)

    # Ensure that the size of the data buffer is a multiple of the size of float32 elements (4 bytes each)
    data_padding = (4 - len(dat) % 4) % 4  # Calculate the number of padding bytes needed to align with float32
    padded_data = dat + b'\x00' * data_padding  # Pad the data buffer with zeros if necessary

    # Create a NumPy array from the padded data buffer
    return np.frombuffer(padded_data, dtype='float32')

class SharedBuf:
    def __init__(self):
        self.buffer = np.array([], dtype='float32')

    def clearbuf(self):
        self.buffer = []

    def addbuf(self, arr):
        self.buffer = np.append(self.buffer, arr)

    def extbuf(self, arr):
        self.buffer = np.append(self.buffer, arr)

    def getlen(self):
        return len(self.buffer)

    def getbuf(self):
        return self.buffer

    def getx(self, x):
        data = self.buffer[0:x]
        self.buffer = self.buffer[x:]
        return data


# record t seconds of audio
def record(t):
    global running
    if running:
        return sdstream.read(t)[0]


def transmit(buf, socket):
    global running
    pickled = buf.tobytes()
    # encrypted_str = encrypt(pickled) # parte da encriptação. substituir em baixo no pickled

    try:
        split_send_bytes(socket, pickled)
    except timeout:
        print("SOCKET TIMEOUT")
        running = False
    except BrokenPipeError:
        print("Recipient disconnected")
        running = False


def record_transmit_thread(serversocket):
    print("***** STARTING RECORD TRANSMIT THREAD *****")
    tbuf = SharedBuf()
    global running

    def recorder_producer(buf):
        global running
        while running:
            sleep(SLEEPTIME)
            data = record(32)
            with item_available:
                item_available.wait_for(lambda: buf.getlen() <= BUFMAX)
                buf.extbuf(data)
                item_available.notify()

        print("RECORDER ENDS HERE")

    def transmitter_consumer(buf, serversocket):
        global running
        while running:
            sleep(SLEEPTIME)
            with item_available:
                item_available.wait_for(lambda: buf.getlen() >= 32)
                transmit(buf.getx(32), serversocket)
                item_available.notify()

        print("TRANSMITTER ENDS HERE")

    rec_thread = Thread(target=recorder_producer, args=(tbuf,))
    tr_thread = Thread(target=transmitter_consumer, args=(tbuf, serversocket))

    rec_thread.start()
    tr_thread.start()

    rec_thread.join()
    tr_thread.join()
    return


# use a sound library to play the buffer
def play(buf):
    # print("playing_audio")
    global running
    if running:
        sdstream.write(buf)


def receive(socket):
    global running
    while running:
        try:
            dat = split_recv_bytes(socket)
            # dat = decrypt(dat)     # parte da encriptação
            buf = np.frombuffer(dat, dtype='float32')  # read decrypted numpy array
            yield buf
        except pickle.UnpicklingError as e:
            print(f"    @@@@@ UNPICKLE ERROR @@@@@   \n DATA RECEIVED {len(dat)} :: {dat}")  # INPUT______ of len = {sys.getsizeof(dat)} ::{decrypt(dat)} :: {str(e)}")
            continue
        except timeout:
            print("SOCKET TIMEOUT")
            yield None
        except ConnectionResetError:
            print("Recipient disconnected")
            yield None


def receive_play_thread(serversocket):
    print("***** STARTING RECEIVE PLAY THREAD *****")
    rbuf = SharedBuf()

    def receiver_producer(buff, serversocket):
        global running
        rece_generator = receive(serversocket)
        while running:
            sleep(SLEEPTIME)
            try:
                data = next(rece_generator)
            except StopIteration:
                break
            if data is None:
                break
            with audio_available:
                audio_available.wait_for(lambda: buff.getlen() <= BUFMAX)
                buff.extbuf(data)
                audio_available.notify()

        print("RECEIVER ENDS HERE")

    def player_consumer(buff):
        while running:
            sleep(SLEEPTIME)
            with audio_available:
                audio_available.wait_for(lambda: buff.getlen() >= 32)
                play(buff.getx(buff.getlen()))
                audio_available.notify()

        print("PLAYER ENDS HERE")

    global running

    rece_thread = Thread(target=receiver_producer, args=(rbuf, serversocket))
    play_thread = Thread(target=player_consumer, args=(rbuf,))
    rece_thread.start()
    play_thread.start()
    # input("press enter to exit")
    # running = False

    rece_thread.join()
    play_thread.join()
    return

def main():
    signal.signal(signal.SIGINT, signal_handler)

    serversocket = connect()
    global running
    t_thread = Thread(target=record_transmit_thread, args=(serversocket,))
    p_thread = Thread(target=receive_play_thread, args=(serversocket,))
    t_thread.start()
    p_thread.start()
    input("press enter to exit")
    running = False
    sdstream.stop()
    t_thread.join()
    p_thread.join()
    serversocket.close()


def connect():
    global source_name
    global SERVER_IP
    global SERVER_PORT
    global destination_name

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((SERVER_IP, SERVER_PORT))

    #source_name = str(input("enter source name :"))
    source_name = FONTE
    print(f"hello {source_name}")
    print(f"message length = {len((source_name + (' ' * (512 - len(source_name)))).encode())}")
    s.send((source_name + (' ' * (512 - len(source_name)))).encode())

    #destination_name = str(input("enter destination name :"))
    destination_name = DESTINO
    s.send((destination_name + (' ' * (512 - len(destination_name)))).encode())
    sleep(2)

    # Set the socket to non-blocking mode
    s.setblocking(False)

    while True:
        try:
            # Attempt to receive data from the socket
            val = s.recv(2)
            if val:
                # Process the received data
                print("Received:", val.decode('utf-8'))
                break
        except socket.error as e:
            # Handle socket errors
            if e.errno == errno.EWOULDBLOCK:
                # No data available, continue waiting
                continue
            else:
                # Handle other socket errors
                print("Socket error:", e)
                break

    if val.decode() != 'go':
        raise TypeError
    # returns socket fd
    s.settimeout(5.0)
    return s


main()
# 2 separate websocket connections for receiving and sending files
# 2 separate threads to handle transmission and playback of the audio files


# start recording and keep sending data


# disconnect server


print("client terminating")