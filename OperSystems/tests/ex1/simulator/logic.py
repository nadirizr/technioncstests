from sys import stderr

ESRCH = 3
EINVAL = 22

class Process:

  def __init__(self, tag=0, parent=None, children=[]):
    self.tag = tag
    self.parent = parent
    self.children = children[:]

class TagsLogic:

  def __init__(self):
    self.processes = [Process()]
    self.tag = 0

  def createChild(self, process_indexes):
    p = self.__getProcessFromIndexes(process_indexes)
    new_process = Process(p.tag / 2, p)
    self.processes.append(new_process)
    p.children.append(new_process)
    return len(self.processes) - 1

  def getTag(self, process_indexes, pid):
    if 0 <= pid < len(self.processes):
      return self.processes[pid].tag
    return -ESRCH

  def setTag(self, process_indexes, pid, tag):
    if tag < 0:
      return -EINVAL

    if not (0 <= pid < len(self.processes)):
      return -ESRCH

    p = self.__getProcessFromIndexes(process_indexes)
    if pid == self.processes.index(p):
      p.tag = tag
      return 0
    for c in p.children:
      if pid == self.processes.index(c):
        c.tag = tag
        return 0
    return -EINVAL

  def getGoodProcesses(self, process_indexes, count):
    if count <= 0:
      return -EINVAL
    
    pids = [pid for pid in xrange(len(self.processes))
            if self.__isProcessGood(self.processes[pid])]
    pids.sort()
    
    if self.tag > self.processes[0].tag:
      pids = [-1] + pids

    return pids[:count]

  def makeGoodProcesses(self, process_indexes):
    if not self.processes:
      return 0

    p = self.__getProcessFromIndexes(process_indexes)
    num_checked = 1
    while p is not None:
      if not self.__isProcessGood(p):
        p.tag = self.__sumChildrenTags(p) + 1
      num_checked += 1
      p = p.parent

    if self.tag <= self.processes[0].tag:
      self.tag = self.processes[0].tag + 1

    return num_checked

  def close(self, process_indexes):
    p = self.__getProcessFromIndexes(process_indexes)
    return self.__removeProcess(p)
    
  def __removeProcess(self, process):
    if not process:
      return -1

    children = process.children[:]
    for c in children:
      self.__removeProcess(c)

    if process.parent:
      process.parent.children.remove(process)
    self.processes.remove(process)
    return 0

  def __getProcessFromIndexes(self, process_indexes):
    p = self.processes[0]
    for i in process_indexes:
      if not (0 <= i < len(p.children)):
        return None
      p = p.children[i]
    return p

  def __isProcessGood(self, process):
    return process.tag > self.__sumChildrenTags(process)

  def __sumChildrenTags(self, process):
    return reduce(lambda x,y:x+y, [c.tag for c in process.children], 0)

  def __printTree(self, process):
    if process is None:
      return

    print >> stderr, "PID(%d).children = %s" % (self.processes.index(process),
                                                [self.processes.index(c) for c in process.children])
    for c in process.children:
      self.__printTree(c)
