#include <iostream>
#include <stdlib.h>

#include "test.h"
#include "util.h"
#include "../AVLTree.h"

const int items_in_random_test = 100;
const int random_tests = 500;

struct IdWithDelta {
  int id;
  int delta;

  IdWithDelta(int id_, int delta_) : id(id_), delta(delta_) {}
  bool operator<(const IdWithDelta& o) const { return id < o.id; }
};

class IdWithDeltaUpdater : public NodeUpdater<IdWithDelta> {
public:
  IdWithDeltaUpdater(AVLTree<IdWithDelta>* tree_) : tree(tree_) {}

  virtual void updateOnLeftRotation(
      Iterator father, Iterator left, Iterator right) {
    father->delta += right->delta;
  }

  virtual void updateOnRightRotation(
      Iterator father, Iterator left, Iterator right) {
    left->delta -= father->delta;
  }

  virtual void updateOnSubstitute(
      Iterator to_remove, Iterator to_substitute) {
    // If this was a leaf, do nothing.
    if (!to_substitute) {
      return;
    }

    // If this was a parent with a single left child, add the parent's delta to
    // the child delta.
    if ((*to_substitute) < (*to_remove)) {
      to_substitute->delta += to_remove->delta;
      return;
    }

    // If the node to remove has no left child, then the substitute node should
    // just take the place of the node to remove, with no change required.
    if (!to_remove.canMoveLeft()) {
      return;
    }

    // If the substitute is bigger than the node to remove, then just replace
    // the delta.
    int temp = to_substitute->delta;
    to_substitute->delta = to_remove->delta;
    to_remove->delta = temp;
  }

private:
  AVLTree<IdWithDelta>* tree;
};

/*
 * Performs a general rotation test, in which it inserts the given number into
 * a new tree, and compares an in-order walk over the tree to the sorted
 * numbers and their respective expected heights in the result tree.
 * @param numbers the array of numbers to insert (size=n)
 * @param sorted the array of sorted numbers to compare to (size=n)
 * @param heights the array of heights to compare result to (size=n)
 * @param deltas the array of expected deltas to compare to (size=n)
 * @param n the size of the arrays
 * @param printDebugInfo whether to print the info of each node
 * @return true if the test succeeded, false otherwise
 */
bool testUpdateRotation(const AVLTree<IdWithDelta>& tree,
                  int *sorted,  int *heights, int *deltas, int n,
                  int printDebugInfo=false) {
  // go over the tree and check the heights
  AVLTree<IdWithDelta>::Iterator iter = tree.begin();
  for (int i = 0; i < n; ++i) {
    ASSERT_TRUE(iter != tree.end());
    if (printDebugInfo) {
      std::cout << "node[" << i << "]: id=" << iter->id << "; height="
                << iter.height() << "; delta=" << iter->delta << std::endl;
    }
    ASSERT_EQUALS(sorted[i], iter->id);
    ASSERT_EQUALS(heights[i], iter.height());
    ASSERT_EQUALS(deltas[i], iter->delta);
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
bool testUpdateInsertRotationLL() {
  AVLTree<IdWithDelta> tree;
  IdWithDeltaUpdater updater(&tree);
  tree.setUpdater(&updater);
  ASSERT_TRUE(tree.insert(IdWithDelta(5, 5)));
  ASSERT_TRUE(tree.insert(IdWithDelta(4, 4)));
  ASSERT_TRUE(tree.insert(IdWithDelta(3, 3)));
  ASSERT_TRUE(tree.insert(IdWithDelta(2, 2)));
  ASSERT_TRUE(tree.insert(IdWithDelta(1, 1)));

  int sorted[]  = { 1, 2, 3,4,5};
  int heights[] = { 0, 1, 0,2,0};
  int deltas[]  = { 1, 5, 3,9,5}; 
  return testUpdateRotation(tree, sorted, heights, deltas, 5);
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
bool testUpdateInsertRotationLR() {
  AVLTree<IdWithDelta> tree;
  IdWithDeltaUpdater updater(&tree);
  tree.setUpdater(&updater);
  ASSERT_TRUE(tree.insert(IdWithDelta(5, 5)));
  ASSERT_TRUE(tree.insert(IdWithDelta(3, 3)));
  ASSERT_TRUE(tree.insert(IdWithDelta(4, 4)));
  ASSERT_TRUE(tree.insert(IdWithDelta(1, 1)));
  ASSERT_TRUE(tree.insert(IdWithDelta(2, 2)));

  int sorted[]  = { 1,2, 3,4,5};
  int heights[] = { 0,1, 0,2,0};
  int deltas[]  = {-1,1,-1,9,5}; 
  return testUpdateRotation(tree, sorted, heights, deltas, 5);
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
bool testUpdateInsertRotationRR() {
  AVLTree<IdWithDelta> tree;
  IdWithDeltaUpdater updater(&tree);
  tree.setUpdater(&updater);
  ASSERT_TRUE(tree.insert(IdWithDelta(1, 1)));
  ASSERT_TRUE(tree.insert(IdWithDelta(2, 2)));
  ASSERT_TRUE(tree.insert(IdWithDelta(3, 3)));
  ASSERT_TRUE(tree.insert(IdWithDelta(4, 4)));
  ASSERT_TRUE(tree.insert(IdWithDelta(5, 5)));

  int sorted[]  = { 1,2, 3,4,5};
  int heights[] = { 0,2, 0,1,0};
  int deltas[]  = {-1,2,-1,4,5};
  return testUpdateRotation(tree, sorted, heights, deltas, 5);
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
bool testUpdateInsertRotationRL() {
  AVLTree<IdWithDelta> tree;
  IdWithDeltaUpdater updater(&tree);
  tree.setUpdater(&updater);
  ASSERT_TRUE(tree.insert(IdWithDelta(1, 1)));
  ASSERT_TRUE(tree.insert(IdWithDelta(3, 3)));
  ASSERT_TRUE(tree.insert(IdWithDelta(2, 2)));
  ASSERT_TRUE(tree.insert(IdWithDelta(5, 5)));
  ASSERT_TRUE(tree.insert(IdWithDelta(4, 4)));

  int sorted[]  = { 1,2, 3,4,5};
  int heights[] = { 0,2, 0,1,0};
  int deltas[]  = {-4,5,-6,9,5};
  return testUpdateRotation(tree, sorted, heights, deltas, 5);
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
bool testUpdateRemoveRotations() {
  AVLTree<IdWithDelta> tree;
  IdWithDeltaUpdater updater(&tree);
  tree.setUpdater(&updater);
  ASSERT_TRUE(tree.insert(IdWithDelta(5, 5)));
  ASSERT_TRUE(tree.insert(IdWithDelta(4, 4)));
  ASSERT_TRUE(tree.insert(IdWithDelta(3, 3)));
  ASSERT_TRUE(tree.insert(IdWithDelta(2, 2)));
  ASSERT_TRUE(tree.insert(IdWithDelta(1, 1)));
  int sorted[]  = {1,2,3,4,5};
  int heights[] = {0,1,0,2,0};
  int deltas[]  = {1,5,3,9,5}; 
  ASSERT_TRUE(testUpdateRotation(tree, sorted, heights, deltas, 5));
  
  ASSERT_TRUE(tree.remove(IdWithDelta(5, 0)));
  int sorted2[]  = {1, 2,3,4};
  int heights2[] = {0, 2,0,1};
  int deltas2[]  = {1,14,3,9};
  ASSERT_TRUE(testUpdateRotation(tree, sorted2, heights2, deltas2, 4));
  
  ASSERT_TRUE(tree.remove(IdWithDelta(1, 0)));
  int sorted3[]  = {2, 3,4};
  int heights3[] = {0, 1,0};
  int deltas3[]  = {2,12,9};
  ASSERT_TRUE(testUpdateRotation(tree, sorted3, heights3, deltas3, 3));
  
  ASSERT_TRUE(tree.remove(IdWithDelta(4, 0)));
  ASSERT_TRUE(tree.insert(IdWithDelta(5, 5)));
  ASSERT_TRUE(tree.insert(IdWithDelta(4, 4)));
  ASSERT_TRUE(tree.insert(IdWithDelta(6, 6)));
  int sorted4[]  = {2, 3,4,5,6};
  int heights4[] = {0, 2,0,1,0};
  int deltas4[]  = {2,12,4,5,6};
  ASSERT_TRUE(testUpdateRotation(tree, sorted4, heights4, deltas4, 5));

  ASSERT_TRUE(tree.remove(IdWithDelta(2, 0)));
  int sorted5[]  = {3,4,5,6};
  int heights5[] = {1,0,2,0};
  int deltas5[]  = {7,4,5,6};
  ASSERT_TRUE(testUpdateRotation(tree, sorted5, heights5, deltas5, 4));

  ASSERT_TRUE(tree.remove(IdWithDelta(6, 0)));
  int sorted6[]  = {3,4,5};
  int heights6[] = {0,1,0};
  int deltas6[]  = {3,9,5};
  ASSERT_TRUE(testUpdateRotation(tree, sorted6, heights6, deltas6, 3));

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
bool testUpdateRemoveWithDoubleRotation() {
  AVLTree<IdWithDelta> tree;
  IdWithDeltaUpdater updater(&tree);
  tree.setUpdater(&updater);
  ASSERT_TRUE(tree.insert(IdWithDelta(5, 5)));
  ASSERT_TRUE(tree.insert(IdWithDelta(2, 2)));
  ASSERT_TRUE(tree.insert(IdWithDelta(8, 8)));
  ASSERT_TRUE(tree.insert(IdWithDelta(1, 1)));
  ASSERT_TRUE(tree.insert(IdWithDelta(4, 4)));
  ASSERT_TRUE(tree.insert(IdWithDelta(6, 6)));
  ASSERT_TRUE(tree.insert(IdWithDelta(10, 10)));
  ASSERT_TRUE(tree.insert(IdWithDelta(3, 3)));
  ASSERT_TRUE(tree.insert(IdWithDelta(7, 7)));
  ASSERT_TRUE(tree.insert(IdWithDelta(9, 9)));
  ASSERT_TRUE(tree.insert(IdWithDelta(12, 12)));
  ASSERT_TRUE(tree.insert(IdWithDelta(11, 11)));
  int sorted[]  = {1,2,3,4,5,6,7,8,9,10,11,12};
  int heights[] = {0,2,0,1,4,1,0,3,0, 2, 0, 1};
  int deltas[]  = {1,2,3,4,5,6,7,8,9,10,11,12};
  ASSERT_TRUE(testUpdateRotation(tree, sorted, heights, deltas, 12));

  ASSERT_TRUE(tree.remove(IdWithDelta(1, 0)));
  int sorted2[]  = { 2,3,4, 5,6,7,8,9,10,11,12};
  int heights2[] = { 0,1,0, 2,1,0,3,0, 2, 0, 1};
  int deltas2[]  = {-5,7,4,-3,6,7,8,9,10,11,12};
  ASSERT_TRUE(testUpdateRotation(tree, sorted2, heights2, deltas2, 11));

  return true;
}

/*
 * Verify that the tree looks like:
 * insert 4,2,7,1,3,5,8,6:
 *       4
 *    /     \
 *   2       7
 *  / \     / \
 * 1   3   5   8
 *          \
 *           6
 *
 * remove 4:
 *       5
 *    /     \
 *   2       7
 *  / \     / \
 * 1   3   6   8
 *
 * remove 7:
 *       5
 *    /     \
 *   2       8
 *  / \     /
 * 1   3   6
 */
bool testUpdateRemoveRootAndSwapSuccessor() {
  AVLTree<IdWithDelta> tree;
  IdWithDeltaUpdater updater(&tree);
  tree.setUpdater(&updater);
  ASSERT_TRUE(tree.insert(IdWithDelta(4, 4)));
  ASSERT_TRUE(tree.insert(IdWithDelta(2, 2)));
  ASSERT_TRUE(tree.insert(IdWithDelta(7, 7)));
  ASSERT_TRUE(tree.insert(IdWithDelta(1, 1)));
  ASSERT_TRUE(tree.insert(IdWithDelta(3, 3)));
  ASSERT_TRUE(tree.insert(IdWithDelta(5, 5)));
  ASSERT_TRUE(tree.insert(IdWithDelta(8, 8)));
  ASSERT_TRUE(tree.insert(IdWithDelta(6, 6)));
  int sorted[]  = {1,2,3,4,5,6,7,8};
  int heights[] = {0,1,0,3,1,0,2,0};
  int deltas[]  = {1,2,3,4,5,6,7,8};
  ASSERT_TRUE(testUpdateRotation(tree, sorted, heights, deltas, 8));

  ASSERT_TRUE(tree.remove(IdWithDelta(4, 0)));
  int sorted2[]  = {1,2,3,5,6,7,8};
  int heights2[] = {0,1,0,2,0,1,0};
  int deltas2[]  = {1,2,3,4,6,7,8};
  ASSERT_TRUE(testUpdateRotation(tree, sorted2, heights2, deltas2, 7));

  ASSERT_TRUE(tree.remove(IdWithDelta(7, 0)));
  int sorted3[]  = {1,2,3,5,6,8};
  int heights3[] = {0,1,0,2,0,1};
  int deltas3[]  = {1,2,3,4,6,7};
  ASSERT_TRUE(testUpdateRotation(tree, sorted3, heights3, deltas3, 6));

  return true;
}

/*
 * Verify that the tree looks like:
 * insert 2,3:
 *    2
 *     \
 *      3
 *
 * remove 2:
 *    3
 *
 * insert 1,4:
 *    3
 *   / \
 *  1   4
 *
 * remove 3:
 *    4
 *   /
 *  1
 */
bool testUpdateRemoveRootWithRightChildAndWithout() {
  AVLTree<IdWithDelta> tree;
  IdWithDeltaUpdater updater(&tree);
  tree.setUpdater(&updater);
  ASSERT_TRUE(tree.insert(IdWithDelta(2, 2)));
  ASSERT_TRUE(tree.insert(IdWithDelta(3, 3)));
  int sorted[]  = {2,3};
  int heights[] = {1,0};
  int deltas[]  = {2,3};
  ASSERT_TRUE(testUpdateRotation(tree, sorted, heights, deltas, 2));

  ASSERT_TRUE(tree.remove(IdWithDelta(2, 0)));
  int sorted2[]  = {3};
  int heights2[] = {0};
  int deltas2[]  = {3};
  ASSERT_TRUE(testUpdateRotation(tree, sorted2, heights2, deltas2, 1));

  ASSERT_TRUE(tree.insert(IdWithDelta(1, 1)));
  ASSERT_TRUE(tree.insert(IdWithDelta(4, 4)));
  int sorted3[]  = {1,3,4};
  int heights3[] = {0,1,0};
  int deltas3[]  = {1,3,4};
  ASSERT_TRUE(testUpdateRotation(tree, sorted3, heights3, deltas3, 3));

  ASSERT_TRUE(tree.remove(IdWithDelta(3, 0)));
  int sorted4[]  = {1,4};
  int heights4[] = {0,1};
  int deltas4[]  = {1,3};
  ASSERT_TRUE(testUpdateRotation(tree, sorted4, heights4, deltas4, 2));

  return true;
}

bool runAVLTreeUpdateTests() {
	// initialize random number generator
	srand( time(NULL) );

  RUN_TEST(testUpdateInsertRotationLL);
  RUN_TEST(testUpdateInsertRotationLR);
  RUN_TEST(testUpdateInsertRotationRR);
  RUN_TEST(testUpdateInsertRotationRL);
  RUN_TEST(testUpdateRemoveRotations);
  RUN_TEST(testUpdateRemoveWithDoubleRotation);
  RUN_TEST(testUpdateRemoveRootAndSwapSuccessor);
  RUN_TEST(testUpdateRemoveRootWithRightChildAndWithout);

  return true;
}
