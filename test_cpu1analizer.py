
import threading
import time

BUFFER_SIZE = 100

array_stat_count = 0
available_proc = 0
slots_filled_sem = threading.Semaphore(0)
slots_empty_sem = threading.Semaphore(BUFFER_SIZE)
bufferMutex = threading.Lock()
array_stat = [None] * BUFFER_SIZE

class KernelProcStat:
    def __init__(self):
        self.name = ""
        self.user = 0
        self.nice = 0
        self.system = 0
        self.idle = 0
        self.iowait = 0
        self.irq = 0
        self.softirq = 0
        self.steal = 0
        self.guest = 0
        self.guest_nice = 0

def get_available_proc():
    global available_proc
    available_proc = os.cpu_count()
    if available_proc == None or available_proc == -1:
        print("Error: Failed to get the number of available processors")
        return -1
    return 0

def get_proc_stat():
    file_to_read = open("/proc/stat", "r")
    lines = file_to_read.readlines()
    file_to_read.close()

    stat = []
    for line in lines:
        if line.startswith("cpu"):
            parts = line.split()
            name = parts[0]
            values = [int(x) for x in parts[1:]]
            stat.append((name, values))

    return stat

def analyzer_proc_stat_thread():
    while True:
        slots_filled_sem.acquire()
        bufferMutex.acquire()

        stat = array_stat[array_stat_count - 1]
        array_stat_count -= 1

        bufferMutex.release()
        slots_empty_sem.release()

        print(f"Name       User       Nice       System     Idle       IOWait     IRQ        SoftIRQ    Steal      Guest      GuestNice")
        print("=" * 110)
        for s in stat:
            name, user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice = s
            print(f"{name:<10s} {user:<10d} {nice:<10d} {system:<10d} {idle:<10d} {iowait:<10d} {irq:<10d} {softirq:<10d} {steal:<10d} {guest:<10d} {guest_nice:<10d}")
        print()

def read_proc_stat_thread():
    while True:
        stat = get_proc_stat()

        slots_empty_sem.acquire()
        bufferMutex.acquire()

        array_stat[array_stat_count] = stat
        array_stat_count += 1

        bufferMutex.release()
        slots_filled_sem.release()

def main():
    if get_available_proc() == -1:
        print("No available processors")
        return

    available_proc = 1  # Incrementing available_proc (for testing purposes)

    threads = []
    threads.append(threading.Thread(target=read_proc_stat_thread))
    threads.append(threading.Thread(target=analyzer_proc_stat_thread))

    for t in threads:
        t.start()

    # Wait for the threads to finish
    time.sleep(10)  # Adjust the sleep duration based on your requirement

    # Set the termination condition
    global array_stat_count
    array_stat_count = BUFFER_SIZE  # Assuming this will terminate the threads

    for t in threads:
        t.join()

if __name__ == "__main__":
    main()