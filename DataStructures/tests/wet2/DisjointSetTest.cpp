#include "test.h"
#include "util.h"
#include "../DisjointSet.h"

/*
 * Tests several invalid operations, making sure that the correct exception is
 * thrown for each one.
 */
bool testIllegalOperations() {
  ASSERT_THROW(DisjointSetInvalidSizeException, DisjointSet(-1));
  ASSERT_THROW(DisjointSetInvalidSizeException, DisjointSet(0));
  ASSERT_THROW(DisjointSetInvalidSizeException, DisjointSet(-1000));

  DisjointSet set(20);

  ASSERT_THROW(DisjointSetInvalidElementException, set.find(-1));
  ASSERT_THROW(DisjointSetInvalidElementException, set.find(20));
  ASSERT_THROW(DisjointSetInvalidElementException, set.find(100));
  ASSERT_NO_THROW(set.find(10));
  ASSERT_NO_THROW(set.find(0));
  ASSERT_NO_THROW(set.find(19));

  ASSERT_THROW(DisjointSetInvalidElementException, set.unite( 1,-1));
  ASSERT_THROW(DisjointSetInvalidElementException, set.unite(-1 ,1));
  ASSERT_THROW(DisjointSetInvalidElementException, set.unite(-1,-1));
  ASSERT_THROW(DisjointSetInvalidElementException, set.unite(19,20));
  ASSERT_THROW(DisjointSetInvalidElementException, set.unite(20,19));
  ASSERT_THROW(DisjointSetInvalidElementException, set.unite(20,20));
  ASSERT_NO_THROW(set.unite(0,1));
  ASSERT_NO_THROW(set.unite(1,2));
  ASSERT_NO_THROW(set.unite(18,19));
  ASSERT_NO_THROW(set.unite(19,19));
  ASSERT_NO_THROW(set.unite(0,0));

  return true;
}

/*
 * Initializes a set of elements, and then unites the elements sequentialy in
 * both directions, checking that the groups are as expected.
 */
bool testUniteAllElementsSequentialy() {
  // First create a set where we unite each of the elements with the one after
  // it sequentialy, so that all elements are eventualy in group 0.
  DisjointSet set1(10);
  for (int i = 0; i < 10; ++i) {
    ASSERT_EQUALS(set1.find(i), i);
  }
  for (int i = 0; i < 9; ++i) {
    ASSERT_EQUALS(set1.unite(i,i+1), 0);
  }
  for (int i = 0; i < 10; ++i) {
    ASSERT_EQUALS(set1.find(i), 0);
  }

  // Next create a set where we unite each of the elements with the one before
  // it sequentialy, so that all elements are eventualy in group 8.
  DisjointSet set2(10);
  for (int i = 0; i < 10; ++i) {
    ASSERT_EQUALS(set2.find(i), i);
  }
  for (int i = 9; i > 0; --i) {
    ASSERT_EQUALS(set2.unite(i-1,i), 8);
  }
  for (int i = 0; i < 10; ++i) {
    ASSERT_EQUALS(set2.find(i), 8);
  }

  return true;
}

/*
 * Initializes a set of elements, and then unites the elements in groups of
 * increasing size as powers of 2. First unites pairs of elements, then unites
 * pairs of pairs, and so on until it unites all of the elements.
 */
bool testUniteAsFullTree() {
  DisjointSet set(64);
  for (int size = 1; size <= 32; size *= 2) {
    for (int index = 0; index < 64; index += 2*size) {
      ASSERT_EQUALS(set.unite(index,index+size), index);
    }
    for (int i = 0; i < 64; ++i) {
      ASSERT_EQUALS(set.find(i), (i - (i % (2*size))));
    }
  }
  for (int i = 0; i < 64; ++i) {
    ASSERT_EQUALS(set.find(i), 0);
  }

  return true;
}

/*
 * Initializes a set of elements, and then unites a few. After that, check that
 * uniting elements in the same group doesn't screw anything up.
 */
bool testUniteAlreadySameGroup() {
  DisjointSet set(10);
  for (int i = 0; i < 10; i += 2) {
    ASSERT_EQUALS(set.unite(i,i+1), i);
  }
  for (int i = 0; i < 10; i += 2) {
    ASSERT_EQUALS(set.unite(i,i+1), i);
  }
  for (int i = 0; i < 10; ++i) {
    ASSERT_EQUALS(set.unite(i,0), 0);
  }
  for (int i = 0; i < 10; ++i) {
    ASSERT_EQUALS(set.unite(i,0), 0);
  }
  for (int i = 0; i < 10; ++i) {
    ASSERT_EQUALS(set.find(i), 0);
  }

  return true;
}

bool checkSetInternalArray(int *expected, DisjointSet& set, int N) {
  // Ugly code meant to get the internal array of the DisjointSet.
  void* void_set = (void*)&set;
  int* elements = (int*)void_set;
  for (int i = 0; i < N; ++i) {
    ASSERT_EQUALS(expected[i], elements[2*i]);
  }

  return true;
}

/*
 * Initializes a set of elements, and then unites a few in a way in which a
 * tree with a few levels is formed. After that performs a find, and verifies
 * that it correctly shrinked the path to the root for all involved elements.
 */
bool testFindShortensPathToRoot() {
  DisjointSet set(8);
  ASSERT_EQUALS(set.unite(1,0), 0);
  ASSERT_EQUALS(set.unite(2,3), 2);
  ASSERT_EQUALS(set.unite(3,1), 0);
  ASSERT_EQUALS(set.unite(4,5), 4);
  ASSERT_EQUALS(set.unite(6,7), 6);
  ASSERT_EQUALS(set.unite(5,7), 4);
  ASSERT_EQUALS(set.unite(5,1), 0);
  int expected[] = { 0, 0, 0, 2, 0, 4, 4, 6 };
  checkSetInternalArray(expected, set, 8);

  ASSERT_EQUALS(set.find(7), 0);
  int expected2[] = { 0, 0, 0, 2, 0, 4, 0, 0 };
  checkSetInternalArray(expected2, set, 8);
  
  ASSERT_EQUALS(set.find(5), 0);
  int expected3[] = { 0, 0, 0, 2, 0, 4, 0, 0 };
  checkSetInternalArray(expected3, set, 8);
  
  ASSERT_EQUALS(set.find(3), 0);
  int expected4[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  checkSetInternalArray(expected4, set, 8);

  return true;
}

int main(int argc, char **argv) {
  RUN_TEST(testIllegalOperations);
  RUN_TEST(testUniteAllElementsSequentialy);
  RUN_TEST(testUniteAsFullTree);
  RUN_TEST(testUniteAlreadySameGroup);
  RUN_TEST(testFindShortensPathToRoot);
}
