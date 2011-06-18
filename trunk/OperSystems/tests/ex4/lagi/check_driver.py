#!/usr/bin/python

import sys

MAX_DEVS = 3

def assertDriver(max_devs, devs):
  driver = open('/proc/driver/vsf', 'r')
  expected = ["%d\n" % MAX_DEVS]
  for dev in devs:
    expected.append(dev.strip() + '\n')
      
  actual = driver.readlines()
  if actual != expected:
    res.writelines(["Fail\n", "Expected: %s\n" % str(expected), "Was: %s\n" % str(actual)])
  else:
    res.writelines(['Ok'])
  driver.close()

f = open(sys.argv[1], 'r')
lines = f.readlines()
f.close()

res = open(sys.argv[1] + '.result', 'w')
assertDriver(int(lines[0].strip()), lines[1:])
res.close()
