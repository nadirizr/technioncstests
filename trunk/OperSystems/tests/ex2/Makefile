# Main Targets
TARGET = short_process test_special
MODULES = MAIN

# Compiler Flags
CC = gcc
CXX = g++
CCFLAGS = -I.. -Ilinux_include -g

# Source Files
SHORT_PROCESS_SRC = tester/short_process.c
TEST_SPECIAL_SRC = tester/test_special.cpp

# Object Files
OBJ_DIR = objects
SHORT_PROCESS_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(SHORT_PROCESS_SRC)},$(OBJ_DIR)/$(f)}
TEST_SPECIAL_OBJ_FILES = ${foreach f,${patsubst %.cpp,%.o,$(TEST_SPECIAL_SRC)},$(OBJ_DIR)/$(f)}
ALL_OBJ_FILES = $(SHORT_PROCESS_OBJ_FILES) $(TEST_SPECIAL_OBJ_FILES)
TEST_JUNK = tmp random

# Target Definitions
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
	
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CCFLAGS) -c $< -o $@
	
short_process: $(SHORT_PROCESS_OBJ_FILES)
	$(CC) -o $@ $(SHORT_PROCESS_OBJ_FILES) $(LDFLAGS)

test_special: $(TEST_SPECIAL_OBJ_FILES)
	$(CXX) -o $@ $(TEST_SPECIAL_OBJ_FILES) $(LDFLAGS)

# Build Targets
all: make_dirs $(TARGET)

clean:
	rm -rf $(ALL_OBJ_FILES) $(TARGET)

test_unit: $(TEST_TARGETS)
	echo -e "\n==============\nUnit Tests\n=============="
	./test_special

test: $(TARGET)
	(cd tests; python test_all.py input)
	
clean_test:
	rm -rf $(TEST_JUNK)

make_dirs:
	mkdir -p objects
	mkdir -p objects/tester
	mkdir -p tmp
