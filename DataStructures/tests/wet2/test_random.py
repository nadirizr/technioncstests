#!/usr/bin/python

import os
import os.path
import sys

# determine amount of files to create and test
NUM_OF_FILES = 1000
if sys.argv[1:]:
    NUM_OF_FILES = int(sys.argv[1])


# path constants
CREATE_FILES_CMD = "(cd simulator ; python " + \
        "create_tests.py " + str(NUM_OF_FILES) + ")"
TEST_FILES_CMD = "python test_files.py"
CREATE_FILES_DIR = "random"
TEST_FILES_DIR = "tmp"


# create paths
try:
  os.makedirs(CREATE_FILES_DIR)
  os.makedirs(TEST_FILES_DIR)
except:
  pass

# run the create_files script
print CREATE_FILES_CMD
os.system(CREATE_FILES_CMD)

# run the test_files script
print TEST_FILES_CMD
os.system(TEST_FILES_CMD)

# cleanup
try:
  os.rmdir(CREATE_FILES_DIR)
  os.rmdir(TEST_FILES_DIR)
except:
  pass
