import subprocess
import unittest
import time
import os
import signal

def test_run_cpuanalyzer():
    # Launch the cpuanalyzer process
    print("Launching cpuanalyzer process...")
    cpuanalyzer_process = subprocess.Popen(["./cpuanalyzer"])

    # Wait for a few seconds
    print("Waiting for cpuanalyzer to run...")
    time.sleep(5)

    # Send the SIGTERM signal to the cpuanalyzer process
    print("Sending SIGTERM signal to cpuanalyzer...")
    os.kill(cpuanalyzer_process.pid, signal.SIGTERM)

    # Wait for the process to terminate
    print("Waiting for cpuanalyzer to terminate...")
    cpuanalyzer_process.wait()

    print("cpuanalyzer terminated.")
    # Check if the program terminated successfully


def main():
    test_run_cpuanalyzer()
    time.sleep(5)

if __name__ == '__main__':
    main()
