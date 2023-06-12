# CPU Usage Analyzer

This is a CPU usage analyzer program that calculates and analyzes the CPU usage of the system. It consists of multiple components, including the reader, analyzer, printer, watchdog, and logger. The program reads the `/proc/stat` file to obtain CPU usage information, performs calculations, and presents the results.

## How to Run

To run the CPU usage analyzer program, follow these steps:

1. Ensure that you have the required dependencies installed, including GCC (GNU Compiler Collection) and Python.

2. Clone or download this repository to your local machine.

3. Open a terminal and navigate to the project directory.

4. Build the program by running the following command:

   make cpuanalyzer

5. Once the compilation is successful, you can run the program by executing the following command:

   ./cpuanalyzer

6. The program will start analyzing the CPU usage and displaying the results.

## Testing

To run the tests for the CPU usage analyzer, make sure you have Python installed on your system.

1. Open a terminal and navigate to the project directory.

2. Run the tests by executing the following command:

   make test

   This command will compile the program and run the test script (`test_cpuanalyzer.py`). The test cases will be executed, and the test results will be displayed.

## Clean Up

To clean up the generated files, use the following command:

make clean

This command will remove the compiled program and object files from the project directory.

---

Feel free to explore the source code files (`cpuanalyzer.c`, `reader_cpuanalyzer.c`, `analyzer_cpuanalyzer.c`, `printer_cpuanalyzer.c`, `watchdog_cpuanalyzer.c`, `logger_cpuanalyzer.c`) for more details about the implementation of the CPU usage analyzer.

If you encounter any issues or have questions, please feel free to create an issue on the GitHub repository.
