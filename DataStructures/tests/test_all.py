#!/bin/python

import sys
import os

USER_PROGRAM = "yaniv"
TEST_DIR = "wet1"

if os.system("cp -f ../%s ." % (USER_PROGRAM)) != 0:
  print """
ERROR: Couldn't find your program!
It should be named '%s' and in your main program dir (one above tests)""" % (
    USER_PROGRAM)

os.system("python %s/test_all.py %s" % (TEST_DIR, " ".join(sys.argv[1:])))
