#!/usr/bin/python

import commands
import os
import sys

def str_to_int(s):
  num = ""
  for c in s:
    if c.isdigit():
      num += c
    else:
      break
  if num:
    return int(num), s[len(num):]
  return 0, s

def run_and_wait(command):
  # run the actual program
  reply = commands.getoutput("./tag_launcher " + str(command))
  pid, reply = str_to_int(reply)
  reply = reply.strip()
  if not pid:
    print reply
    return

  # wait for the process to finish
  while True:
    try:
      os.kill(pid, 0) 
      time.sleep(1)
    except:
      break
  
  print reply

run_and_wait(" ".join(sys.argv[1:]))
