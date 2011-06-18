#!/usr/bin/python

import commands

o = commands.getoutput('lsmod | grep vsf')
if '' != o:
  print "There's a module running!"
  print "Trying to remove the module: ",
  o = commands.getoutput('rmmod vsf')
  if '' != o:
    print o
    print "Failed to remove the module, please remove it manually and run ",
    print "the test again"
  else:
    print "Done"

print "..."
