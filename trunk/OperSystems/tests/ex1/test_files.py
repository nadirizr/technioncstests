#!/usr/bin/python

import commands
import glob
import os
import re
import sys
import time
import unittest


DEFAULT_PROGRAM_PATH = "./tag_launcher /usr/bin/python ./tag_wrapper.py"
PROGRAM_PATH = os.environ.get("TAGS_PROGRAM", DEFAULT_PROGRAM_PATH)
TESTS_DIR = os.environ.get("TAGS_TESTS_DIR", "random")
TEMP_DIR = os.environ.get("TAGS_TMP_DIR", "tmp")
TESTS_INPUT_SUFFIX = ".in.txt"
TESTS_OUTPUT_SUFFIX = ".out.txt"
TESTS_COMMANDS_SUFFIX = ".real.in.txt"

def str_to_int(s):
    num = ""
    for c in s:
        if c.isdigit():
            num += c
        else:
            break
    return int(num)

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
    test_in_file = TESTS_DIR + os.sep + name + TESTS_INPUT_SUFFIX
    test_out_file = TEMP_DIR + os.sep + name + TESTS_OUTPUT_SUFFIX
    test_command_file = TEMP_DIR + os.sep + name + TESTS_COMMANDS_SUFFIX
    expected_out_file = TESTS_DIR + os.sep + name + TESTS_OUTPUT_SUFFIX
    test_command = "%s %s %s %s" % (
        PROGRAM_PATH, test_in_file, test_out_file, test_command_file)
    
    # run the actual program
    pid = commands.getoutput(test_command)
    pid = str_to_int(pid)

    # wait for the process to finish
    while True:
      try:
        os.kill(pid, 0) 
        time.sleep(1)
      except:
        break
    
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
        
        real_line_pids = real_line_words[2:]
        expected_line_pids = expected_line_words[2:]
        if "0" in real_line_pids:
          real_line_pids.remove("0")
          if len(expected_line_pids) > 2:
            expected_line_pids.pop()

        if "1" in real_line_pids:
          real_line_pids.remove("1")
          if len(expected_line_pids) > 2:
            expected_line_pids.pop()
        real_line_words = ["DONE", str(len(real_line_pids))] + real_line_pids
        expected_line_words = ["DONE", str(len(expected_line_pids))] + expected_line_pids

        return " ".join(real_line_words), " ".join(expected_line_words)
    
    def testRun(self):
        print "(%s) ... " % self.name,
        sys.stdout.flush()
        real_out_file, real_out, expected_out_file, expected_out = \
            run_program(self.name)
        real_out = real_out.split("\n")
        expected_out = expected_out.split("\n")
        for i in range(0, len(expected_out)):
            if real_out[i] and real_out[i].startswith("DONE") and real_out[i].split() > 2:
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


# create all of the actual test suites
test_files = glob.glob(TESTS_DIR + os.sep + "*.in.txt")
test_files = [f[len(TESTS_DIR + os.sep):f.rindex(TESTS_INPUT_SUFFIX)] for f in test_files]
test_cases = [TestProgramRun(f) for f in test_files]

# create the suite and run them
suite = unittest.TestSuite()
suite.addTests(test_cases)
if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite)
