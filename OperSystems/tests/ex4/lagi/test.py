#!/usr/bin/python

import sys
import os
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

def checkFilesExist(files):
  for file in files:
    if not os.path.exists(file):
      print "Can't find file: %s" % file
      return False
  return True

def test(test_name, cond):
  print test_name, '.' * (60 - len(test_name)),
  if not cond:
    print "FAIL"
    sys.exit(1)
  else:
    print "OK"

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
if options.should_compile:
  if not checkFilesExist(['../../Makefile', '../../vsf.c', '../../vsf.h']):
    sys.exit(1)
  
  if os.path.exists('../../vsf.o'):
    os.remove('../../vsf.o')
  commands.getoutput('cd ../.. && make')

# Verify that adding the module with no parameter fails
if not checkFilesExist(['../../vsf.o']):
  print "Compile failid!"
  sys.exit(1)

# Check inserting a module with no parameter
test("Test insmod without param",
     commands.getoutput('insmod ../../vsf.o').find('incorrect module parameters') != -1)

