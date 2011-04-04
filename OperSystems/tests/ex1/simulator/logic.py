ESRCH = 3
EINVAL = 22

class Process:
  def __init__(self, tag=0, parent=None, children=[]):
    self.parent = parent
    self.children = children
    self.tag = tag

class TagsLogic:

  def __init__(self):
    self.processes = [Process()]

  def createChild(self, process_indexes):
    p = self.__getProcessFromIndexes(process_indexes)
    new_process = Process(p)
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

    p = self.__getProcessFromIndexes(process_indexes)
    if not (0 <= pid < len(self.processes)):
      return -ESRCH

    if pid == self.processes.index(p):
      p.tag = tag
      return 0
    for c in p.children:
      if pid == self.processes.index(c):
        c.tag = tag
        return 0
    return -EINVAL

  def getGoodProcesses(self, process_indexes, count):
    if count < 0:
      return -EINVAL
    
    pids = [pid for pid in xrange(len(self.processes))
            if self.__isProcessGood(self.processes[pid])]:
    pids.sort()
    return pids[:count]

  def makeGoodProcesses(self, process_indexes):
    p = self.__getProcessFromIndexes(process_indexes)
    num_checked = 0
    while p is not None:
      if not self.__isProcessGood(p):
        p.tag = self.__sumChildrenTags(p) + 1
      num_checked += 1
      p = p.parent
    return num_checked

  def close(self, process_indexes):
    p = self.__getProcessFromIndexes(process_indexes)
    for c in self.children:
      self.close(c)
    self.__removeProcess(p)
    return 0
    
  def __removeProcess(process):
    self.processes.remove(process)
    if process.parent is not None:
      process.parent.children.remove(process)

  def __getProcessFromIndexes(self, process_indexes):
    p = self.processes[0]
    for i in process_indexes:
      if not (0 <= i < len(p.children)):
        return -1
      p = p.children[i]
    return p

  def __isProcessGood(self, process):
    return process.tag > self.__sumChildrenTags(process)

  def __sumChildrenTags(self, process):
    return reduce(lambda x,y:x+y, [c.tag for c in process.children], 0)
