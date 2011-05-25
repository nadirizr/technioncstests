#include <pthread.h>
#include <stdlib.h>
#include "hash_table_test.h"
#include "../../hash_table.h"

test_result_t* test_hash_table_basic() {
  hash_table_t* ht = ht_init(100);
  ht_iterator_t it;
  int i, **values;

  ASSERT_NOT_NULL(ht, "Bad hash table init");

  values = (int**)malloc(sizeof(int*) * 1000);
  ASSERT_NOT_NULL(values, "malloc failed");
  for (i = 0; i < 1000; ++i) {
    values[i] = malloc(sizeof(int));
    ASSERT_NOT_NULL(values[i], "malloc failed");
    *(values[i]) = 10*i;
    ASSERT(ht_put(ht, i, values[i]), "Value not inserted");
  }

  for (i = 0; i < 1000; ++i) {
    ASSERT_EQUALS_INT(*(values[i]), *((int*)ht_get(ht, i)), "Missing value");
    ASSERT(ht_has_key(ht, i), "Missing key");
  }

  HT_FOR_EACH(ht, it) {
    ASSERT(HT_ITERATOR_KEY(it) < 1000, "Invalid key");
    ASSERT_CONTAINS(values, 1000, HT_ITERATOR_VALUE(int, it), "Missing value");
  }

  for (i = 0; i < 1000; ++i) {
    ASSERT(ht_remove(ht, i), "Element was not removed");
  }
  ASSERT(ht_first(ht) == ht_end(ht),
         "Hash table first iterator not end on empty hash table");

  i = 0;
  HT_FOR_EACH(ht, it) {
    ++i;
  }
  ASSERT_EQUALS_INT(i, 0, "Elements in hash table");


  ht_destroy(ht);
  for (i = 0; i < 1000; ++i) {
    free(values[i]);
  }
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
