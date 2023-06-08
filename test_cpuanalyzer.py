import subprocess
import unittest

class TestCPUAnalyzer(unittest.TestCase):

    def test_open_proc_stat_file(self):
        result = subprocess.run(['./cpuanalyzer'], stdout=subprocess.PIPE)
        output = result.stdout.decode('utf-8')  # decode the bytes object into string
        self.assertIn('open_proc_stat_file PASSED', output)

    def test_get_available_proc(self):
        result = subprocess.run(['./cpuanalyzer'], stdout=subprocess.PIPE)
        output = result.stdout.decode('utf-8')  # decode the bytes object into string
        self.assertIn('get_available_proc PASSED', output)

    def test_get_proc_stat(self):
        result = subprocess.run(['./cpuanalyzer'], stdout=subprocess.PIPE)
        output = result.stdout.decode('utf-8')  # decode the bytes object into string
        self.assertIn('get_proc_stat PASSED', output)


if __name__ == '__main__':
    unittest.main()
