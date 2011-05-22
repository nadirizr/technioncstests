from sys import stderr

from tag_util import *

ESRCH = 3
EINVAL = 22

class TagsLogic:

  def __init__(self):
    self.state = MainState()
    self.tag = 0

  def createChild(self, process_indexes):
    p = self.state.getProcessForIndexes(process_indexes)
    new_process = self.state.addNewProcess(p.tag / 2, p)
    return self.state.getPIDForProcess(new_process)

  def getTag(self, process_indexes, pid):
    if 0 <= pid < len(self.state.getProcesses()):
      return self.state.getProcessForPID(pid).tag
    return -ESRCH

  def setTag(self, process_indexes, pid, tag):
    if not (0 <= pid < len(self.state.getProcesses())):
      return -ESRCH

    if tag < 0:
      return -EINVAL

    p = self.state.getProcessForIndexes(process_indexes)
    if pid == self.state.getPIDForProcess(p):
      p.tag = tag
      return 0
    for c in p.children:
      if pid == self.state.getPIDForProcess(c):
        c.tag = tag
        return 0
    return -EINVAL

  def getGoodProcesses(self, process_indexes, count):
    if count < 0:
      return -EINVAL
    
    pids = [pid for pid in self.state.getPIDs()
            if self.__isProcessGood(self.state.getProcessForPID(pid))]
    pids.sort()
    
    if self.tag > self.state.getMainProcess().tag:
      pids = [-1] + pids
    pids = [0, 1] + pids

    return pids[:count]

  def makeGoodProcesses(self, process_indexes):
    if not self.state.getProcesses():
      return 0

    p = self.state.getProcessForIndexes(process_indexes)
    num_checked = 1
    while p is not None:
      if not self.__isProcessGood(p):
        p.tag = self.__sumChildrenTags(p) + 1
      num_checked += 1
      p = p.parent

    if self.tag <= self.state.getMainProcess().tag:
      self.tag = self.state.getMainProcess().tag + 1

    return num_checked

  def close(self, process_indexes):
    self.state.removeProcessAndChildren(
        self.state.getProcessForIndexes(process_indexes))
    return 0
    
  def __isProcessGood(self, process):
    return process.tag > self.__sumChildrenTags(process)

  def __sumChildrenTags(self, process):
    return reduce(lambda x,y:x+y, [c.tag for c in process.children], 0)
