#!/usr/bin/python

import sys
import os
import commands
import errno
import time

commands.getoutput('chmod u+x check_driver.py')

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

def writeExpected(max_devs, devs):
  tmp = open('expected', 'w')
  lines = [str(max_devs) + '\n']
  for dev in devs:
    lines.append(dev + '\n')
  tmp.writelines(lines)
  tmp.close()
  
    
def assertDriver(msg, max_devs, devs):
  startTest(msg)
  
  writeExpected(max_devs, devs)

  commands.getoutput('./check_driver.py expected')
  time.sleep(0.25)
  out = open('expected.result', 'r').read()
  os.remove('expected')
  os.remove('expected.result')
  if out == 'Ok':
    passed()
  else:
    failed(out)
    sys.exit(1)

def validateSlave():
  commands.getoutput('gcc -o slave slave.c')

def assertOpenFails(name, device, perm, expected_err):
  validateSlave()
  startTest(name)
  err = _assertOpenFails(device, perm, expected_err)
  if err:
    failed(err)
    sys.exit(1)
  else:
    passed()
  
def _assertOpenFails(device, perm, expected_err):
  err = commands.getoutput("./slave %s %s" % (device, perm))
  if err == "0":
    return "Open was supposed to fail"
  else:
    if err == str(expected_err):
      return None
    return "Expected errno [%d] but was [%s]" % (expected_err, err)
  
def remove(files):
  for file in files:
    if os.path.exists(file):
      os.remove(file)
  
def assertOpens(name, device, perm, max_devs, devs):
  validateSlave()
  startTest(name)
  writeExpected(max_devs, devs)
  flag = commands.getoutput("./slave %s %s expected" % (device, perm))
  if flag != "0":
    remove(['expected', 'expected.result'])
    failed("Flag was: %s" % flag)
  res = open('expected.result', 'r').read()
  if res != 'Ok':
    remove(['expected', 'expected.result'])
    failed(res)
  remove(['expected', 'expected.result'])
  passed()
  
  
    

def mknod(name, major, minor):
  if os.path.exists(name):
    os.remove(name)
  commands.getoutput("mknod %s c %s %s" % (name, major, minor))

def testEquals(test_name, actual, expected):
  if not run_test(test_name, actual == expected, "Expected: %s, Actual: %s" % (expected, actual)):
    sys.exit(1)
    
def run_test(test_name, cond, msg=""):
  startTest(test_name)
  if not cond:
    failed(msg)
    return False
  else:
    passed()
    return True

def startTest(name):
  print name, '.' * (60 - len(name)),
  
def failed(msg):
  print "FAIL"
  print msg

def passed():
  print "OK"
  return True

######################################################
# Actual code

try:
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
    if not checkFilesExist(['../../Makefile', '../../vsf.h']):
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
    print "/proc/driver/vsf isn't found or has bad permissions (expected only read for user)"
    sys.exit(1)
  assertDriver("Test vsf driver exists", 3, [])
  
  # Now lets create some nodes and play with them
  major = commands.getoutput('cat /proc/devices | grep vsf | cut -d" " -f1')
  test("Device appears in devices list",
       major != '')
  # Create the nod
  mknod("vsf_cntrl", major, "0")

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
  assertDriver("Test 1 dev in driver", 3, ['254(0) 255(0) *'])
  
  # Check taken minors
  assertCreateFail(read_minor=252, write_minor=255, rc=-1, msg="Test create with read=252 and write=255 minors fails")
  assertCreateFail(read_minor=252, write_minor=254, rc=-1, msg="Test create with read=252 and write=254 minors fails")
  assertCreateFail(read_minor=254, write_minor=253, rc=-1, msg="Test create with read=254 and write=253 minors fails")
  assertCreateFail(read_minor=255, write_minor=253, rc=-1, msg="Test create with read=255 and write=253 minors fails")
  
  # Check open permissions
  mknod("vsf_read254", major, "254")
  assertOpenFails("Test open reader with WRONLY fails", "vsf_read254", os.O_WRONLY, errno.EPERM)
  assertOpenFails("Test open reader with RDWR fails", "vsf_read254", os.O_RDWR, errno.EPERM)
  assertOpenFails("Test open reader with APPEND fails", "vsf_read254", os.O_APPEND, errno.EPERM)
  assertOpenFails("Test open reader with RDONLY|CREAT fails", "vsf_read254", os.O_RDONLY|os.O_CREAT, errno.EPERM)
  assertOpens("Test open reader with RDONLY", "vsf_read254", os.O_RDONLY, 3, ['254(1) 255(0) *'])
  
  mknod("vsf_write255", major, "255")
  assertOpenFails("Test open writer with RDONLY fails", "vsf_write255", os.O_RDONLY, errno.EPERM)
  assertOpenFails("Test open writer with RDWR fails", "vsf_write255", os.O_RDWR, errno.EPERM)
  assertOpenFails("Test open writer with APPEND fails", "vsf_write255", os.O_APPEND, errno.EPERM)
  assertOpenFails("Test open writer with WRONLY|CREAT fails", "vsf_write255", os.O_WRONLY|os.O_CREAT, errno.EPERM)
  assertOpens("Test open writer with WRONLY", "vsf_write255", os.O_WRONLY, 3, ['254(0) 255(1) *'])

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
finally:
  # Removing the vsf
  commands.getoutput('rmmod vsf')

  if os.path.exists('slave'):
    os.remove('slave')
  if os.path.exists('fops'):
    os.remove('fops')
  if os.path.exists('vsf_read254'):
    os.remove('vsf_read254')
  if os.path.exists('vsf_write255'):
    os.remove('vsf_write255')
  if os.path.exists('vsf_cntrl'):
    os.remove('vsf_cntrl')
