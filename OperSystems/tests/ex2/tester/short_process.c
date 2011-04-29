#include <sched.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "hw2_syscalls.h"


#define MAX_STRING_INPUT_SIZE 65536

#define CHILD_EXECUTABLE "./short_process"

#define MAX_CHILDREN 1000


/*** Process Childs ***/

typedef struct {
  int index;
  int pid;
  FILE* out_pipe;
  FILE* in_pipe;
} ProcessChild;

ProcessChild* children[MAX_CHILDREN];
int num_children;

FILE* out_pipe;
FILE* in_pipe;

int add_child_process(int pid, FILE* child_in_pipe, FILE* child_out_pipe) {
  if (num_children == MAX_CHILDREN) {
    return -1;
  }

  children[num_children] = (ProcessChild*)malloc(sizeof(ProcessChild));
  children[num_children]->index = num_children;
  children[num_children]->pid = pid;
  children[num_children]->in_pipe = child_in_pipe;
  children[num_children]->out_pipe = child_out_pipe;
  num_children++;
  return num_children;
}

int remove_child_process(int index) {
  int i;
  ProcessChild* to_delete;

  if (index < 0 || index >= MAX_CHILDREN) {
    return -1;
  }
  to_delete = children[index];

  for (i = num_children - 1; i > index; --i) {
    children[i-1] = children[i];
  }
  pclose(to_delete->in_pipe);
  pclose(to_delete->out_pipe);
  waitpid(to_delete->pid, NULL, 0);
  free(to_delete);

  num_children--;
  children[num_children] = NULL;
  return num_children;
}


/*** Handling Commands ***/

int handle_create_child(char* arguments) {
  char buffer[MAX_STRING_INPUT_SIZE];
  int pid = 0;
  FILE* child_in_pipe = NULL;
  FILE* child_out_pipe = NULL;

  /* Create the child pipes. */
  int in_pipe_fd[2];
  int out_pipe_fd[2];
  if (pipe(in_pipe_fd) < 0 || pipe(out_pipe_fd) < 0) {
    return -1;
  }

  /* Create the new process. */
  pid = fork();
  if (pid < 0) {
    return -1;
  }
  if (pid == 0) {
    /* If this is the child process, set everything up and go to do_work. */
    in_pipe = fdopen(in_pipe_fd[0], "r");
    out_pipe = fdopen(out_pipe_fd[1], "w");
    close(in_pipe_fd[1]);
    close(out_pipe_fd[0]);
    do_work();
    /* This will block until we exit, but just in case... */
    exit(0);
  }

  /* Set up the child pipes. */
  child_in_pipe = fdopen(out_pipe_fd[0], "r");
  child_out_pipe = fdopen(in_pipe_fd[1], "w");
  close(out_pipe_fd[1]);
  close(in_pipe_fd[0]);
  
  /* Get the child process's PID. */
  if (fgets(buffer, MAX_STRING_INPUT_SIZE, child_in_pipe) == NULL) {
    return -1;
  }
  sscanf(buffer, "PID %d", &pid);
  
  /* Add the new process to the list. */
  add_child_process(pid, child_in_pipe, child_out_pipe);

  return pid;
}

int handle_remaining_time(char* arguments) {
  int pid = 0;
  int remaining_time = 0;

  if (sscanf(arguments, "%d", &pid) != 1) {
    return -10000;
  }

  remaining_time = short_query_remaining_time(pid);
  
  if (remaining_time < 0) {
    return -errno;
  }
  return remaining_time;
}

int handle_overdue_time(char* arguments) {
  int pid = 0;
  int overdue_time = 0;

  if (sscanf(arguments, "%d", &pid) != 1) {
    return -10000;
  }
  
  overdue_time = short_query_overdue_time(pid);
  
  if (overdue_time < 0) {
    return -errno;
  }
  return overdue_time;
}

int handle_get_scheduler(char* arguments) {
  int pid = 0;
  int policy = 0;

  if (sscanf(arguments, "%d", &pid) != 1) {
    return -10000;
  }

  policy = sched_getscheduler(pid);
  
  if (policy < 0) {
    return -errno;
  }
  return policy;
}

int handle_set_short(char* arguments) {
  int pid = 0, requested_time = 0;
  int rc = 0;
  struct sched_param params;

  if (sscanf(arguments, "%d %d", &pid, &requested_time) != 2) {
    return -10000;
  }
  
  params.sched_priority = requested_time;
  rc = sched_setscheduler(pid, 4, &params);
  if (rc < 0) {
    return -errno;
  }
  return rc;
}

int handle_do_work(char* arguments) {
  int work_time;
  int start_time, current_time;
  int a = 0, b = 1, c, i;

  if (sscanf(arguments, "%d", &work_time) != 1) {
    return -10000;
  }
  work_time = work_time * CLOCKS_PER_SEC / 1000;
  
  start_time = current_time = clock();
  for (i = 0; (current_time - start_time) < work_time; ++i) {
    c = a + b;
    a = b;
    b = c;
    
    if (i % 1000000 == 0) {
      current_time = clock();
    }
  }
  
  return ((current_time - start_time) * 1000 / CLOCKS_PER_SEC);
}

int handle_stats(char* arguments) {
  struct switch_info stats[150];
  int num, i;

  num = get_scheduler_statistic((struct switch_info*)stats);

  for (i = 0; i < num; ++i) {
    fprintf(out_pipe, "(%d,%d,%d,%d,%ul,%d);");
  }
  
  return 0;
}

int handle_close(char* arguments) {
  int i;
  for (i = num_children - 1; i >= 0; --i) {
    hand_command_to_child(i, "CLOSE");
  }
  
  fprintf(out_pipe, "CLOSED 0\n");
  fflush(out_pipe);
  exit(0);
  return 0;
}

int hand_command_to_child(int index, char* line) {
  char buffer[MAX_STRING_INPUT_SIZE];

  if (index < 0 || index >= num_children || children[index] == NULL) {
    return -1;
  }

  fprintf(children[index]->out_pipe, "%s", line);
  fflush(children[index]->out_pipe);
  
  if (strncmp(line, "CLOSE", 5) == 0) {
    remove_child_process(index);
    fprintf(out_pipe, "CLOSED 0\n");
    fflush(out_pipe);
    return 0;
  }

  if (fgets(buffer, MAX_STRING_INPUT_SIZE, children[index]->in_pipe) == NULL) {
    return -1;
  }
  fprintf(out_pipe, "%s", buffer);
  fflush(out_pipe);

  return 0;
}

#define EQUALS(str1,str2) (strcmp((str1),(str2)) == 0)
#define HANDLE(cmd, fn) if (EQUALS((line), (cmd))) { rc = fn(arguments); }

int handle_command(char* line) {
  int rc = -10000;
  int async = 0;
  char* arguments = strchr(line, ' ');
  if (arguments != NULL) {
    arguments[0] = '\0';
    arguments++;
  }

  if (EQUALS(line, "ASYNC")) {
    async = 1;
    line = arguments;
    arguments = strchr(line, ' ');
    if (arguments != NULL) {
      arguments[0] = '\0';
      arguments++;
    }

    fprintf(out_pipe, "DONE 0\n");
    fflush(out_pipe);
  } 

  HANDLE("CREATE_CHILD", handle_create_child);
  HANDLE("REMAINING_TIME", handle_remaining_time);
  HANDLE("OVERDUE_TIME", handle_overdue_time);
  HANDLE("GET_POLICY", handle_get_scheduler);
  HANDLE("SET_SHORT", handle_set_short);
  HANDLE("DO_WORK", handle_do_work);
  HANDLE("STATS", handle_stats);
  if (EQUALS(line, "CLOSE")) {
    return handle_close(arguments);
  }

  if (!async) {
    fprintf(out_pipe, "DONE %d\n", rc);
    fflush(out_pipe);
  }

  return rc;
}


/*** Parsing ***/

int parse(char* line) {
  char* line_child = NULL;
  char* newline = NULL;
  int child_index = 0;

  /* Check if this line is meant for a child process. */
  if (isdigit(line[0])) {
    /* Find the space character. */
    line_child = strchr(line, '/');
    if (line_child == NULL) {
      line_child = strchr(line, ' ');
    }
    line_child[0] = '\0';
    line_child++;

    /* Now get the child process index and hand the command to it. */
    child_index = atoi(line);
    return hand_command_to_child(child_index, line_child);
  }

  /* We need to handle this command ourselves. */
  newline = strchr(line, '\n');
  if (newline != NULL) {
    newline[0] = '\0';
  }
  return handle_command(line);
}


/*** Main ***/

int do_work() {
  char buffer[MAX_STRING_INPUT_SIZE];
  int i;

  /* Initialize globals. */
  num_children = 0;
  for (i = 0; i < MAX_CHILDREN; ++i) {
    children[i] = NULL;
  }

  /* Print our PID first. */
  fprintf(out_pipe, "PID %d\n", getpid());
  fflush(out_pipe);

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
