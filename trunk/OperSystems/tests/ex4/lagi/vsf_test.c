#include "vsf_test.h"

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
    usleep(500000);
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
    ASSERT_EQUALS_INT(0, (int)rc, "mp_barrier(...) caused an error");
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
    ASSERT_EQUALS_INT(0, (int)rc, "mp_barrier(...) caused an error");
  }
  ASSERT_EQUALS_INT(5, check.barrier_counter, "barrier wasn't released");

  ASSERT_EQUALS_INT(0, pthread_create(&another_thread, NULL, (void*)&inc_counter, &check),
                    "thread failed to init");
  ASSERT_EQUALS_INT(0, pthread_join(another_thread, (void**)&rc), "Couldn't join");
  ASSERT_EQUALS_INT(-1, (int)rc, "mp_barrier(...) caused an error");

  mp_destroybarrier(check.con, check.bar);
  mp_destroy(check.con);
  TEST_SUCCESS();
}

void catch_barrier(struct barrier_check_t* check) {
  long rc = mp_barrier(check->con, check->bar);
  pthread_exit((void*)rc);
}

test_result_t* test_barrier_stress_test() {
  pthread_t threads[2];
  struct barrier_check_t check = { NULL, NULL, 0, 0 };
  long i, rc1, rc2;

  check.con = mp_init();
  ASSERT_NOT_NULL(check.con, "context failed to init");

  for (i = 0; i < 1000; ++i) {
    check.bar = mp_initbarrier(check.con, 1);
    ASSERT_NOT_NULL(check.bar, "barrier fail to init");

    /* start the 2 threads */
    int starter = rand() % 2;
    int other = (starter + 1) % 2;
    ASSERT_EQUALS_INT(0, pthread_create(&threads[starter], NULL, (void*)&catch_barrier, &check),
                      "thread failed to init");
    ASSERT_EQUALS_INT(0, pthread_create(&threads[other], NULL, (void*)&catch_barrier, &check),
                      "thread failed to init");

    /* wait for the whole thing to end */
    ASSERT_EQUALS_INT(0, pthread_join(threads[starter], (void**)&rc1), "Couldn't join");
    ASSERT_EQUALS_INT(0, pthread_join(threads[other], (void**)&rc2), "Couldn't join");
    ASSERT((rc1 == 0 && rc2 == -1) || (rc1 == -1 && rc2 == 0), "fail to handle racing threads");

    mp_destroybarrier(check.con, check.bar);
  }

  mp_destroy(check.con);
  TEST_SUCCESS();
}

test_suite_t* test_barrier() {
  srand(time(NULL));
  test_suite_t* results = suite_init();
  RUN_TEST(results, test_barrier_bad_init);
  RUN_TEST(results, test_barrier_usage);
  RUN_TEST(results, test_barrier_reuse_not_allowed);
  RUN_TEST(results, test_barrier_stress_test);

  return results;
}
