#! /usr/bin/python

import re
import commands

# create input file
f = open('/tmp/stats.in','w')
f.write('GET_STATS\nCLOSE')
f.close()

# run the short process
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

l = []
for k in reasons.keys():
  l.append(k)
l.sort()

for k in l:
  print "%s: %d" % (k, len(reasons[k]))
