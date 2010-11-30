#!/usr/bin/python

import sys
import os
import os.path
import commands
import glob
import re
import unittest


#PROGRAM_PATH = "python mtm_cellular_dummy.py"
PROGRAM_PATH = os.environ.get("YANIV_PROGRAM", ".."+os.path.sep+"yaniv")
#PROGRAM_PATH = "python simulator_logic.py"
TESTS_DIR = os.environ.get("YANIV_TESTS_DIR", "random")
TEMP_DIR = os.environ.get("YANIV_TMP_DIR", "tmp")
TESTS_INPUT_SUFFIX = ".in.txt"
TESTS_OUTPUT_SUFFIX = ".out.txt"
RUN_VALGRIND = bool(os.environ.get("YANIV_RUN_VALGRIND",0))


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
    test_in = TESTS_DIR + os.path.sep + name + TESTS_INPUT_SUFFIX
    test_out = TEMP_DIR + os.path.sep + name + TESTS_OUTPUT_SUFFIX
    expected_out = TESTS_DIR + os.path.sep + name + TESTS_OUTPUT_SUFFIX
    test_command = PROGRAM_PATH + " " + test_in + " > " + test_out
    
    # run the actual program
    os.system(test_command)
    
    # compare the output
    real_out = file(test_out, "r").read()
    expected_out = file(expected_out, "r").read()
    
    # delete temp files if there was no error
    try:
        if real_out == expected_out:
            os.remove(test_out)
            os.rmdir(TEMP_DIR)
    except:
        pass
    
    return (real_out, expected_out)

def run_valgrind(name):
    """
    Runs the given test name by locating the correct test file input, and then
    running the program through valgrind and returning the error summary line.
    """
    # set up the file names to run
    test_in = TESTS_DIR + os.path.sep + name + TESTS_INPUT_SUFFIX
    test_command = "valgrind --leak-check=full " + PROGRAM_PATH \
                     + " " + test_in

    # run the actual commmand
    return commands.getoutput(test_command)


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
    
    def testRun(self):
        print "(%s) ... " % self.name,
        sys.stdout.flush()
        real_out, expected_out = run_program(self.name)
        real_out = real_out.split("\n")
        expected_out = expected_out.split("\n")
        for i in range(0, len(expected_out)):
            self.assertEquals(
                " ".join(real_out[i].split()),
                " ".join(expected_out[i].split()),
                self.name + (": line %d differs in output:\n" +
                    "real_out: '%s'\n!=\nexpected: '%s'") % (
                    (i+1), real_out[i], expected_out[i]))
        self.assertEquals(len(expected_out), len(real_out),
            self.name + ": length of expected output and actual output differ")


class TestValgrindRun(unittest.TestCase):
    """
    This is the base test case which performs a single run of the program,
    using valgrind, and checks that it contains no errors.
    """
    
    def __init__(self, name):
        self.name = name
        unittest.TestCase.__init__(self, "testValgrind")
        
    def id(self):
        return "TestProgramRun.testValgrind(" + self.name + ")"
    
    def testValgrind(self):
        print "(%s) ... " % self.name,
        sys.stdout.flush()
        val_res = run_valgrind(self.name)
        self.assertNotEquals(re.search(
                 "ERROR SUMMARY: 0 errors from 0 contexts", val_res), None,
                 "errors found by valgrind:\n%s" % val_res)
        self.assertNotEquals(re.search(
                 "in use at exit: 0 bytes in 0 blocks", val_res),
                 None, "memory leaks found by valgrind:\n%s" % val_res)



# create all of the actual test suites
test_files = glob.glob(TESTS_DIR + os.path.sep + "*.in.txt")
test_files = [f[len(TESTS_DIR + os.path.sep):f.rindex(TESTS_INPUT_SUFFIX)] for f in test_files]
if RUN_VALGRIND:
    test_cases = [TestValgrindRun(f) for f in test_files]
else:
    test_cases = [TestProgramRun(f) for f in test_files]

# create the suite and run them
suite = unittest.TestSuite()
suite.addTests(test_cases)
if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite)
