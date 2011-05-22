#ifndef _UNITTEST_H_
#define _UNITTEST_H_

#include <stdio.h>
#include <string.h>

#define FALSE 0
#define TRUE 1

typedef struct test_suite_t test_suite_t;
typedef struct test_result_t test_result_t;

#define RUN_SUITE(var, suite_name) \
  printf("|##############################################################################|\n", #suite_name); \
  printf("|  Running suite %-62s|\n", #suite_name); \
  var = suite_name(); \
  printf("|  Passed: %d/%-66d|\n", suite_passed(var), suite_ran(var)); \
  printf("|##############################################################################|\n"); \
  suite_destroy(var);

#define RUN_TEST(suite, test_name) \
  printf("|    Running %s ...", #test_name); \
  do { int i_0=0; for(;i_0<(43-strlen(#test_name));++i_0) printf(" "); } while(0); \
  suite_add_result(suite, test_name());

#define TEST_SUCCESS() \
  printf("OK%17s|\n", " "); \
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
    printf("FAIL%15s|\n", " "); \
    printf("|      reason:   %-62s|\n", message); \
    printf("|      location: %s:%-4d", __FILE__, __LINE__); \
    do { int i_0=0; for(;i_0<(57-strlen(__FILE__));++i_0) printf(" "); printf("|\n"); } while(0); \
    return result_init(FALSE); \
  }

#define FAIL(message) \
  ASSERT(FALSE, message)

#define ASSERT_NULL(expression, message) \
  ASSERT(expression == NULL, message)

#define ASSERT_NOT_NULL(expression, message) \
  ASSERT(expression != NULL, message)

#define ASSERT_EQUALS_INT(expected, actual, message) \
  if (expected != actual) { \
    printf("FAIL%15s|\n", " "); \
    printf("|      reason:   expected %-10d got %-10d%28s|\n", expected, actual, " "); \
    printf("|                %62s|\n", message); \
    printf("|      location: %s:%-4d", __FILE__, __LINE__); \
    do { int i_0=0; for(;i_0<(57-strlen(__FILE__));++i_0) printf(" "); printf("|\n"); } while(0); \
    return result_init(FALSE); \
  }

#define ASSERT_CONTAINS(array, size, element, message) \
  do { \
    int i_array, found = 0; \
    for (i_array = 0; i_array < size; ++i_array) { \
      if (array[i_array] == element) { found = 1; break; } \
    } \
    if (!found) { \
      FAIL(message); \
    } \
  } while (0)


#endif /* _UNITTEST_H_ */
