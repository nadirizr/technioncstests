#ifndef TEST_UTIL_H
#define TEST_UTIL_H

/*
 * This is a simple linked list implementation used for testing.
 */
class TestLinkedList {
public:
  TestLinkedList(int value_, TestLinkedList* next_);
  ~TestLinkedList();

  int value;
  TestLinkedList* next;
};

/*
 * This is a simple sorted set implementation used for testing.
 */
class TestSortedSet {
public:
  TestSortedSet();
  ~TestSortedSet();

  TestLinkedList* getRoot();
  
  /*
   * Adds the number to the set in a sorted order, with no repeats.
   * @return whether the added node was added.
   */
  bool add(int num);

private:
  bool add(int num, TestLinkedList* curr);

  TestLinkedList* root;
};

#endif
