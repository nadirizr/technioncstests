#include "hash_table_test.h"
#include "../../src/hash_table.h"

test_result_t* test_hash_table_basic() {
  hash_table_t* ht = ht_init(100);

  ASSERT_NOT_NULL(ht, "Bad hash table init");
  FAIL("Not implemented");

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
