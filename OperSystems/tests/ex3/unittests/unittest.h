#ifndef _UNITTEST_H_
#define _UNITTEST_H_

#include <stdio.h>

#define FALSE 0
#define TRUE 1

typedef struct test_suite_t test_suite_t;
typedef struct test_result_t test_result_t;

#define RUN_SUITE(var, suite_name) \
  printf("|*****Running suite %s*****\n", #suite_name); \
  var = suite_name(); \
  printf("|Passed: %d/%d\n", suite_passed(var), suite_ran(var)); \
  printf("|**************************\n");

#define RUN_TEST(suite, test_name) \
  printf("|Running %s ... ", #test_name); \
  suite_add_result(suite, test_name());

#define TEST_SUCCESS() \
  printf("OK\n"); \
  return result_init(TRUE)

test_suite_t* suite_init();
void suite_destroy(test_suite_t* suite);
void suite_add_result(test_suite_t* suite, test_result_t* result);
int suite_passed(test_suite_t* suite);
int suite_ran(test_suite_t* suite);

test_result_t* result_init(int test_passed);
void result_destroy(test_result_t* result);

/****************** ASSERTIONS *******************/

#define ASSERT(expression, message) \
  if (!(expression)) { \
    printf("FAIL\n"); \
    printf("|    reason:   %s\n", message); \
    printf("|    location: %s:%d\n", __FILE__, __LINE__); \
    return result_init(FALSE); \
  }

#define FAIL(message) \
  ASSERT(FALSE, message)

#define ASSERT_NOT_NULL(expression, message) \
  ASSERT(expression != NULL, message)

#endif /* _UNITTEST_H_ */
