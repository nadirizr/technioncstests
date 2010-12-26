import sys
import os
import os.path
import random

TEST_DIR = ".." + os.path.sep + "random"
TEST_FILE_PATTERN = "test%d"

SIMULATOR_PATH = "python simulator.py"
TEST_INPUT_SUFFIX = ".in.txt"
TEST_OUTPUT_SUFFIX = ".out.txt"
TEST_ERROR_SUFFIX = ".err.txt"

MIN_COMMANDS = 10
MAX_COMMANDS = 100
ERROR_CHANCE = 0.05


# these are all the cards that we currently have in hand
suites = ["CLUB", "DIAMOND", "SPADE", "HEART"]
cards_per_suite = {
  "CLUB": [],
  "DIAMOND": [],
  "SPADE": [],
  "HEART": [],
}
# and this is the largest card number we can have
N = 0


# seed the random number generator
random.seed()


##############################
# Utility Functions          #
##############################

def getRandomHighestCard():
  return random.randint(5, 500)

def getRandomSuite():
  return suites[random.randrange(len(suites))]

def getRandomInvalidSuite():
  return chr(random.randrange(ord("a"),ord("z")))

def getSuiteLetter(suite):
  return suite[0].lower()

def getRandomInvalidCard():
  suite = getRandomInvalidSuite()
  number = random.randrange(0, 2*N)
  return (number, suite)

def getRandomExistingCard():
  suite = getRandomSuite()
  cards = cards_per_suite[suite]
  if not cards:
    for s in suites:
      if cards_per_suite[s]:
        cards = cards_per_suite[s]
  if not cards:
    return None

  return (cards[random.randrange(len(cards))], suite)

def getCardSuiteCount(number):
  return len([s for s in suites if number in cards_per_suite[s]])

def getRandomExistingStraight():
  suite = getRandomSuite()
  cards = cards_per_suite[suite]
  if not cards:
    for s in suites:
      if cards_per_suite[s]:
        cards = cards_per_suite[s]
  if not cards:
    return None

  ic = random.randrange(len(cards))
  straight = [cards[ic]]
  ic += 1
  while (ic < len(cards)) and (cards[ic] == straight[-1] + 1):
    straight += [cards[ic]]
    ic += 1
  return (straight, suite)


##############################
# Command Creation Functions #
##############################

def createInit():
  global N
  N = getRandomHighestCard()
  return "Init %d" % N

def createTakeCardForStraightWithJoker():
  straight_tuple = getRandomExistingStraight()
  if straight_tuple is None:
    return createTakeCardRandom()
  (straight, suite) = straight_tuple

  if random.random() < 0.5 and (straight[-1]+2 <= N):
    card_number = straight[-1]+2
  elif straight[0]-2 > 0:
    card_number = straight[0]-2
  else:
    return None

  if card_number not in cards_per_suite[suite]:
    cards_per_suite[suite].append(card_number)
    cards_per_suite[suite].sort()

  return "TakeCard %d%s" % (card_number, getSuiteLetter(suite))

def createTakeCardForStraight():
  straight_tuple = getRandomExistingStraight()
  if straight_tuple is None:
    return createTakeCardRandom()
  (straight, suite) = straight_tuple

  if random.random() < 0.5 and (straight[-1]+1 <= N):
    card_number = straight[-1]+1
  elif straight[0]-1 > 0:
    card_number = straight[0]-1
  else:
    return None

  if card_number not in cards_per_suite[suite]:
    cards_per_suite[suite].append(card_number)
    cards_per_suite[suite].sort()

  return "TakeCard %d%s" % (card_number, getSuiteLetter(suite))

def createTakeCardForSeries():
  card_tuple = getRandomExistingCard()
  if card_tuple is None:
    return createTakeCardRandom()
  (number, suite) = card_tuple

  for i in range(3):
    if getCardSuiteCount(number) < len(suites):
      break
    (number, suite) = getRandomExistingCard()

  for s in suites:
    if number not in cards_per_suite[s]:
      return "TakeCard %d%s" % (number, getSuiteLetter(s))
  return createTakeCardRandom()

def createTakeCardRandom():
  number = None
  suite = None
  if random.random() < ERROR_CHANCE/2:
    number = 0
  elif random.random() < ERROR_CHANCE/2:
    suite = getRandomInvalidSuite()
  
  if number is None:
    number = random.randrange(1,N+1)
  if suite is None:
    suite = getRandomSuite()
  return "TakeCard %d%s" % (number, getSuiteLetter(suite))

def createDropCardGood():
  if random.random() < ERROR_CHANCE:
    (card_number, card_suite) = getRandomInvalidCard()
  else:
    card_tuple = getRandomExistingCard()
    if card_tuple is None:
      return createDropCardMissing()
    (card_number, card_suite) = getRandomExistingCard()
  return "DropCard %d%s" % (card_number, getSuiteLetter(card_suite))

def createDropCardMissing():
  if random.random() < ERROR_CHANCE:
    (card_number, card_suite) = getRandomInvalidCard()
  else:
    card_suite = getRandomSuite()
    card_number = random.randrange(1,N+1)
  return "DropCard %d%s" % (card_number, getSuiteLetter(card_suite))

def createHandValue():
  return "HandValue"

def createNumCardsOfSuite():
  if random.random() < ERROR_CHANCE:
    suite = getRandomInvalidSuite()
  else:
    suite = getRandomSuite()

  return "NumCardsOfSuite %s" % getSuiteLetter(suite)

def createMostValuableSeries():
  return "MostValuableSeries"

def createLongestStraightWithoutJoker():
  return "LongestStraightWithoutJoker"

def createLongestStraightWithJoker():
  return "LongestStraightWithJoker"

def createQuit():
  return "Quit"

# this dictionary holds all available commands to create as keys, and the
# values are the associated creation functions for those commands and their
# probability of appearing
commands = { }
commands["yaniv"] = [
    (createTakeCardForStraightWithJoker, 0.3),
    (createTakeCardForStraight, 0.5),
    (createTakeCardForSeries, 0.5),
    (createTakeCardRandom, 0.5),
    (createDropCardGood, 0.5),
    (createDropCardMissing, 0.5),
    (createHandValue, 0.2),
    (createNumCardsOfSuite, 0.2),
    (createMostValuableSeries, 0.2),
    (createLongestStraightWithoutJoker, 0.2),
    (createLongestStraightWithJoker, 0.2),
]

# sum all of the chances together for each set of commands
sumChances = { }
sumChances["yaniv"] = reduce(lambda s,c:s+c[1], commands["yaniv"], 0.0)

# the initial commands to put at the head of the input file
INITIAL_COMMANDS = { }
INITIAL_COMMANDS["yaniv"] = [createInit()]
# the final commands to put at the end of the input file
FINAL_COMMANDS = { }
FINAL_COMMANDS["yaniv"] = [createQuit()]

##############################
# Logic Functions            #
##############################


# creates a new random command
def createNewCommand(cmd_list, cmd_sum):
    chance = random.random() * cmd_sum
    sum = 0.0
    for c in cmd_list:
        sum += c[1]
        if chance < sum:
            return c[0]()

# creates a new random set of commands for a file
def createNewCommandSet(cmd_list, cmd_sum, init_commands, fin_commands):
    num_lines = random.randint(MIN_COMMANDS, MAX_COMMANDS) - 2
    return init_commands + \
        [createNewCommand(cmd_list, cmd_sum) for i in range(num_lines)] + \
        fin_commands
    
# creates a new random file from a given command mode
def createNewFile(i, mode):
    # create the .in file
    filepath = TEST_DIR + os.path.sep + (TEST_FILE_PATTERN % i) + TEST_INPUT_SUFFIX
    f = file(filepath, "w")
    
    # create the lines
    in_cmds = createNewCommandSet(commands[mode], sumChances[mode],
                                  INITIAL_COMMANDS[mode], FINAL_COMMANDS[mode])
    for c in in_cmds:
        f.write(c + "\n")
    f.close()
    
    # run the simulator to get the outputs
    outpath = TEST_DIR + os.path.sep + (TEST_FILE_PATTERN % i) + TEST_OUTPUT_SUFFIX
    runSimulator(filepath, outpath)

# runs the simulator for the given input file, creating error and output files
def runSimulator(inpath, outpath):
    os.system(SIMULATOR_PATH + " " + inpath + " > " + outpath)

# creates the simulated output and error for each of the files in input_dir
def createOutputs(input_dir):
    from glob import glob
    inputs = glob(input_dir + os.path.sep + "*" + TEST_INPUT_SUFFIX)
    outputs = [f[:-3]+TEST_OUTPUT_SUFFIX for f in inputs]
    for i in range(len(inputs)):
        runSimulator(inputs[i], outputs[i])

# determine amount of files to create
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
            print "       Possible modes: yaniv"
            sys.exit(1)
parseArgs()
if not modes:
    modes = ["yaniv"]

if not input_dir:
    # make sure test dir exists
    try:
        os.makedirs(TEST_DIR)
    except:
        pass

    # create the files
    for i in range(num_files):
        createNewFile(i, modes[0])
else:
    createOutputs(input_dir)
