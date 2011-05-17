# Main Targets
TARGET = message_sender
MODULES = MAIN

# Compiler Flags
CC = gcc
CXX = g++
CCFLAGS = -I.. -I../src -g
LDFLAGS = -L.. -lmp

# Source Files
MESSAGE_SENDER_SRC = message_sender.c

# Object Files
OBJ_DIR = objects
MESSAGE_SENDER_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(MESSAGE_SENDER_SRC)},$(OBJ_DIR)/$(f)}
ALL_OBJ_FILES = $(MESSAGE_SENDER_OBJ_FILES)
TEST_JUNK = tmp random

# Target Definitions
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
	
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CCFLAGS) -c $< -o $@
	
message_sender: $(MESSAGE_SENDER_OBJ_FILES)
	$(CC) -o $@ $(MESSAGE_SENDER_OBJ_FILES) $(LDFLAGS)

# Build Targets
all: make_dirs $(TARGET)

clean:
	rm -rf $(ALL_OBJ_FILES) $(TARGET)

clean_test:
	rm -rf $(TEST_JUNK)

make_dirs:
	mkdir -p objects
	mkdir -p objects/tester
	mkdir -p tmp
