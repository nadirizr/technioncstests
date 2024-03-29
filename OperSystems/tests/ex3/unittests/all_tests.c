#include "list_test.h"
#include "hash_table_test.h"
#include "context_test.h"
#include "barrier_test.h"
#include "bad_send_test.h"

int main() {
  test_suite_t* suite;
  RUN_SUITE(suite, test_list);
  RUN_SUITE(suite, test_hash_table);
  RUN_SUITE(suite, test_context);
  RUN_SUITE(suite, test_barrier);
  RUN_SUITE(suite, test_bad_send);

  return 0;
}
