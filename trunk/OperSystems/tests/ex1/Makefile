# Main Targets
TARGET = tag_process tag_launcher
MODULES = MAIN

TEST_TARGETS = test_scenarios

# Compiler Flags
CC = gcc
CXX = g++
CCFLAGS = -I.. -g

# Source Files
TAG_PROCESS_SRC = tester/tag_process.c
TAG_LAUNCHER_SRC = tester/tag_launcher.c

# Test Source Files
SCENARIOS_TEST_SRC = test_scenarios.c
TEST_JUNK = random tmp

# Object Files
OBJ_DIR = objects
TAG_PROCESS_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(TAG_PROCESS_SRC)},$(OBJ_DIR)/$(f)}
TAG_LAUNCHER_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(TAG_LAUNCHER_SRC)},$(OBJ_DIR)/$(f)}
SCENARIOS_TEST_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(SCENARIOS_TEST_SRC)},$(OBJ_DIR)/$(f)}
ALL_OBJ_FILES = $(TAG_PROCESS_OBJ_FILES) $(TAG_LAUNCHER_OBJ_FILES)
ALL_TEST_OBJ_FILES = $(SCENARIOS_TEST_OBJ_FILES) $(ALL_OBJ_FILES)

# Target Definitions
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
	
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CCFLAGS) -c $< -o $@
	
tag_process: $(TAG_PROCESS_OBJ_FILES)
	$(CC) -o $@ $(TAG_PROCESS_OBJ_FILES) $(LDFLAGS)

tag_launcher: $(TAG_LAUNCHER_OBJ_FILES)
	$(CC) -o $@ $(TAG_LAUNCHER_OBJ_FILES) $(LDFLAGS)

# Test Target Definitions
test_scenarios: $(SCENARIOS_TEST_OBJ_FILES)
	$(CXX) -o $@ $(SCENARIOS_TEST_OBJ_FILES) $(LDFLAGS)
	
# Build Targets
all: make_dirs $(TARGET) $(TEST_TARGETS)

clean:
	rm -rf $(OBJ_FILES) $(ALL_TEST_OBJ_FILES) $(TARGET) $(TEST_TARGETS)

test_unit: $(TEST_TARGETS)
	echo -e "\n==============\nUnit Tests\n=============="
	python ./tag_launcher.py ./test_scenarios

test_system: $(TARGET)
	echo -e "\n==============\nInput Tests\n=============="
	(cd tests; python test_all.py input)
	echo -e "\n==============\nSysten Tests\n=============="
	(cd tests; python test_all.py random 500)

test: test_unit test_system

clean_test:
	rm -rf $(TEST_JUNK)

make_dirs:
	mkdir -p objects
	mkdir -p objects/tester
	mkdir -p tmp
