import psutil
import time


def get_network_usage(interval=1):
    """
    Measures the network usage (upload and download) over a specified interval.

    Args:
    interval (int): The time in seconds over which to measure the network usage.

    Returns:
    (float, float): Upload rate (bytes/sec), Download rate (bytes/sec)
    """
    # Get the initial network statistics
    initial_stats = psutil.net_io_counters()
    initial_bytes_sent = initial_stats.bytes_sent
    initial_bytes_recv = initial_stats.bytes_recv

    # Wait for the specified interval
    time.sleep(interval)

    # Get the network statistics again
    final_stats = psutil.net_io_counters()
    final_bytes_sent = final_stats.bytes_sent
    final_bytes_recv = final_stats.bytes_recv

    # Calculate the rates
    upload_rate = (final_bytes_sent - initial_bytes_sent) / interval
    download_rate = (final_bytes_recv - initial_bytes_recv) / interval

    return upload_rate, download_rate


# Example usage
if __name__ == "__main__":
    interval = 1  # Measure every 1 second
    while True:
        upload_rate, download_rate = get_network_usage(interval)
        print(f"Upload rate: {upload_rate / 1024:.2f} KB/s, Download rate: {download_rate / 1024:.2f} KB/s")
