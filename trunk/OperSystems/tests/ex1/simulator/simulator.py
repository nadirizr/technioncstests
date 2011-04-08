#!/usr/bin/python

# Given a file of input commands in the format described in commands.txt,
# outputs the results of the various commands.
# This simulator assumes that the testing process is the son of the init
# process, as it is run using the tag_launcher.
#
# Called as:
# ./simulator.py <input_file>

import sys

from logic import *

class TagsParser:

  def __init__(self):
    self.logic = TagsLogic()

  def parse(self, input_stream):
    line = input_stream.readline()
    while line:
      self.parseLine(line)
      line = input_stream.readline()

  def parseLine(self, line):
    args = line.split()
    if not args:
      print
      return

    cmd = args[0]
    if not cmd or cmd[0] == "#":
      print line,
      return 0

    process_indexes = []
    if ("/" in args[0]) or args[0][0].isdigit():
      process_indexes = args[0].split("/")
      process_indexes = [int(a) for a in process_indexes]
      cmd = args[1]
      
    if cmd == "CREATE_CHILD":
      rc = self.logic.createChild(process_indexes)
      print "DONE %d" % rc
      return rc

    elif cmd == "GET_TAG":
      pid = int(args[-1])
      rc = self.logic.getTag(process_indexes, pid)
      print "DONE %d" % rc
      return rc

    elif cmd == "SET_TAG":
      tag = int(args[-1])
      pid = int(args[-2])
      rc = self.logic.setTag(process_indexes, pid, tag)
      print "DONE %d" % rc

    elif cmd == "GET_GOOD_PROCESSES":
      count = int(args[-1])
      pids = self.logic.getGoodProcesses(process_indexes, count)
      if type(pids) == int:
        print "DONE %d" % pids
        return pids
      if type(pids) == list:
        if len(pids) > 0:
          print "DONE %d %s" % (len(pids), " ".join([str(p) for p in pids]))
        else:
          print "DONE 0"
        return len(pids)
      return -1

    elif cmd == "MAKE_GOOD_PROCESSES":
      rc = self.logic.makeGoodProcesses(process_indexes)
      print "DONE %d" % rc
      return rc

    elif cmd == "CLOSE":
      return self.logic.close(process_indexes)

file_stream = sys.stdin
if sys.argv[1:]:
  file_stream = file(sys.argv[1], "r")

parser = TagsParser()
parser.parse(file_stream)

if file_stream != sys.stdin:
  file_stream.close()
