#include <stdlib.h>
#include "list_test.h"
#include "../../list.h"

bool int_cmp(void* int1, void* int2) {
  if (int1 == NULL || int2 == NULL) {
    return FALSE;
  }
  return ((*((int*)int1) == *((int*)int2)) ? TRUE : FALSE);
}

test_result_t* test_list_basic() {
  list_t* list = list_init_with_compare(int_cmp);
  list_iterator_t it = NULL;
  int i, **values;

  ASSERT_NOT_NULL(list, "Bad list init");
  values = (int**) malloc(sizeof(int*) * 1000);

  for (i = 0; i < 1000; ++i) {
    values[i] = malloc(sizeof(int));
    (*values[i]) = i;
    ASSERT(list_push_back(list, values[i]), "Value not inserted");
  }

  ASSERT_EQUALS_INT(1000, list_num_elements(list), "Bad number of elements");

  i = 0;
  LIST_FOR_EACH(list, it) {
    ASSERT(i < 1000, "Too many elements");
    ASSERT_EQUALS_INT(*values[i], *LIST_ITERATOR_VALUE(int, it), "Missing value");
    ++i;
  }
  ASSERT_EQUALS_INT(i, 1000, "Not enough elements");

  for (i = 0; i < 500; ++i) {
    ASSERT(list_remove(list, values[i]), "Element was not removed");
    ASSERT_EQUALS_INT(1000-i-1, list_num_elements(list), "Item not removed from counter");
  }
  ASSERT_EQUALS_INT(500, list_num_elements(list), "Not all keys removed");
  
  for (i = 500; i < 750; ++i) {
    ASSERT(list_pop_front(list), "Element was not removed");
    ASSERT_EQUALS_INT(1000-i-1, list_num_elements(list), "Item not removed from counter");
  }
  ASSERT_EQUALS_INT(250, list_num_elements(list), "Not all keys removed");

  for (i = 750; i < 1000; ++i) {
    ASSERT(list_pop_back(list), "Element was not removed");
    ASSERT_EQUALS_INT(1000-i-1, list_num_elements(list), "Item not removed from counter");
  }
  ASSERT_EQUALS_INT(0, list_num_elements(list), "Not all keys removed");
  
  ASSERT(list_empty(list), "List not empty");

  list_destroy(list);
  for (i = 0; i < 1000; ++i) {
    free(values[i]);
  }
  free(values);

  TEST_SUCCESS();
}

test_suite_t* test_list() {
  test_suite_t* results = suite_init();
  RUN_TEST(results, test_list_basic);

  return results;
}
