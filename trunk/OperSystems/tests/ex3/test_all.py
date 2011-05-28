#!/usr/bin/python

import os
import os.path
import sys
import glob

print

def usage_and_exit():
    print "Usage: python test_all.py [num of files] [size=small|medium|big|low-high]"
    print "                          [rerun|random|input] [test=testN] [clean] [help]"
    print
    print "       help         - Displays this usage and exits"
    print "       random       - Run random tests"
    print "       input        - Run input tests"
    print "       other        - Run tests from other people"
    print "       rerun        - Does not generate new random tests, only reruns failed ones"
    print "       num of files - Designate number of random test files to test"
    print "       size         - The size of each test. It can be one of these:"
    print "                      size=small : 20-100 lines in each test"
    print "                      size=medium: 100-500 lines in each test"
    print "                      size=big   : 500-2000 lines in each test"
    print "                      size=M-N   : M-N lines in each test"
    print "       test         - Designate specific tests to rerun. For example:"
    print "                      test=test1"
    print "                      test=test387,test108,test1"
    print "       clean        - Cleans all of the files from the previous run"
    print
    print "       DEFAULT: Run medium size random tests and input tests, on 20 files."
    print
    print "       NOTES  : Assumes that this is the project directory, and that"
    print "                the file 'libmp.a' is there compiled."
    print "                Also, if you are running on VMWARE use the option 'sync_print'"
    sys.exit(1)


# gather command line arguments
NUM_OF_FILES = 20
RANDOM = False
INPUT = False
OTHER = False
RERUN = False
CLEAN = False
SPECIFIC_TEST = ""
MIN_COMMANDS = 100
MAX_COMMANDS = 500
for arg in sys.argv[1:]:
    if arg == "help":
        usage_and_exit()
    elif arg == "sync_print":
        pass
    elif arg == "random":
        RANDOM = True
    elif arg == "input":
        INPUT = True
    elif arg == "other":
        OTHER = True
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
    elif arg[:len("test=")] == "test=":
        SPECIFIC_TEST = arg[len("test="):]
    else:
        try:
            NUM_OF_FILES = int(arg)
            RANDOM = True
        except:
            usage_and_exit()
    
# if no flags were given on input or random, do both by default
if not INPUT and not RANDOM and not CLEAN and not SPECIFIC_TEST and not OTHER:
    INPUT = True
    RANDOM = True
    OTHER = True

# if rerun is on, set NUM_OF_FILES to be zero
if RERUN:
    RANDOM = True
    NUM_OF_FILES = 0

if SPECIFIC_TEST:
    NUM_OF_FILES = 0

if (not RERUN) and (not SPECIFIC_TEST):
    CLEAN = True

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

TEST_ZVI = "./zvi_test > /dev/null"
TEST_VELICH = "./velich_test"
TEST_UNITTESTS = "./extern_unittest"


# run the test_files script on all files in all input dirs
if INPUT:
  print
  print "========================"
  print "Input Tests"
  print "========================"
  os.system(TEST_UNITTESTS)
  input_dirs = glob.glob(INPUT_FILES_DIR + os.sep + "*")
  for dir in input_dirs:
    print
    print "*** Tests from %s: ***" % dir
    os.environ["THREADS_TESTS_DIR"] = dir
    os.system(TEST_FILES_CMD)
    del os.environ["THREADS_TESTS_DIR"]

# run other people's tests
if OTHER:
    print
    print "========================"
    print "Tests By Others"
    print "========================"
    os.system(TEST_ZVI)
    os.system(TEST_VELICH)

# run the random files script
if RANDOM:
    print
    print "========================"
    print "Random Tests"
    print "========================"
    os.environ["THREADS_MIN_COMMANDS"] = str(MIN_COMMANDS)
    os.environ["THREADS_MAX_COMMANDS"] = str(MAX_COMMANDS)
    os.system(TEST_RANDOM_CMD)

# run specific tests
if SPECIFIC_TEST:
    print
    print "========================"
    print "Specific Tests"
    print "========================"
    os.environ["THREADS_SPECIFIC_TEST"] = SPECIFIC_TEST
    os.system(TEST_RANDOM_CMD)
