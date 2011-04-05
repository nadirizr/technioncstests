import os
import sys

TAG_PROCESS_EXECUTABLE = "./tag_process"

class TagsWrapperParser:

  def __init__(self, input_stream=sys.stdin, output_stream=sys.stdout,
                     tag_process_read_pipe="", tag_process_write_pipe=""):
    self.input_stream = input_stream
    self.output_stream = output_stream
    self.tag_process_read_pipe = tag_process_read_pipe
    self.tag_process_write_pipe = tag_process_write_pipe

    self.tester_pid = os.getpid()
    self.pids = []
    self.__initializeTagProcess()

  def close(self):
    if self.tag_process_write_pipe:
      self.tag_process_write_pipe.close()
    if self.tag_process_read_pipe:
      self.tag_process_read_pipe.close()

  def parse(self):
    line = self.input_stream.readline()
    while line:
      self.parseLine(line)
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

    if ("/" in args[0]) or ('0' <= args[0][0] <= '9'):
      cmd = args[1]

    if cmd == "CREATE_CHILD":
      reply = self.__sendToTagProcess(line)
      reply_args = reply.split()
      try:
        reply_args[-1] = str(self.pids.index(int(reply_args[-1])))
        print >> self.output_stream, " ".join(reply_args)
        return 0
      except:
        print >> self.output_stream, reply
      return -1

    elif cmd == "GET_TAG":
      args[-1] = str(self.pids[int(args[-1])])
      print >> self.output_stream, self.__sendToTagProcess(" ".join(args))
      return 0

    elif cmd == "SET_TAG":
      args[-2] = str(self.pids[int(args[-2])])
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
      self.__sendToTagProcess(line, wait_for_input=False)
      return 0

  def __initializeTagProcess(self):
    if not (self.tag_process_write_pipe and self.tag_process_read_pipe):
      return

    if os.fork() == 0:
      os.execv(TAG_PROCESS_EXECUTABLE, [TAG_PROCESS_EXECUTABLE,
                                        self.tag_process_write_pipe,
                                        self.tag_process_read_pipe])
      sys.exit(0)
    else:
      self.tag_process_read_pipe = open(self.tag_process_read_pipe, "r")
      self.tag_process_write_pipe = open(self.tag_process_write_pipe, "w")

  def __sendToTagProcess(self, line, wait_for_input=True):
    if not (self.tag_process_write_pipe and self.tag_process_read_pipe):
      return ""

    if not line.endswith("\n"):
      line += "\n"

    self.tag_process_write_pipe.write(line)
    self.tag_process_write_pipe.flush()

    if not wait_for_input:
      return ""

    return self.tag_process_read_pipe.readline()

file_input_stream = sys.stdin
tag_process_read_pipe = ""
tag_process_write_pipe = ""
if sys.argv[1:]:
  file_input_stream = file(sys.argv[1], "r")
if sys.argv[2:] and sys.argv[3:]:
  tag_process_read_pipe = sys.argv[2]
  tag_process_write_pipe = sys.argv[3]

parser = TagsWrapperParser()
parser.parse(file_input_stream, sys.stdout, tag_process_read_pipe, tag_process_write_pipe)
parser.close()

if file_input_stream != sys.stdin:
  file_input_stream.close()
