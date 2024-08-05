import json
import os
import time
import tkinter as tk
from datetime import datetime
from threading import Thread

import psutil
import requests

loop = True
data_dict = {}
interval = 1
server_url = 'https://tidycity.logimade.pt/server/api/road-to-recovery/network/upload-metrics/'  # Replace with your server URL


def on_closing():
    print('Window is closing...')
    global loop
    loop = False
    network_thread.join()  # Ensure the network monitoring thread has stopped
    print("\nSending data to server...")
    send_data_to_server(data_dict, server_url)

    os.makedirs(os.path.dirname(os.path.join(os.getcwd(), 'NetworkLogs/')), exist_ok=True)
    file_name = f"metrics_{datetime.now().strftime('%Y-%m-%d_%H-%M-%S')}.json"

    # Define the file path
    file_path = os.path.join(os.getcwd(), 'NetworkLogs', file_name)

    # Write the received data to the JSON file
    with open(file_path, 'w') as file:
        json.dump(data_dict, file, indent=4)

    root.destroy()


def get_network_usage(interval=1):
    """
    Measures the network usage (upload and download) over a specified interval.

    Args:
    interval (int): The time in seconds over which to measure the network usage.

    Returns:
    (float, float): Upload rate (bytes/sec), Download rate (bytes/sec)
    """
    try:
        initial_stats = psutil.net_io_counters()
        initial_bytes_sent = initial_stats.bytes_sent
        initial_bytes_recv = initial_stats.bytes_recv

        time.sleep(interval)

        final_stats = psutil.net_io_counters()
        final_bytes_sent = final_stats.bytes_sent
        final_bytes_recv = final_stats.bytes_recv

        upload_rate = (final_bytes_sent - initial_bytes_sent) / interval
        download_rate = (final_bytes_recv - initial_bytes_recv) / interval

        return upload_rate, download_rate

    except Exception as e:
        print(f"An error occurred: {e}")
        return 0.0, 0.0


def send_data_to_server(data, url):
    """
    Sends data to the specified server URL via HTTP POST.

    Args:
    data (dict): The data to send.
    url (str): The server URL.
    """
    try:
        response = requests.post(url, json=data)
        response.raise_for_status()  # Raises HTTPError for bad responses
        print(f"Data sent successfully. Server response: {response.text}")
    except requests.RequestException as e:
        print(f"Failed to send data: {e}")


def update_labels(upload_rate, download_rate):
    upload_label.config(text=f"Upload rate: {upload_rate:.2f} Mbps")
    download_label.config(text=f"Download rate: {download_rate:.2f} Mbps")


def network_monitor():
    while loop:
        upload_rate, download_rate = get_network_usage(interval)
        timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
        data_dict[timestamp] = {
            'upload_rate': round(upload_rate / 1024 / 1024 * 8, 2),  # Convert to KB/s, convert to MB/s, and Mbps
            'download_rate': round(download_rate / 1024 / 1024 * 8, 2)  # Convert to KB/s, convert to MB/s, and Mbps
        }
        # print(f"Upload rate: {upload_rate / 1024:.2f} KB/s, Download rate: {download_rate / 1024:.2f} KB/s")
        # print(f"Upload rate: {upload_rate / 1024 / 1024 * 8 :.2f} Mbps, Download rate: {download_rate / 1024 / 1024 * 8:.2f} Mbps")
        # print(data_dict)

        if loop:
            update_labels(data_dict[timestamp]['upload_rate'], data_dict[timestamp]['download_rate'])


if __name__ == "__main__":
    # Create the main window
    root = tk.Tk()
    root.title("Network Monitor")

    # Set the window size
    root.geometry("300x200")

    # Create labels for upload and download rates
    upload_label = tk.Label(root, text="Upload rate: 0.00 Mbps")
    upload_label.pack(pady=10)

    download_label = tk.Label(root, text="Download rate: 0.00 Mbps")
    download_label.pack(pady=10)

    # Bind the window close event to the on_closing function
    root.protocol("WM_DELETE_WINDOW", on_closing)

    # Start the network monitoring in a separate thread
    network_thread = Thread(target=network_monitor)
    network_thread.start()

    # Run the application
    root.mainloop()
