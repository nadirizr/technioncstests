#ifndef TEST_UTIL_H
#define TEST_UTIL_H

/*
 * This is a simple linked list implementation used for testing.
 */
class LinkedList {
public:
  LinkedList(int value_, LinkedList* next_);
  ~LinkedList();

  int value;
  LinkedList* next;
};

/*
 * This is a simple sorted set implementation used for testing.
 */
class SortedSet {
public:
  SortedSet();
  ~SortedSet();

  LinkedList* getRoot();
  
  /*
   * Adds the number to the set in a sorted order, with no repeats.
   * @return whether the added node was added.
   */
  bool add(int num);

private:
  bool add(int num, LinkedList* curr);

  LinkedList* root;
};

#endif
