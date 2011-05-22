#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "mp_interface.h"

#define MAX_STRING_INPUT_SIZE 65536

#define MAX_THREADS 1000
#define DEFAULT_NUM_THREADS 20

#define ERROR_INVALID_LINE -10000
#define FINISH_THREAD -9999


/*** Contex and Barrier ***/
context_t* main_context;
barrier_t* main_barrier;

barrier_t* start_barrier;
barrier_t* finish_barrier;


/*** Process Threads ***/

typedef struct {
  int index;
  pthread_t tid;
} ProcessThread;

ProcessThread* threads[MAX_THREADS];
int num_threads;
pthread_mutex_t threads_lock;

int add_myself_to_threads() {
  int my_index;

  pthread_mutex_lock(&threads_lock);

  if (num_threads == MAX_THREADS) {
    pthread_mutex_unlock(&threads_lock);
    return -1;
  }

  my_index = num_threads;
  threads[my_index] = (ProcessThread*)malloc(sizeof(ProcessThread));
  threads[my_index]->index = my_index;
  threads[my_index]->tid = pthread_self();
  ++num_threads;

  pthread_mutex_unlock(&threads_lock);

  return my_index;  /* Return the new thread's index. */
}

int remove_thread(int index) {
  if (index < 0 || index >= MAX_THREADS) {
    return -1;
  }

  pthread_mutex_lock(&threads_lock);

  free(threads[index]);
  threads[index] = NULL;

  pthread_mutex_unlock(&threads_lock);

  return 0;
}


/*** Handling Commands in Threads ***/

int thread_handle_create_barrier(int index, char* arguments, int line_num) {
  int n = 0;

  /* Verify argument validity. */
  if (!arguments || sscanf(arguments, "%d", &n) != 1) {
    return ERROR_INVALID_LINE;
  }

  /* Perform the command. */
  main_barrier = mp_initbarrier(main_context, n);
  
  /* Handle errors. */
  if (main_barrier == NULL) {
    printf("[Thread %d]: ERROR [line %d]: Barrier Creation Failed!\n",
           index + 1, line_num);
    fflush(stdout);
    return -1;
  }

  /* Handle success. */
  printf("[Thread %d]: Barrier Created\n", index + 1);
  fflush(stdout);
  return 0;
}

int thread_handle_destroy_barrier(int index, char* arguments, int line_num) {
  /* Perform the command. */
  mp_destroybarrier(main_context, main_barrier);
  main_barrier = NULL;

  /* Handle success. */
  printf("[Thread %d]: Barrier Destroyed\n", index + 1);
  fflush(stdout);
  return 0;
}

int thread_handle_barrier(int index, char* arguments, int line_num) {
  int rc = 0;

  /* Perform the command. */
  rc = mp_barrier(main_context, main_barrier);
  
  /* Handle errors. */
  if (rc < 0) {
    printf("[Thread %d]: ERROR [line %d]: Barrier Failed (rc = %d)\n",
           index + 1, line_num, rc);
    fflush(stdout);
    return rc;
  }

  /* Handle success. */
  printf("[Thread %d]: Barrier Passed\n", index + 1);
  fflush(stdout);
  return 0;
}

int thread_handle_send(int index, char* arguments, int line_num) {
  char* message = NULL;
  int message_len = 0;
  int target_id = 0;
  int rc = 0;
  int flags = 0;
  int is_sync = 0;
  int is_urgent = 0;

  /* Verify argument validity. */
  if (!arguments || sscanf(arguments, "%d ", &target_id) != 1) {
    return ERROR_INVALID_LINE;
  }

  --target_id;
  if (target_id < 0 || target_id >= num_threads) {
    return ERROR_INVALID_LINE;
  }

  message = strchr(arguments, ' ');
  if (message == NULL) {
    return ERROR_INVALID_LINE;
  }
  ++message;
  message_len = strlen(message) + 1;
  if (message_len < 1) {
    return ERROR_INVALID_LINE;
  }

  /* Handle additional arguments. */
  flags = 0;
  if (strstr(message, "SYNC ") != NULL) {
    is_sync = 1;
    flags |= SEND_SYNC;
  }
  if (strstr(message, "URGENT ") != NULL) {
    is_urgent = 1;
    flags |= SEND_URGENT;
  }

  /* Perform the command. */
  rc = mp_send(main_context, &(threads[target_id]->tid),
               message, message_len, flags);
  
  /* Handle errors. */
  if (rc < 0) {
    printf("[Thread %d]: ERROR [line %d]: Send Failed (Flags:",
           index + 1, line_num);
    if (is_urgent) {
      printf(" URGENT");
    }
    if (is_sync) {
      printf(" SYNC");
    }
    if (flags == 0) {
      printf(" None");
    }
    printf(") (rc = %d)!\n", rc);
    fflush(stdout);
    return -1;
  }

  /* Handle success. */
  printf("[Thread %d]: Send Successfull (Flags:", index + 1);
  if (is_urgent) {
    printf(" URGENT");
  }
  if (is_sync) {
    printf(" SYNC");
  }
  if (flags == 0) {
    printf(" None");
  }
  printf(")\n");
  fflush(stdout);
  return 0;
}

int thread_handle_broadcast(int index, char* arguments, int line_num) {
  char* message = NULL;
  int message_len = 0;
  int rc = 0;
  int flags = 0;
  int is_sync = 0;
  int is_urgent = 0;

  /* Verify argument validity. */
  if (!arguments) {
    return ERROR_INVALID_LINE;
  }

  message = arguments;
  message_len = strlen(message) + 1;
  if (message_len < 1) {
    return ERROR_INVALID_LINE;
  }

  /* Handle additional arguments. */
  flags = 0;
  if (strstr(message, "SYNC ") != NULL) {
    is_sync = 1;
    flags |= SEND_SYNC;
  }
  if (strstr(message, "URGENT ") != NULL) {
    is_urgent = 1;
    flags |= SEND_URGENT;
  }

  /* Perform the command. */
  rc = mp_broadcast(main_context, message, message_len, flags);
  
  /* Handle errors. */
  if (rc < 0) {
    printf("[Thread %d]: ERROR [line %d]: Broadcast Failed (Flags:",
           index + 1, line_num);
    if (is_urgent) {
      printf(" URGENT");
    }
    if (is_sync) {
      printf(" SYNC");
    }
    if (flags == 0) {
      printf(" None");
    }
    printf(") (rc = %d)!\n", rc);
    fflush(stdout);
    return -1;
  }

  /* Handle success. */
  printf("[Thread %d]: Broadcast Successfull (Flags:", index + 1);
  if (is_urgent) {
    printf(" URGENT");
  }
  if (is_sync) {
    printf(" SYNC");
  }
  if (flags == 0) {
    printf(" None");
  }
  printf(")\n");
  fflush(stdout);
  return 0;
}

int thread_handle_close(int index, char* arguments, int line_num) {
  thread_handle_broadcast(index, "FINISH", line_num);
  return FINISH_THREAD;
}

#define EQUALS(str1,str2) (strcmp((str1),(str2)) == 0)
#define HANDLE(cmd, fn) \
  if (EQUALS((line), (cmd))) { \
    rc = fn(my_index, arguments, line_num); \
  }

int thread_handle_command(int my_index, char* line, int line_num) {
  int rc = ERROR_INVALID_LINE;
  char* arguments = NULL;

  /* Find the command arguments. */
  arguments = strchr(line, ' ');
  if (arguments != NULL) {
    arguments[0] = '\0';
    arguments++;
  }

  /* Handle barrier commands. */
  HANDLE("CREATE_BARRIER", thread_handle_create_barrier);
  HANDLE("DESTROY_BARRIER", thread_handle_destroy_barrier);
  HANDLE("BARRIER", thread_handle_barrier);

  /* Handle message passing commands. */
  HANDLE("SEND", thread_handle_send);
  HANDLE("BROADCAST", thread_handle_broadcast);

  /* Handle finish commands. */
  HANDLE("CLOSE", thread_handle_close);

  return rc;
}

void thread_main(void* arg) {
  char buffer[MAX_STRING_INPUT_SIZE];
  char* line = NULL;
  int len;
  int my_index;
  int line_num;
  int rc = 0;
  int finished = 0;

  /* First thing we register ourselves. */
  my_index = add_myself_to_threads();
  if (my_index < 0) {
    printf("ERROR: thread %d couldn't be registered!\n", (int)pthread_self());
    pthread_exit((void*)-1);
    return;
  }

  /* Then we regiter ourselves to the context. */
  mp_register(main_context);
  printf("[Thread %d]: Registered (Thread ID = %d)\n",
         my_index + 1, (int)pthread_self());
  fflush(stdout);
  
  /* Then we hold at the start barrier. */
  mp_barrier(main_context, start_barrier);

  /* Now we can start the main loop, where we wait for commands and messages. */
  while (mp_recv(main_context, buffer, MAX_STRING_INPUT_SIZE,
                 &len, RECV_SYNC) == 0) {
    if (strstr(buffer, "<COMMAND") == buffer) {
      /* Handle this message as a command. */

      /* Find the actual command, and extract the line number. */
      line = strchr(buffer, '>');
      if (line == NULL) {
        printf("[Thread %d]: ERROR [line %d]: Line doesn't have command prefix!\n",
               my_index + 1, line_num);
        fflush(stdout);
        pthread_exit((void*)ERROR_INVALID_LINE);
        return;
      }
      ++line;
      sscanf(buffer, "<COMMAND %d>", &line_num);

      /* Do the actual command and handle special return codes. */
      rc = thread_handle_command(my_index, line, line_num);
      if (rc == ERROR_INVALID_LINE) {
        printf("[Thread %d]: ERROR [line %d]: Invalid Line!\n",
               my_index + 1, line_num);
        fflush(stdout);
        pthread_exit((void*)ERROR_INVALID_LINE);
        return;
      }
      if (rc == FINISH_THREAD) {
        finished = 1;
        break;
      }
    } else {
      /* Handle this message as a regular thread-to-thread message. */

      printf("[Thread %d]: Received Message (length = %d): '%s'\n",
             my_index + 1, len, buffer);
      fflush(stdout);

      /* Check if we got the finish message. */
      if (EQUALS(buffer, "FINISH")) {
        finished = 1;
        break;
      }
    }
  }

  /* If we havn't finished nicely, we had an mp_recv failure. */
  if (!finished) {
    printf("[Thread %d]: ERROR [line %d]: Receive Failed (Flags: SYNC)!\n",
           my_index + 1, line_num);
    fflush(stdout);
  }

  /* Wait at the finish barrier before ending. */
  mp_barrier(main_context, finish_barrier);

  /* Unregister ourselves before dying. */
  mp_unregister(main_context);
  printf("[Thread %d]: Unregistered (Thread ID = %d)\n",
         my_index + 1, (int)pthread_self());
  fflush(stdout);
  
  pthread_exit(0);
}


/*** Handling Commands in Main ***/

int hand_command_to_thread(int index, char* line, int line_num) {
  char buffer[MAX_STRING_INPUT_SIZE];
  pthread_t target_id = threads[index - 1]->tid;
  int buffer_len;

  /* Verify input. */
  --index;  /* index should be between 1 and num_threads in text. */
  if (index < 0 || index >= num_threads || threads[index] == NULL) {
    printf("ERROR [line %d]: Invalid thread index %d!\n", line_num, index + 1);
    fflush(stdout);
    return -1;
  }

  /* Create the command to send with our prefix. */
  buffer_len = snprintf(buffer, MAX_STRING_INPUT_SIZE, "<COMMAND %d>%s",
                        line_num, line) + 1;
  buffer[buffer_len - 1] = '\0';

  /* Send the command to the child. */
  if (mp_send(main_context, &target_id, buffer, buffer_len, SEND_SYNC) < 0) {
    printf("ERROR [line %d]: Error while sending command to thread %d!\n",
           line_num, index + 1);
    fflush(stdout);
    return -1;
  }

  /* If this is the CLOSE command, return the FINISH_THREAD status. */
  if (EQUALS(line, "CLOSE")) {
    return FINISH_THREAD;
  }

  return 0;
}


/*** Parsing ***/

int parse(int line_num, char* line) {
  char* line_thread = NULL;
  char* new_line = NULL;
  int thread_index = 0;

  /* Throw away the terminating end-line if necessary. */
  new_line = strchr(line, '\n');
  if (new_line != NULL) {
    new_line[0] = '\0';
  }

  /* Check if this line is meant for a child process. */
  if (isdigit(line[0])) {
    /* Find the space character, and from it the line for the thread. */
    line_thread = strchr(line, ' ');
    if (line_thread == NULL) {
      printf("ERROR [line %d]: Line doesn't have a space as required!\n", line_num);
      return ERROR_INVALID_LINE;
    }
    line_thread[0] = '\0';
    line_thread++;

    /* Now get the thread index and hand the command to it. */
    if (sscanf(line, "%d", &thread_index) != 1) {
      printf("ERROR [line %d]: Line doesn't start with thread number!\n", line_num);
      return ERROR_INVALID_LINE;
    }

    /* Finally, let the thread have it. */
    return hand_command_to_thread(thread_index, line_thread, line_num);
  } else if (EQUALS(line, "CLOSE")) {
    /* In this case we simply hand the close over to the first thread. */
    return hand_command_to_thread(1, line, line_num);
  } else {
    printf("ERROR [line %d]: Line doesn't start with thread number!\n", line_num);
    return ERROR_INVALID_LINE;
  }
}


/*** Main ***/

int do_work() {
  int requested_num_threads = DEFAULT_NUM_THREADS;
  char buffer[MAX_STRING_INPUT_SIZE];
  char message_buffer[MAX_STRING_INPUT_SIZE];
  int message_len;
  pthread_t thread_id;
  int i;
  int rc = 0;

  /* Read from the input the number of threads to manage. */
  if (fgets(buffer, MAX_STRING_INPUT_SIZE, stdin) == NULL) {
    printf("ERROR [line 0]: Input error on INIT line!\n");
    return 1;
  }
  if (sscanf(buffer, "INIT %d", &requested_num_threads) != 1) {
    printf("ERROR [line 0]: First line is not INIT!\n");
    return 1;
  }
  if (requested_num_threads < 1 || requested_num_threads >= MAX_THREADS) {
    printf("ERROR [line 0]: Invalid number of threads on INIT!\n");
    return 1;
  }

  /* Initialize globals. */
  main_context = mp_init();
  start_barrier = mp_initbarrier(main_context, requested_num_threads + 1);
  finish_barrier = mp_initbarrier(main_context, requested_num_threads + 1);

  num_threads = 0;
  for (i = 0; i < MAX_THREADS; ++i) {
    threads[i] = NULL;
  }

  /* Register ourselves at the context. */
  mp_register(main_context);
  printf("Main Thread Regsitered\n");

  /* Initialize all threads. */
  for (i = 0; i < requested_num_threads; ++i) {
    pthread_create(&thread_id, NULL, (void*)&thread_main, NULL);
  }

  /* Wait on a barrier for all threads. */
  mp_barrier(main_context, start_barrier);

  /* Start the command reading loop. */
  for (i = 1; fgets(buffer, MAX_STRING_INPUT_SIZE, stdin) != NULL; ++i) {
    /* Read up any pending messages. */
    while ((mp_recv(main_context, message_buffer, MAX_STRING_INPUT_SIZE,
            &message_len, 0) == 0) && (message_len != 0)) {};
    if (message_len != 0) {
      /* If we got here, then there was an error in the mp_recv function. */
      printf("ERROR [line %d]: Receive Failed (Flags: None)!\n", i);
      fflush(stdout);
      return 1;
    }

    /* Handle the command itself. */
    rc = parse(i, buffer);
    if (rc == ERROR_INVALID_LINE) {
      /* This means there was an unrecoverable error, so quit. */
      return 1;
    }
    if (rc == FINISH_THREAD) {
      /* This means we are done so break. */
      break;
    }
  }

  /* If we reach the end, wait for everyone to finish. */
  mp_barrier(main_context, finish_barrier);
  for (i = 0; i < requested_num_threads; ++i) {
    pthread_join(threads[i]->tid, NULL);
    remove_thread(i);
  }

  /* Now do some cleanup before exit. */
  mp_destroybarrier(main_context, start_barrier);
  mp_destroybarrier(main_context, finish_barrier);

  mp_unregister(main_context);
  printf("Main Thread Unregistered\n");

  mp_destroy(main_context);

  return 0;
}

int main(int argc, char* argv[]) {
  return do_work();
}
