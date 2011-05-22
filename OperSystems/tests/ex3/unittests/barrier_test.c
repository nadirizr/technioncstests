#include <stdlib.h>
#include <pthread.h>
#include "barrier_test.h"
#include "../../src/mp_interface.h"

test_result_t* test_barrier_bad_init() {
  context_t* con = NULL;

  /* check NULL context */
  ASSERT_NULL(mp_initbarrier(NULL, 0), "barrier should fail to init");
  ASSERT_NULL(mp_initbarrier(NULL, 10), "barrier should fail to init");
  /* check bad N */
  con = mp_init();
  ASSERT_NOT_NULL(con, "context not initialized");
  ASSERT_NULL(mp_initbarrier(con, 0), "barrier should fail to init");
  ASSERT_NULL(mp_initbarrier(con, -1), "barrier should fail to init");
  mp_destroy(con);

  TEST_SUCCESS();
}

struct barrier_check_t {
  context_t* con;
  barrier_t* bar;
  int counter;
  int barrier_counter;
};

void inc_counter(struct barrier_check_t* check) {
  long rc;
  ++check->counter;

  rc = mp_barrier(check->con, check->bar);
  ++check->barrier_counter;

  pthread_exit((void*)rc);
}

test_result_t* test_barrier_usage() {
  pthread_t threads[5];
  int i;
  struct barrier_check_t check = { NULL, NULL, 0, 0 };
  long rc;

  check.con = mp_init();
  ASSERT_NOT_NULL(check.con, "context failed to init");
  check.bar = mp_initbarrier(check.con, 5);
  ASSERT_NOT_NULL(check.bar, "barrier fail to init");

  /* start 4 threads */
  for (i = 0; i < 4; ++i) {
    ASSERT_EQUALS_INT(0, pthread_create(&threads[i], NULL, (void*)&inc_counter, &check),
                      "thread failed to init");
  }
  /* wait for all threads to get to the barrier */
  while (check.counter < 4) {
    ASSERT_EQUALS_INT(0, pthread_yield(), "main thread failed to yield");
  }
  /* check that all got to the barrier and non passed it */
  ASSERT_EQUALS_INT(4, check.counter, "thread didn't start its run");
  ASSERT_EQUALS_INT(0, check.barrier_counter, "barrier didn't stop the thread");
  /* start the last thread to release the whole thing */
  ASSERT_EQUALS_INT(0, pthread_create(&threads[4], NULL, (void*)&inc_counter,
                                      &check), "last thread's creation failed");

  /* wait for the whole thing to end */
  for (i = 0; i < 5; ++i) {
    ASSERT_EQUALS_INT(0, pthread_join(threads[i], (void**)&rc), "Couldn't join");
    ASSERT_EQUALS_INT(0, rc, "mp_barrier(...) caused an error");
  }
  ASSERT_EQUALS_INT(5, check.counter, "last thread didn't start");
  ASSERT_EQUALS_INT(5, check.barrier_counter, "barrier wasn't released");

  mp_destroybarrier(check.con, check.bar);
  mp_destroy(check.con);
  TEST_SUCCESS();
}

test_result_t* test_barrier_reuse_not_allowed() {
  pthread_t threads[5], another_thread;
  struct barrier_check_t check = { NULL, NULL, 0, 0 };
  long i, rc;

  check.con = mp_init();
  ASSERT_NOT_NULL(check.con, "context failed to init");
  check.bar = mp_initbarrier(check.con, 5);
  ASSERT_NOT_NULL(check.bar, "barrier fail to init");

  /* start 4 threads */
  for (i = 0; i < 5; ++i) {
    ASSERT_EQUALS_INT(0, pthread_create(&threads[i], NULL, (void*)&inc_counter, &check),
                      "thread failed to init");
  }
  /* wait for the whole thing to end */
  for (i = 0; i < 5; ++i) {
    ASSERT_EQUALS_INT(0, pthread_join(threads[i], (void**)&rc), "Couldn't join");
    ASSERT_EQUALS_INT(0, rc, "mp_barrier(...) caused an error");
  }
  ASSERT_EQUALS_INT(5, check.barrier_counter, "barrier wasn't released");

  ASSERT_EQUALS_INT(0, pthread_create(&another_thread, NULL, (void*)&inc_counter, &check),
                    "thread failed to init");
  ASSERT_EQUALS_INT(0, pthread_join(another_thread, (void**)&rc), "Couldn't join");
  ASSERT_EQUALS_INT(-1, rc, "mp_barrier(...) caused an error");

  mp_destroybarrier(check.con, check.bar);
  mp_destroy(check.con);
  TEST_SUCCESS();
}

test_suite_t* test_barrier() {
  test_suite_t* results = suite_init();
  RUN_TEST(results, test_barrier_bad_init);
  RUN_TEST(results, test_barrier_usage);
  RUN_TEST(results, test_barrier_reuse_not_allowed);

  return results;
}
