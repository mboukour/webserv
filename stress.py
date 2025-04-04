import requests
import concurrent.futures
import time

# Define the target URL for stress testing
url = "http://127.0.0.1:8080"  # Change this to your server's URL

# Number of threads to simulate simultaneous clients
num_threads = 2

# Timeout value in seconds
timeout = 5  # Set a timeout of 5 seconds

# Function to send a GET request to the server
def send_request():
    try:
        response = requests.get(url, timeout=timeout)  # Adding the timeout here
        print(f"Request sent, Status Code: {response.status_code}")
    except requests.exceptions.Timeout:
        print(f"Request timed out after {timeout} seconds")
    except requests.exceptions.RequestException as e:
        print(f"Request failed: {e}")

# Function to perform the stress test
def stress_test():
    with concurrent.futures.ThreadPoolExecutor(max_workers=num_threads) as executor:
        futures = [executor.submit(send_request) for _ in range(1000)]  # Adjust number of requests here
        for future in concurrent.futures.as_completed(futures):
            future.result()  # Wait for completion of all tasks

if __name__ == "__main__":
    start_time = time.time()
    print("Starting stress test...")
    stress_test()
    print(f"Stress test completed in {time.time() - start_time:.2f} seconds.")
