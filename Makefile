# Makefile fot the CPU usage analizer: 
CC = gcc
# enabling the clang compiler
CLANG = clang 
CFLAGS = -Wall -Wextra -Werror -std=c99 -Weverything  -Wno-unused-parameter -Wno-unused-variable

PROGRAM = cpuanalyzer
SRCS = cpuanalyzer.c \
       reader_cpuanalyzer.c \
       analyzer_cpuanalyzer.c \
       printer_cpuanalyzer.c \
       watchdog_cpuanalyzer.c \
       logger_cpuanalyzer.c
OBJS = $(SRCS:.c=.o)

# Add the name of your test script here
TEST_DIR = tests
TEST_SCRIPT = $(TEST_DIR)/test_cpuanalyzer.py

.PHONY: all clean test

# Default target
all: $(PROGRAM)

# Rule for building the program
$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROGRAM) $(OBJS)

# Rule to compile the source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Target to run the test script
test: $(PROGRAM)
	python $(TEST_SCRIPT)

# Target to clean up all generated files
clean:
	rm -f $(PROGRAM) $(OBJS)