# Main Targets
TARGET = unittest message_sender zvi_test
MODULES = MAIN

# Compiler Flags
CC = gcc
CXX = g++
CCFLAGS = -I.. -I../src -g
LDFLAGS = -L.. -lmp -lpthread

# Source Files
MESSAGE_SENDER_SRC = message_sender.c
UNITTEST_SRC = unittests/all_tests.c unittests/unittest.c unittests/hash_table_test.c unittests/list_test.c
ZVI_TEST_SRC = zvi_test.c

# Object Files
OBJ_DIR = objects
MESSAGE_SENDER_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(MESSAGE_SENDER_SRC)},$(OBJ_DIR)/$(f)}
UNITTEST_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(UNITTEST_SRC)},$(OBJ_DIR)/$(f)}
ZVI_TEST_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(ZVI_TEST_SRC)},$(OBJ_DIR)/$(f)}
ALL_OBJ_FILES =  $(UNITTEST_OBJ_FILES) $(MESSAGE_SENDER_OBJ_FILES) $(ZVI_TEST_OBJ_FILES)
TEST_JUNK = tmp random

# Target Definitions
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
	
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CCFLAGS) -c $< -o $@
	
unittest: $(UNITTEST_OBJ_FILES)
	$(CC) -o $@ $(UNITTEST_OBJ_FILES) $(LDFLAGS)

message_sender: $(MESSAGE_SENDER_OBJ_FILES)
	$(CC) -o $@ $(MESSAGE_SENDER_OBJ_FILES) $(LDFLAGS)

zvi_test: $(ZVI_TEST_OBJ_FILES)
	$(CC) -o $@ $(ZVI_TEST_OBJ_FILES) $(LDFLAGS)

# Build Targets
all: make_dirs $(TARGET)

clean:
	rm -rf $(ALL_OBJ_FILES) $(TARGET)

clean_test:
	rm -rf $(TEST_JUNK)

make_dirs:
	mkdir -p objects
	mkdir -p objects/tester
	mkdir -p objects/unittests
	mkdir -p tmp

# Test Targets
test: unittest
	./unittest
