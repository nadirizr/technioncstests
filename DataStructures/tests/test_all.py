#!/usr/bin/python

import sys
import os

TEST_PACKAGES = {
  "wet1": "yaniv",
  "wet2": "servers",
}
DEFAULT_PACKAGE = "wet1"

# Select test package and handle command line arguments.
args = sys.argv[1:]
chosen_package = None
for a in sys.argv[1:]:
  if a in TEST_PACKAGES.keys():
    if chosen_package is not None and chosen_package != a:
      print "ERROR: Too many test packages specified! ('%s' AND '%s')" % (
          chosen_package, a)
      sys.exit(1)
    chosen_package = a
    args.remove(a)
if chosen_package is None:
  chosen_package = DEFAULT_PACKAGE
TEST_DIR = chosen_package
USER_PROGRAM = TEST_PACKAGES[chosen_package]

# Copy over executable.
if os.system("cp -f ../%s ." % (USER_PROGRAM)) != 0:
  print """
ERROR: Couldn't find your program!
It should be named '%s' and in your main program dir (one above tests)""" % (
    USER_PROGRAM)

# Run the tests.
os.system("cd %s && python test_all.py %s" % (TEST_DIR, " ".join(args)))
