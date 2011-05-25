#include <stdlib.h>
#include <pthread.h>
#include "barrier_test.h"
#include "../../mp_interface.h"

struct context {
  context_t* con;
  barrier_t* bar[6];
};

struct results {
  int first_register;
  int second_register;
  int third_register;
};

void act(struct context* con) {
  struct results* res = malloc(sizeof(struct results));

  mp_unregister(con->con);
  mp_barrier(con->con, con->bar[0]);

  res->first_register = mp_register(con->con);
  mp_barrier(con->con, con->bar[1]);

  res->second_register = mp_register(con->con);
  mp_barrier(con->con, con->bar[2]);

  mp_unregister(con->con);
  mp_barrier(con->con, con->bar[3]);

  res->third_register = mp_register(con->con);
  mp_barrier(con->con, con->bar[4]);

  mp_unregister(con->con);
  mp_barrier(con->con, con->bar[5]);

  pthread_exit((void*)res);
}

test_result_t* test_context_usage() {
  pthread_t threads[5];
  struct context context;
  long i;
  struct results* results;

  context.con = mp_init();
  ASSERT_NOT_NULL(context.con, "context failed to init");
  for (i = 0; i < 6; ++i) {
    context.bar[i] = mp_initbarrier(context.con, 5);
    ASSERT_NOT_NULL(context.bar[i], "barrier failed to init");
  }

  /* verify register was successful. */
  for (i = 0; i < 5; ++i) {
    ASSERT_EQUALS_INT(0, pthread_create(&threads[i], NULL, (void*)&act, &context), "thread create failed");
  }
  for (i = 0; i < 5; ++i) {
    ASSERT_EQUALS_INT(0, pthread_join(threads[i], (void**)&results), "couldn't join");
    ASSERT_EQUALS_INT(0, results->first_register, "register failed");
    ASSERT_EQUALS_INT(-1, results->second_register, "register while already registered didn't fail");
    ASSERT_EQUALS_INT(0, results->third_register, "re-register after unregister failed");
    free(results);
  }

  for (i = 0; i < 6; ++i) {
    mp_destroybarrier(context.con, context.bar[i]);
  }
  mp_destroy(context.con);
  TEST_SUCCESS();
}

test_suite_t* test_context() {
  test_suite_t* results = suite_init();
  RUN_TEST(results, test_context_usage);

  return results;
}
