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

MIN_COMMANDS = int(os.environ.get("THREADS_MIN_COMMANDS",100))
MAX_COMMANDS = int(os.environ.get("THREADS_MAX_COMMANDS",500))
ERROR_CHANCE = 0.05


##############################
# Test State Classes         #
##############################

class TestState:
  def __init__(self, num_threads):
    self.num_threads = num_threads
    self.threads = range(1, num_threads + 1)

    self.barrier_counter = -1
    self.waiting_at_barrier = []
    self.waiting_at_sync = []
    
    self.event = 0

test_state = None

# seed the random number generator
random.seed()


##############################
# Utility Functions          #
##############################

def getRandomThreadCount():
  return random.randint(2, MAX_COMMANDS / 10)

def getRandomThread():
  return random.randint(1, test_state.num_threads)

def getBlockedThreads():
  return test_state.waiting_at_barrier + test_state.waiting_at_sync

def getRandomAvailableThread():
  t = getRandomThread()
  while t in getBlockedThreads():
    t = getRandomThread()
  return t

def blockAtBarrier(thread_id):
  if thread_id not in test_state.waiting_at_barrier:
    test_state.waiting_at_barrier.append(thread_id)

def blockAtSync(thread_id):
  if thread_id not in test_state.waiting_at_sync:
    test_state.waiting_at_sync.append(thread_id)

LETTERS = [chr(c) for c in range(ord('a'), ord('z') + 1)]
def getRandomMessage():
  message_len = random.randint(10, 20)
  return "".join([LETTERS[random.randrange(len(LETTERS))]
                  for i in range(message_len)])


##############################
# Command Creation Functions #
##############################

def createInit():
  test_state = TestState(getRandomThreadCount())
  return "INIT %d" % test_state.num_threads

def createCreateBarrier():
  t = getRandomThread()

  num = 0
  if random.random() < ERROR_CHANCE:
    num = random.randint(-10, 0)
  else:
    num = random.randint(1, test_state.num_threads)

  if num >= 1:
    test_state.barrier_counter = num
  return "%d CREATE_BARRIER %d" % (t, num)

def createBarrier():
  t = getRandomAvailableThread()
  blockAtBarrier(t)

  if len(test_state.waiting_at_barrier) == test_state.barrier_counter:
    freeAllBlocking()
    test_state.barrier_counter = -1

  return "%d BARRIER" % t

def createDestroyBarrier():
  t = getRandomAvailableThread()
  test_state.waiting_at_barrier = []
  return "%d DESTROY_BARRIER" % t

def createBarrierCommand():
  if test_state.barrier_counter < 0 and not test_state.waiting_at_barrier:
    return createCreateBarrier()

  if test_state.barrier_counter < len(test_state.waiting_at_barrier):
    return createBarrier()

  if test_state.barrier_counter < 0 and test_state.waiting_at_barrier:
    if random.random() < ERROR_CHANCE:
      return createBarrier()
    else:
      return createDestroyBarrier()

  return createBarrier()

def createSend():
  t = getRandomAvailableThread()
  target = getRandomThread()
  message = getRandomMessage()
  return "%d SEND %d %s" % (t, target, message)

def createSendSync():
  t = getRandomAvailableThread()
  target = getRandomThread()
  message = getRandomMessage()

  if not isAvailableThread(target):
    blockAtSync(t)

  event = self.event
  self.event += 1
  return "%d SEND %d SYNC %s <EVENT %d %d>" % (t, target, message, event, 1)

def createBroadcast():
  t = getRandomAvailableThread()
  message = getRandomMessage()
  return "%d BROADCAST %s" % (t, message)

def createBroadcastSync():
  t = getRandomAvailableThread()
  message = getRandomMessage()

  if blockedThreads():
    blockAtSync(t)

  event = self.event
  self.event += 1
  return "%d BROADCAST SYNC %s <EVENT %d %d>" % (t, message, event, 1)

def createClose():
  command = ""
  if test_state.barrier_counter >= 0 and test_state.waiting_at_barrier:
    while len(test_state.waiting_at_barrier) <= test_state.barrier_counter:
      command += createBarrier() + "\n"
    command += createDestroyBarrier() + "\n"
  return command + "CLOSE"

# This dictionary holds all available commands to create as keys, and the
# values are the associated creation functions for those commands and their
# probability of appearing (before normalization).
commands = { }
commands["threads"] = [
    (createBarrierCommand, 0.5),
    (createSend, 0.25),
    (createSendSync, 0.25),
    (createBroadcast, 0.25),
    (createBroadcastSync, 0.25),
]

# Sum all of the chances together for each set of commands.
sumChances = { }
sumChances["threads"] = reduce(lambda s,c:s+c[1], commands["threads"], 0.0)

# The initial commands to put at the head of the input file.
INITIAL_COMMANDS = { }
INITIAL_COMMANDS["threads"] = [createInit]
# The final commands to put at the end of the input file.
FINAL_COMMANDS = { }
FINAL_COMMANDS["threads"] = [createClose]


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
  return [c() for c in init_commands] + \
         [createNewCommand(cmd_list, cmd_sum) for i in range(num_lines)] + \
         [c() for c in fin_commands]
    
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
