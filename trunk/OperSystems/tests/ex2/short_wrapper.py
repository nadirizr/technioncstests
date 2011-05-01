import os
import sys
import time

from short_util import *

SHORT_PROCESS_EXECUTABLE = "./short_process"

SHORT_PROCESS_WRITE_PIPE = "/tmp/short_process_write_pipe"
SHORT_PROCESS_READ_PIPE = "/tmp/short_process_read_pipe"

def create_pipes():
  try:
    os.mkfifo(SHORT_PROCESS_READ_PIPE)
    os.mkfifo(SHORT_PROCESS_WRITE_PIPE)
  except:
    pass

class ShortWrapperParser:

  def __init__(self, input_stream=sys.stdin, output_stream=sys.stdout,
               command_output_stream=None, short_process_read_pipe="",
               short_process_write_pipe=""):
    self.input_stream = input_stream
    self.output_stream = output_stream
    self.command_output_stream = command_output_stream
    self.short_process_read_pipe = short_process_read_pipe
    self.short_process_write_pipe = short_process_write_pipe

    self.tester_pid = os.getpid()
    self.state = MainState()
    self.pids = {}

    create_pipes()
    self.__initializeShortProcess()

  def close(self):
    if self.short_process_write_pipe:
      self.short_process_write_pipe.close()
    if self.short_process_read_pipe:
      self.short_process_read_pipe.close()

  def parse(self):
    line_number = 1
    line = self.input_stream.readline()
    while line:
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
      print >> self.output_stream, line,
      return 0

    process_indexes = []
    if ("/" in args[0]) or ('0' <= args[0][0] <= '9'):
      process_indexes = args[0].split("/")
      process_indexes = [int(a) for a in process_indexes]
      cmd = args[1]

    async = False
    if cmd == "ASYNC":
      async = True
      cmd = args[2]

    if cmd == "CREATE_CHILD":
      reply = self.__sendToShortProcess(line)
      reply_args = reply.split()
      try:
        new_pid = int(reply_args[-1])
        if new_pid > 0:
          new_process = self.state.addNewProcess(
              parent=self.state.getProcessForIndexes(process_indexes))
          new_process.pid = new_pid
          self.pids[new_pid] = new_process
          reply_args[-1] = str(self.state.getPIDForProcess(new_process))
          print >> self.output_stream, " ".join(reply_args)
          return 0
        return -1
      except:
        print >> self.output_stream, reply
      return -1

    elif cmd == "SET_SHORT":
      if 0 <= int(args[-2]) < len(self.state.getProcesses()):
        args[-2] = str(self.state.getProcessForPID(int(args[-2])).pid)
      print >> self.output_stream, self.__sendToShortProcess(" ".join(args))
      return 0

    elif cmd == "REMAINING_TIME":
      if 0 <= int(args[-1]) < len(self.state.getProcesses()):
        args[-1] = str(self.state.getProcessForPID(int(args[-1])).pid)
      reply = self.__sendToShortProcess(" ".join(args))
      reply_args = reply.split()
      try:
        remaining_time = int(reply_args[-1])
        if remaining_time > 0:
          remaining_time = (remaining_time / 5) * 5
        reply_args[-1] = str(remaining_time)
        print >> self.output_stream, " ".join(reply_args)
      except:
        print >> self.output_stream, reply
      return 0

    elif cmd == "OVERDUE_TIME":
      if 0 <= int(args[-1]) < len(self.state.getProcesses()):
        args[-1] = str(self.state.getProcessForPID(int(args[-1])).pid)
      reply = self.__sendToShortProcess(" ".join(args))
      reply_args = reply.split()
      try:
        overdue_time = int(reply_args[-1])
        if overdue_time > 0:
          overdue_time = (overdue_time / 5) * 5
        reply_args[-1] = str(overdue_time)
        print >> self.output_stream, " ".join(reply_args)
      except:
        print >> self.output_stream, reply
      return 0

    elif cmd == "GET_POLICY":
      if 0 <= int(args[-1]) < len(self.state.getProcesses()):
        args[-1] = str(self.state.getProcessForPID(int(args[-1])).pid)
      print >> self.output_stream, self.__sendToShortProcess(" ".join(args))
      return 0

    elif cmd == "GET_PARAM":
      if 0 <= int(args[-1]) < len(self.state.getProcesses()):
        args[-1] = str(self.state.getProcessForPID(int(args[-1])).pid)
      print >> self.output_stream, self.__sendToShortProcess(" ".join(args))
      return 0

    elif cmd == "DO_WORK":
      print >> self.output_stream, self.__sendToShortProcess(" ".join(args))
      return 0

    elif cmd == "GET_STATS":
      reply = self.__sendToShortProcess(line)
      reply_args = reply.split()
      
      try:
        stat_strs = reply_args[2:]
        for i in xrange(len(stat_strs)):
          stat_info = stat_strs[i]
          
          res = re.search("prev_pid=([0-9]+)", stat_info)
          prev_pid = int(res.group(1))
          if self.tester_pid == prev_pid:
            prev_pid = -1
          elif prev_pid in self.pids.keys():
            prev_pid = self.state.getPIDForProcess(self.pids[prev_pid])
          stat_info = re.sub("prev_pid=([0-9]+)", "prev_pid=%d" % prev_pid, stat_info)
            
          res = re.search("next_pid=([0-9]+)", stat_info)
          next_pid = int(res.group(2))
          if self.tester_pid == next_pid:
            next_pid = -1
          elif next_pid in self.pids.keys():
            next_pid = self.state.getPIDForProcess(self.pids[next_pid])
          stat_info = re.sub("next_pid=([0-9]+)", "next_pid=%d" % next_pid, stat_info)

          stat_strs[i] = stat_info
        reply_args[2:] = stat_strs
            
        print >> self.output_stream, " ".join(reply_args)
        return 0
      except:
        print >> self.output_stream, reply
      return -1

    elif cmd == "CLOSE":
      try:
        wait_for_input = len(process_indexes) > 0 
        self.__sendToShortProcess(line, wait_for_input)

        if len(process_indexes) == 0:
          self.__printTreeWithPIDs()

        self.state.removeProcessAndChildren(
            self.state.getProcessForIndexes(process_indexes))
        return 0
      except:
        return -1

    else:
      print >> self.output_stream, self.__sendToShortProcess(" ".join(args))
      return 0

  def __initializeShortProcess(self):
    if not (self.short_process_write_pipe and self.short_process_read_pipe):
      return

    self.state.processes = []

    child_pid = os.fork()
    if child_pid == 0:
      os.execv(SHORT_PROCESS_EXECUTABLE, [SHORT_PROCESS_EXECUTABLE,
                                          self.short_process_write_pipe,
                                          self.short_process_read_pipe])
      sys.exit(0)
    else:
      self.short_process_write_pipe = open(self.short_process_write_pipe, "w")
      self.short_process_read_pipe = open(self.short_process_read_pipe, "r")
      new_process = self.state.addNewProcess(parent=None)
      new_process.pid = child_pid
      self.pids[child_pid] = new_process
      self.short_process_read_pipe.readline()

  def __sendToShortProcess(self, line, wait_for_input=True):
    if not (self.short_process_write_pipe and self.short_process_read_pipe):
      return ""

    if not line.endswith("\n"):
      line += "\n"

    self.__writeToCommandOutput(line)

    self.short_process_write_pipe.write(line)
    self.short_process_write_pipe.flush()

    if not wait_for_input:
      return ""

    #time.sleep(0.1)
    reply = self.short_process_read_pipe.readline().strip()

    self.__writeToCommandOutput("--> %s\n" % reply)
    return reply

  def __writeToCommandOutput(self, line):
    if not self.command_output_stream:
      return
    self.command_output_stream.write(line)
    self.command_output_stream.flush()

  def __printTreeWithPIDs(self):
    self.__writeToCommandOutput(
        "\n\nFinal Process Tree (Processes ordered according to creation):\n")
    for p in self.state.getProcesses():
      self.__writeToCommandOutput("PID(%d)\t: %s\n" % (
        p.pid, [c.pid for c in p.getChildren()]))


file_input_stream = sys.stdin
file_output_stream = sys.stdout
command_output_stream = None
short_process_read_pipe = SHORT_PROCESS_READ_PIPE
short_process_write_pipe = SHORT_PROCESS_WRITE_PIPE
if sys.argv[1:]:
  file_input_stream = file(sys.argv[1], "r")
  file_output_stream = None
  command_output_stream = sys.stdout
if sys.argv[2:]:
  file_output_stream = file(sys.argv[2], "w")
  command_output_stream = None
if sys.argv[3:]:
  command_output_stream = file(sys.argv[3], "w")
if sys.argv[4:] and sys.argv[5:]:
  short_process_read_pipe = sys.argv[4]
  short_process_write_pipe = sys.argv[5]

parser = ShortWrapperParser(
    file_input_stream, file_output_stream, command_output_stream,
    short_process_read_pipe, short_process_write_pipe)
parser.parse()
parser.close()

os.system("killall short_process")

if file_input_stream != sys.stdin:
  file_input_stream.close()
