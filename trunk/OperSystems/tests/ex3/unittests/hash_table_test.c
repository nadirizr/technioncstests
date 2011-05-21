#include <stdlib.h>
#include "hash_table_test.h"
#include "../../src/hash_table.h"

test_result_t* test_hash_table_basic() {
  hash_table_t* ht = ht_init(100);
  int i, *value, *keys, **values;

  ASSERT_NOT_NULL(ht, "Bad hash table init");

  for (i = 0; i < 1000; ++i) {
    value = malloc(sizeof(int));
    (*value) = 10*i;
    ht_put(ht, i, value);
  }

  ASSERT_EQUALS_INT(1000, ht_num_elements(ht), "Bad number of elements");

  keys = malloc(1000*sizeof(int));
  ASSERT_EQUALS_INT(1000, ht_all_keys(ht, keys), "Bad number  of keys");
  ASSERT_EQUALS_INT(1000, ht_all_values(ht, (ht_value_t*)values), "Bad number of values");
  for (i = 0; i < 1000; ++i) {
    ASSERT_CONTAINS(keys, 1000, i, "Missing key");
    ASSERT_EQUALS_INT(10*i, ((int)(*((int*)ht_get(ht, i)))), "Missing value");
    ASSERT(ht_has_key(ht, i), "Missing key");
  }

  for (i = 0; i < 1000; ++i) {
    ASSERT(ht_remove(ht, i), "Element was not removed");
    ASSERT_EQUALS_INT(1000-i-1, ht_num_elements(ht), "Item not removed from counter");
  }
  ASSERT_EQUALS_INT(0, ht_all_keys(ht, keys), "Not all keys removed");

  free(keys);
  free(values);
  ht_destroy(ht);
  TEST_SUCCESS();
}

test_result_t* test_hash_table_concurrent() {
  hash_table_t* ht = ht_init(100);

  ASSERT_NOT_NULL(ht, "Bad hash table init");

  ht_destroy(ht);
  TEST_SUCCESS();
}

test_suite_t* test_hash_table() {
  test_suite_t* results = suite_init();
  RUN_TEST(results, test_hash_table_basic);
  RUN_TEST(results, test_hash_table_concurrent);

  return results;
}