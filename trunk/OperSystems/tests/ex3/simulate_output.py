import sys
import os

SIMULATOR_DIR = "simulator"
SIMULATOR_COMMAND = "python simulator.py"

if not sys.argv[1:]:
  print "Usage: python simulate_output.py <input file>"
  sys.exit(1)

input_file = os.path.realpath(sys.argv[1])
output_file = input_file[:input_file.find(".in.txt")] + ".out.txt"

command = "cd %s ; %s %s > %s" % (SIMULATOR_DIR, SIMULATOR_COMMAND, input_file, output_file)
print "Creating Output: '%s' -> '%s'" % (input_file, output_file)
os.system(command)
