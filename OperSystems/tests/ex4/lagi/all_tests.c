#include "vsf_test.h"

int main() {
  test_suite_t* suite;
  RUN_SUITE(suite, test_vsf);

  return 0;
}
