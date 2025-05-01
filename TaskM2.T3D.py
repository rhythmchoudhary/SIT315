import threading
import time
from queue import Queue
from collections import defaultdict
from datetime import datetime
import os

# Configuration
TOP_N = 3
BUFFER_SIZE = 10
NUM_PRODUCERS = 1
NUM_CONSUMERS = 2

data_queue = Queue(BUFFER_SIZE)
traffic_data_by_hour = defaultdict(lambda: defaultdict(int))
lock = threading.Lock()

# Helper: extract date and hour from timestamp
def get_date_hour(timestamp_str):
    dt = datetime.strptime(timestamp_str, "%Y-%m-%d %H:%M")
    return dt.date(), dt.hour

# Producer Thread
def producer_thread(file_path, thread_id):
    with open(file_path, 'r') as file:
        for line in file:
            if not line.strip():
                continue
            data_queue.put(line.strip())
            print(f"Producer-{thread_id} enqueued: {line.strip()}")
            time.sleep(0.1)
    for _ in range(NUM_CONSUMERS):
        data_queue.put(None)

# Consumer Thread
def consumer_thread(thread_id):
    while True:
        line = data_queue.get()
        if line is None:
            data_queue.task_done()
            break
        try:
            parts = line.split()
            if len(parts) != 4:
                raise ValueError("Malformed line")
            timestamp_str = f"{parts[0]} {parts[1]}"
            light_id = parts[2]
            cars = int(parts[3])
            date, hour = get_date_hour(timestamp_str)

            with lock:
                traffic_data_by_hour[(date, hour)][light_id] += cars
            print(f"Consumer-{thread_id} processed: {line}")
        except ValueError as e:
            print(f"Consumer-{thread_id} error: {e} -> {line}")
        finally:
            data_queue.task_done()

# Display top N congested traffic lights per hour per day
def display_top_congested():
    print("\nTop Congested Traffic Lights Per Hour Per Day:")
    for (date, hour) in sorted(traffic_data_by_hour.keys()):
        light_data = traffic_data_by_hour[(date, hour)]
        sorted_lights = sorted(light_data.items(), key=lambda x: -x[1])[:TOP_N]
        print(f"{date} {hour:02d}:00")
        for light_id, count in sorted_lights:
            print(f"  {light_id}: {count} cars")

# Generate sample data if not present
def create_sample_data(file_path):
    if os.path.exists(file_path):
        return
    with open(file_path, 'w') as f:
        f.write(
            "2025-05-01 00:00 TL1 20\n"
            "2025-05-01 00:05 TL2 15\n"
            "2025-05-01 00:10 TL3 25\n"
            "2025-05-01 01:00 TL1 30\n"
            "2025-05-01 01:05 TL2 35\n"
            "2025-05-01 01:10 TL4 20\n"
            "2025-05-01 02:00 TL1 10\n"
            "2025-05-01 02:05 TL2 40\n"
            "2025-05-01 02:10 TL3 35\n"
            "2025-05-02 00:10 TL1 30\n"
            "2025-05-02 00:20 TL2 35\n"
            "2025-05-02 00:30 TL3 15\n"
            "2025-05-02 01:00 TL4 45\n"
            "2025-05-02 01:30 TL1 20\n"
            "2025-05-02 01:40 TL2 30\n"
        )

# Main Function
def main():
    input_file = 'traffic_data.txt'
    create_sample_data(input_file)

    producers = [threading.Thread(target=producer_thread, args=(input_file, i)) for i in range(NUM_PRODUCERS)]
    consumers = [threading.Thread(target=consumer_thread, args=(i,)) for i in range(NUM_CONSUMERS)]

    for p in producers: p.start()
    for c in consumers: c.start()

    for p in producers: p.join()
    data_queue.join()
    for c in consumers: c.join()

    display_top_congested()

if __name__ == "__main__":
    main()
