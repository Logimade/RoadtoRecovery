import psutil
import time
import json
import requests

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

if __name__ == "__main__":
    data_dict = {}
    interval = 1
    server_url = 'http://yourserver.com/endpoint'  # Replace with your server URL

    try:
        while True:
            upload_rate, download_rate = get_network_usage(interval)
            timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
            data_dict[timestamp] = {
                'upload_rate': upload_rate / 1024,  # Convert to KB/s
                'download_rate': download_rate / 1024  # Convert to KB/s
            }
            print(f"Upload rate: {upload_rate / 1024:.2f} KB/s, Download rate: {download_rate / 1024:.2f} KB/s")
            print(data_dict)
    except KeyboardInterrupt:
        print("\nProgram terminated by user.")
        send_data_to_server(data_dict, server_url)  # Send data to the server only upon termination
