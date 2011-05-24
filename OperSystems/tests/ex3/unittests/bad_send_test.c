#include <stdlib.h>
#include <pthread.h>
#include "bad_send_test.h"
#include "../../src/mp_interface.h"

struct test_context {
  context_t* con;
  pthread_t target;
  int should_register;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
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
      pthread_cond_signal(&con->cond);
      pthread_exit((void*)res);
    }
  }

  res->send_result = mp_send(con->con, &con->target, "message", 8, 0);

  /* if we got here, the thread is registered, so unregister */
  mp_unregister(con->con);
  pthread_cond_signal(&con->cond);
  pthread_exit((void*)res);
}

test_result_t* test_send_receive_unregistered() {
  pthread_t writer;
  struct test_context writer_context;
  struct results* results;

  /* build the test context */
  writer_context.con = mp_init();
  ASSERT_NOT_NULL(writer_context.con, "context failed to init");
  writer_context.target = pthread_self();
  ASSERT_EQUALS_INT(0, pthread_mutex_init(&writer_context.mutex, NULL),
                    "mutex failed to init");
  ASSERT_EQUALS_INT(0, pthread_cond_init(&writer_context.cond, NULL),
                    "writer condition badly initiated");

  /*
   * Case #1:
   * The reader isn't registered.
   * Writer tries to send a message and should fail.
   */
  writer_context.should_register = TRUE;
  ASSERT_EQUALS_INT(0, pthread_create(&writer, NULL, (void*)&write, &writer_context),
                    "creation of writer thread failed");
  pthread_cond_wait(&writer_context.cond, &writer_context.mutex); // wait for the writer to finish

  ASSERT_EQUALS_INT(0, pthread_join(writer, (void**)&results), "couldn't join");
  ASSERT_EQUALS_INT(0, results->reg_result, "writer failed to register");
  ASSERT_EQUALS_INT(-1, results->send_result,
                    "send to a thread that didn't register should fail");
  free(results);

  /*
   * Case #2:
   * The reader is registered. the writer isn't.
   * Writer tries to send a message and should fail.
   */
  writer_context.should_register = FALSE;
  mp_register(writer_context.con);
  ASSERT_EQUALS_INT(0, pthread_create(&writer, NULL, (void*)&write, &writer_context),
                    "creation of writer thread failed");
  pthread_cond_wait(&writer_context.cond, &writer_context.mutex); // wait for the writer to finish
  mp_unregister(writer_context.con);

  ASSERT_EQUALS_INT(0, pthread_join(writer, (void**)&results), "couldn't join");
  ASSERT_EQUALS_INT(-1, results->send_result, "send to a thread that didn't register should fail");
  free(results);

  /*
   * Case #3:
   * The reader unregistered.
   * Writer tries to send a message and should fail.
   */
  writer_context.should_register = TRUE;
  ASSERT_EQUALS_INT(0, pthread_create(&writer, NULL, (void*)&write, &writer_context),
                    "creation of writer thread failed");
  pthread_cond_wait(&writer_context.cond, &writer_context.mutex); // wait for the writer to finish

  ASSERT_EQUALS_INT(0, pthread_join(writer, (void**)&results), "couldn't join");
  ASSERT_EQUALS_INT(0, results->reg_result, "writer failed to register");
  ASSERT_EQUALS_INT(-1, results->send_result,
                    "send to a thread that didn't register should fail");
  free(results);
  
  /* destroy the test context */
  pthread_mutex_destroy(&writer_context.mutex);
  pthread_cond_destroy(&writer_context.cond);
  mp_destroy(writer_context.con);
  TEST_SUCCESS();
}

test_suite_t* test_bad_send() {
  test_suite_t* results = suite_init();
  RUN_TEST(results, test_send_receive_unregistered);

  return results;
}
