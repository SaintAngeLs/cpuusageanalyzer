# Makefile fot the CPU usage analizer: 
CC = gcc
# enabling the clang compiler
CLANG = clang
CFLAGS = -Wall -Werror -std=c99

PROGRAM = cpuanalyzer
SRCS = cpuanalyzer.c
OBJS = $(SRCS:.c=.o)

# Add the name of your test script here
TEST_SCRIPT = test_cpuanalyzer.py

.PHONY: all clean clang test

# pointing the default target
all: $(PROGRAM)

# Rule fot the program building
$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROGRAM) $(OBJS)

# Rule to compile the source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Targe for the Clang compilation
clang:
	$(MAKE) CC=$(CLANG) all

# Target to run the test script
test: $(PROGRAM)
	python $(TEST_SCRIPT)

# Ratget to clean up all the generated files
clean:
	rm -f $(PROGRAM) $(OBJS)
