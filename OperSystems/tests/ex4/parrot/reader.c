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
	int fd, res, cur = 1, j ;
	static char buf[4096];

	if (argc > 4) {
		fprintf(stderr, "wrong parameters\n");
		return 1;
	}
	fd = open(argv[cur++], O_RDONLY);
	if (fd < 0) {
		return errno;
	}
	if (argc < 3)
		return 0;
	res = read(fd, buf, 4096);
	if (res <= 0) {
		printf("error while reading, errno is %d.\n", errno);
		return errno;
	}

	if (argc > 2) {
		int t = strncmp(argv[cur], buf, res);
		if (t) {
			printf("read failed. strings '%s', '%.*s' do not match\n", cur, res, buf);
			return 200;
		}
	}
	return 0;
}
