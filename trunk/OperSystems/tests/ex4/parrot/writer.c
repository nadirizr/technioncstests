#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "vsf.h"

int main(int argc, char** argv) {
	int fd, res;

	if (argc > 3) {
		fprintf(stderr, "wrong parameters\n");
		return 1;
	}
	fd = open(argv[1], O_WRONLY);
	if (fd < 0) {
		return errno;
	}
	res = write(fd, argv[2], strlen(argv[2]));
	if (res < 0) {
		return errno;
	}
	return 0;
}
