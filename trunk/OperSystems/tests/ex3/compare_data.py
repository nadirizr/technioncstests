import sys
import re
import os

class MessageFile:

  def __init__(self, file_path):
    self.file_path = file_path
    self.file = file(file_path, "r")
    self.lines = self.file.readlines()
    self.lines = [l for l in self.lines if not l.startswith("="*60)]
    
    self.messages = []
    current_msg = ""
    for l in self.lines:
      if re.match("^\[Message [0-9]+\]:", l):
        self.messages.append(current_msg)
        current_msg = ""
        continue
      current_msg += l
    if current_msg:
      self.messages 

  def get_messages(self):
    return self.messages

file1 = None
file2 = None
if sys.argv[2:]:
  if os.system("./dm %s > /tmp/dm1" % sys.argv[1]):
    print "ERROR: Couldn't open File1: '%s'" % sys.argv[1]
  if os.system("./dm %s > /tmp/dm2" % sys.argv[2]):
    print "ERROR: Couldn't open File2: '%s'" % sys.argv[2]

  file1 = MessageFile("/tmp/dm1")
  file2 = MessageFile("/tmp/dm2")
else:
  print "Usage: python compare_data.py <file 1> <file 2>"
  sys.exit(1)

messages1 = file1.get_messages()
messages2 = file2.get_messages()
missing_in_file2 = [i+1 for i in range(len(messages1)) if messages1[i] not in messages2]
missing_in_file1 = [i+1 for i in range(len(messages2)) if messages2[i] not in messages1]

print "File1 ('%s') Total Messages: %d" % (sys.argv[1], len(messages1))
print "File2 ('%s') Total Messages: %d" % (sys.argv[2], len(messages2))
print

if not missing_in_file1 and not missing_in_file2:
  print "Files are Equal!"
  sys.exit(0)

print "Messages in File1 that are NOT in File2:\n%s" % ", ".join([str(i) for i in missing_in_file2])
print "Messages in File2 that are NOT in File1:\n%s" % ", ".join([str(i) for i in missing_in_file1])
sys.exit(1)
