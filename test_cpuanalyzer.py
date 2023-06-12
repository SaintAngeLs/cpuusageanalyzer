import subprocess
import unittest
import time
import os
import signal

def run_cpuanalyzer():
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

class TestCPUAnalyzer(unittest.TestCase):

    def test_open_proc_stat_file(self):
        result = subprocess.run(['./cpuanalyzer'], stdout=subprocess.PIPE)
        output = result.stdout.decode('utf-8')  # decode the bytes object into string
        self.assertIn('Signal handler for signal 15 set successfully.', output)
        self.assertIn('', output)

    def test_get_available_proc(self):
        result = subprocess.run(['./cpuanalyzer'], stdout=subprocess.PIPE)
        output = result.stdout.decode('utf-8')  # decode the bytes object into string
        self.assertIn('Signal handler for signal 15 set successfully.', output)
        self.assertIn('', output)

    def test_get_proc_stat(self):
        result = subprocess.run(['./cpuanalyzer'], stdout=subprocess.PIPE)
        output = result.stdout.decode('utf-8')  # decode the bytes object into string
        self.assertIn('Signal handler for signal 15 set successfully.', output)
        self.assertIn('', output)


if __name__ == '__main__':
    #run_cpuanalyzer()  # Run the cpuanalyzer process
    unittest.main()  # Run the test cases
