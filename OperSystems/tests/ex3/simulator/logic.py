from sys import stderr

ERROR_STOP = -10000

class Message:
  def __init__(self, origin, destination, message, is_sync, is_urgent, is_broadcast):
    self.origin = origin
    self.destination = destination
    self.message = message
    self.is_sync = is_sync
    self.is_urgent = is_urgent
    self.is_broadcast = is_broadcast

class ThreadsLogic:

  def __init__(self, num_threads):
    self.num_threads = num_threads
    self.messages_for_thread = {}
    for t in range(1, self.num_threads + 1):
      self.messages_for_thread[t] = []

    self.barrier_counter = -1
    self.waiting_at_barrier = []

    self.waiting_broadcasters = {}
    self.finished_broadcasters = []

  def close(self, thread_index):
    self.barrier_counter = -1
    self.waiting_at_barrier = []
    self.messages_for_thread = {}
    self.num_threads = 0
    self.waiting_broadcasters = {}
    self.finished_broadcasters = []

  def createBarrier(self, thread_index, n):
    if n < 1:
      return -1

    if thread_index in self.waiting_at_barrier or \
       thread_index in self.waiting_broadcasters:
      return ERROR_STOP

    self.barrier_counter = n
    self.waiting_at_barrier = []
    return 0
  
  def destroyBarrier(self, thread_index):
    if thread_index in self.waiting_at_barrier or \
       thread_index in self.waiting_broadcasters:
      return ERROR_STOP

    self.barrier_counter = -1
    self.waiting_at_barrier = []

  def barrier(self, thread_index):
    if self.barrier_counter < 0:
      return -1

    if thread_index in self.waiting_at_barrier or \
       thread_index in self.waiting_broadcasters:
      return ERROR_STOP

    self.waiting_at_barrier.append(thread_index)

    if len(self.waiting_at_barrier) == self.barrier_counter:
      passed = self.waiting_at_barrier[:]
      self.waiting_at_barrier = []
      self.barrier_counter = -1
      return passed
    return 0

  def send(self, thread_index, to, is_sync, is_urgent, message):
    if to not in self.messages_for_thread:
      return -1

    if thread_index in self.waiting_at_barrier or \
       thread_index in self.waiting_broadcasters:
      return ERROR_STOP

    message = Message(thread_index, to, message, is_sync, is_urgent, False)
    if is_urgent:
      self.messages_for_thread[to].insert(0, message)
    else:
      self.messages_for_thread[to].append(message)

    if not is_sync:
      return 0
    return 1

  def broadcast(self, thread_index, is_sync, is_urgent, message):
    if thread_index in self.waiting_at_barrier or \
       thread_index in self.waiting_broadcasters:
      return ERROR_STOP

    destinations = []
    for (t, q) in self.messages_for_thread.items():
      if t == thread_index:
        continue

      message_t = Message(thread_index, t, message, is_sync, is_urgent, True)
      if is_urgent:
        q.insert(0, message_t)
      else:
        q.append(message_t)

      destinations.append(t)

    if not is_sync:
      return 0

    self.waiting_broadcasters[thread_index] = destinations
    return 1

  def deliverAllMessages(self):
    delivered = []
    for (t, q) in self.messages_for_thread.items():
      if t in self.waiting_at_barrier:
        continue

      for m in q:
        if m.is_broadcast and m.origin in self.waiting_broadcasters:
          self.waiting_broadcasters[m.origin].remove(m.destination)

          if not self.waiting_broadcasters[m.origin]:
            del self.waiting_broadcasters[m.origin]
            self.finished_broadcasters.append(m.origin)

      delivered += q
      self.messages_for_thread[t] = []

    return delivered

  def finishedBroadcasters(self):
    finished = self.finished_broadcasters[:]
    self.finished_broadcasters = []
    return finished
