#!/usr/bin/python

# Given an input file of commands, and outputs the same output as the official
# tests to the standard output. If no file is given, reads from stdin.
# Optionally, if given the flag --print_tasks, print before every step the
# current tasks.
#
# Called as:
# ./simulator.py <input_file> [--print_tasks]

import sys

from logic import *

class ServersParser:

  def __init__(self, print_tasks=False):
    self.logic = None
    self.print_tasks = print_tasks

  def parse(self, input_stream):
    line = input_stream.readline()
    while line:
      if print_tasks:
        self.printTasks()
      self.parseLine(line)
      line = input_stream.readline()

  def parseLine(self, line):
    args = line.split()
    if not args:
      return

    cmd = args[0]
    if not cmd or cmd[0] == "#":
      print line,
      return STATUS_SUCCESS
    
    if cmd != "Init" and self.logic is None:
        print "%s: Invalid_input" % cmd
        return STATUS_INVALID_INPUT
    if cmd == "Init" and self.logic is not None:
        print "Init was already called."
        return STATUS_FAILURE

    if cmd == "Init":
      K = int(args[1])
      if K <= 0:
        print "Init failed."
        return STATUS_FAILURE
      self.logic = ServersLogic(K)
      print "Init done."
      return STATUS_SUCCESS

    elif cmd == "EnqueueRequest":
      try:
        reqID, serverID, priority = tuple([int(a) for a in args[1].split(",")])
      except:
        print "Enqueue Request: Invalid_input"
        return STATUS_INVALID_INPUT
      if reqID < 0 or serverID < 0 or serverID >= self.logic.K or priority < 0:
        print "Enqueue Request: Invalid_input"
        return STATUS_INVALID_INPUT

      retval = self.logic.enqueueRequest(reqID, serverID, priority)
      if retval == STATUS_SUCCESS:
        print "Enqueue Request: Success"
      else:
        print "Enqueue Request: Failure"
      return retval

    elif cmd == "DequeueRequest":
      try:
        reqID = int(args[1])
      except:
        print "Dequeue Request: Invalid_input"
        return STATUS_INVALID_INPUT
      if reqID < 0:
        print "Dequeue Request: Invalid_input"
        return STATUS_INVALID_INPUT

      retval = self.logic.dequeueRequest(reqID)
      if retval == STATUS_SUCCESS:
        print "Dequeue Request: Success"
      else:
        print "Dequeue Request: Failure"
      return retval

    elif cmd == "GetRequestPriority":
      try:
        reqID = int(args[1])
      except:
        print "GetRequestPriority: Invalid_input"
        return STATUS_INVALID_INPUT
      if reqID < 0:
        print "GetRequestPriority: Invalid_input"
        return STATUS_INVALID_INPUT

      priority = self.logic.getRequestPriority(reqID)
      if priority == STATUS_FAILURE:
        print "GetRequestPriority: Failure"
        return STATUS_FAILURE
      else:
        print "GetRequestPriority: Success %d" % priority
        return STATUS_SUCCESS

    elif cmd == "GetHighestPriorityRequest":
      reqID = self.logic.getHighestPriorityRequest()
      if reqID == STATUS_FAILURE:
        print "GetHighestPriorityRequest: Failure"
        return STATUS_FAILURE
      else:
        print "GetHighestPriorityRequest: Success %d" % reqID
        return STATUS_SUCCESS

    elif cmd == "LowerOldRequestsPriority":
      try:
        reqID, delta = tuple([int(a) for a in args[1].split(",")])
      except:
        print "LowerOldRequestsPriority: Invalid_input"
        return STATUS_INVALID_INPUT
      if reqID < 0 or delta < 0:
        print "LowerOldRequestsPriority: Invalid_input"
        return STATUS_INVALID_INPUT

      retval = self.logic.lowerOldRequestsPriority(reqID, delta)
      print "LowerOldRequestsPriority: Success"
      return STATUS_SUCCESS

    elif cmd == "GetHandlingServerId":
      try:
        reqID = int(args[1])
      except:
        print "GetHandlingServerId: Invalid_input"
        return STATUS_INVALID_INPUT
      if reqID < 0:
        print "GetHandlingServerId: Invalid_input"
        return STATUS_INVALID_INPUT

      serverID = self.logic.getHandlingServerID(reqID)
      if serverID == STATUS_FAILURE:
        print "GetHandlingServerId: Failure"
        return STATUS_FAILURE
      else:
        print "GetHandlingServerId: Success, %d" % serverID
        return STATUS_SUCCESS

    elif cmd == "KillServer":
      try:
        serverID, newServerID = tuple([int(a) for a in args[1].split(",")])
      except:
        print "KillServer: Invalid_input"
        return STATUS_INVALID_INPUT
      if serverID < 0 or newServerID < 0:
        print "KillServer: Invalid_input"
        return STATUS_INVALID_INPUT

      retval = self.logic.killServer(serverID, newServerID)
      if retval == STATUS_FAILURE:
        print "KillServer: Failure"
        return STATUS_FAILURE
      else:
        print "KillServer: Success"
        return STATUS_SUCCESS

    elif cmd == "Quit":
      del self.logic
      self.logic = None
      print "Quit done."
      return STATUS_SUCCESS
  
  def printTasks(self):
    print "[ ",
    for t in self.logic.tasks.values():
      print "{reqID=%d,serverID=%d,priority=%d}, " % (t.reqID, t.serverID, t.priority),
    print

file_stream = sys.stdin
print_tasks = False
if sys.argv[1:]:
  file_stream = file(sys.argv[1], "r")
if sys.argv[2:] and sys.argv[2] == "--print_tasks":
  print_tasks = True

parser = ServersParser(print_tasks)
parser.parse(file_stream)

if file_stream != sys.stdin:
  file_stream.close()
