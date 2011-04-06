import sys
import os
import random
from tag_util import *
from progressbar import *

TEST_DIR = ".." + os.sep + "random"
TEST_FILE_PATTERN = "test%d"

SIMULATOR_PATH = "python simulator.py"
TEST_INPUT_SUFFIX = ".in.txt"
TEST_OUTPUT_SUFFIX = ".out.txt"

MIN_COMMANDS = int(os.environ.get("TAGS_MIN_COMMANDS",100))
MAX_COMMANDS = int(os.environ.get("TAGS_MAX_COMMANDS",500))
ERROR_CHANCE = 0.05


##############################
# Test State Classes         #
##############################

state = None

# seed the random number generator
random.seed()


##############################
# Utility Functions          #
##############################

def getRandomPID():
  return random.randint(-1000, 100000)

def getRandomProcess():
  return random.choice(state.getProcesses())

def getProcessIndexAtParent(process):
  return process.getIndexAtParent()

def getRandomChildOrProcess(process):
  return random.choice([process] + process.getChildren())

def getProcessPID(process):
  return state.getPIDForProcess(process)

def getProcessHierarchy(process):
  return process.getHierarchy()

def generateHierarchyLine(hier_list):
  if not hier_list:
    return ""

  new_list = [str(h) for h in hier_list]
  new_list.reverse()
  return "/".join(new_list) + " "

def getProcessHierarchyLine(process):
  return generateHierarchyLine(getProcessHierarchy(process))

def removeProcess(process):
  state.removeProcessAndChildren(process)

def getRandomTag():
  return random.randint(1,20000)
  

##############################
# Command Creation Functions #
##############################

def createCreateChild():
  process = getRandomProcess()
  state.addNewProcess(process.tag / 2, process)
  return "%sCREATE_CHILD" % getProcessHierarchyLine(process)

def createGetTag():
  process = getRandomProcess()

  if random.random() < ERROR_CHANCE:
    return "%sGET_TAG -1" % getProcessHierarchyLine(process)

  process_get = getRandomProcess()
  return "%sGET_TAG %d" % (getProcessHierarchyLine(process),
                           getProcessPID(process_get))

def createSetTag():
  process = getRandomProcess()

  if random.random() < ERROR_CHANCE:
    return "%sSET_TAG -1 %d" % (getProcessHierarchyLine(process),
                                getRandomTag())

  if random.random() < ERROR_CHANCE:
    process_set = getRandomProcess()
    return "%sSET_TAG %d %d" % (getProcessHierarchyLine(process),
                                getProcessPID(process_set),
                                getRandomTag())
  process_set = getRandomChildOrProcess(process)
  if random.random() < ERROR_CHANCE:
    return "%sSET_TAG %d %d" % (getProcessHierarchyLine(process),
                                getProcessPID(process_set),
                                -getRandomTag())
  return "%sSET_TAG %d %d" % (getProcessHierarchyLine(process),
                              getProcessPID(process_set),
                              getRandomTag())

def createGetGoodProcesses():
  process = getRandomProcess()

  if random.random() < ERROR_CHANCE:
    return "%sGET_GOOD_PROCESSES %d" % (getProcessHierarchyLine(process),
                                        random.randint(-10,0))

  return "%sGET_GOOD_PROCESSES %d" % (getProcessHierarchyLine(process),
                                      random.randint(1,100))

def createMakeGoodProcesses():
  process = getRandomProcess()
  return "%sMAKE_GOOD_PROCESSES" % getProcessHierarchyLine(process)

def createClose():
  process = getRandomProcess()
  if process == state.processes[0]:
    return ""

  close_line = "%sCLOSE" % getProcessHierarchyLine(process)
  removeProcess(process)
  return close_line

# This dictionary holds all available commands to create as keys, and the
# values are the associated creation functions for those commands and their
# probability of appearing (before normalization).
commands = { }
commands["tags"] = [
    (createCreateChild, 0.5),
    (createGetTag, 0.5),
    (createSetTag, 0.5),
    (createGetGoodProcesses, 0.3),
    (createMakeGoodProcesses, 0.3),
#    (createClose, 0.0),
]

# Sum all of the chances together for each set of commands.
sumChances = { }
sumChances["tags"] = reduce(lambda s,c:s+c[1], commands["tags"], 0.0)

# The initial commands to put at the head of the input file.
INITIAL_COMMANDS = { }
INITIAL_COMMANDS["tags"] = []
# The final commands to put at the end of the input file.
FINAL_COMMANDS = { }
FINAL_COMMANDS["tags"] = ["CLOSE"]


##############################
# Logic Functions            #
##############################

# Creates a new random command.
def createNewCommand(cmd_list, cmd_sum):
  chance = random.random() * cmd_sum
  sum = 0.0
  for c in cmd_list:
    sum += c[1]
    if chance < sum:
      return c[0]()

# Creates a new random set of commands for a file.
def createNewCommandSet(cmd_list, cmd_sum, init_commands, fin_commands):
  global state
  state = MainState()

  num_lines = random.randint(MIN_COMMANDS, MAX_COMMANDS) - \
              (len(init_commands) + len(fin_commands))
  return init_commands + \
      [createNewCommand(cmd_list, cmd_sum) for i in range(num_lines)] + \
       fin_commands
    
# Creates a new random file from a given command mode.
def createNewFile(i, mode):
    # Create the .in file.
    filepath = TEST_DIR + os.sep + (TEST_FILE_PATTERN % i) + TEST_INPUT_SUFFIX
    f = file(filepath, "w")
    
    # Create the lines.
    in_cmds = createNewCommandSet(commands[mode], sumChances[mode],
                                  INITIAL_COMMANDS[mode], FINAL_COMMANDS[mode])
    for c in in_cmds:
        f.write(c + "\n")
    f.close()
    
    # Run the simulator to get the outputs.
    outpath = TEST_DIR + os.sep + (TEST_FILE_PATTERN % i) + TEST_OUTPUT_SUFFIX
    runSimulator(filepath, outpath)

# Runs the simulator for the given input file, creating error and output files.
def runSimulator(inpath, outpath):
    os.system(SIMULATOR_PATH + " " + inpath + " > " + outpath)

# Creates the simulated output and error for each of the files in input_dir.
def createOutputs(input_dir):
    from glob import glob
    inputs = glob(input_dir + os.sep + "*" + TEST_INPUT_SUFFIX)
    outputs = [f[:-3]+TEST_OUTPUT_SUFFIX for f in inputs]
    if not inputs:
      return

    widgets = ['Creating Outputs: ', Percentage(), ' (', ETA(), ') ', Bar(),
               ' ', FileTransferSpeed()]
    pbar = ProgressBar(widgets=widgets, maxval=len(inputs)).start()
    for i in range(len(inputs)):
        runSimulator(inputs[i], outputs[i])
        pbar.update(i+1)
    pbar.finish()
    print

# Determine amount of files to create.
num_files = 1
modes = []
input_dir = ""
def parseArgs():
    if len(sys.argv) > 1:
        global num_files
        global modes
        global input_dir
        modes = []
        fail = False
        for arg in sys.argv[1:]:
            if arg[:2] == "-i":
                input_dir = arg[2:]
            elif arg not in commands.keys():
                try:
                    num_files = int(arg)
                except:
                    fail = True
                    break
            else:
                if arg not in modes:
                    modes += [arg]                    
        if fail:
            print "Usage: create_tests.py [-i<dir>] [<mode>] [<num of files>]"
            print "       Possible modes: %s" % commands.keys()
            sys.exit(1)
parseArgs()
if not modes:
    modes = ["tags"]

if not input_dir:
    # Make sure test dir exists.
    try:
        os.makedirs(TEST_DIR)
    except:
        pass

    # If there are no files to be created, exit.
    if num_files <= 0:
      sys.exit(0)

    # Create the files.
    widgets = ['Creating Tests: ', Percentage(), ' (', ETA(), ') ', Bar(), ' ',
               FileTransferSpeed()]
    pbar = ProgressBar(widgets=widgets, maxval=num_files).start()
    for i in range(num_files):
        createNewFile(i, modes[0])
        pbar.update(i+1)
    pbar.finish()
else:
    createOutputs(input_dir)
