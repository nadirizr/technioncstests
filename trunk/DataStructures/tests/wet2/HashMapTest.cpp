#include "test.h"
#include "LinkedListTest.h"
#include "../HashMap.h"

#include <map>
#include <stdlib.h>

const int NUM_ITEMS_IN_RANDOM_TEST = 1000;
const int NUM_RANDOM_TESTS = 500;

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
  ASSERT_THROW(HashMapInvalidCapacityException, (HashMap<int,int>(hasher, -1)));
  ASSERT_THROW(HashMapInvalidCapacityException, (HashMap<int,int>(hasher, 0)));
  ASSERT_THROW(HashMapInvalidCapacityException, (HashMap<int,int>(hasher, -1000)));

  // Invalid load factors.
  ASSERT_THROW(HashMapInvalidLoadFactorException, (HashMap<int,int>(hasher, 100, -1)));
  ASSERT_THROW(HashMapInvalidLoadFactorException, (HashMap<int,int>(hasher, 100, 0)));
  ASSERT_THROW(HashMapInvalidLoadFactorException, (HashMap<int,int>(hasher, 100, -1000)));

  // And one good one.
  ASSERT_NO_THROW((HashMap<int,int>(hasher, 100, 4)));

  return true;
}

int expectedCapacityOnIncrease(int size) {
  if (size <= 4)  return  4;
  if (size <= 8)  return  8;
  if (size <= 16) return 16;
  return -1;
}

int expectedCapacityOnDecrease(int size) {
  if (size <= 2)  return  4;
  if (size <= 4)  return  8;
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
    int j = i*10;
    ASSERT_FALSE(map.insert(i, j));
    ASSERT_TRUE(map.exists(i));
    ASSERT_EQUALS((*map.get(i)), i*10);
    ASSERT_EQUALS(map.size(), i+1);
    ASSERT_EQUALS(map.capacity(), expectedCapacityOnIncrease(i+1));
  }
  // now remove the elements and check the capacity
  for (int i = 15; i >= 0; --i) {
    ASSERT_TRUE(map.remove(i));
    ASSERT_FALSE(map.exists(i));
    ASSERT_EQUALS((map.get(i)), NULL);
    ASSERT_EQUALS(map.size(), i);
    ASSERT_EQUALS(map.capacity(), expectedCapacityOnDecrease(i));
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
  int i = 10;
  ASSERT_FALSE(map.insert(1, i));
  ASSERT_TRUE(map.exists(1));
  ASSERT_EQUALS((*map.get(1)), 10);
  ASSERT_EQUALS(map.size(),1);
  ASSERT_EQUALS(map.capacity(), 16);

  // overwrite the value
  i = 20;
  ASSERT_TRUE(map.insert(1, i));
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
  i = 30;
  ASSERT_FALSE(map.insert(1, i));
  ASSERT_TRUE(map.exists(1));
  ASSERT_EQUALS((*map.get(1)), 30);
  ASSERT_EQUALS(map.size(), 1);
  ASSERT_EQUALS(map.capacity(), 16);

  // overwrite the value again
  i = 10;
  ASSERT_TRUE(map.insert(1, i));
  ASSERT_TRUE(map.exists(1));
  ASSERT_EQUALS((*map.get(1)), 10);
  ASSERT_EQUALS(map.size(), 1);
  ASSERT_EQUALS(map.capacity(), 16);

  return true;
}

struct DataType {
  int a;
  double b;
  const char* c;

  bool operator<(const DataType& other) const {
    return (a < other.a);
  }
};

class DataTypeHasher : public Hasher<DataType> {
public:
  virtual int hashCode(const DataType& data) {
    return (data.a + (int)(data.b * 10.0));
  }
};

/*
 * Tests the hash map with a custom data type.
 * This is mainly to make sure the template code doesn't only work for
 * integers.
 */
bool testDataTypeAsKey() {
  HashMap<DataType,double> map(new DataTypeHasher());

  // Insert a bunch of DataTypes.
  for (int i = 0; i < 20; ++i) {
    DataType data = { i, double(i)/20.0, "hello" };
    ASSERT_FALSE(map.insert(data, i * 100.0));
    ASSERT_EQUALS(map.size(), (i+1));
  }

  // Now remove those DataTypes.
  for (int i = 19; i >= 0; --i) {
    DataType data = { i, double(i)/20.0, "hello" };
    ASSERT_TRUE(map.exists(data));
    ASSERT_EQUALS((*map.get(data)), (i * 100.0));
    ASSERT_TRUE(map.remove(data));
    ASSERT_EQUALS(map.size(), i);
  }

  return true;
}

/*
 * Tests a bunch of random insertions and removals of elements into the
 * HashMap, while comparing those with insertions and removals from an
 * stl::map.
 */
bool testRandomInsertionsAndRemovals() {
  HashMap<int,int> map(new IdentityHasher());
  std::map<int,int> real_map;

  // First perform the insertions.
  int num_unique = 0;
  for (int i = 0; i < NUM_ITEMS_IN_RANDOM_TEST; ++i) {
    int num = rand() % 1000;
    bool exists = (real_map.find(num) != real_map.end());

    real_map[num] = i;
    ASSERT_EQUALS(exists, map.insert(num, i));

    if (!exists) {
      ++num_unique;
    }
    ASSERT_EQUALS(map.size(), num_unique);
  }

  // Now compare values.
  std::map<int,int>::iterator i;
  for (i = real_map.begin(); i != real_map.end(); ++i) {
    ASSERT_TRUE(map.exists(i->first));
    ASSERT_EQUALS((i->second), (*(map.get(i->first))));
  }

  // Now remove all values.
  for (i = real_map.begin(); i != real_map.end(); ++i) {
    ASSERT_TRUE(map.remove(i->first));
    --num_unique;
    ASSERT_EQUALS(map.size(), num_unique);
  }

  return true;
}

int main(int argc, char **argv) {
  ASSERT_TRUE(testLinkedList());

	// initialize random number generator
	srand( time(NULL) );

  RUN_TEST(testIllegalOperations);
  RUN_TEST(testRehashing);
  RUN_TEST(testOverwrite);
  RUN_TEST(testDataTypeAsKey);
  RUN_TEST_N_TIMES(testRandomInsertionsAndRemovals, NUM_RANDOM_TESTS);
}
