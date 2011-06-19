#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  if (argc < 3 || argc > 4) {
    printf("Usage: %s <device name> <flags> [expected_driver_file]\n", argv[0]);
    return 1;
  }

  int fd = open(argv[1], atoi(argv[2]));
  printf("%d\n", (fd == -1 ? errno : 0));

  if (argc == 4) {
    char buff[100];
    sprintf(buff, "./check_driver.py %s", argv[3]);
    system(buff);
  }
  
  if (fd != -1) {
    close(fd);
  }

  return 0;
}
