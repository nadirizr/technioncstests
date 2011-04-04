# Main Targets
TARGET = tag_launcher
MODULES = MAIN

TEST_TARGETS = test_scenarios test_pitfalls

# Compiler Flags
CC = gcc
CCFLAGS = -I.. -g

# Source Files
MAIN_SRC = tag_launcher.c

# Test Source Files
SCENARIOS_TEST_SRC = test_scenarios.c
PITFALLS_TEST_SRC = test_pitfalls.c
TEST_JUNK = tests/random tests/tmp

# Object Files
OBJ_DIR = objects
SRC_FILES = $(MAIN_SRC)
OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(SRC_FILES)},$(OBJ_DIR)/$(f)}
SCENARIOS_TEST_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(SCENARIOS_TEST_SRC)},$(OBJ_DIR_TEST)/$(f)}
PITFALLS_TEST_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(PITFALLS_TEST_SRC)},$(OBJ_DIR_TEST)/$(f)}
ALL_OBJ_FILES = $(OBJ_FILES)
ALL_TEST_OBJ_FILES = $(SCENARIOS_TEST_OBJ_FILES) $(PITFALLS_TEST_OBJ_FILES) $(ALL_OBJ_FILES)

# Target Definitions
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
	
$(TARGET): $(OBJ_FILES)
	$(CC) -o $@ $(ALL_OBJ_FILES) $(LDFLAGS)

# Test Target Definitions
test_scenarios: $(SCENARIOS_TEST_OBJ_FILES)
	$(CC) -o $@ $(SCENARIOS_TEST_OBJ_FILES) $(LDFLAGS)
	
test_pitfalls: $(PITFALLS_TEST_OBJ_FILES)
	$(CC) -o $@ $(PITFALLS_TEST_OBJ_FILES) $(LDFLAGS)

# Build Targets
all: make_dirs $(TARGET) $(TEST_TARGETS)

clean:
	rm -rf $(OBJ_FILES) $(ALL_TEST_OBJ_FILES) $(TARGET) $(TEST_TARGETS)

test_unit: $(TEST_TARGETS)
	echo -e "\n==============\nUnit Tests\n=============="
	./test_scenarios
	./test_pitfalls

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
	mkdir -p tests/tmp
