STATUS_SUCCESS = 0
STATUS_FAILURE = -1
STATUS_ALLOCATION_ERROR = -2
STATUS_INVALID_INPUT = -3

class Task:
  def __init__(self, reqID, serverID, priority):
    self.reqID = reqID
    self.serverID = serverID
    self.priority = priority

class ServersLogic:

  def __init__(self, K):
    self.K = K
    self.tasks = {}
    self.deadServers = []

  def enqueueRequest(self, reqID, serverID, priority):
    if reqID in self.tasks.keys() or serverID in self.deadServers:
      return STATUS_FAILURE
    self.tasks[reqID] = Task(reqID, serverID, priority)
    return STATUS_SUCCESS

  def dequeueRequest(self, reqID):
    if reqID not in self.tasks.keys():
      return STATUS_FAILURE
    del self.tasks[reqID]
    return STATUS_SUCCESS

  def getRequestPriority(self, reqID):
    if reqID not in self.tasks.keys():
      return STATUS_FAILURE
    return self.tasks[reqID].priority

  def getHighestPriorityRequest(self):
    if not self.tasks:
      return STATUS_FAILURE

    maxPriority = self.tasks.values()[0].priority
    maxID = self.tasks.values()[0].reqID
    for t in self.tasks.values():
      if t.priority > maxPriority or (
         t.priority == maxPriority and t.reqID > maxID):
        maxPriority = t.priority
        maxID = t.reqID
    return maxID

  def lowerOldRequestsPriority(self, reqID, delta):
    for t in self.tasks.values():
      if t.reqID <= reqID:
        t.priority -= delta
        if t.priority < 0: t.priority = 0
    return STATUS_SUCCESS

  def getHandlingServerID(self, reqID):
    if reqID not in self.tasks.keys():
      return STATUS_FAILURE
    return self.tasks[reqID].serverID

  def killServer(self, serverID, newServerID):
    if serverID in self.deadServers:
      return STATUS_FAILURE

    for t in self.tasks.values():
      if t.serverID == serverID:
        t.serverID = newServerID

    self.deadServers += [serverID]

    return STATUS_SUCCESS
