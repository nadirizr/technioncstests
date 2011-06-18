#!/usr/bin/python

import sys
import commands

USAGE = "USAGE:\n\
%s [flags]\n\
  -NoCompile - prevents the test from compiling the code" % (sys.argv[0])

def parseOptions(args):
  should_compile = True
  for arg in args[1:]:
    if arg == '-NoCompile':
      should_compile = False
    else:
      print USAGE
      sys.exit(1)
  return Options(should_compile)

class Options:
  
  def __init__(self, should_compile=True):
    self.should_compile = should_compile
options = parseOptions(sys.argv)

# Check whether a module already exists
o = commands.getoutput('lsmod | grep vsf')
if '' != o:
  print "There's a module running!"

  # Try to remove the running module
  print "Trying to remove the module: ",
  o = commands.getoutput('rmmod vsf')
  if '' != o:
    print o
    print "Failed to remove the module, please remove it manually and run ",
    print "the test again"
  else:
    print "Done"

# Now try to add the module
# First search for the files and compile them
print options.should_compile

# Verify that adding the module with no parameter fails
#o = commands.getoutput('



