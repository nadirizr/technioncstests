#include "LinkedListTest.h"

#include "test.h"
#include "util.h"
#include "../LinkedList.h"

/*
 * Performs a sequential insertion of several elements, followed by their
 * removal.
 */
bool testListInsertionAndRemoval() {
  LinkedList<int> list;

  for (int i = 0; i < 10; ++i) {
    list.insertFirst(i);
    ASSERT_EQUALS(list.size(), i+1);
  }
  int j = 9;
  for (LinkedList<int>::Iterator it = list.begin(); it != list.end(); ++it) {
    ASSERT_EQUALS((*it), j);
    --j;
  }
  for (int i = 9; i >= 0; --i) {
    list.remove(i);
    ASSERT_EQUALS(list.size(), i);
  }
  ASSERT_EQUALS(list.begin(), list.end());

  return true;
}

bool testInsertingSameElementAndRemoving() {
  LinkedList<int> list;

  for (int i = 0; i < 10; ++i) {
    list.insertFirst(100);
    ASSERT_EQUALS(list.size(), i+1);
  }
  for (LinkedList<int>::Iterator it = list.begin(); it != list.end(); ++it) {
    ASSERT_EQUALS((*it), 100);
  }
  for (int i = 0; i < 10; ++i) {
    list.remove(100);
    ASSERT_EQUALS(list.size(), 9-i);
  }
  ASSERT_EQUALS(list.begin(), list.end());

  return true;
}

bool testLinkedList() {
  RUN_TEST(testListInsertionAndRemoval);
  RUN_TEST(testInsertingSameElementAndRemoving);

  return true;
}
