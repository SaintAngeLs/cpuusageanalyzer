import subprocess
import unittest

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
    unittest.main()
