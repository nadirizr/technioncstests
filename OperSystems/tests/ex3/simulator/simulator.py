#!/usr/bin/python

# Given a file of input commands in the format described in commands.txt,
# outputs the results of the various commands.
#
# Called as:
# ./simulator.py <input_file>

import sys

from logic import *

OK_STOP = -9999
ERROR_STOP = -10000

THREAD_PREFIX   = "[Thread %d]: %s"
THREAD_ERROR    = "[Thread %d]: ERROR [line %d]: %s!"
THREAD_ERROR_RC = "[Thread %d]: ERROR [line %d]: %s (rc = %d)!"

class ThreadsParser:

  def __init__(self):
    self.logic = None
    self.num_threads = 0

  def parse(self, input_stream):
    line_num = 1
    line = input_stream.readline()
    while line:
      rc = self.parseLine(line, line_num)

      if rc == ERROR_STOP:
        print "ERROR: Unrecoverable Error!"
        break
      if rc == OK_STOP:
        break

      line_num += 1
      line = input_stream.readline()

  def parseLine(self, line, line_num):
    # Get the arguments, and the command.
    args = line.split()
    if not args:
      print
      return 0
    cmd = args[0]

    # Handle empty lines and comments.
    if not cmd or cmd[0] == "#":
      print line,
      return 0

    # Get the thread index, if it exists.
    thread_index = -1
    if args[0][0].isdigit():
      thread_index = int(args[0])
      cmd = args[1]
      args = args[1:]

    # Handle the inialization command.
    if not self.logic:
      if cmd != "INIT":
        return ERROR_STOP

      self.num_threads = int(args[-1])
      self.logic = ThreadsLogic(self.num_threads)

      print "Main Thread Registered"
      for t in range(1, self.num_threads + 1):
        print THREAD_PREFIX % (t, "Registered (Thread ID = %d)" % t)
      return 0

    if self.logic and cmd == "INIT":
      return ERROR_STOP

    # Handle commands that don't require a thread ID.
    if cmd == "CLOSE" or cmd == "NOP":
      if thread_index < 0:
        thread_index = 1

    # If we don't have a thread index by now, this is an error.
    if thread_index < 0 or thread_index > self.num_threads:
      print "ERROR [line %d]: Invalid thread index %d!" % (
          line_num, thread_index)
      return ERROR_STOP

    # Handle the various commands.
    rc = 0

    if cmd == "NOP":
      rc = 0

    elif cmd == "CREATE_BARRIER":
      n = int(args[-1])
      rc = self.logic.createBarrier(thread_index, n)
      
      if rc < 0:
        print THREAD_ERROR % (
            thread_index, line_num, "Barrier Creation Failed")
      else:
        print THREAD_PREFIX % (thread_index, "Barrier Created")

    elif cmd == "DESTROY_BARRIER":
      self.logic.destroyBarrier(thread_index)
      print THREAD_PREFIX % (thread_index, "Barrier Destroyed")
      rc = 0

    elif cmd == "BARRIER":
      rc = self.logic.barrier(thread_index)
      
      if type(rc) == list:
        for t in rc:
          print THREAD_PREFIX % (t, "Barrier Passed")
        rc = 0
      elif rc < 0:
        print THREAD_ERROR_RC % (
            thread_index, line_num, "Barrier Failed", rc)

    elif cmd == "SEND":
      to = int(args[1])
      is_sync = "SYNC" in args
      is_urgent = "URGENT" in args
      rc = self.logic.send(
          thread_index, to, is_sync, is_urgent, " ".join(args[2:]))
      
      flags = ""
      if is_urgent: flags += " URGENT"
      if is_sync:   flags += " SYNC"
      if not flags: flags = " None"
      if rc < 0:
        print THREAD_ERROR_RC % (
            thread_index, line_num, "Send Failed (Flags:%s)" % flags, rc)
      elif rc == 0:
        print THREAD_PREFIX % (
            thread_index, "Send Successfull (Flags:%s)" % flags)

    elif cmd == "BROADCAST":
      is_sync = "SYNC" in args
      is_urgent = "URGENT" in args
      rc = self.logic.broadcast(
          thread_index, is_sync, is_urgent, " ".join(args[1:]))
      
      flags = ""
      if is_urgent: flags += " URGENT"
      if is_sync:   flags += " SYNC"
      if not flags: flags = " None"
      if rc < 0:
        print THREAD_ERROR_RC % (
            thread_index, line_num, "Broadcast Failed (Flags:%s)" % flags, rc)
      elif rc == 0:
        print THREAD_PREFIX % (
            thread_index, "Broadcast Successfull (Flags:%s)" % flags)

    elif cmd == "CLOSE":
      self.parseLine("%d BROADCAST FINISH" % thread_index, line_num)
      rc = self.logic.close(thread_index)

      for t in range(1, self.num_threads + 1):
        print THREAD_PREFIX % (t, "Unregistered (Thread ID = %d)" % t)
      print "Main Thread Unregistered"
      return OK_STOP

    # After the commands, deliver all pending messages.
    delivered = self.logic.deliverAllMessages()
    for m in delivered:
      # We have now received this message.
      print THREAD_PREFIX % (
          m.destination,
          "Received Message (length = %d): '%s'" % (
              len(m.message) + 1, m.message))

      # For non-sync messages, we are done.
      if not m.is_sync:
        continue

      # If this was a broadcast, we only want to print at the end.
      if not m.is_broadcast:
        flags = ""
        if m.is_urgent: flags += " URGENT"
        if m.is_sync:   flags += " SYNC"
        if not flags:   flags = " None"
        print THREAD_PREFIX % (
            m.origin, "Send Successfull (Flags:%s)" % flags)

    # Finally, go over the broadcasters and print them as well.
    broadcasters = self.logic.finishedBroadcasters()
    for b in broadcasters:
      flags = ""
      if m.is_urgent: flags += " URGENT"
      if m.is_sync:   flags += " SYNC"
      if not flags:   flags = " None"
      print THREAD_PREFIX % (
          m.origin, "Broadcast Successfull (Flags:%s)" % flags)

    # Return the return code from the command.
    return rc
    

file_stream = sys.stdin
if sys.argv[1:]:
  file_stream = file(sys.argv[1], "r")

parser = ThreadsParser()
parser.parse(file_stream)

if file_stream != sys.stdin:
  file_stream.close()
