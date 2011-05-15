#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mp_interface.h"

#define MAX_STRING_INPUT_SIZE 65536

#define UNKNOWN_COMMAND -10000
#define ILLEGAL_ARGS -10001
#define INDEX_OUT_OF_RANGE -10002
#define INIT_FAILED -10003
#define TOO_MANY_CONTEXTS -10010
#define TOO_MANY_BARRIERS -10011

#define MAX_CONTEXTS 50
#define MAX_BARRIERS 50

/*** Contexts ***/

int num_contexts;
context_t* contexts[MAX_CONTEXTS];

/*** Barriers ***/

int num_barriers;
barrier_t* barriers[MAX_BARRIERS];

/*** I/O ***/

FILE* out_pipe;
FILE* in_pipe;

/*** Handling Commands ***/

/*
 * Arguments:
 * [0] - Number of thread to do the init.
 */
int handle_context_init(char* arguments) {
  int tid = 0;

  if (!arguments || sscanf(arguments, "%d", &tid) != 1) {
    return ILLEGAL_ARGS;
  }

  if (num_contexts == MAX_CONTEXTS) {
    return TOO_MANY_CONTEXTS;
  }

  // TODO hand command to thread
  contexts[num_contexts] = mp_init();
  
  if (contexts[num_contexts] == NULL) {
    return INIT_FAILED;
  }
  ++num_contexts;
  return 0;
}

/*
 * Arguments:
 * [0] - Number of thread to do the register.
 * [1] - Number of context to register.
 */
int handle_context_register(char* arguments) {
  int rc;
  int tid = 0;
  int context = 0;

  if (!arguments || sscanf(arguments, "%d %d", &tid, &context) != 2) {
    return ILLEGAL_ARGS;
  }

  if (context >= num_contexts) {
    return INDEX_OUT_OF_RANGE;
  }

  // TODO hand command to thread
  rc = mp_register(contexts[context]);
  
  return rc;
}

/*
 * Arguments:
 * [0] - Number of thread to do the unregister.
 * [1] - Number of context to unregister.
 */
int handle_context_unregister(char* arguments) {
  int tid = 0;
  int context = 0;

  if (!arguments || sscanf(arguments, "%d %d", &tid, &context) != 2) {
    return ILLEGAL_ARGS;
  }

  if (context >= num_contexts) {
    return INDEX_OUT_OF_RANGE;
  }

  // TODO hand command to thread
  mp_unregister(contexts[context]);
  
  return 0;
}

/*
 * Arguments:
 * [0] - Number of thread to do the destroy.
 * [1] - Number of context to destroy.
 */
int handle_context_destroy(char* arguments) {
  int i;
  int tid = 0;
  int context = 0;

  if (!arguments || sscanf(arguments, "%d %d", &tid, &context) != 2) {
    return ILLEGAL_ARGS;
  }

  if (context < 0 || context >= num_contexts) {
    return INDEX_OUT_OF_RANGE;
  }

  // TODO hand command to thread
  mp_destroy(contexts[context]);
  for (i = context; i < num_contexts - 1; ++i) {
    contexts[i] = contexts[i+1];
  }
  --num_contexts;
  contexts[num_contexts] = NULL;
  
  return 0;
}

/*
 * Arguments:
 * [0] - Number of thread to do the init.
 * [1] - Number of context to pass to the barrier.
 * [2] - N to pass to the barrier.
 */
int handle_barrier_init(char* arguments) {
  int tid = 0, context = 0, n = 0;

  if (!arguments || sscanf(arguments, "%d %d %d", &tid, &context, &n) != 3) {
    return ILLEGAL_ARGS;
  }

  if (num_barriers == MAX_BARRIERS) {
    return TOO_MANY_BARRIERS;
  }

  // TODO hand command to thread
  barriers[num_barriers] = mp_initbarrier(contexts[context], n);
  
  if (barriers[num_barriers] == NULL) {
    return INIT_FAILED;
  }
  ++num_barriers;
  return 0;
}

/*
 * Arguments:
 * [0] - Number of thread to do the destroy.
 * [1] - Number of context to pass to the barrier.
 * [2] - Number of barrier to destroy.
 */
int handle_barrier_destroy(char* arguments) {
  int i;
  int tid = 0, context = 0, barrier = 0;

  if (!arguments || sscanf(arguments, "%d %d %d", &tid, &context, &barrier) != 3) {
    return ILLEGAL_ARGS;
  }

  if (context < 0 || context >= num_contexts || barrier < 0 || barrier >= num_barriers) {
    return INDEX_OUT_OF_RANGE;
  }

  // TODO hand command to thread
  mp_destroybarrier(contexts[context], barriers[barrier]);
  for (i = barrier; i < num_barriers - 1; ++i) {
    barriers[i] = barriers[i+1];
  }
  --num_barriers;
  barriers[num_barriers] = NULL;
  
  return 0;
}

/*
 * Arguments:
 * [0] - Number of thread.
 * [1] - Number of context to pass to the barrier.
 * [2] - Number of barrier.
 */
int handle_barrier_wait(char* arguments) {
  int rc = 0, tid = 0, context = 0, barrier = 0;

  if (!arguments || sscanf(arguments, "%d %d %d", &tid, &context, &barrier) != 3) {
    return ILLEGAL_ARGS;
  }

  if (context < 0 || context >= num_contexts || barrier < 0 || barrier >= num_barriers) {
    return INDEX_OUT_OF_RANGE;
  }

  // TODO hand command to thread
  rc = mp_barrier(contexts[context], barriers[barrier]);
  
  return rc;
}

int handle_close(char* arguments) {
  // TODO close threads
  
  fprintf(out_pipe, "CLOSED 0\n");
  fflush(out_pipe);
  exit(0);
  return 0;
}

#define EQUALS(str1,str2) (strcmp((str1),(str2)) == 0)
#define HANDLE(cmd, fn) if (EQUALS((line), (cmd))) { rc = fn(arguments); }
#define HANDLE_NO_OUTPUT(cmd, fn) if (EQUALS((line), (cmd))) { return fn(arguments); }

int handle_command(char* line) {
  int rc = UNKNOWN_COMMAND;
  char* arguments = strchr(line, ' ');
  if (arguments != NULL) {
    arguments[0] = '\0';
    arguments++;
  }

  /* Context commands */
  HANDLE("INIT_CONTEXT", handle_context_init);
  HANDLE("REGISTER_CONTEXT", handle_context_register);
  HANDLE("UNREGISTER_CONTEXT", handle_context_unregister);
  HANDLE("DESTROY_CONTEXT", handle_context_destroy);
  /* Barrier commands */
  HANDLE("INIT_BARRIER", handle_barrier_init);
  HANDLE("DESTROY_BARRIER", handle_barrier_destroy);
  HANDLE("WAIT_BARRIER", handle_barrier_wait);
  /* Communication commands */
  //HANDLE("SEND", handle_send);
  //HANDLE("BROADCAST", handle_broadcast);
  //HANDLE("RECEIVE", handle_receive);
  /* Close */
  HANDLE_NO_OUTPUT("CLOSE", handle_close);

  switch (rc) {
    case UNKNOWN_COMMAND:
      fprintf(out_pipe, "Unknown command\n");
      break;
    case ILLEGAL_ARGS:
      fprintf(out_pipe, "Illegal args\n");
      break;
    case INDEX_OUT_OF_RANGE:
      fprintf(out_pipe, "Index out of range\n");
      break;
    case INIT_FAILED:
      fprintf(out_pipe, "Init failed\n");
      break;
    case TOO_MANY_CONTEXTS:
      fprintf(out_pipe, "Max number of contexts reached\n");
      break;
    case TOO_MANY_BARRIERS:
      fprintf(out_pipe, "Max number of barriers reached\n");
      break;
    default:
      fprintf(out_pipe, "DONE %d\n", rc);
      break;
  }
  fflush(out_pipe);

  return rc;
}


/*** Parsing ***/

int parse(char* line) {
  char* newline = NULL;

  newline = strchr(line, '\n');
  if (newline != NULL) {
    newline[0] = '\0';
  }
  return handle_command(line);
}


/*** Main ***/

int do_work() {
  int i;
  char buffer[MAX_STRING_INPUT_SIZE];

  num_contexts = 0;
  for (i = 0; i < MAX_CONTEXTS; ++i) {
    contexts[i] = NULL;
  }

  num_barriers = 0;
  for (i = 0; i < MAX_BARRIERS; ++i) {
    barriers[i] = NULL;
  }

  /* Start the command reading loop. */
  while ( fgets(buffer, MAX_STRING_INPUT_SIZE, in_pipe) != NULL ) {
    parse(buffer);
  }

  /* If we reach the end, close all the open streams. */
  fclose(in_pipe);
  fclose(out_pipe);

  return 0;
}

int main(int argc, char* argv[]) {
  if (argc >= 3) {
    in_pipe = fopen(argv[1], "r");
    out_pipe = fopen(argv[2], "w");
  } else {
    in_pipe = stdin;
    out_pipe = stdout;
  }

  return do_work();
}
