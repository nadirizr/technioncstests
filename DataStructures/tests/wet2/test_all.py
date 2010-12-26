#!/usr/bin/python

import os
import os.path
import sys
import glob


def usage_and_exit():
    print "Usage: python test_all.py [num of files] [random|input|valgrind] [help]"
    print
    print "       help         - Displays this usage and exits"
    print "       random       - Run random tests"
    print "       input        - Run input tests"
    print "       valgrind     - Run tests with valgrind as well"
    print "                     (warning - this could take a while)"
    print "       num of files - Designate number of random test files to test"
    print
    print "       DEFAULT: Run random and input tests, no valgrind on 500 files."
    print
    print "       NOTES  : Assumes that this is the project directory, and that"
    print "                the executable name is 'servers'."
    sys.exit(1)


# gather command line arguments
NUM_OF_FILES = 500
VALGRIND = False
RANDOM = False
INPUT = False
for arg in sys.argv[1:]:
    if arg == "help":
        usage_and_exit()
    if arg == "valgrind":
	VALGRIND = True
    elif arg == "random":
        RANDOM = True
    elif arg == "input":
        INPUT = True
    else:
        try:
            NUM_OF_FILES = int(arg)
        except:
            usage_and_exit()
    
# if no flags were given on input or random, do both by default
if not INPUT and not RANDOM:
    INPUT = RANDOM = True

# path constants
TEST_RANDOM_CMD = "python test_random.py " + str(NUM_OF_FILES)
TEST_FILES_CMD = "python test_files.py"
INPUT_FILES_DIR = "inputs"


# run the test_files script on all files in all input dirs
print "========================"
print "Input Tests"
print "========================"
input_dirs = glob.glob(INPUT_FILES_DIR + os.path.sep + "*")
for dir in input_dirs:
    print
    if INPUT:
        print "*** Tests from %s: ***" % dir
        os.environ["SERVERS_TESTS_DIR"] = dir
        os.system(TEST_FILES_CMD)
        del os.environ["SERVERS_TESTS_DIR"]
    if VALGRIND:
        print "*** Tests from %s (valgrind): ***" % dir
        os.environ["SERVERS_TESTS_DIR"] = dir
        os.environ["SERVERS_RUN_VALGRIND"] = "1"
        os.system(TEST_FILES_CMD)
        del os.environ["SERVERS_RUN_VALGRIND"]
        del os.environ["SERVERS_TESTS_DIR"]

# run the random files script
if RANDOM:
    print
    print "========================"
    print "Random Tests"
    print "========================"
    os.system(TEST_RANDOM_CMD)

