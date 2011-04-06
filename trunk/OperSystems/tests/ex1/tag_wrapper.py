import os
import sys
import time

from tag_util import *

TAG_PROCESS_EXECUTABLE = "./tag_process"

TAG_PROCESS_WRITE_PIPE = "/tmp/tag_process_write_pipe"
TAG_PROCESS_READ_PIPE = "/tmp/tag_process_read_pipe"

def create_pipes():
    try:
      os.mkfifo(TAG_PROCESS_READ_PIPE)
      os.mkfifo(TAG_PROCESS_WRITE_PIPE)
    except:
      pass

class TagsWrapperParser:

  def __init__(self, input_stream=sys.stdin, output_stream=sys.stdout,
                     tag_process_read_pipe="", tag_process_write_pipe=""):
    self.input_stream = input_stream
    self.output_stream = output_stream
    self.tag_process_read_pipe = tag_process_read_pipe
    self.tag_process_write_pipe = tag_process_write_pipe

    self.tester_pid = os.getpid()
    self.state = MainState()
    self.pids = {}

    create_pipes()
    self.__initializeTagProcess()

  def close(self):
    if self.tag_process_write_pipe:
      self.tag_process_write_pipe.close()
    if self.tag_process_read_pipe:
      self.tag_process_read_pipe.close()

  def parse(self):
    line_number = 1
    line = self.input_stream.readline()
    while line:
      print >> sys.stderr, "// Parsing Line [%d]: '%s'" % (line_number, line.strip())
      self.parseLine(line)
      line_number += 1
      line = self.input_stream.readline()

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
    if ("/" in args[0]) or ('0' <= args[0][0] <= '9'):
      process_indexes = args[0].split("/")
      process_indexes = [int(a) for a in process_indexes]
      cmd = args[1]

    if cmd == "CREATE_CHILD":
      reply = self.__sendToTagProcess(line)
      reply_args = reply.split()
      try:
        new_pid = int(reply_args[-1])
        if new_pid > 0:
          new_process = self.state.addNewProcess(
              tag=0, parent=self.getProcessForIndexes(process_indexes))
          new_process.pid = new_pid
          self.pids[new_pid] = new_process
          reply_args[-1] = str(self.pids.index(new_pid))
          print >> self.output_stream, " ".join(reply_args)
          return 0
        return -1
      except:
        print >> self.output_stream, reply
      return -1

    elif cmd == "GET_TAG":
      if 0 <= int(args[-1]) < len(self.state.getProcesses()):
        args[-1] = str(self.state.getProcessForPID(int(args[-1])).pid)
      print >> self.output_stream, self.__sendToTagProcess(" ".join(args))
      return 0

    elif cmd == "SET_TAG":
      if 0 <= int(args[-2]) < len(self.state.getProcesses()):
        args[-2] = str(self.state.getProcessForPID(int(args[-2])).pid)
      print >> self.output_stream, self.__sendToTagProcess(" ".join(args))
      return 0

    elif cmd == "GET_GOOD_PROCESSES":
      reply = self.__sendToTagProcess(line)
      reply_args = reply.split()
      try:
        pids = reply_args[2:]
        tester_pid_str = str(self.tester_pid)
        if tester_pid_str in pids:
          reply_args[2 + reply_args.index(tester_pid_str)] = "-1"
        print >> self.output_stream, " ".join(reply_args)
        return 0
      except:
        print >> self.output_stream, reply
      return -1

    elif cmd == "MAKE_GOOD_PROCESSES":
      reply = self.__sendToTagProcess(line)
      reply_args = reply.split()
      try:
        reply_args[-1] = str(int(reply_args[-1]) - 2)
        print >> self.output_stream, " ".join(reply_args)
        return 0
      except:
        print >> self.output_stream, reply
      return -1

    elif cmd == "CLOSE":
      try:
        wait_for_input = len(process_indexes) > 0 
        self.__sendToTagProcess(line, wait_for_input)
        self.state.removeProcessAndChildren(
            self.state.getProcessForIndexes(process_indexes))
        return 0
      except:
        return -1

  def __initializeTagProcess(self):
    if not (self.tag_process_write_pipe and self.tag_process_read_pipe):
      return

    child_pid = os.fork()
    if child_pid == 0:
      os.execv(TAG_PROCESS_EXECUTABLE, [TAG_PROCESS_EXECUTABLE,
                                        self.tag_process_write_pipe,
                                        self.tag_process_read_pipe])
      sys.exit(0)
    else:
      self.tag_process_write_pipe = open(self.tag_process_write_pipe, "w")
      self.tag_process_read_pipe = open(self.tag_process_read_pipe, "r")
      self.pids.append(child_pid)
      self.tag_process_read_pipe.readline()

  def __sendToTagProcess(self, line, wait_for_input=True):
    if not (self.tag_process_write_pipe and self.tag_process_read_pipe):
      return ""

    if not line.endswith("\n"):
      line += "\n"

    self.tag_process_write_pipe.write(line)
    self.tag_process_write_pipe.flush()

    if not wait_for_input:
      return ""

    time.sleep(0.1)
    return self.tag_process_read_pipe.readline().strip()


file_input_stream = sys.stdin
tag_process_read_pipe = TAG_PROCESS_READ_PIPE
tag_process_write_pipe = TAG_PROCESS_WRITE_PIPE
if sys.argv[1:]:
  file_input_stream = file(sys.argv[1], "r")
if sys.argv[2:] and sys.argv[3:]:
  tag_process_read_pipe = sys.argv[2]
  tag_process_write_pipe = sys.argv[3]

parser = TagsWrapperParser(file_input_stream, sys.stdout,
                           tag_process_read_pipe, tag_process_write_pipe)
parser.parse()
parser.close()

os.system("killall tag_process")

if file_input_stream != sys.stdin:
  file_input_stream.close()
