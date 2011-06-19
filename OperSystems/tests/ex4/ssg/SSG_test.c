#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#include "vsf.h"

#define check(s,b) check_func(s,b,__FILE__,__LINE__)

typedef struct vsf_command_parameters param;
void check_func(const char* msg, int cond, const char* file, int line);
int message_transaction_reader_first(const char* msg, int read_fd,
		int read_size, int write_fd, int write_size);
int message_transaction_writer_first(const char* msg, int read_fd,
		int read_size, int write_fd, int write_size);

int number_of_fails = 0;

int main() {
	int retval;
	char buf[5];
	int ctr_fd, read_fd1, read_fd2, read_fd3;
	int write_fd1, write_fd2, write_fd3, write_fd4;
	param vsf1, vsf2, vsf3;
	vsf1.read_minor = 1;
	vsf1.write_minor = 11;
	vsf2.read_minor = 2;
	vsf2.write_minor = 12;
	vsf3.read_minor = 3;
	vsf3.write_minor = 13;

	printf("\nStarting Test...\n\n");
	printf("Opening the controller\n");
	ctr_fd = open("/root/vsf_controller", O_RDONLY);
	if (ctr_fd == -1) {
		printf("\tunable to open the controller, exiting test\n");
		return 1;
	} else {
		printf("\tctrlr opened successfully\n\n");
	}

	check("trying to read with the controller", read(ctr_fd, buf, 5) < 0);
	check("ioctl creating legal vsf", ioctl(ctr_fd, VSF_CREATE, &vsf1) == 0);
	check("ioctl creating vsf with already used minors", ioctl(ctr_fd, VSF_CREATE, &vsf1) != 0);
	check("ioctl creating legal vsf", ioctl(ctr_fd, VSF_CREATE, &vsf2) == 0);
	check("ioctl removing non-existing vsf", ioctl(ctr_fd, VSF_FREE, &vsf3) != 0);
	check("ioctl creating legal vsf", ioctl(ctr_fd, VSF_CREATE, &vsf3) == 0);
	check("ioctl removing existing vsf", ioctl(ctr_fd, VSF_FREE, &vsf3) == 0);
	check("opening existing vsf with legal flags", open("/root/vsf_reader_01",
			O_RDONLY) != -1);
	check("trying to open reading vsf with illegal flags", open(
			"/root/vsf_reader_02", O_WRONLY) == -1);
	read_fd1 = open("/root/vsf_reader_01", O_RDONLY);
	check("opening existing vsf with legal flags", read_fd1 != -1);

	check("trying ioctl with reader", ioctl(read_fd1, VSF_FREE, &vsf2) == -1);

	check("trying to open writing vsf with illegal flags", open(
			"/root/vsf_writer_11", O_RDONLY) == -1);
	write_fd1 = open("/root/vsf_writer_11", O_WRONLY);
	check("opening writer with legal flags", write_fd1 != -1);
	write_fd2 = open("/root/vsf_writer_12", O_WRONLY);
	check("opening writer with legal flags", write_fd2 != -1);
	check("Sending & receiving a message, reader first", message_transaction_reader_first(
			"Gal was here", read_fd1, 20, write_fd1, 13));
	check("Receiving a message with not enough reader space, reader first",
			message_transaction_reader_first("shaul wasn't here", read_fd1, 7,
					write_fd1, 18));

	check("Sending & receiving a message, writer first",
				message_transaction_writer_first("shaul wasn't here", read_fd1, 7,
						write_fd1, 18));
	check("Receiving a message with not enough reader space, writer first",
					message_transaction_writer_first("shaul wasn't here", read_fd1, 7,
							write_fd1, 18));

	if (number_of_fails == 0) {
		printf("\nAll tests passed successfully!\n");
	} else {
		printf("\nWarning: %d tests failed\n", number_of_fails);
	}
	return 0;
}

void check_func(const char* msg, int cond, const char* file, int line) {
	static int testnum = 0;
	testnum++;
	if (cond == 0) {
		printf("Test (%2d) FAIL: %s\n", testnum, msg);
		printf("\tTest failed in File: %s Line: %d\n", file, line);
		number_of_fails++;
		return;
	}
	printf("Test (%2d) PASS: %s\n", testnum, msg);
	return;
}

int message_transaction_reader_first(const char* msg, int read_fd,
		int read_size, int write_fd, int write_size) {
	int min_size = (read_size < write_size ? read_size : write_size);
	min_size = (min_size < 4096 ? min_size : 4096);
	fflush(stdout);
	fflush(stderr);
	pid_t reader_fork_result = fork();
	if (reader_fork_result == 0) {
		//reader
		char* buf = (char*) malloc(read_size);
		int bytes_sent = read(read_fd, buf, read_size);
		int retval =
				(memcmp(buf, msg, min_size) == 0 && bytes_sent == min_size);
		free(buf);
		exit(retval);
	}

	fflush(stdout);
	fflush(stderr);
	pid_t writer_fork_result = fork();
	if (writer_fork_result == 0) {
		//writer
		int bytes_sent = write(write_fd, msg, write_size);
		exit(bytes_sent == min_size);
	}

	int read_status, write_status;
	waitpid(reader_fork_result, &read_status, 0);
	waitpid(writer_fork_result, &write_status, 0);
	return WEXITSTATUS(read_status) && WEXITSTATUS(write_status);
}

int message_transaction_writer_first(const char* msg, int read_fd,
		int read_size, int write_fd, int write_size) {
	int min_size = (read_size < write_size ? read_size : write_size);
	min_size = (min_size < 4096 ? min_size : 4096);
	fflush(stdout);
	fflush(stderr);

	pid_t writer_fork_result = fork();
	if (writer_fork_result == 0) {
		//writer
		int bytes_sent = write(write_fd, msg, write_size);
		exit(bytes_sent == min_size);
	}

	fflush(stdout);
	fflush(stderr);

	pid_t reader_fork_result = fork();
	if (reader_fork_result == 0) {
		//reader
		char* buf = (char*) malloc(read_size);
		int bytes_sent = read(read_fd, buf, read_size);
		int retval =
				(memcmp(buf, msg, min_size) == 0 && bytes_sent == min_size);
		free(buf);
		exit(retval);
	}

	int read_status, write_status;
	waitpid(reader_fork_result, &read_status, 0);
	waitpid(writer_fork_result, &write_status, 0);
	return WEXITSTATUS(read_status) && WEXITSTATUS(write_status);
}
