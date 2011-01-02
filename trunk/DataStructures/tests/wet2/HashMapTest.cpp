#include "test.h"
#include "LinkedListTest.h"
#include "../HashMap.h"

class IdentityHasher : public Hasher<int> {
public:
  virtual int hashCode(const int& num) {
    return num;
  }
};

/*
 * Tests several invalid operations, making sure that the correct exception is
 * thrown for each one.
 */
bool testIllegalOperations() {
  IdentityHasher* hasher = new IdentityHasher();
  
  // Invalid hasher.
  ASSERT_THROW(HashMapInvalidHasherException, (HashMap<int,int>(NULL)));

  // Invalid capacities.
  ASSERT_THROW(HashMapInvalidSizeException, (HashMap<int,int>(hasher, -1)));
  ASSERT_THROW(HashMapInvalidSizeException, (HashMap<int,int>(hasher, 0)));
  ASSERT_THROW(HashMapInvalidSizeException, (HashMap<int,int>(hasher, -1000)));

  // Invalid load factors.
  ASSERT_THROW(HashMapInvalidSizeException, (HashMap<int,int>(hasher, 100, -1)));
  ASSERT_THROW(HashMapInvalidSizeException, (HashMap<int,int>(hasher, 100, 0)));
  ASSERT_THROW(HashMapInvalidSizeException, (HashMap<int,int>(hasher, 100, -1000)));

  // And one good one.
  ASSERT_NO_THROW((HashMap<int,int>(hasher, 100, 4)));

  return true;
}

int expectedCapacity(int size) {
  if (size <= 4)  return  4;
  if (size <= 8)  return  8;
  if (size <= 16) return 16;
  return -1;
}

/*
 * Tests rehashing happens by inserting and deleting elements.
 */
bool testRehashing() {
  HashMap<int,int> map(new IdentityHasher(), 4, 1);

  // check the the value is not found in the map
  for (int i = 0; i < 16; ++i) {
    ASSERT_FALSE(map.exists(i));
  }
  ASSERT_EQUALS(map.size(), 0);
  ASSERT_EQUALS(map.capacity(), 4);

  // reach the hash map's full capacity
  for (int i = 0; i < 16; ++i) {
    ASSERT_FALSE(map.insert(i, i*10));
    ASSERT_TRUE(map.exists(i));
    ASSERT_EQUALS((*map.get(i)), i*10);
    ASSERT_EQUALS(map.size(), i+1);
    ASSERT_EQUALS(map.capacity(), expectedCapacity(i+1));
  }
  // now remove the elements and check the capacity
  for (int i = 15; i >= 0; --i) {
    ASSERT_TRUE(map.remove(i));
    ASSERT_FALSE(map.exists(i));
    ASSERT_EQUALS((map.get(i)), NULL);
    ASSERT_EQUALS(map.size(), i);
    ASSERT_EQUALS(map.capacity(), expectedCapacity(i));
  }

  return true;
}
/*
 * Tests overwrite happens when a key is reinserted into the map.
 */
bool testOverwrite() {
  HashMap<int,int> map(new IdentityHasher());

  // check the the value is not found in the map
  ASSERT_FALSE(map.exists(1));
  ASSERT_EQUALS(map.size(), 0);
  ASSERT_EQUALS(map.capacity(), 16);

  // insert the value for the first time
  ASSERT_FALSE(map.insert(1, 10));
  ASSERT_TRUE(map.exists(1));
  ASSERT_EQUALS((*map.get(1)), 10);
  ASSERT_EQUALS(map.size(),1);
  ASSERT_EQUALS(map.capacity(), 16);

  // overwrite the value
  ASSERT_TRUE(map.insert(1, 20));
  ASSERT_TRUE(map.exists(1));
  ASSERT_EQUALS((*map.get(1)), 20);
  ASSERT_EQUALS(map.size(), 1);
  ASSERT_EQUALS(map.capacity(), 16);

  // remove the value
  ASSERT_TRUE(map.remove(1));
  ASSERT_FALSE(map.exists(1));
  ASSERT_EQUALS(map.get(1), NULL);
  ASSERT_EQUALS(map.size(), 0);
  ASSERT_EQUALS(map.capacity(), 16);

  // reinsert the value
  ASSERT_FALSE(map.insert(1, 30));
  ASSERT_TRUE(map.exists(1));
  ASSERT_EQUALS((*map.get(1)), 30);
  ASSERT_EQUALS(map.size(), 1);
  ASSERT_EQUALS(map.capacity(), 16);

  // overwrite the value again
  ASSERT_TRUE(map.insert(1, 10));
  ASSERT_TRUE(map.exists(1));
  ASSERT_EQUALS((*map.get(1)), 10);
  ASSERT_EQUALS(map.size(), 1);
  ASSERT_EQUALS(map.capacity(), 16);

  return true;
}

int main(int argc, char **argv) {
  testLinkedList();

  RUN_TEST(testIllegalOperations);
  RUN_TEST(testOverwrite);
}
