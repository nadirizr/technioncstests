#include "vsf.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
  struct vsf_command_parameters params;
  int fd;
  int rc;
  
  if (argc != 4) {
    printf("Usage: %s <command> <read minor> <write minor>\n", argv[0]);
    return 1;
  }

  if ((strcmp(argv[1], "C") != 0) && (strcmp(argv[1], "F") != 0)) {
    printf("ERROR: Command must be 'C' or 'F': '%s'\n", argv[1]);
    printf("Usage: %s <command> <read minor> <write minor>\n", argv[0]);
    return 1;
  }

  if (sscanf(argv[2], "%d", &(params.read_minor)) != 1) {
    printf("ERROR: The given read minor was not a number: '%s'\n", argv[2]);
    printf("Usage: %s <command> <read minor> <write minor>\n", argv[0]);
    return 1;
  }
  
  if (sscanf(argv[3], "%d", &(params.write_minor)) != 1) {
    printf("ERROR: The given write minor was not a number: '%s'\n", argv[3]);
    printf("Usage: %s <command> <read minor> <write minor>\n", argv[0]);
    return 1;
  }
  
  if (params.read_minor < 1 || params.read_minor > 255 ||
      params.write_minor < 1 || params.write_minor > 255) {
    printf("ERROR: Minors must be in range [1,255]\n");
    printf("Usage: %s <command> <read minor> <write minor>\n", argv[0]);
    return 1;
  }
  
  fd = open("vsf_cntrl", O_RDONLY);
  if (strcmp(argv[1], "C") == 0) {
    rc = ioctl(fd, VSF_CREATE, (unsigned long)(&params));
  
    if (rc == 0) {
      printf("Created VSF for [read_minor = %d, write_minor = %d]\n",
             params.read_minor, params.write_minor);
    } else {
      printf("Failed to create VSF for [read_minor = %d, write_minor = %d]: RC = %d\n",
             params.read_minor, params.write_minor, rc);
    }
  } else if (strcmp(argv[1], "F") == 0) {
    rc = ioctl(fd, VSF_FREE, (unsigned long)(&params));
  
    if (rc == 0) {
      printf("Freed VSF for [read_minor = %d, write_minor = %d]\n",
             params.read_minor, params.write_minor);
    } else {
      printf("Failed to free VSF for [read_minor = %d, write_minor = %d]: RC = %d\n",
             params.read_minor, params.write_minor, rc);
    }
  }
  close(fd);
  
  return 0;
}
