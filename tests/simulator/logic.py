Suites = {
  "CLUB": 0,
  "DIAMOND": 1,
  "SPADE": 2,
  "HEART": 3,
}

STATUS_SUCCESS = 0
STATUS_FAILURE = -1
STATUS_ALLOCATION_ERROR = -2
STATUS_INVALID_INPUT = -3

class YanivLogic:

  def __init__(self, N):
    self.N = N
    self.cards_per_suite = [[] for s in Suites]

  def takeCard(self, card_number, card_suite):
    if card_number <= 0 or card_number > self.N:
      return STATUS_INVALID_INPUT
    if card_suite not in Suites:
      return STATUS_INVALID_INPUT

    if card_number in self.cards_per_suite[Suites[card_suite]]:
      return STATUS_FAILURE

    self.cards_per_suite[Suites[card_suite]].append(card_number)
    self.cards_per_suite[Suites[card_suite]].sort()
    return STATUS_SUCCESS

  def dropCard(self, card_number, card_suite):
    if card_number <= 0 or card_number > self.N:
      return STATUS_INVALID_INPUT
    if card_suite not in Suites:
      return STATUS_INVALID_INPUT

    if card_number not in self.cards_per_suite[Suites[card_suite]]:
      return STATUS_FAILURE
    index = self.cards_per_suite[Suites[card_suite]].index(card_number)
    del self.cards_per_suite[Suites[card_suite]][index]
    return STATUS_SUCCESS

  def getHandValue(self):
    hand_value = 0
    for suite_cards in self.cards_per_suite:
      hand_value += self._sumList(suite_cards)
    return hand_value

  def getNumCardsOfSuite(self, card_suite):
    return len(self.cards_per_suite[Suites[card_suite]])

  def getMostValuableSeries(self):
    best_series = 0
    best_score = 0
    best_num = 0
    for num in range(1,self.N+1):
      num_series = len([s for s in Suites.values() if num in self.cards_per_suite[s]])
      num_score = num_series * num
      #print "// getMostValuableSeries: num=%d, num_series=%d" % (num, num_series)
      if ((num_score > best_score) or (num_score == best_score and num > best_num)):
        best_series = num_series
        best_score = num_score
        best_num = num
    return (best_num, best_series)

  def getLongestStraightWithoutJoker(self):
    best_straight_suite = Suites.keys()[0]
    best_straight = []
    for s in Suites.keys():
      curr_straight = []
      for c in self.cards_per_suite[Suites[s]]:
        if curr_straight and (c-1) != curr_straight[-1]:
          if self._compareStraights(
              best_straight, best_straight_suite, curr_straight, s):
            best_straight = curr_straight
            best_straight_suite = s
          curr_straight = []
        curr_straight.append(c)
        #print "best_straight = %s(%s), curr_straight = %s(%s)" % (best_straight, best_straight_suite, curr_straight, s)
      if self._compareStraights(
          best_straight, best_straight_suite, curr_straight, s):
        best_straight = curr_straight
        best_straight_suite = s
    return (best_straight, best_straight_suite)

  def getLongestStraightWithJoker(self):
    best_straight_suite = ""
    best_straight = []
    for s in Suites.keys():
      #print "%s: %s" % (s, self.cards_per_suite[Suites[s]])
      curr_straight = []
      ic = 0
      while ic < len(self.cards_per_suite[Suites[s]]):
        c = self.cards_per_suite[Suites[s]][ic]
        if curr_straight and (0 not in curr_straight) and (c-2) == curr_straight[-1]:
          curr_straight.append(0)
        elif curr_straight and (c-1) != curr_straight[-1]:
          if 0 not in curr_straight:
            curr_straight.append(0)
          if self._compareStraights(
              best_straight, best_straight_suite, curr_straight, s):
            best_straight = curr_straight
            best_straight_suite = s
          if 0 in curr_straight:
            curr_straight = curr_straight[curr_straight.index(0)+1:]
            continue
          else:
            curr_straight = []
        curr_straight.append(c)
        ic += 1
        #print "best_straight = %s(%s), curr_straight = %s(%s)" % (best_straight, best_straight_suite, curr_straight, s)
      if 0 not in curr_straight:
        curr_straight.append(0)
      if self._compareStraights(
          best_straight, best_straight_suite, curr_straight, s):
        best_straight = curr_straight
        best_straight_suite = s
    if 0 in best_straight:
      best_straight.remove(0)
    return (best_straight, best_straight_suite)

  def _sumList(self, l):
    sum = 0
    for i in range(len(l)):
      n = l[i]
      if n == 0:
        n = l[i-1]+1
      sum += n
    return sum

  def _compareStraights(self, s1, s1_suite, s2, s2_suite):
    if len(s1) < len(s2):
      return True

    sum1 = self._sumList(s1)
    sum2 = self._sumList(s2)
    if len(s1) == len(s2) and sum1 < sum2:
      return True

    if len(s1) == len(s2) and sum1 == sum2 and \
       Suites.get(s1_suite, -1) < Suites.get(s2_suite, -1):
      return True

    return False
