#! /usr/bin/python

import re
import commands

def write_commands(cmds):
  # create input file
  f = open('/tmp/stats.in','w')
  cmds.append('')
  f.write('\n'.join(cmds))
  f.close()

def get_stats():
  write_commands(['GET_STATS','CLOSE'])

  commands.getoutput("(./short_process < /tmp/stats.in) > /tmp/stats.out")

  f = open('/tmp/stats.out', 'r')
  lines = f.readlines()
  f.close()

  stats = lines[1].split('] [')
  stats[0] = stats[0][10:]
  stats[-1] = stats[-1][:-2]

  reasons = {}
  for stat in stats:
    m = re.search('reason=([\\d]+)', stat)
    r = m.group(1)
    if not reasons.has_key(r):
      reasons[r] = []
    reasons[r].append(stat)

  return reasons

def run_commands(cmds):
  write_commands(cmds)
  commands.getoutput("echo /tmp/stats.in | ./short_process")

# check reason 2 (exit)
def test_exit():
  print "check reason 2 (exit) ....",
  run_commands(['CLOSE'])
  result = get_stats()
  if result.has_key('2'):
    print "OK"
  else:
    print "FAIL"
    
# check reason 3 (yield)
def test_yield():
  print "check reason 3 (yield) ...",
  run_commands(['YIELD', 'CLOSE'])
  result = get_stats()
  if result.has_key('3'):
    print "OK"
  else:
    print result.keys()
    print "FAIL"


test_exit()
test_yield()
