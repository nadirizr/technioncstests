#!/usr/bin/python

import commands
import glob
import os
import re
import sys
import time
import unittest


DEFAULT_PROGRAM_PATH = "./threads_process"
PROGRAM_PATH = os.environ.get("THREADS_PROGRAM", DEFAULT_PROGRAM_PATH)
SPECIFIC_TEST = os.environ.get("THREADS_SPECIFIC_TEST", "")
TESTS_DIR = os.environ.get("THREADS_TESTS_DIR", "random")
TEMP_DIR = os.environ.get("THREADS_TMP_DIR", "tmp")
TESTS_INPUT_SUFFIX = ".in.txt"
TESTS_OUTPUT_SUFFIX = ".out.txt"
TESTS_COMMANDS_SUFFIX = ".real.in.txt"

MAX_EVENT_ERROR_THRESHOLD = 0.1

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
    expected_out_file = TESTS_DIR + os.sep + name + TESTS_OUTPUT_SUFFIX
    test_command = "%s %s > %s" % (PROGRAM_PATH, test_in_file, test_out_file)
    
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
    
    return (test_out_file, real_out, expected_out_file, expected_out)


class TestProgramRun(unittest.TestCase):
    """
    This is the base test case which performs a single run of the program, and
    compares it's output to the expected one.
    """
    
    def __init__(self, name):
        self.name = name
        self.display_name = "<<< %s >>>" % self.name
        unittest.TestCase.__init__(self, "testRun")
        
    def id(self):
        return "TestProgramRun.testRun(" + self.name + ")"

    def checkRegistration(self, real_out_file, expected_out_file, real_out, expected_out):
        # check INIT line
        self.assertEquals(
            " ".join(real_out[0].split()),
            " ".join(expected_out[0].split()),
            self.display_name + (": INIT line differs or missing in output:\n" +
                "real_out (file:'%s' line:1)\t: '%s'\n!=\n" +
                "expected (file:'%s' line:1)\t: '%s'") % (
                real_out_file, real_out[0],
                expected_out_file, expected_out[0]))

        # check main registration and unregistration
        self.assertEquals(
            " ".join(real_out[1].split()),
            " ".join(expected_out[1].split()),
            self.display_name +
            (": Main registration line differs or missing in output:\n" +
             "real_out (file:'%s' line:2)\t: '%s'\n!=\n" +
             "expected (file:'%s' line:2)\t: '%s'") % (
             real_out_file, real_out[0],
             expected_out_file, expected_out[0]))
        self.assertEquals(
            " ".join(real_out[-1].split()),
            " ".join(expected_out[-1].split()),
            self.display_name +
            (": Main unregistration line differs or missing in output:\n" +
             "real_out (file:'%s' line:%d)\t: '%s'\n!=\n" +
             "expected (file:'%s' line:%d)\t: '%s'") % (
             real_out_file, len(real_out), real_out[-1],
             expected_out_file, len(expected_out), expected_out[-1]))

        # check other thread registration
        for i in range(2, len(expected_out)):
            if not re.match("^\[Thread [0-9]+\]: Registered", expected_out[i]):
                break
            res = re.match("^\[Thread [0-9]+\]: (Registered)", real_out[i])
            self.assertEquals(res.group(1), "Registered",
                self.display_name +
                ": Missing thread registration in output." +
                "Not all threads registered\n")
        # check other thread unregistration
        for i in range(2, len(expected_out)):
            if not re.match("^\[Thread [0-9]+\]: Unregistered", expected_out[-i]):
                break
            res = re.match("^\[Thread [0-9]+\]: (Unregistered)", real_out[-i])
            self.assertEquals(res.group(1), "Unregistered",
                self.display_name +
                ": Missing thread unregistration in output." +
                "Not all threads unregistered\n")        

    def checkOutputLines(self, real_out_file, expected_out_file, real_out, expected_out):
        # sort the lines in each line list
        real_out = [(i+1, real_out[i]) for i in xrange(len(real_out))]
        expected_out = [(i+1, expected_out[i]) for i in xrange(len(expected_out))]
        compare = lambda x,y: cmp(x[1],y[1])
        real_out.sort(compare)
        expected_out.sort(compare)

        # compare the output
        for i in xrange(len(expected_out)):
            if re.match("^Main Thread Registered", expected_out[i][1]):
                continue
            if re.match("^Main Thread Unregistered", expected_out[i][1]):
                continue
            if re.match("^\[Thread [0-9]+\]: Registered", expected_out[i][1]):
                continue
            if re.match("^\[Thread [0-9]+\]: Unregistered", expected_out[i][1]):
                continue
            if re.match("^[0-9]+: ", expected_out[i][1]):
                continue

            self.assertEquals(
                " ".join(real_out[i][1].split()),
                " ".join(expected_out[i][1].split()),
                self.display_name +
                (": Line differs or missing in SORTED output:\n" +
                 "real_out (file:'%s' line:%d)\t: '%s'\n!=\n" +
                 "expected (file:'%s' line:%d)\t: '%s'\n" +
                 "This could mean that a message wasn't passed correctly " +
                 "or that a barrier failed for some reason.\n") % (
                 real_out_file, real_out[i][0], real_out[i][1],
                 expected_out_file, expected_out[i][0], expected_out[i][1]))
        self.assertEquals(len(expected_out), len(real_out),
            self.display_name +
            ": Number of lines in expected output and actual output differ")

    def checkEvents(self, real_out_file, expected_out_file, real_out, expected_out):
        # collect the event information, and check if user events appear in the
        # correct order
        real_events = {}
        expected_events = {}
        current_user_event = 0
        for i in range(len(real_out)):
            r = real_out[i]
            
            if re.match("^[0-9]+: ", r):
                continue

            res = re.search("<EVENT ([0-9a-zA-Z]+) ([0-9]+)>", r)
            if res:
                # add the event for later comparison
                event_id = res.group(1)
                event_counter = res.group(2)
                if event_id not in real_events:
                    real_events[event_id] = [i+1, []]
                real_events[event_id][1].append(event_counter)

                # if this is a user event, verify that the order between them is
                # correct (going up)
                if int(event_counter) == 0:
                  self.assertEquals(
                      (int(event_id) >= current_user_event),
                      True,
                      self.display_name +
                      (": User EVENT %d appeared after EVENT %d!\n" %
                        (int(event_id), current_user_event)) +
                        "This means the order of events in the test was not " +
                        "as it should have been even though it had to be!\n" +
                        "Maybe a barrier problem or a SEND_SYNC problem?\n")
                  current_user_event = int(event_id)
                        
        for i in range(len(expected_out)):
            e = expected_out[i]
            
            if re.match("^[0-9]+: ", e):
                continue

            res = re.search("<EVENT ([0-9a-zA-Z]+) ([0-9]+)>", e)
            if res:
                event_id = res.group(1)
                event_counter = res.group(2)
                if event_id not in expected_events:
                    expected_events[event_id] = [i+1, []]
                expected_events[event_id][1].append(event_counter)

        # make sure every event appears in both places, and that they are in
        # the same order in both
        errors = []
        error_count = 0
        total_count = 0
        not_found_error = False
        different_lengths_error = False
        for e in expected_events.keys():
            expected_event = expected_events[e]
            real_event = real_events.get(e, [0, "<NOT FOUND>"])
            total_count += len(expected_event[1])
            if expected_event[1] != real_event[1]:
                if len(expected_event[1]) != len(real_event[1]):
                    error_count += abs(
                        len(expected_event[1]) - len(real_event[1]))
                    different_lengths_error = True
                else:
                    error_count += \
                        len([ie for ie in range(len(expected_event[1]))
                             if expected_event[1][ie] != real_event[1][ie]]) / 2
                errors.append((e, real_event, expected_event))
            if real_event[1] == "<NOT FOUND>":
                not_found_error = True
                errors[0] = errors[-1]
                break

        # if we have too many event errors, fail
        if not_found_error or different_lengths_error or \
           error_count > (MAX_EVENT_ERROR_THRESHOLD * total_count):
            (e, real_event, expected_event) = errors[0]
            special_message = ""
            if not_found_error:
                special_message = "Also, the following EVENT was not found:"
            elif different_lengths_error:
                special_message = "Also, at least one EVENT instance was missing."
            self.assertEquals(
                expected_event[1], real_event[1],
                self.display_name +
                (": There were %d/%d EVENTs not in order!\n" % (
                 len(errors), len(expected_events))) +
                special_message +
                ("First EVENT %s not found or in incorrect order in output:\n" +
                 "real_out (file:'%s' line:%d)\t: Order is %s\n!=\n" +
                 "expected (file:'%s' line:%d)\t: Order is %s\n" +
                 "This means the order of events in the test was not as it " +
                 "should have been even though it had to be!\n" +
                 "Maybe a barrier problem or a SEND_SYNC problem?\n") % (
                 e, real_out_file, real_event[0], real_event[1],
                 expected_out_file, expected_event[0], expected_event[1]))

        # finally, check if we have too many events in the real events
        self.assertEquals(len(expected_events), len(real_events),
          self.display_name + ": Too many EVENTs found in real output")
    
    def testRun(self):
        print "(%s) ... " % self.name,
        sys.stdout.flush()

        # run the program
        real_out_file, real_out, expected_out_file, expected_out = \
            run_program(self.name)
        real_out = real_out.split("\n")
        expected_out = expected_out.split("\n")

        # check if all registration lines appear
        self.checkRegistration(real_out_file, expected_out_file, real_out, expected_out)

        # check if all output lines appear
        self.checkOutputLines(real_out_file, expected_out_file, real_out, expected_out)

        # check if all events appear in order
        self.checkEvents(real_out_file, expected_out_file, real_out, expected_out)

        # delete temp files if there was no error
        try:
            os.remove(test_out_file)
            os.rmdir(TEMP_DIR)
        except:
            pass


# create all of the actual test suites
if SPECIFIC_TEST:
	test_files = []
	test_patterns = SPECIFIC_TEST.split(",")
	for pattern in test_patterns:
		test_files += glob.glob("%s%s%s%s" % (
			TESTS_DIR, os.sep, pattern, TESTS_INPUT_SUFFIX))
else:
	test_files = glob.glob(TESTS_DIR + os.sep + "*.in.txt")
test_files.sort()
test_files = [f[len(TESTS_DIR + os.sep):f.rindex(TESTS_INPUT_SUFFIX)] for f in test_files]
test_cases = [TestProgramRun(f) for f in test_files]

# create the suite and run them
suite = unittest.TestSuite()
suite.addTests(test_cases)
if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite)
