# Main Targets
TARGET = short_process
MODULES = MAIN

# Compiler Flags
CC = gcc
CXX = g++
CCFLAGS = -I.. -Ilinux_include -g

# Source Files
SHORT_PROCESS_SRC = tester/short_process.c

# Object Files
OBJ_DIR = objects
SHORT_PROCESS_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(SHORT_PROCESS_SRC)},$(OBJ_DIR)/$(f)}
ALL_OBJ_FILES = $(SHORT_PROCESS_OBJ_FILES)
TEST_JUNK = tmp random

# Target Definitions
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
	
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CCFLAGS) -c $< -o $@
	
short_process: $(SHORT_PROCESS_OBJ_FILES)
	$(CC) -o $@ $(SHORT_PROCESS_OBJ_FILES) $(LDFLAGS)

# Build Targets
all: make_dirs $(TARGET)

clean:
	rm -rf $(ALL_OBJ_FILES) $(TARGET)

test: $(TARGET)
	(cd tests; python test_all.py input)
	
clean_test:
	rm -rf $(TEST_JUNK)

make_dirs:
	mkdir -p objects
	mkdir -p objects/tester
	mkdir -p tmp
