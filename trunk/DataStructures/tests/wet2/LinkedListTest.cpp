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

bool testFindingValues() {
  LinkedList<int> list;

  for (int i = 0; i < 10; ++i) {
    list.insertFirst(i);
    ASSERT_EQUALS(list.size(), i+1);
  }
  for (int i = 0; i < 10; ++i) {
    LinkedList<int>::Iterator it = list.find(i);
    ASSERT_EQUALS((*it), i);
  }

  return true;
}

bool testMergingLists() {
  // First merge the standard case.
  LinkedList<int> list1, list2;
  for (int i = 0; i < 10; ++i) {
    list1.insertLast(i);
    list2.insertLast(i+10);
    ASSERT_EQUALS(list1.size(), i+1);
    ASSERT_EQUALS(list2.size(), i+1);
  }
  list1.merge(list2);
  ASSERT_EQUALS(list1.size(), 20);
  ASSERT_EQUALS(list2.size(), 0);
  ASSERT_EQUALS(list2.begin(), list2.end());
  int j = 0;
  for (LinkedList<int>::Iterator it = list1.begin(); it != list1.end(); ++it) {
    ASSERT_EQUALS((*it), j);
    ++j;
  }
  ASSERT_EQUALS(j, 20);

  // Now merge an empty list.
  LinkedList<int> list3;
  list1.merge(list3);
  ASSERT_EQUALS(list1.size(), 20);
  ASSERT_EQUALS(list3.size(), 0);
  j = 0;
  for (LinkedList<int>::Iterator it = list1.begin(); it != list1.end(); ++it) {
    ASSERT_EQUALS((*it), j);
    ++j;
  }
  ASSERT_EQUALS(j, 20);

  // Now merge into an empty list.
  list3.merge(list1);
  ASSERT_EQUALS(list3.size(), 20);
  ASSERT_EQUALS(list1.size(), 0);
  ASSERT_EQUALS(list1.begin(), list1.end());
  j = 0;
  for (LinkedList<int>::Iterator it = list3.begin(); it != list3.end(); ++it) {
    ASSERT_EQUALS((*it), j);
    ++j;
  }
  ASSERT_EQUALS(j, 20);

  return true;
}

bool testLinkedList() {
  RUN_TEST(testListInsertionAndRemoval);
  RUN_TEST(testInsertingSameElementAndRemoving);
  RUN_TEST(testFindingValues);
  RUN_TEST(testMergingLists);

  return true;
}
