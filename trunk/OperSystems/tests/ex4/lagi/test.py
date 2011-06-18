#!/usr/bin/python

import sys
import os
import commands

MAX_DEVS = 3

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
  if not run_test(test_name, cond):
    sys.exit(1)

def assertCreateFail(read_minor, write_minor, rc, msg):
  testEquals(msg,
             commands.getoutput("./fops C %d %d" % (read_minor, write_minor)),
             "Failed to create VSF for [read_minor = %d, write_minor = %d]: RC = %d" % (read_minor, write_minor, rc))
    
def assertCreate(read_minor, write_minor, msg):
  testEquals(msg,
             commands.getoutput("./fops C %d %d" % (read_minor, write_minor)),
             "Created VSF for [read_minor = %d, write_minor = %d]" % (read_minor, write_minor))
    
def assertDriver(msg, max_devs, devs):
  driver = open('/proc/driver/vsf', 'r')
  expected = ["%d\n" % MAX_DEVS]
  for dev in devs:
    if len(dev) == 4:
      expected.append("%d(%d) %d(%d) *\n" % dev)
    else:
      expected.append("%d(%d) %d(%d) %s:%d\n" % dev)
      
  testEquals(msg,
             driver.readlines(),
             expected)
  driver.close()

def testEquals(test_name, actual, expected):
  if not run_test(test_name, actual == expected):
    print "  Expected: %s, Actual: %s" % (expected, actual)
    sys.exit(1)
    
def run_test(test_name, cond):
  print test_name, '.' * (60 - len(test_name)),
  if not cond:
    print "FAIL"
    return False
  else:
    print "OK"
    return True

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

# Check inserting actually works
testEquals("Test insmod works",
           commands.getoutput('insmod ../../vsf.o max_vsf_devices=%d' % MAX_DEVS),
           '')

# Check vsf appears in lsmod
test("Test vsf appears in lsmod",
     commands.getoutput('lsmod | grep vsf').startswith('vsf'))

# Check vsf driver appears and has the correct max vsf
if not commands.getoutput('ls -l /proc/driver/vsf').startswith("-r--------"):
  print "Missing vsf file or bad permissions"
  sys.exit(1)
assertDriver("Test vsf driver exists", 3, [])

# Now lets create some nodes and play with them
major = commands.getoutput('cat /proc/devices | grep vsf | cut -d" " -f1')
test("Device appears in devices list",
     major != '')
# Create the nod
if os.path.exists('vsf_cntrl'):
  os.remove('vsf_cntrl')
commands.getoutput("mknod vsf_cntrl c %s 0" % major)
# compile the helper file
if not os.path.exists('fops'):
  o = commands.getoutput("gcc -o fops -I../.. fileops.c")
  if o != '':
    print "Couldn't compile fileops.c:\n%s" % o
    sys.exit(1)

assertCreateFail(read_minor=0, write_minor=1, rc=-1, msg="Test create with read minor 0 fails")
assertCreateFail(read_minor=1, write_minor=0, rc=-1, msg="Test create with write minor 0 fails")
assertCreateFail(read_minor=10, write_minor=10, rc=-1, msg="Test create with same read and write minors fails")
assertCreate(read_minor=254, write_minor=255, msg="Test create with read=254 and write=255 minors")
assertDriver("Test 1 dev in driver", 3, [(254,0, 255,0)])
# Check removing the vsf works
testEquals("Test rmmod works",
           commands.getoutput('rmmod vsf'),
           '')
           
# Check inserting a module with parameter 0 fails
test("Test insmod with param = 0",
     commands.getoutput('insmod ../../vsf.o max_vsf_devices=0').find('incorrect module parameters') != -1)
     
# Check inserting a module with parameter 1000001
test("Test insmod with param = 1000001",
     commands.getoutput('insmod ../../vsf.o max_vsf_devices=1000001').find('incorrect module parameters') != -1)
     
# Check inserting with 1000000 works
testEquals("Test insmod with param = 1000000 works",
           commands.getoutput('insmod ../../vsf.o max_vsf_devices=1000000'),
           '')

# Removing the vsf
commands.getoutput('rmmod vsf')
