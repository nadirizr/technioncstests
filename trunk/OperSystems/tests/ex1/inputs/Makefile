# Main Targets
TARGET = assignment_test_v3 test1 usom_test wet1-tst-2-first-syscalls-fixed

# Compiler Flags
CC = gcc
CXX = g++
CCFLAGS = -I../.. -g

# Test Source Files
ASSIGNMENT_TEST_SRC = assignment_test_v3.c
TEST1_TEST_SRC = test1.c
USOM_TEST_SRC = usom_test.c
WET1_TST2_TEST_SRC = wet1-tst-2-first-syscalls-fixed.c

# Object Files
OBJ_DIR = objects
ASSIGNMENT_TEST_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(ASSIGNMENT_TEST_SRC)},$(OBJ_DIR)/$(f)}
TEST1_TEST_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(TEST1_TEST_SRC)},$(OBJ_DIR)/$(f)}
USOM_TEST_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(USOM_TEST_SRC)},$(OBJ_DIR)/$(f)}
WET1_TST2_TEST_OBJ_FILES = ${foreach f,${patsubst %.c,%.o,$(WET1_TST2_TEST_SRC)},$(OBJ_DIR)/$(f)}
ALL_TEST_OBJ_FILES = $(ASSIGNMENT_TEST_OBJ_FILES) $(TEST1_TEST_OBJ_FILES) $(USOM_TEST_OBJ_FILES) $(WET1_TST2_TEST_OBJ_FILES)

# Target Definitions
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
	
assignment_test_v3: $(ASSIGNMENT_TEST_OBJ_FILES)
	$(CC) -o $@ $(ASSIGNMENT_TEST_OBJ_FILES) $(LDFLAGS)

test1: $(TEST1_TEST_OBJ_FILES)
	$(CC) -o $@ $(TEST1_TEST_OBJ_FILES) $(LDFLAGS)

usom_test: $(USOM_TEST_OBJ_FILES)
	$(CC) -o $@ $(USOM_TEST_OBJ_FILES) $(LDFLAGS)

wet1-tst-2-first-syscalls-fixed: $(WET1_TST2_TEST_OBJ_FILES)
	$(CC) -o $@ $(WET1_TST2_TEST_OBJ_FILES) $(LDFLAGS)

# Build Targets
all: make_dirs $(TARGET)

clean:
	rm -rf $(ALL_TEST_OBJ_FILES) $(TARGET)

test: $(TEST_TARGETS)
	echo -e "\n==============\nExternal Tests\n=============="
	(cd .. ; python ./tag_launcher.py ./inputs/assignment_test_v3)
	(cd .. ; python ./tag_launcher.py ./inputs/test1)
	(cd .. ; python ./tag_launcher.py ./inputs/usom_test)
	(cd .. ; python ./tag_launcher.py ./wet1-tst-2-first-syscalls-fixed)

clean_test:
	rm -rf $(TEST_JUNK)

make_dirs:
	mkdir -p objects
