#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <device name> <flags>\n", argv[0]);
    return 1;
  }

  int fd = open(argv[1], atoi(argv[2]));
  printf("%d\n", (fd == -1 ? errno : 0));
  if (fd != -1) {
    close(fd);
  }
  return 0;
}
