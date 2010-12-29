#include <stdlib.h>

#include "test.h"
#include "util.h"
#include "../AVLTree.h"

const int items_in_random_test = 100;
const int random_tests = 500;

/*
 * Performs a general rotation test, in which it inserts the given number into
 * a new tree, and compares an in-order walk over the tree to the sorted
 * numbers and their respective expected heights in the result tree.
 * @param numbers the array of numbers to insert (size=n)
 * @param sorted the array of sorted numbers to compare to (size=n)
 * @param heights the array of heights to compare result to (size=n)
 * @param n the size of the arrays
 * @return true if the test succeeded, false otherwise
 */
bool testRotation(const AVLTree<int>& tree,
                  int *sorted,  int *heights,  int n) {
  // go over the tree and check the heights
  AVLTree<int>::Iterator iter = tree.begin();
  for (int i = 0; i < n; ++i) {
    ASSERT_TRUE(iter != tree.end());
    ASSERT_EQUALS(sorted[i], iter.value());
    ASSERT_EQUALS(heights[i], iter.height());
    ++iter;
  }
  // check that there are no more elements in the tree
  ASSERT_TRUE(iter == tree.end());
  
  return true;
}

/*
 * Verify that the tree looks like:
 *     4
 *    / \
 *   2   5
 *  / \
 * 1   3
 *
 * By forcing 2 LL rotations to occur.
 */
bool testInsertRotationLL() {
  AVLTree<int> tree;
  ASSERT_TRUE(tree.insert(5));
  ASSERT_TRUE(tree.insert(4));
  ASSERT_TRUE(tree.insert(3));
  ASSERT_TRUE(tree.insert(2));
  ASSERT_TRUE(tree.insert(1));

  int sorted[]  = {1,2,3,4,5};
  int heights[] = {0,1,0,2,0};
  return testRotation(tree, sorted, heights, 5);
}

/*
 * Verify that the tree looks like:
 *
 *     4
 *    / \
 *   2   5
 *  / \
 * 1   3
 *
 * By forcing 2 LR rotations to occur.
 */
bool testInsertRotationLR() {
  AVLTree<int> tree;
  ASSERT_TRUE(tree.insert(5));
  ASSERT_TRUE(tree.insert(3));
  ASSERT_TRUE(tree.insert(4));
  ASSERT_TRUE(tree.insert(1));
  ASSERT_TRUE(tree.insert(2));

  int sorted[]  = {1,2,3,4,5};
  int heights[] = {0,1,0,2,0};
  return testRotation(tree, sorted, heights, 5);
}

/*
 * Verify that the tree looks like:
 *
 *      2
 *     / \
 *    1   4
 *       / \
 *      3   5
 *
 * By forcing 2 RR rotations to occur.
 */
bool testInsertRotationRR() {
  AVLTree<int> tree;
  ASSERT_TRUE(tree.insert(1));
  ASSERT_TRUE(tree.insert(2));
  ASSERT_TRUE(tree.insert(3));
  ASSERT_TRUE(tree.insert(4));
  ASSERT_TRUE(tree.insert(5));

  int sorted[]  = {1,2,3,4,5};
  int heights[] = {0,2,0,1,0};
  return testRotation(tree, sorted, heights, 5);
}

/*
 * Verify that the tree looks like:
 *
 *      2
 *     / \
 *    1   4
 *       / \
 *      3   5
 *
 * By forcing 2 RL rotations to occur.
 */
bool testInsertRotationRL() {
  AVLTree<int> tree;
  ASSERT_TRUE(tree.insert(1));
  ASSERT_TRUE(tree.insert(3));
  ASSERT_TRUE(tree.insert(2));
  ASSERT_TRUE(tree.insert(5));
  ASSERT_TRUE(tree.insert(4));

  int sorted[]  = {1,2,3,4,5};
  int heights[] = {0,2,0,1,0};
  return testRotation(tree, sorted, heights, 5);
}

/*
 * Verify that the tree looks like:
 * insert 5,4,3,2,1:
 *     4
 *    / \
 *   2   5
 *  / \
 * 1   3
 *
 * remove 5:
 *     4        2
 *    /   LL   / \
 *   2    =>  1   4
 *  / \          /
 * 1   3        3
 *
 * remove 1:
 *  2          3
 *   \   RL   / \
 *    4  =>  2   4
 *   /
 *  3
 *
 * remove 4, and insert 5,4,6:
 *   3
 *  / \
 * 2   5
 *    / \
 *   4   6
 *
 * remove 2:
 *   3           5
 *    \    RR   / \
 *     5   =>  3   6
 *    / \       \
 *   4   6       4
 *
 * remove 6:
 *   5        4
 *  /   LR   / \
 * 3    =>  3   5
 *  \
 *   4
 */
bool testRemoveRotations() {
  AVLTree<int> tree;
  ASSERT_TRUE(tree.insert(5));
  ASSERT_TRUE(tree.insert(4));
  ASSERT_TRUE(tree.insert(3));
  ASSERT_TRUE(tree.insert(2));
  ASSERT_TRUE(tree.insert(1));
  int sorted[]  = {1,2,3,4,5};
  int heights[] = {0,1,0,2,0};
  ASSERT_TRUE(testRotation(tree, sorted, heights, 5));
  
  ASSERT_TRUE(tree.remove(5));
  int sorted2[]  = {1,2,3,4};
  int heights2[] = {0,2,0,1};
  ASSERT_TRUE(testRotation(tree, sorted2, heights2, 4));
  
  ASSERT_TRUE(tree.remove(1));
  int sorted3[]  = {2,3,4};
  int heights3[] = {0,1,0};
  ASSERT_TRUE(testRotation(tree, sorted3, heights3, 3));
  
  ASSERT_TRUE(tree.remove(4));
  ASSERT_TRUE(tree.insert(5));
  ASSERT_TRUE(tree.insert(4));
  ASSERT_TRUE(tree.insert(6));
  int sorted4[]  = {2,3,4,5,6};
  int heights4[] = {0,2,0,1,0};
  ASSERT_TRUE(testRotation(tree, sorted4, heights4, 5));

  ASSERT_TRUE(tree.remove(2));
  int sorted5[]  = {3,4,5,6};
  int heights5[] = {1,0,2,0};
  ASSERT_TRUE(testRotation(tree, sorted5, heights5, 4));

  ASSERT_TRUE(tree.remove(6));
  int sorted6[]  = {3,4,5};
  int heights6[] = {0,1,0};
  ASSERT_TRUE(testRotation(tree, sorted6, heights6, 3));

  return true;
}

/*
 * Taken from an example during the lecture.
 * After initial insertion of 5,2,8,1,4,6,10,3,7,9,12,11 the Tree should look like that:
 *     ___5___
 *    /       \
 *   2       __8__
 *  / \     /     \
 * 1   4   6      10
 *    /      \    /  \
 *   3        7  9   12
 *                   /
 *                  11
 *
 * After removing the 1, we should get a double rotation that will make
 * the tree look like:
 * First rotation RL:
 *     ___5___
 *    /       \
 *   3       __8__
 *  / \     /     \
 * 2   4   6      10
 *          \    /  \
 *           7  9   12
 *                  /
 *                 11
 * and the second rotation, RR:
 *       ___8___
 *      /       \
 *     5        10
 *    / \      /  \
 *   3   6    9   12
 *  / \   \       /
 * 2   4   7     11
 */
bool testRemoveWithDoubleRotation() {
  AVLTree<int> tree;
  ASSERT_TRUE(tree.insert(5));
  ASSERT_TRUE(tree.insert(2));
  ASSERT_TRUE(tree.insert(8));
  ASSERT_TRUE(tree.insert(1));
  ASSERT_TRUE(tree.insert(4));
  ASSERT_TRUE(tree.insert(6));
  ASSERT_TRUE(tree.insert(10));
  ASSERT_TRUE(tree.insert(3));
  ASSERT_TRUE(tree.insert(7));
  ASSERT_TRUE(tree.insert(9));
  ASSERT_TRUE(tree.insert(12));
  ASSERT_TRUE(tree.insert(11));
  int sorted[]  = {1,2,3,4,5,6,7,8,9,10,11,12};
  int heights[] = {0,2,0,1,4,1,0,3,0, 2, 0, 1};
  ASSERT_TRUE(testRotation(tree, sorted, heights, 12));

  ASSERT_TRUE(tree.remove(1));
  int sorted2[]  = {2,3,4,5,6,7,8,9,10,11,12};
  int heights2[] = {0,1,0,2,1,0,3,0, 2, 0, 1};
  ASSERT_TRUE(testRotation(tree, sorted2, heights2, 11));

  return true;
}

bool testRemoveRoot() {
  AVLTree<int> tree;

  ASSERT_TRUE(tree.insert(1));
  ASSERT_TRUE(tree.insert(2));
  ASSERT_TRUE(tree.insert(3));

  AVLTree<int>::Iterator i = tree.begin();
  ASSERT_EQUALS(*i, 1);
  ASSERT_EQUALS(i.height(), 0);
  ++i;
  ASSERT_EQUALS(*i, 2);
  ASSERT_EQUALS(i.height(), 1);
  ++i;
  ASSERT_EQUALS(*i, 3);
  ASSERT_EQUALS(i.height(), 0);
  ++i;
  ASSERT_EQUALS(i, tree.end());

  ASSERT_EQUALS(*tree.find(2), 2);
  ASSERT_TRUE(tree.remove(2));

  i = tree.begin();
  ASSERT_EQUALS(*i, 1);
  ASSERT_EQUALS(i.height(), 0);
  ++i;
  ASSERT_EQUALS(*i, 3);
  ASSERT_EQUALS(i.height(), 1);
  ++i;
  ASSERT_EQUALS(i, tree.end());

  return true;
}

bool testTreeBalance3Bug() {
  AVLTree<int> tree;

  ASSERT_TRUE(tree.insert(74));
  ASSERT_TRUE(tree.insert(86));
  ASSERT_TRUE(tree.insert(37));
  ASSERT_TRUE(tree.insert(89));
  ASSERT_FALSE(tree.remove(43));
  ASSERT_TRUE(tree.insert(68));
  ASSERT_TRUE(tree.insert(48));

  int sorted[]  = {37,48,68,74,86,89};
  int heights[] = {0,1,0,2,1,0};
  ASSERT_TRUE(testRotation(tree, sorted, heights, 6));

  ASSERT_TRUE(tree.insert(71));
  ASSERT_FALSE(tree.remove(43));
  ASSERT_TRUE(tree.remove(86));

  int sorted2[]  = {37,48,68,71,74,89};
  int heights2[] = {0,1,2,0,1,0};
  ASSERT_TRUE(testRotation(tree, sorted2, heights2, 6));

  ASSERT_FALSE(tree.remove(18));
  ASSERT_FALSE(tree.remove(92));
  ASSERT_TRUE(tree.insert(9));

  int sorted3[]  = {9,37,48,68,71,74,89};
  int heights3[] = {0,1,0,2,0,1,0};
  ASSERT_TRUE(testRotation(tree, sorted3, heights3, 7));

  ASSERT_FALSE(tree.remove(50));
  ASSERT_FALSE(tree.remove(56));
  ASSERT_TRUE(tree.insert(67));
  ASSERT_TRUE(tree.insert(42));
  ASSERT_FALSE(tree.remove(19));
  ASSERT_TRUE(tree.insert(73));

  int sorted4[]  = {9,37,42,48,67,68,71,73,74,89};
  int heights4[] = {0,2,0,1,0,3,1,0,2,0};
  ASSERT_TRUE(testRotation(tree, sorted4, heights4, 10));

  return true;
}

bool testRandomInsertions() {
  AVLTree<int> tree;

  SortedSet set;
  int num;
  for (int i = 0; i < items_in_random_test; ++i) {
    num = rand() % 1000;
    bool added = set.add(num); 
    ASSERT_EQUALS(added, tree.insert(num));
  }
  
  LinkedList* curr = set.getRoot()->next;
  for (AVLTree<int>::Iterator i = tree.begin(); i != tree.end(); ++i, curr = curr->next) {
    ASSERT_TRUE(curr);
    ASSERT_EQUALS(curr->value, i.value());

    int balance = i.balance();
    ASSERT_TRUE(balance <= 1 && balance >= -1);
  }
  ASSERT_EQUALS(NULL, curr);
  return true;
}

bool testPath(const AVLTree<int> &tree, AVLTree<int>::Iterator pathIter, const int *path,  int n) {
  // use the iterator and check the path
  for (int i = 0; i < n; ++i) {
    ASSERT_TRUE(pathIter != tree.end());
    ASSERT_EQUALS(path[i], pathIter.value());
    ++pathIter;
  }
  // check that there are no more elements in the tree
  ASSERT_TRUE(pathIter == tree.end());
  
  return true;
}

/**
 *       ___8___
 *      /       \
 *     5        10
 *    / \      /  \
 *   3   6    9   12
 *  / \   \       /
 * 2   4   7     11
 */
bool testPathIterator() {
  AVLTree<int> tree;
  ASSERT_TRUE(tree.insert(5));
  ASSERT_TRUE(tree.insert(2));
  ASSERT_TRUE(tree.insert(8));
  ASSERT_TRUE(tree.insert(1));
  ASSERT_TRUE(tree.insert(4));
  ASSERT_TRUE(tree.insert(6));
  ASSERT_TRUE(tree.insert(10));
  ASSERT_TRUE(tree.insert(3));
  ASSERT_TRUE(tree.insert(7));
  ASSERT_TRUE(tree.insert(9));
  ASSERT_TRUE(tree.insert(12));
  ASSERT_TRUE(tree.insert(11));
  ASSERT_TRUE(tree.remove(1));
  int pathFor2[]  = {8, 5, 3, 2};
  int pathFor4[]  = {8, 5, 3, 4};
  int pathFor7[]  = {8, 5, 6, 7};
  int pathFor9[]  = {8,10, 9   };
  int pathFor11[] = {8,10,12,11};

  int i = 2;
  ASSERT_TRUE(testPath(tree, tree.findPath(&i), pathFor2,  4)); i++;
  ASSERT_TRUE(testPath(tree, tree.findPath(&i), pathFor2,  3)); i++;
  ASSERT_TRUE(testPath(tree, tree.findPath(&i), pathFor4,  4)); i++;
  ASSERT_TRUE(testPath(tree, tree.findPath(&i), pathFor4,  2)); i++;
  ASSERT_TRUE(testPath(tree, tree.findPath(&i), pathFor7,  3)); i++;
  ASSERT_TRUE(testPath(tree, tree.findPath(&i), pathFor7,  4)); i++;
  ASSERT_TRUE(testPath(tree, tree.findPath(&i), pathFor7,  1)); i++;
  ASSERT_TRUE(testPath(tree, tree.findPath(&i), pathFor9,  3)); i++;
  ASSERT_TRUE(testPath(tree, tree.findPath(&i), pathFor9,  2)); i++;
  ASSERT_TRUE(testPath(tree, tree.findPath(&i), pathFor11, 4)); i++;
  ASSERT_TRUE(testPath(tree, tree.findPath(&i), pathFor11, 3)); i++;

  return true;
}

int main(int argc, char **argv) {
	// initialize random number generator
	srand( time(NULL) );

  RUN_TEST(testInsertRotationLL);
  RUN_TEST(testInsertRotationLR);
  RUN_TEST(testInsertRotationRR);
  RUN_TEST(testInsertRotationRL);
  RUN_TEST(testRemoveRotations);
  RUN_TEST(testRemoveWithDoubleRotation);
  RUN_TEST(testRemoveRoot);
  RUN_TEST(testTreeBalance3Bug);
  RUN_TEST_N_TIMES(testRandomInsertions,random_tests);
  RUN_TEST(testPathIterator);
}
