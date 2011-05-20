#include <stdlib.h>
#include "unittest.h"

struct test_suite_t {
  test_result_t* first;
  test_result_t* last;
  int passed;
  int ran;
};

struct test_result_t {
  int test_passed;
  test_result_t* next;
};

test_suite_t* suite_init() {
  test_suite_t* suite = malloc(sizeof(test_suite_t));
  suite->first = NULL;
  suite->last = NULL;
  suite->passed = 0;
  suite->ran = 0;
  return suite;
}

void suite_destroy(test_suite_t* suite) {
  test_result_t* tmp;
  suite->last = NULL;
  while (suite->first != NULL) {
    tmp = suite->first;
    suite->first = tmp->next;
    result_destroy(tmp);
  }
  suite->passed = 0;
  suite->ran = 0;
  free(suite);
}

void suite_add_result(test_suite_t* suite, test_result_t* result) {
  if (suite->first == NULL) {
    suite->first = result;
    suite->last = result;
  } else {
    suite->last->next = result;
  }
  ++suite->ran;
  if (result->test_passed) {
    ++suite->passed;
  }
}

int suite_passed(test_suite_t* suite) {
  return suite->passed;
}

int suite_ran(test_suite_t* suite) {
  return suite->ran;
}

test_result_t* result_init(int test_passed) {
  test_result_t* result = malloc(sizeof(test_result_t));
  result->test_passed = test_passed;
  result->next = NULL;
}

void result_destroy(test_result_t* result) {
  result->next = NULL;
  free(result);
}

