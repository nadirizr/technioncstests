class ProcessState:

  def __init__(self, parent=None, tag=None, children=None, pid=0):
    self.parent = parent
    
    self.tag = tag
    if not self.tag and self.parent:
      self.tag = self.parent.tag / 2
    elif not self.tag:
      self.tag = 0
    
    self.children = children
    if not self.children:
      self.children = []

    self.pid = pid

  def getChildren(self):
    return self.children

  def addChild(self, process_state):
    self.children.append(process_state)

  def removeChild(self, process_state):
    self.children.remove(process_state)

  def getIndexAtParent(self):
    if self.parent:
      return self.parent.children.index(self)
    return -1

  def getHierarchy(self):
    children_indexes = []
    p = self
    while p and p.parent:
      children_indexes.append(p.getIndexAtParent())
      p = p.parent
    return children_indexes

class MainState:

  def __init__(self, initial_tag=0):
    self.initial_tag = 0
    self.processes = [ProcessState()]
  
  def getMainProcess(self):
    return self.processes[0]

  def getProcesses(self):
    return self.processes

  def getPIDs(self):
    return range(len(self.processes))

  def getPIDForProcess(self, process_state):
    if process_state in self.processes:
      return self.processes.index(process_state)
    return -1

  def getProcessForPID(self, pid):
    if 0 <= pid < len(self.processes):
      return self.processes[pid]
    return None

  def getProcessForIndexes(self, process_indexes):
    p = self.getMainProcess()
    for i in process_indexes:
      p = p.children[i]
    return p

  def addNewProcess(self, tag, parent):
    new_process = ProcessState(parent, tag)
    parent.addChild(new_process)
    self.processes.append(new_process)
    return new_process

  def removeProcessAndChildren(self, process_state):
    process_children = process_state.children[:]
    for c in process_children:
      self.removeProcessAndChildren(c)

    self.processes.remove(process_state)
    if process_state.parent:
      process_state.parent.removeChild(process_state)

  def printProcessTree(self):
    for i in range(len(self.processes)):
      print "PID[%d] = %s" % (i, [self.getPIDForProcess(c) for c in self.processes[i].getChildren()])
