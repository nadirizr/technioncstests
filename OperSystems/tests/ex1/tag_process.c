#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "syscall_tags.h"


#define MAX_STRING_INPUT_SIZE 256

#define CHILD_EXECUTABLE "./tag_process"

#define MAX_GOOD_PROCESSES 1000

#define MAX_CHILDREN 100


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
  free(to_delete);

  num_children--;
  children[num_children] = NULL;
  return num_children;
}


/*** Handling Commands ***/

int handle_create_child(char* arguments) {
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
    /* This will block until we exit. */
  }

  /* Set up the child pipes. */
  child_in_pipe = fdopen(out_pipe_fd[0], "r");
  child_out_pipe = fdopen(in_pipe_fd[1], "w");
  close(out_pipe_fd[1]);
  close(in_pipe_fd[0]);
  
  /* Get the child process's PID. */
  fscanf(child_in_pipe, "PID %d", &pid);
  
  /* Add the new process to the list. */
  add_child_process(pid, child_in_pipe, child_out_pipe);

  return pid;
}

int handle_get_tag(char* arguments) {
  int arg = atoi(arguments);
  int tag = 0;

  tag = gettag(arg);
  
  if (tag < 0) {
    if (errno == ESRCH) {
      return -ESRCH;
    }
    if (errno == EINVAL) {
      return -EINVAL;
    }
  }
  return tag;
}

int handle_set_tag(char* arguments) {
  int arg1 = 0, arg2 = 0;
  int rc = 0;

  char* argument2 = strchr(arguments, ' ');
  if (argument2 != NULL) {
    argument2[0] = '\0';
    argument2++;
  }
  arg1 = atoi(arguments);
  arg2 = atoi(argument2);

  rc = settag(arg1, arg2);
  
  if (rc < 0) {
    if (errno == ESRCH) {
      return -ESRCH;
    }
    if (errno == EINVAL) {
      return -EINVAL;
    }
  }
  return rc;
}

int handle_get_good_processes(char* arguments) {
  int array[MAX_GOOD_PROCESSES] = {0};
  int arg = atoi(arguments);
  int rc = 0;

  if (arg < 1 || arg >= MAX_GOOD_PROCESSES) {
    return -1;
  }

  rc = getgoodprocesses(array, arg);

  if (rc < 0) {
    if (errno == ESRCH) {
      return -ESRCH;
    }
    if (errno == EINVAL) {
      return -EINVAL;
    }
  }
  return rc;
}

int handle_make_good_processes(char* arguments) {
  int num_checked = makegoodprocesses();

  if (num_checked < 0) {
    if (errno == ESRCH) {
      return -ESRCH;
    }
    if (errno == EINVAL) {
      return -EINVAL;
    }
  }
  return num_checked;
}

int handle_close(char* arguments) {
  int i;
  for (i = num_children - 1; i >= 0; --i) {
    hand_command_to_child(i, "CLOSE");
    remove_child_process(i);
  }
  return 0;
}

int hand_command_to_child(int index, char* line) {
  char buffer[MAX_STRING_INPUT_SIZE];

  if (index < 0 || index >= num_children || children[index] == NULL) {
    return -1;
  }

  fprintf(children[index]->out_pipe, "%s", line);
  if (fgets(buffer, MAX_STRING_INPUT_SIZE, children[index]->in_pipe) == NULL) {
    return -1;
  }
  fprintf(out_pipe, "%s", buffer);
  fflush(out_pipe);

  if (strncmp(buffer, "CLOSED", 6) == 0) {
    remove_child_process(index);
  }

  return 0;
}


int handle_command(char* line) {
  int rc = -1;
  char* arguments = strchr(line, ' ');
  if (arguments != NULL) {
    arguments[0] = '\0';
    arguments++;
  }

  if (strcmp(line, "CREATE_CHILD") == 0) {
    rc = handle_create_child(arguments);
  }
  if (strcmp(line, "GET_TAG") == 0) {
    rc = handle_get_tag(arguments);
  }
  if (strcmp(line, "SET_TAG") == 0) {
    rc = handle_set_tag(arguments);
  }
  if (strcmp(line, "GET_GOOD_PROCESSES") == 0) {
    rc = handle_get_good_processes(arguments);
  }
  if (strcmp(line, "MAKE_GOOD_PROCESSES") == 0) {
    rc = handle_make_good_processes(arguments);
  }
  if (strcmp(line, "CLOSE") == 0) {
    rc = handle_close(arguments);
    fprintf(out_pipe, "CLOSED %d\n", rc);
    fflush(out_pipe);
    exit(0);
  }
  fprintf(out_pipe, "DONE %d\n", rc);
  fflush(out_pipe);

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
    if (parse(buffer) < 0) {
      break;
    }
  }

  return 0;
}

int main() {
  in_pipe = stdin;
  out_pipe = stdout;

  return do_work();
}
