#!/usr/bin/python

import os
import os.path
import sys
import glob


def usage_and_exit():
    print "Usage: python test_all.py [num of files] [size=small|medium|big|low-high]"
    print "                          [rerun|random|input] [clean] [help]"
    print
    print "       help         - Displays this usage and exits"
    print "       random       - Run random tests"
    print "       input        - Run input tests"
    print "       rerun        - Does not generate new random tests, only reruns failed ones"
    print "       num of files - Designate number of random test files to test"
    print "       size         - The size of each test. It can be one of these:"
    print "                      size=small : 20-100 lines in each test"
    print "                      size=medium: 100-500 lines in each test"
    print "                      size=big   : 500-2000 lines in each test"
    print "                      size=M-N   : M-N lines in each test"
    print "       clean        - Cleans all of the files from the previous run"
    print
    print "       DEFAULT: Run medium size random tests and input tests, on 500 files."
    print
    print "       NOTES  : Assumes that this is the project directory, and that"
    print "                the file 'syscall_tags.h' is there."
    sys.exit(1)


# gather command line arguments
NUM_OF_FILES = 500
RANDOM = False
INPUT = False
RERUN = False
CLEAN = False
MIN_COMMANDS = 100
MAX_COMMANDS = 500
for arg in sys.argv[1:]:
    if arg == "help":
        usage_and_exit()
    elif arg == "random":
        RANDOM = True
    elif arg == "input":
        INPUT = True
    elif arg == "rerun":
        RERUN = True
    elif arg == "clean":
        CLEAN = True
    elif arg[:len("size=")] == "size=":
        size = arg[len("size="):]
        if size == "small":
            MIN_COMMANDS = 20
            MAX_COMMANDS = 100
        elif size == "medium":
            MIN_COMMANDS = 100
            MAX_COMMANDS = 500
        elif size == "big":
            MIN_COMMANDS = 500
            MAX_COMMANDS = 2000
        else:
            try:
                dash = size.index("-")
                MIN_COMMANDS = int(size[:dash])
                MAX_COMMANDS = int(size[dash+1:])
            except:
                usage_and_exit()
    else:
        try:
            NUM_OF_FILES = int(arg)
        except:
            usage_and_exit()
    
# if no flags were given on input or random, do both by default
if not INPUT and not RANDOM and not CLEAN:
    INPUT = RANDOM = True

# if rerun is on, set NUM_OF_FILES to be zero
if RERUN:
    RANDOM = True
    NUM_OF_FILES = 0

if CLEAN:
    try:
        os.system("rm -rf ./random/*")
        os.system("rm -rf ./tmp/*")
    except:
        print "Clean Failed!"

# path constants
TEST_RANDOM_CMD = "python test_random.py " + str(NUM_OF_FILES)
TEST_FILES_CMD = "python test_files.py"
INPUT_FILES_DIR = "inputs"


# run the test_files script on all files in all input dirs
if INPUT:
  print
  print "========================"
  print "Input Tests"
  print "========================"
  input_dirs = glob.glob(INPUT_FILES_DIR + os.sep + "*")
  for dir in input_dirs:
    print
    print "*** Tests from %s: ***" % dir
    os.environ["TAGS_TESTS_DIR"] = dir
    os.system(TEST_FILES_CMD)
    del os.environ["TAGS_TESTS_DIR"]

# run the random files script
if RANDOM:
    print
    print "========================"
    print "Random Tests"
    print "========================"
    os.environ["TAGS_MIN_COMMANDS"] = str(MIN_COMMANDS)
    os.environ["TAGS_MAX_COMMANDS"] = str(MAX_COMMANDS)
    os.system(TEST_RANDOM_CMD)

