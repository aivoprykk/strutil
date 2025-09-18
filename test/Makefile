# Makefile for numstr tests
# Usage:
#   make          - Build test executable
#   make test     - Build and run tests
#   make clean    - Clean build artifacts
#   make debug    - Build with debug symbols
#   make valgrind - Run tests with valgrind (if available)

# Compiler and flags
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -O2
DEBUG_CFLAGS = -std=c99 -Wall -Wextra -pedantic -g -O0 -DDEBUG
INCLUDES = -I../include -I../../logger_common/include

# Source files
SRC_DIR = ..
TEST_SRC = test_numstr.c
NUMSTR_SRC = $(SRC_DIR)/numstr.c

# Output
TEST_EXECUTABLE = test_numstr
OBJECTS = test_numstr.o numstr.o

# Default target
all: $(TEST_EXECUTABLE)

# Build test executable
$(TEST_EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile test file
test_numstr.o: $(TEST_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile numstr source
numstr.o: $(NUMSTR_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Build and run tests
test: $(TEST_EXECUTABLE)
	@echo "Running numstr tests..."
	@./$(TEST_EXECUTABLE)

# Debug build
debug: CFLAGS = $(DEBUG_CFLAGS)
debug: clean $(TEST_EXECUTABLE)

# Run with valgrind (if available)
valgrind: $(TEST_EXECUTABLE)
	@if command -v valgrind >/dev/null 2>&1; then \
		echo "Running tests with valgrind..."; \
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TEST_EXECUTABLE); \
	else \
		echo "Valgrind not found, running tests normally..."; \
		./$(TEST_EXECUTABLE); \
	fi

# Quick test run (build and run in one command)
run: test

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TEST_EXECUTABLE)

# Show help
help:
	@echo "Available targets:"
	@echo "  all       - Build test executable (default)"
	@echo "  test      - Build and run tests"
	@echo "  run       - Alias for test"
	@echo "  debug     - Build with debug symbols"
	@echo "  valgrind  - Run tests with valgrind (if available)"
	@echo "  clean     - Clean build artifacts"
	@echo "  help      - Show this help message"

# Phony targets
.PHONY: all test run debug valgrind clean help
