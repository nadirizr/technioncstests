#include "hash_table_test.h"

int main() {
  test_suite_t* suite;
  RUN_SUITE(suite, test_hash_table);

  return 0;
}
