import sys
import os
import os.path
import random
from logic import Task
from progressbar import *

TEST_DIR = ".." + os.path.sep + "random"
TEST_FILE_PATTERN = "test%d"

SIMULATOR_PATH = "python simulator.py"
TEST_INPUT_SUFFIX = ".in.txt"
TEST_OUTPUT_SUFFIX = ".out.txt"
TEST_ERROR_SUFFIX = ".err.txt"

MIN_COMMANDS = int(os.environ.get("SERVERS_MIN_COMMANDS",100))
MAX_COMMANDS = int(os.environ.get("SERVERS_MAX_COMMANDS",500))
ERROR_CHANCE = 0.05


# This is the initial amount of servers.
K = 0
# These are all the tasks that we currently have.
tasks = {}
# This is the last reqID that was used.
last_reqID = 0
# These are the dead servers.
dead_servers = []


# seed the random number generator
random.seed()


##############################
# Utility Functions          #
##############################

def getRandomServerCount():
  return random.randint(1,10000)

def getNewReqID():
  global last_reqID
  if random.random() < 0.1:
    return random.randint(0,last_reqID)
  return last_reqID + random.randint(0,100)

def getExistingReqID():
  if not tasks:
    return getNewReqID()
  return tasks.keys()[random.randrange(len(tasks.keys()))]

def getRandomPriority():
  return random.randint(0,2*K)

def getRandomExistingPriority():
  if not tasks:
    return getRandomPriority()
  return tasks.values()[random.randrange(len(tasks.values()))].priority

def getRandomServer():
  return random.randrange(K)

def getDeadServer():
  if not dead_servers:
    return getRandomServer()
  return dead_servers[random.randrange(len(dead_servers))]

def getLiveServer():
  if len(dead_servers) == K:
    return getDeadServer()
  s = random.randrange(K)
  while s in dead_servers:
    s = random.randrange(K)
  return s

def getNumTasksForServer(serverID):
  return len([t for t in tasks.values() if t.serverID == serverID])

def getEmptyServer():
  s = getLiveServer()
  if s in dead_servers:
    return s
  if getNumTasksForServer(s) < 3:
    return s
  for i in range(3):
    s = getLiveServer()
    if getNumTasksForServer(s) < 3:
      return s
  return s

def getBusyServer():
  s = getLiveServer()
  if s in dead_servers:
    return s
  if getNumTasksForServer(s) >= 3:
    return s
  for i in range(3):
    s = getLiveServer()
    if getNumTasksForServer(s) >= 3:
      return s
  return s

def generateErrorRequest():
  reqID = 0
  serverID = 0
  priority = 0
  if random.random() < 0.3:
    reqID = random.randint(-10000,-1)
  else:
    reqID = getNewReqID()
  if random.random() < 0.3:
    serverID = random.randint(-2*K,2*K)
  else:
    serverID = getDeadServer()
    if serverID not in dead_servers:
      serverID = -1
  if random.random() < 0.3:
    priority = random.randint(-10000,-1)
  else:
    priority = random.randint(0,100)
    
  return "EnqueueRequest %d,%d,%d" % (reqID,serverID,priority)

##############################
# Command Creation Functions #
##############################

def createInit():
  global K
  K = getRandomServerCount()
  return "Init %d" % K

def createEnqueueRequestForBusyServer():
  if random.random() < ERROR_CHANCE:
    return generateErrorRequest()
  
  reqID = getNewReqID()
  global last_reqID
  last_reqID = reqID + 1
  serverID = getBusyServer()
  if random.random() < 0.5:
    priority = getRandomExistingPriority()
  else:
    priority = getRandomPriority()

  global tasks
  tasks[reqID] = Task(reqID,serverID,priority)
  
  return "EnqueueRequest %d,%d,%d" % (reqID,serverID,priority)

def createEnqueueRequestForNewServer():
  if random.random() < ERROR_CHANCE:
    return generateErrorRequest()
  
  reqID = getNewReqID()
  global last_reqID
  last_reqID = reqID + 1
  serverID = getEmptyServer()
  if random.random() < 0.5:
    priority = getRandomExistingPriority()
  else:
    priority = getRandomPriority()

  global tasks
  tasks[reqID] = Task(reqID,serverID,priority)
  
  return "EnqueueRequest %d,%d,%d" % (reqID,serverID,priority)

def createDequeueRequest():
  reqID = getExistingReqID()
  if random.random() < ERROR_CHANCE:
    reqID = random.randint(-2*last_reqID,2*last_reqID)
  if reqID in tasks.keys():
    del tasks[reqID]

  return "DequeueRequest %d" % reqID

def createGetRequestPriority():
  reqID = getExistingReqID()
  if random.random() < ERROR_CHANCE:
    reqID = random.randint(-2*last_reqID,2*last_reqID)

  return "GetRequestPriority %d" % reqID

def createGetHighestPriorityRequest():
  return "GetHighestPriorityRequest"

def createLowerOldRequestsPriority():
  reqID = random.randint(0,last_reqID)
  if random.random() < ERROR_CHANCE:
    reqID = random.randint(-2*last_reqID,2*last_reqID)
  delta = getRandomExistingPriority()
  if random.random() < ERROR_CHANCE:
    delta = getRandomPriority()
  elif random.random() < ERROR_CHANCE:
    delta = random.randint(-2*K,2*K)

  if delta >= 0:
    global tasks
    for t in tasks.values():
      if t.reqID <= reqID:
        t.priority -= delta
        if t.priority < 0: t.priority = 0

  return "LowerOldRequestsPriority %d,%d" % (reqID,delta)

def createGetHandlingServerID():
  reqID = getExistingReqID()
  if random.random() < ERROR_CHANCE:
    reqID = random.randint(-2*last_reqID,2*last_reqID)

  return "GetHandlingServerId %d" % reqID

def createKillServerBusy():
  serverID = getBusyServer()
  if random.random() < ERROR_CHANCE:
    serverID = random.randint(-2*K,2*K)
  global dead_servers
  if serverID >= 0 and serverID < K and serverID not in dead_servers:
    dead_servers += [serverID]
  newServerID = getRandomServer()
  return "KillServer %d,%d" % (serverID,newServerID)

def createKillServerEmpty():
  serverID = getEmptyServer()
  if random.random() < ERROR_CHANCE:
    serverID = random.randint(-2*K,2*K)
  global dead_servers
  if serverID >= 0 and serverID < K and serverID not in dead_servers:
    dead_servers += [serverID]
  newServerID = getRandomServer()
  return "KillServer %d,%d" % (serverID,newServerID)

def createQuit():
  return "Quit"

# This dictionary holds all available commands to create as keys, and the
# values are the associated creation functions for those commands and their
# probability of appearing (before normalization).
commands = { }
commands["servers"] = [
    (createEnqueueRequestForBusyServer, 0.5),
    (createEnqueueRequestForNewServer, 0.5),
    (createDequeueRequest, 0.5),
    (createGetRequestPriority, 0.3),
    (createGetHighestPriorityRequest, 0.3),
    (createLowerOldRequestsPriority, 0.3),
    (createGetHandlingServerID, 0.3),
    (createKillServerBusy, 0.3),
    (createKillServerEmpty, 0.3),
]

# Sum all of the chances together for each set of commands.
sumChances = { }
sumChances["servers"] = reduce(lambda s,c:s+c[1], commands["servers"], 0.0)

# The initial commands to put at the head of the input file.
INITIAL_COMMANDS = { }
INITIAL_COMMANDS["servers"] = [createInit()]
# The final commands to put at the end of the input file.
FINAL_COMMANDS = { }
FINAL_COMMANDS["servers"] = [createQuit()]

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
    num_lines = random.randint(MIN_COMMANDS, MAX_COMMANDS) - 2
    return init_commands + \
        [createNewCommand(cmd_list, cmd_sum) for i in range(num_lines)] + \
        fin_commands
    
# Creates a new random file from a given command mode.
def createNewFile(i, mode):
    # Create the .in file.
    filepath = TEST_DIR + os.path.sep + (TEST_FILE_PATTERN % i) + TEST_INPUT_SUFFIX
    f = file(filepath, "w")
    
    # Create the lines.
    in_cmds = createNewCommandSet(commands[mode], sumChances[mode],
                                  INITIAL_COMMANDS[mode], FINAL_COMMANDS[mode])
    for c in in_cmds:
        f.write(c + "\n")
    f.close()
    
    # Run the simulator to get the outputs.
    outpath = TEST_DIR + os.path.sep + (TEST_FILE_PATTERN % i) + TEST_OUTPUT_SUFFIX
    runSimulator(filepath, outpath)

# Runs the simulator for the given input file, creating error and output files.
def runSimulator(inpath, outpath):
    os.system(SIMULATOR_PATH + " " + inpath + " > " + outpath)

# Creates the simulated output and error for each of the files in input_dir.
def createOutputs(input_dir):
    from glob import glob
    inputs = glob(input_dir + os.path.sep + "*" + TEST_INPUT_SUFFIX)
    outputs = [f[:-3]+TEST_OUTPUT_SUFFIX for f in inputs]
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
            print "       Possible modes: servers"
            sys.exit(1)
parseArgs()
if not modes:
    modes = ["servers"]

if not input_dir:
    # Make sure test dir exists.
    try:
        os.makedirs(TEST_DIR)
    except:
        pass

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
