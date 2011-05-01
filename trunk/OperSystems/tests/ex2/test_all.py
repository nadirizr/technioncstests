#!/usr/bin/python

import os
import os.path
import sys
import glob


def usage_and_exit():
    print "Usage: python test_all.py [input|clean] [test=testN][help]"
    print
    print "       help         - Displays this usage and exits"
    print "       input        - Run input tests"
    print "       test         - Designate specific tests to rerun. For example:"
    print "                      test=test1"
    print "                      test=test387,test108,test1"
    print "       clean        - Cleans all of the files from the previous run"
    print
    print "       DEFAULT: Run all input files."
    print
    print "       NOTES  : Assumes that this is the project directory, and that"
    print "                the file 'hw2_syscalls.h' is there."
    sys.exit(1)


# gather command line arguments
INPUT = False
CLEAN = False
SPECIFIC_TEST = ""
for arg in sys.argv[1:]:
    if arg == "help":
        usage_and_exit()
    elif arg == "input":
        INPUT = True
    elif arg == "clean":
        CLEAN = True
    elif arg[:len("test=")] == "test=":
        SPECIFIC_TEST = arg[len("test="):]
    else:
        usage_and_exit()
    
# if no flags were given on input or random, do both by default
if not INPUT and not CLEAN and not SPECIFIC_TEST:
    INPUT = True

if CLEAN:
    try:
        os.system("rm -rf ./random/*")
        os.system("rm -rf ./tmp/*")
    except:
        print "Clean Failed!"

# path constants
TEST_FILES_CMD = "python test_files.py"
INPUT_FILES_DIR = "inputs"


# run the test_files script on all files in all input dirs
if INPUT:
    print
    print "========================"
    print "Input Tests"
    print "========================"
    os.environ["SHORT_TESTS_DIR"] = INPUT_FILES_DIR
    os.system(TEST_FILES_CMD)
    del os.environ["SHORT_TESTS_DIR"]
  
# run specific tests
if SPECIFIC_TEST:
    print
    print "========================"
    print "Specific Tests"
    print "========================"
    os.environ["SHORT_TESTS_DIR"] = INPUT_FILES_DIR
    os.environ["SHORT_SPECIFIC_TEST"] = SPECIFIC_TEST
    os.system(TEST_FILES_CMD)
    del os.environ["SHORT_TESTS_DIR"]
