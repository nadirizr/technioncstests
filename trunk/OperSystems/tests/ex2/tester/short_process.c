#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


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

/*
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
  int i;

  rc = getgoodprocesses(array, arg);

  if (rc < 0) {
    if (errno == ESRCH) {
      rc = -ESRCH;
    }
    if (errno == EINVAL) {
      rc = -EINVAL;
    }
    fprintf(out_pipe, "DONE %d\n", rc);
    fflush(out_pipe);
    return rc;
  }

  fprintf(out_pipe, "DONE %d", rc);
  for (i = 0; i < rc; ++i) {
    fprintf(out_pipe, " %d", array[i]);
  }
  fprintf(out_pipe, "\n");
  fflush(out_pipe);
  
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
*/

int handle_set_short(char* arguments) {

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

#define EQUALS(str1,str2) (strcmp(str1,str2) == 0)
#define HANDLE(cmd, fn) if (EQUALS(line, cmd)) { rc = fn(arguments); }

int handle_command(char* line) {
  int rc = -1;
  char* arguments = strchr(line, ' ');
  if (arguments != NULL) {
    arguments[0] = '\0';
    arguments++;
  }

  /*
  HANDLE("CREATE_CHILD", handle_create_child);
  HANDLE("GET_TAG", handle_get_tag);
  HANDLE("SET_TAG", handle_set_tag);
  HANDLE("GET_GOOD_PROCESSES", handle_get_good_processes);
  HANDLE("MAKE_GOOD_PROCESSES", handle_make_good_processes);
  */
  HANDLE("SET_SHORT", handle_set_short);
  if (EQUALS(line, "CLOSE")) {
    return handle_close(arguments);
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
