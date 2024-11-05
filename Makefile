# Makefile for words.c with four test files

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Target executable
TARGET = words

# Source file
SRC = words.c

# Test files
TEST_FILES = sample.txt sample2.txt sample3.txt sample4.txt

# Output directory
OUTPUT_DIR = output

# Output files for each test
OUTPUT_FILES = $(patsubst %, $(OUTPUT_DIR)/%.out, $(basename $(TEST_FILES)))

# Default target
all: $(TARGET)

# Compile the program
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Run tests and save output
test: $(TARGET) $(OUTPUT_FILES)

$(OUTPUT_DIR)/%.out: %.txt $(TARGET)
	@mkdir -p $(OUTPUT_DIR)
	./$(TARGET) $< > $@

# Clean up object files, executable, and test output
clean:
	rm -f $(TARGET)
	rm -rf $(OUTPUT_DIR)

.PHONY: all test clean
