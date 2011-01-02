#include "util.h"

#include <stdlib.h>

#include "test.h"

// --- LinkedList Methods ---

TestLinkedList::TestLinkedList(int value_, TestLinkedList* next_) :
  value(value_), next(next_) {}

TestLinkedList::~TestLinkedList() {
  if (next) {
    delete next;
  }
}

// --- SortedSet Methods ---

TestSortedSet::TestSortedSet() :
  root(new TestLinkedList(-1, NULL)) {}

TestSortedSet::~TestSortedSet() {
  if (root) {
    delete root;
  }
}

TestLinkedList* TestSortedSet::getRoot() {
  return root;
}

bool TestSortedSet::add(int num) {
  if (num < 0) {
    FAIL("No negative numbers!!");
  }
  return add(num, root);
}

bool TestSortedSet::add(int num, TestLinkedList* curr) {
  if (curr->next == NULL) {
    curr->next = new TestLinkedList(num, NULL);
    return true;
  }
  if (curr->next->value == num) {
    return false;
  }
  if (curr->next->value > num) {
    curr->next = new TestLinkedList(num, curr->next);
    return true;
  }
  // curr->next->value > num
  return add(num, curr->next);
}
