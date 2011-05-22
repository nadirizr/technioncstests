#include <pthread.h>
#include <stdlib.h>
#include "hash_table_test.h"
#include "../../src/hash_table.h"

test_result_t* test_hash_table_basic() {
  hash_table_t* ht = ht_init(100);
  int i, *keys, **values, **all_values;

  ASSERT_NOT_NULL(ht, "Bad hash table init");

  values = (int**)malloc(sizeof(int*) * 1000);
  ASSERT_NOT_NULL(values, "malloc failed");
  for (i = 0; i < 1000; ++i) {
    values[i] = malloc(sizeof(int));
    ASSERT_NOT_NULL(values[i], "malloc failed");
    *(values[i]) = 10*i;
    ASSERT(ht_put(ht, i, values[i]), "Value not inserted");
  }

  ASSERT_EQUALS_INT(1000, ht_num_elements(ht), "Bad number of elements");

  keys = malloc(1000*sizeof(int));
  ASSERT_EQUALS_INT(1000, ht_all_keys(ht, keys), "Bad number  of keys");
  all_values = malloc(1000*sizeof(int*));
  ASSERT_EQUALS_INT(1000, ht_all_values(ht, (ht_value_t*)all_values), "Bad number of values");
  for (i = 0; i < 1000; ++i) {
    ASSERT_CONTAINS(keys, 1000, i, "Missing key");
    ASSERT_CONTAINS(all_values, 1000, values[i], "Missing value");
    ASSERT_EQUALS_INT(*(values[i]), *((int*)ht_get(ht, i)), "Missing value");
    ASSERT(ht_has_key(ht, i), "Missing key");
  }

  for (i = 0; i < 1000; ++i) {
    ASSERT(ht_remove(ht, i), "Element was not removed");
    ASSERT_EQUALS_INT(1000-i-1, ht_num_elements(ht), "Item not removed from counter");
  }
  ASSERT_EQUALS_INT(0, ht_all_keys(ht, keys), "Not all keys removed");

  ht_destroy(ht);
  for (i = 0; i < 1000; ++i) {
    free(values[i]);
  }
  free(keys);
  free(all_values);
  free(values);

  TEST_SUCCESS();
}

test_result_t* test_hash_table_concurrent() {
  hash_table_t* ht = ht_init(100);
  pthread_t threads[50];
  int i;

  ASSERT_NOT_NULL(ht, "Bad hash table init");
  for (i = 0; i < 50; ++i) {
    FAIL("Not implemented");
  }

  ht_destroy(ht);

  TEST_SUCCESS();
}

test_suite_t* test_hash_table() {
  test_suite_t* results = suite_init();
  RUN_TEST(results, test_hash_table_basic);
  //RUN_TEST(results, test_hash_table_concurrent);

  return results;
}
