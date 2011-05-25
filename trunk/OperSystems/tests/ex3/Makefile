# Main Targets
TARGET = all_unittest threads_process zvi_test velich_test arik_test
MODULES = MAIN

# Compiler Flags
CC = gcc
CXX = g++
CCFLAGS = -I.. -g
LDFLAGS = -L.. -lmp -lpthread

# Source Files
THREADS_PROCESS_SRC = tester/threads_process.c
UNITTEST_SRC = unittests/unittest.c unittests/barrier_test.c unittests/context_test.c unittests/bad_send_test.c
ALL_UNITTEST_SRC = unittests/all_tests.c unittests/hash_table_test.c unittests/list_test.c $(UNITTEST_SRC)
EXTERN_UNITTEST_SRC = unittests/external_tests.c $(UNITTEST_SRC)
ZVI_TEST_SRC = zvi/zvi_test.c
VELICH_TEST_SRC = velich/mp_tester.c velich/mp_tester_library.c
ARIK_TEST_SRC = arik/tst_mp.c

# Object Files
OBJ_DIR = objects
THREADS_PROCESS_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(THREADS_PROCESS_SRC)},$(OBJ_DIR)/$(f)}
ALL_UNITTEST_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(ALL_UNITTEST_SRC)},$(OBJ_DIR)/$(f)}
EXTERN_UNITTEST_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(EXTERN_UNITTEST_SRC)},$(OBJ_DIR)/$(f)}
ZVI_TEST_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(ZVI_TEST_SRC)},$(OBJ_DIR)/$(f)}
VELICH_TEST_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(VELICH_TEST_SRC)},$(OBJ_DIR)/$(f)}
ARIK_TEST_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(ARIK_TEST_SRC)},$(OBJ_DIR)/$(f)}
ALL_OBJ_FILES =  $(ALL_UNITTEST_OBJ_FILES) $(THREADS_PROCESS_OBJ_FILES) $(ZVI_TEST_OBJ_FILES) $(VELICH_TEST_OBJ_FILES) $(ARIK_TEST_OBJ_FILES)
TEST_JUNK = tmp random

# Target Definitions
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
	
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CCFLAGS) -c $< -o $@
	
all_unittest: $(ALL_UNITTEST_OBJ_FILES)
	$(CC) -o $@ $(ALL_UNITTEST_OBJ_FILES) $(LDFLAGS)

extern_unittest: $(EXTERN_UNITTEST_OBJ_FILES)
	$(CC) -o $@ $(EXTERN_UNITTEST_OBJ_FILES) $(LDFLAGS)

threads_process: $(THREADS_PROCESS_OBJ_FILES)
	$(CC) -o $@ $(THREADS_PROCESS_OBJ_FILES) $(LDFLAGS)

zvi_test: $(ZVI_TEST_OBJ_FILES)
	$(CC) -o $@ $(ZVI_TEST_OBJ_FILES) $(LDFLAGS)

velich_test: $(VELICH_TEST_OBJ_FILES)
	$(CC) -o $@ $(VELICH_TEST_OBJ_FILES) $(LDFLAGS)

arik_test: $(ARIK_TEST_OBJ_FILES)
	$(CC) -o $@ $(ARIK_TEST_OBJ_FILES) $(LDFLAGS)

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
	mkdir -p objects/zvi
	mkdir -p objects/velich
	mkdir -p objects/arik
	mkdir -p tmp

# Test Targets
test_extern: extern_unittest zvi_test velich_test
	./extern_unittest
	./zvi_test > /dev/null
	./velich_test

test_all: all_unittest zvi_test velich_test
	./all_unittest
	./zvi_test > /dev/null
	./velich_test

test: all test_all
