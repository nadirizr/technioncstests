#include "util.h"

#include <stdlib.h>

#include "test.h"

// --- LinkedList Methods ---

LinkedList::LinkedList(int value_, LinkedList* next_) :
  value(value_), next(next_) {}

LinkedList::~LinkedList() {
  if (next) {
    delete next;
  }
}

// --- SortedSet Methods ---

SortedSet::SortedSet() :
  root(new LinkedList(-1, NULL)) {}

SortedSet::~SortedSet() {
  if (root) {
    delete root;
  }
}

LinkedList* SortedSet::getRoot() {
  return root;
}

bool SortedSet::add(int num) {
  if (num < 0) {
    FAIL("No negative numbers!!");
  }
  return add(num, root);
}

bool SortedSet::add(int num, LinkedList* curr) {
  if (curr->next == NULL) {
    curr->next = new LinkedList(num, NULL);
    return true;
  }
  if (curr->next->value == num) {
    return false;
  }
  if (curr->next->value > num) {
    curr->next = new LinkedList(num, curr->next);
    return true;
  }
  // curr->next->value > num
  return add(num, curr->next);
}
