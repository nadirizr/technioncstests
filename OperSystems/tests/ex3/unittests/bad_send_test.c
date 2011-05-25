#include <stdlib.h>
#include <pthread.h>
#include "bad_send_test.h"
#include "../../mp_interface.h"

struct test_context {
  context_t* con;
  pthread_t target;
  int should_register;
  int should_unregister;
  int context_null;
};

struct results {
  int reg_result;
  int send_result;
};

void write(struct test_context* con) {
  struct results* res = malloc(sizeof(struct results));

  if (con->should_register) {
    res->reg_result = mp_register(con->con);
    if (res->reg_result != 0) {
      pthread_exit((void*)res);
    }
    if (con->should_unregister) {
      mp_unregister(con->con);
    }
  }

  if (con->context_null) {
    res->send_result = mp_send(NULL, &con->target, "message", 8, 0);
  } else {
    res->send_result = mp_send(con->con, &con->target, "message", 8, 0);
  }

  /* if we got here, the thread is registered, so unregister */
  mp_unregister(con->con);
  pthread_exit((void*)res);
}

/*
 * Case #1:
 * The reader isn't registered.
 * Writer tries to send a message and should fail.
 */
test_result_t* test_send_to_thread_that_didnt_register() {
  pthread_t writer;
  struct test_context writer_context;
  struct results* results;

  /* build the test context */
  writer_context.con = mp_init();
  ASSERT_NOT_NULL(writer_context.con, "context failed to init");
  writer_context.target = pthread_self();
  writer_context.should_register = TRUE;
  writer_context.should_unregister = FALSE;
  writer_context.context_null = FALSE;

  ASSERT_EQUALS_INT(0, pthread_create(&writer, NULL, (void*)&write, &writer_context),
                    "creation of writer thread failed");

  ASSERT_EQUALS_INT(0, pthread_join(writer, (void**)&results), "couldn't join");
  ASSERT_EQUALS_INT(0, results->reg_result, "writer failed to register");
  ASSERT_EQUALS_INT(-1, results->send_result,
                    "send to a thread that didn't register should fail");
  free(results);

  /* destroy the test context */
  mp_destroy(writer_context.con);
  TEST_SUCCESS();
}

/*
 * Case #2:
 * The reader is registered. the writer isn't.
 * Writer tries to send a message and should fail.
 */
test_result_t* test_send_from_thread_that_didnt_register() {
  pthread_t writer;
  struct test_context writer_context;
  struct results* results;

  /* build the test context */
  writer_context.con = mp_init();
  ASSERT_NOT_NULL(writer_context.con, "context failed to init");
  writer_context.target = pthread_self();
  writer_context.should_register = FALSE;
  writer_context.context_null = FALSE;

  mp_register(writer_context.con);
  ASSERT_EQUALS_INT(0, pthread_create(&writer, NULL, (void*)&write, &writer_context),
                    "creation of writer thread failed");
  mp_unregister(writer_context.con);

  ASSERT_EQUALS_INT(0, pthread_join(writer, (void**)&results), "couldn't join");
  ASSERT_EQUALS_INT(-1, results->send_result, "send to a thread that didn't register should fail");
  free(results);

  /* destroy the test context */
  mp_destroy(writer_context.con);
  TEST_SUCCESS();
}

/*
 * Case #3:
 * The reader unregistered.
 * Writer tries to send a message and should fail.
 */
test_result_t* test_send_to_tread_that_unregistered() {
  pthread_t writer;
  struct test_context writer_context;
  struct results* results;

  /* build the test context */
  writer_context.con = mp_init();
  ASSERT_NOT_NULL(writer_context.con, "context failed to init");
  writer_context.target = pthread_self();
  writer_context.should_register = TRUE;
  writer_context.should_unregister = FALSE;
  writer_context.context_null = FALSE;

  mp_register(writer_context.con);
  usleep(1000);
  mp_unregister(writer_context.con);

  ASSERT_EQUALS_INT(0, pthread_create(&writer, NULL, (void*)&write, &writer_context),
                    "creation of writer thread failed");

  ASSERT_EQUALS_INT(0, pthread_join(writer, (void**)&results), "couldn't join");
  ASSERT_EQUALS_INT(0, results->reg_result, "writer failed to register");
  ASSERT_EQUALS_INT(-1, results->send_result,
                    "send to a thread that unregistered should fail");
  free(results);
  
  /* destroy the test context */
  mp_destroy(writer_context.con);
  TEST_SUCCESS();
}

/*
 * Case #4:
 * The reader is registered.
 * Writer tries to send a message after unregistering, should fail.
 */
test_result_t* test_send_from_thread_that_unregistered() {
  pthread_t writer;
  struct test_context writer_context;
  struct results* results;

  /* build the test context */
  writer_context.con = mp_init();
  ASSERT_NOT_NULL(writer_context.con, "context failed to init");
  writer_context.target = pthread_self();
  writer_context.should_register = TRUE;
  writer_context.should_unregister = TRUE;
  writer_context.context_null = FALSE;

  mp_register(writer_context.con);
  ASSERT_EQUALS_INT(0, pthread_create(&writer, NULL, (void*)&write, &writer_context),
                    "creation of writer thread failed");
  mp_unregister(writer_context.con);

  ASSERT_EQUALS_INT(0, pthread_join(writer, (void**)&results), "couldn't join");
  ASSERT_EQUALS_INT(-1, results->send_result, "sending from an unregistered thread should fail");
  free(results);

  /* destroy the test context */
  mp_destroy(writer_context.con);
  TEST_SUCCESS();
}

/*
 * Case #5:
 * The context is NULL.
 * Writer tries to send a message, should fail.
 */
test_result_t* test_send_with_NULL_context() {
  pthread_t writer;
  struct test_context writer_context;
  struct results* results;

  /* build the test context */
  writer_context.con = mp_init();
  ASSERT_NOT_NULL(writer_context.con, "context failed to init");
  writer_context.target = pthread_self();
  writer_context.should_register = TRUE;
  writer_context.should_unregister = FALSE;
  writer_context.context_null = TRUE;

  mp_register(writer_context.con);
  ASSERT_EQUALS_INT(0, pthread_create(&writer, NULL, (void*)&write, &writer_context),
                    "creation of writer thread failed");
  mp_unregister(writer_context.con);

  ASSERT_EQUALS_INT(0, pthread_join(writer, (void**)&results), "couldn't join");
  ASSERT_EQUALS_INT(0, results->reg_result, "writer failed to register");
  ASSERT_EQUALS_INT(-1, results->send_result, "send to a NULL context should fail");
  free(results);

  /* destroy the test context */
  mp_destroy(writer_context.con);
  TEST_SUCCESS();
}

test_suite_t* test_bad_send() {
  test_suite_t* results = suite_init();
  RUN_TEST(results, test_send_to_thread_that_didnt_register);
  RUN_TEST(results, test_send_from_thread_that_didnt_register);
  RUN_TEST(results, test_send_to_tread_that_unregistered);
  RUN_TEST(results, test_send_from_thread_that_unregistered);
  RUN_TEST(results, test_send_with_NULL_context);

  return results;
}
