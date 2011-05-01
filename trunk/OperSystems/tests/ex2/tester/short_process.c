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

char* async_trap_file;

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

int handle_set_async_trap(char* arguments) {
  FILE* async_trap = fopen(async_trap_file, "w");
  fclose(async_trap);
  return 0;
}

int handle_print_async_trap(char* arguments) {
  int num_of_calls = 0, i = 0;
  char buffer[MAX_STRING_INPUT_SIZE];
  int buffer_size = 0;
  FILE* async_trap;

  if (!arguments || sscanf(arguments, "%d", &num_of_calls) != 1) {
    return -10000;
  }
  
  while (i < num_of_calls) {
    async_trap = fopen(async_trap_file, "r");
    buffer_size = 0;
    for (i = 0; i < num_of_calls; ++i) {
      if (i > 0) {
        buffer[buffer_size] = '\n';
        ++buffer_size;
      }
      if (!fgets(buffer + buffer_size, MAX_STRING_INPUT_SIZE, async_trap)) {
        sleep(1);
        break;
      }
      buffer_size = strlen(buffer);
    }
    fclose(async_trap);
  }

  if (buffer_size > 0) {
    fprintf(out_pipe, buffer);
    fflush(out_pipe);
  }
  
  return num_of_calls;
}

int handle_remaining_time(char* arguments) {
  int pid = 0;
  int remaining_time = 0;

  if (!arguments || sscanf(arguments, "%d", &pid) != 1) {
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

  if (!arguments || sscanf(arguments, "%d", &pid) != 1) {
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

  if (!arguments || sscanf(arguments, "%d", &pid) != 1) {
    return -10000;
  }

  policy = sched_getscheduler(pid);
  
  if (policy < 0) {
    return -errno;
  }
  return policy;
}

int handle_get_param(char* arguments) {
  int pid = 0;
  struct sched_param param; 

  if (!arguments || sscanf(arguments, "%d", &pid) != 1) {
    return -10000;
  }

  if (sched_getparam(pid, &param) < 0) {
    return -errno;
  }
  return param.sched_priority;
}

int handle_set_param(char* arguments) {
  int pid = 0, requested_time = 0;
  int rc = 0;
  struct sched_param params;

  if (!arguments || sscanf(arguments, "%d %d", &pid, &requested_time) != 2) {
    return -10000;
  }
  
  params.sched_priority = requested_time;
  rc = sched_setparam(pid, &params);
  if (rc < 0) {
    return -errno;
  }
  return rc;
}

int handle_set_short(char* arguments) {
  int pid = 0, requested_time = 0;
  int rc = 0;
  struct sched_param params;

  if (!arguments || sscanf(arguments, "%d %d", &pid, &requested_time) != 2) {
    return -10000;
  }
  
  params.sched_priority = requested_time;
  rc = sched_setscheduler(pid, 4, &params);
  if (rc < 0) {
    return -errno;
  }
  return rc;
}

int handle_set_scheduler(char* arguments) {
  int pid = 0, policy = 0, requested_time = 0;
  int rc = 0;
  struct sched_param params;

  if (!arguments || sscanf(arguments, "%d %d %d",
                           &pid, &policy, &requested_time) != 3) {
    return -10000;
  }
  
  params.sched_priority = requested_time;
  rc = sched_setscheduler(pid, policy, &params);
  if (rc < 0) {
    return -errno;
  }
  return rc;
}

int handle_do_work(char* arguments) {
  int work_time, start_time, current_time, temp_time;
  int is_short = 0, use_overdue = 0;
  int pid = getpid();
  int a = 0, b = 1, c, i;

  if (!arguments || sscanf(arguments, "%d", &work_time) != 1) {
    return -10000;
  }
  
  if (sched_getscheduler(pid) == SCHED_SHORT) {
    is_short = 1;
    start_time = 0;
    temp_time = short_query_remaining_time(pid);
    if (temp_time > 0 && work_time <= temp_time) {
      start_time = -temp_time;
      work_time = -(temp_time - work_time);
    } else {
      start_time = -temp_time;
      work_time -= temp_time;
      temp_time = short_query_overdue_time(pid);
      start_time += temp_time;
      work_time += temp_time;
      use_overdue = 1;
    }
    current_time = start_time;
  } else {
    work_time = work_time * CLOCKS_PER_SEC / 1000;
    start_time = clock();
    work_time += start_time;
  }
  
  for (i = 0; current_time < work_time; ++i) {
    c = a + b;
    a = b;
    b = c;
    
    if (i % 1000 == 0) {
      if (!is_short) {
        current_time = clock();
      } else if (use_overdue) {
        current_time = short_query_overdue_time(pid);
      } else {
        current_time = -short_query_remaining_time(pid);
      }
    }
  }
  
  if (is_short) {
    return (current_time - start_time);
  }
  return ((current_time - start_time) * 1000 / CLOCKS_PER_SEC);
}

int handle_do_work_and_yield_at_halftime(char* arguments) {
  int work_time;
  int start_time, current_time, first;
  int a = 0, b = 1, c, i;
  first = 0;

  if (!arguments || sscanf(arguments, "%d", &work_time) != 1) {
    return -10000;
  }
  work_time = work_time * CLOCKS_PER_SEC / 1000;
  
  start_time = current_time = clock();
  for (i = 0; (current_time - start_time) < work_time; ++i) {
    if (((current_time - start_time) == (work_time / 2)) && first == 0) {
      first = 1;
      fprintf(out_pipe, "YIELDING %d\n", getpid());
      fflush(out_pipe);
      sched_yield();
    }
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

  fprintf(out_pipe, "DONE %d", num);
  for (i = 0; i < num; ++i) {
    fprintf(out_pipe, " [prev_pid=%d,next_pid=%d,prev_policy=%d,next_policy=%d,time=%ul,reason=%d]", stats[i].previous_pid, stats[i].next_pid, stats[i].previous_policy, stats[i].next_policy, stats[i].time, stats[i].reason);
  }
  fprintf(out_pipe, "\n");
  fflush(out_pipe);
  
  return num;
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
#define HANDLE_NO_OUTPUT(cmd, fn) if (EQUALS((line), (cmd))) { return fn(arguments); }

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
  HANDLE("SET_ASYNC_TRAP", handle_set_async_trap);
  HANDLE_NO_OUTPUT("PRINT_ASYNC_TRAP", handle_print_async_trap);
  HANDLE("REMAINING_TIME", handle_remaining_time);
  HANDLE("OVERDUE_TIME", handle_overdue_time);
  HANDLE("GET_POLICY", handle_get_scheduler);
  HANDLE("GET_PARAM", handle_get_param);
  HANDLE("SET_PARAM", handle_set_param);
  HANDLE("SET_SHORT", handle_set_short);
  HANDLE("SET_SCHEDULER", handle_set_scheduler);
  HANDLE("DO_WORK", handle_do_work);
  HANDLE("DO_WORK_AND_YIELD", handle_do_work_and_yield_at_halftime);
  HANDLE_NO_OUTPUT("GET_STATS", handle_stats);
  HANDLE_NO_OUTPUT("CLOSE", handle_close);

  if (!async) {
    fprintf(out_pipe, "DONE %d\n", rc);
    fflush(out_pipe);
  } else {
    FILE* async_trap = fopen(async_trap_file, "a");
    fprintf(async_trap, "%d DONE %d\n", getpid(), rc);
    fflush(async_trap);
    fclose(async_trap);
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
  async_trap_file = NULL;

  if (argc >= 3) {
    in_pipe = fopen(argv[1], "r");
    out_pipe = fopen(argv[2], "w");
  } else {
    in_pipe = stdin;
    out_pipe = stdout;
  }

  async_trap_file = mktemp("/tmp/async_XXXXXX.trap");

  int rc = do_work();

  unlink(async_trap_file);

  return rc;
}
