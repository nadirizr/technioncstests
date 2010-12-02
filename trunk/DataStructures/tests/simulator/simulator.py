#!/usr/bin/python

# Given an input file of commands, and outputs the same output as the official
# tests to the standard output. If no file is given, reads from stdin.
# Optionally, if given the flag --print_hand, print before every step the
# current hand.
#
# Called as:
# ./simulator.py <input_file> [--print_hand]

import sys

from logic import *

LetterToSuite = {
  "c": "CLUB",
  "d": "DIAMOND",
  "s": "SPADE",
  "h": "HEART",
}
SuiteToLetter = {
  "CLUB": "c",
  "DIAMOND": "d",
  "SPADE": "s",
  "HEART": "h",
}

class YanivParser:

  def __init__(self, print_hand=False):
    self.logic = None
    self.print_hand = print_hand

  def parse(self, input_stream):
    line = input_stream.readline()
    while line:
      if print_hand:
        self.printHand()
      self.parseLine(line)
      line = input_stream.readline()

  def parseLine(self, line):
    args = line.split()
    cmd = args[0]

    if not cmd or cmd[0] == "#":
      print line,
      return STATUS_SUCCESS
    
    if cmd != "Init" and self.logic is None:
        print "%s: Invalid_input" % cmd
        return STATUS_INVALID_INPUT
    if cmd == "Init" and self.logic is not None:
        print "Init was already called."
        return STATUS_FAILURE

    if cmd == "Init":
      N = int(args[1])
      if N <= 0:
        print "Init failed."
        return STATUS_FAILURE
      self.logic = YanivLogic(N)
      print "Init done."
      return STATUS_SUCCESS
    elif cmd == "TakeCard":
      try:
        card_suite = LetterToSuite[args[1][-1].lower()]
        card_number = int(args[1][:-1])
      except:
        print "TakeCard: Invalid_input"
        return STATUS_INVALID_INPUT

      retval = self.logic.takeCard(card_number, card_suite)
      if retval == STATUS_SUCCESS:
        print "TakeCard: Success"
      elif retval == STATUS_INVALID_INPUT:
        print "TakeCard: Invalid_input"
      else:
        print "TakeCard: Failure"
      return retval
    elif cmd == "DropCard":
      try:
        card_suite = LetterToSuite[args[1][-1].lower()]
        card_number = int(args[1][:-1])
      except:
        print "DropCard: Invalid_input"
        return STATUS_INVALID_INPUT

      retval = self.logic.dropCard(card_number, card_suite)
      if retval == STATUS_SUCCESS:
        print "DropCard: Success"
      elif retval == STATUS_INVALID_INPUT:
        print "DropCard: Invalid_input"
      else:
        print "DropCard: Failure"
      return retval
    elif cmd == "HandValue":
      handvalue = self.logic.getHandValue()
      print "HandValue: Success %d" % handvalue
      return STATUS_SUCCESS
    elif cmd == "NumCardsOfSuite":
      try:
        card_suite = LetterToSuite[args[1].lower()]
      except:
        print "NumCardsOfSuite: Invalid_input"
        return STATUS_INVALID_INPUT

      numcards = self.logic.getNumCardsOfSuite(card_suite)
      print "NumCardsOfSuite: Success %d" % numcards
      return STATUS_SUCCESS
    elif cmd == "MostValuableSeries":
      (best_series_num, best_series_count) = self.logic.getMostValuableSeries()
      best_series_score = best_series_num * best_series_count
      if best_series_count != 0:
        print "MostValuableSeries: Success, Card: %d, Total Value: %d" % (
            best_series_num, best_series_score)
        return STATUS_SUCCESS
      else:
        print "MostValuableSeries: Failure"
        return STATUS_FAILURE
    elif cmd == "LongestStraightWithoutJoker":
      (best_straight, best_straight_suite) = self.logic.getLongestStraightWithoutJoker()
      if best_straight:
        print "LongestStraightWithoutJoker: Success, %d%s-%d%s" % (
            best_straight[0], SuiteToLetter[best_straight_suite],
            best_straight[-1], SuiteToLetter[best_straight_suite])
        return STATUS_SUCCESS
      else:
        print "LongestStraightWithoutJoker: Failure"
        return STATUS_FAILURE
    elif cmd == "LongestStraightWithJoker":
      (best_straight, best_straight_suite) = self.logic.getLongestStraightWithJoker()
      if 0 in best_straight:
        del best_straight[best_straight.index(0)]
      if best_straight:
        print "LongestStraightWithJoker: Success, %d%s-%d%s" % (
            best_straight[0], SuiteToLetter[best_straight_suite],
            best_straight[-1], SuiteToLetter[best_straight_suite])
        return STATUS_SUCCESS
      else:
        print "LongestStraightWithJoker: Failure"
        return STATUS_FAILURE
    elif cmd == "Quit":
      del self.logic
      self.logic = None
      print "Quit done."
      return STATUS_SUCCESS
  
  def printHand(self):
    for s in Suites.keys():
      if self.logic is None:
        cards = []
      else:
        cards = self.logic.cards_per_suite[Suites[s]]
      print "%s%s," % (SuiteToLetter[s].upper(), cards),
    print

file_stream = sys.stdin
print_hand = False
if sys.argv[1:]:
  file_stream = file(sys.argv[1], "r")
if sys.argv[2:] and sys.argv[2] == "--print_hand":
  print_hand = True

parser = YanivParser(print_hand)
parser.parse(file_stream)

if file_stream != sys.stdin:
  file_stream.close()
