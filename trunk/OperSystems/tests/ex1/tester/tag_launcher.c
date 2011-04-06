#include <errno.h>
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

  for (i = 1; i < argc; ++i) {
    argv[i-1] = argv[i];
  }
  argv[argc-1] = NULL;

  settag(getpid(), 0);

  rc = fork();
  if (rc == 0) {
    rc = execv(argv[0], argv);
  } else {
    printf("%d\n",rc);
    fflush(stdout);
  }

  return rc;
}
