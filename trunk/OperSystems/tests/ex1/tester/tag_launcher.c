#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "syscall_tags.h"

int main(int argc, char* argv[]) {
  char** child_argv;
  int i;
  int rc;

  if (argc < 2) {
    printf("No executable given!\n");
    printf("Usage: tag_launcher <executable> [<extra params>]\n");
    return 0;
  }

  child_argv = (char**)malloc(sizeof(char*)*(argc-1));
  for (i = 1; i < argc; ++i) {
    child_argv[i-1] = argv[i];
  }

  settag(getpid(), 0);

  if (fork() == 0) {
    rc = execv("./tag_looper", child_argv);
  }
  free(child_argv);

  return rc;
}
