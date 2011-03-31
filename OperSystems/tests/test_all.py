#!/usr/bin/python

import sys
import os
from glob import glob

TEST_PACKAGES = {
  "ex1": "tag_tester",
}
DEFAULT_PACKAGE = "ex1"

# Check for messages.
if os.path.isdir(".messages"):
  messages = glob(".messages/*.txt")
  read_already = []
  if os.path.isfile(".messages/read"):
    read_already = file(".messages/read","r").readlines()
    read_already = [s.strip() for s in read_already]
  for m in messages:
    if m not in read_already:
      message_text = open(m,"r").read()
      print
      print "=== Message '%s' ===" % m
      print
      print message_text
      print "=== End of Message '%s' ===" % m
      raw_input("Press any key to continue...")
  out_file = file(".messages/read","w")
  out_file.write("\n".join(messages) + "\n")
  out_file.close()

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
It should be named '%s' and in your main program dir (one above tests).""" % (
    USER_PROGRAM)
  sys.exit(1)

# Run the tests.
os.system("cd %s && python test_all.py %s" % (TEST_DIR, " ".join(args)))
