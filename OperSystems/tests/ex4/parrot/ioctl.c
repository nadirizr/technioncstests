#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <fcntl.h>
#include "vsf.h"

int main(int argc, char** argv) {
	int fd, res;
	const char* contname = "/dev/vsf_controller";
	int command = VSF_CREATE;
	int i=1, j=0;
	if (argc < 3) {
		fprintf(stderr, "Wrong parameters\n");
		fprintf(stderr, "Usage: %s [create | free] read_minor write_minor [cntroller_name] \n", argv[0]);
		return 1;
	}
	if (!strcmp(argv[i], "FREE") || !strcmp(argv[i], "free"))
		command = VSF_FREE;
	if (command == VSF_FREE || !strcmp(argv[i], "CREATE") || !strcmp(argv[i], "create"))
		i++;
	if ( argc > i + 2 )
		contname = argv[i+2];
	fd = open(contname, O_RDONLY);
	if (fd < 0)
		return errno;
	{
		struct vsf_command_parameters param = { atoi(argv[i]), atoi(argv[i+1]) };
		res = ioctl(fd, command, &param);
	}		
	if (res < 0)
		return errno;
	return 0;
}
