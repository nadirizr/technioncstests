#!/usr/bin/python

import sys
import os
import os.path
import commands
import glob
import re
import unittest


DEFAULT_PROGRAM_PATH = "./tester/tag_launcher python ./tester/tag_wrapper.py"
PROGRAM_PATH = os.environ.get("TAGS_PROGRAM", DEFAULT_PROGRAM_PATH)
TESTS_DIR = os.environ.get("TAGS_TESTS_DIR", "random")
TEMP_DIR = os.environ.get("TAGS_TMP_DIR", "tmp")
TESTS_INPUT_SUFFIX = ".in.txt"
TESTS_OUTPUT_SUFFIX = ".out.txt"

TAG_PROCESS_WRITE_PIPE = "/tmp/tag_process_write_pipe"
TAG_PROCESS_READ_PIPE = "/tmp/tag_process_read_pipe"


def create_pipes():
    try:
      os.remove(TAG_PROCESS_READ_PIPE)
      os.remove(TAG_PROCESS_WRITE_PIPE)
    except:
      pass
    try:
      os.mkfifo(TAG_PROCESS_READ_PIPE)
      os.mkfifo(TAG_PROCESS_WRITE_PIPE)
    except:
      pass

def run_program(name):
    """
    Runs the given test name by locating the correct test file input and output
    and then compares the output - returning both the real and expected ouput
    and error.
    """
    # make sure the temp dir exists
    try:
        os.makedirs(TEMP_DIR)
    except:
        pass
    
    # set up all file names for run
    test_in_file = TESTS_DIR + os.path.sep + name + TESTS_INPUT_SUFFIX
    test_out_file = TEMP_DIR + os.path.sep + name + TESTS_OUTPUT_SUFFIX
    expected_out_file = TESTS_DIR + os.path.sep + name + TESTS_OUTPUT_SUFFIX
    test_command = "%s %s %s %s > %s" % (
        PROGRAM_PATH, test_in_file, TAG_PROCESS_READ_PIPE,
        TAG_PROCESS_WRITE_PIPE, test_out_file)
    
    # run the actual program
    os.system(test_command)
    
    # compare the output
    try:
      real_out = file(test_out_file, "r").read()
    except:
      raise RuntimeError(
          ("Output file from your code ('%s') does not exist!\n" +
           "Possible reasons:\n" +
           "1) SEGMENTATION FAULT in your code prevented output\n" +
           "2) No more free space on hard drive (especially on T2)\n" +
           "   You could try deleting tests/random and tests/tmp directories") %
          (test_out_file))
    expected_out = file(expected_out_file, "r").read()
    
    # delete temp files if there was no error
    try:
        if real_out == expected_out:
            os.remove(test_out_file)
            os.rmdir(TEMP_DIR)
    except:
        pass
    
    return (test_out_file, real_out, expected_out_file, expected_out)


class TestProgramRun(unittest.TestCase):
    """
    This is the base test case which performs a single run of the program, and
    compares it's output to the expected one.
    """
    
    def __init__(self, name):
        self.name = name
        unittest.TestCase.__init__(self, "testRun")
        
    def id(self):
        return "TestProgramRun.testRun(" + self.name + ")"

    def fixGetGoodProcessesOutput(self, real_line, expected_line):
        real_line_words = real_line.split()
        expected_line_words = expected_line.split()
        
        if "0" in real_line_words:
          real_line_words.remove("0")
          expected_line_words.pop()

        if "1" in real_line_words:
          real_line_words.remove("1")
          expected_line_words.pop()

        return " ".join(real_line_words), " ".join(expected_line_words)
    
    def testRun(self):
        print "(%s) ... " % self.name,
        sys.stdout.flush()
        real_out_file, real_out, expected_out_file, expected_out = \
            run_program(self.name)
        real_out = real_out.split("\n")
        expected_out = expected_out.split("\n")
        for i in range(0, len(expected_out)):
            if "DONE" in real_out[i] and real_out[i].split() > 2:
                real_out[i], expected_out[i] = self.fixGetGoodProcessesOutput(
                    real_out[i], expected_out[i])
            self.assertEquals(
                " ".join(real_out[i].split()),
                " ".join(expected_out[i].split()),
                self.name + (": line %d differs in output:\n" +
                    "real_out (file:'%s')\t: '%s'\n!=\n" +
                    "expected (file:'%s')\t: '%s'") % (
                    (i+1), real_out_file, real_out[i],
                    expected_out_file, expected_out[i]))
        self.assertEquals(len(expected_out), len(real_out),
            self.name + ": length of expected output and actual output differ")


# create the named pipe that will bridge the python and C
create_pipes()

# create all of the actual test suites
test_files = glob.glob(TESTS_DIR + os.path.sep + "*.in.txt")
test_files = [f[len(TESTS_DIR + os.path.sep):f.rindex(TESTS_INPUT_SUFFIX)] for f in test_files]
test_cases = [TestProgramRun(f) for f in test_files]

# create the suite and run them
suite = unittest.TestSuite()
suite.addTests(test_cases)
if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite)
