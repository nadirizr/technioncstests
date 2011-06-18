/*
 * hw4_test.c
 *
 *  Created on: Jun 7, 2011
 *      Author: Roy
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/unistd.h>
#include <unistd.h>
#include <asm/errno.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include "vsf.h"

#define MAX_LENGTH 200
#define ONE_PAGE_BUFFER_LENGTH 4096

#define NUMBER_OF_DEVICES								7
#define NUMBER_OF_STRESSING_ITERATIONS					20

#define TEST_OUTPUT "out"
#define PATH_PREFIX "/dev/"
#define DELAY 20000

#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2

#define LOG_RESULT(message,expected,actual,counter,condition) 																\
	do {																													\
		char final_message[MAX_LENGTH];																						\
		char *final_result;																									\
		if(condition)																										\
		{																													\
			counter++;																										\
			final_result = "PASSED.";																						\
		}																													\
		else																												\
			final_result = "FAILED.";																						\
		if(actual >= 0)																										\
			sprintf(final_message,"%-80s Expected = %-30s, Actual = %-10d... %s\n",message,expected,actual,final_result);			\
		else																												\
		{																													\
			char errno_string[MAX_LENGTH];																					\
			convert_errno_to_string(errno_string);																			\
			sprintf(final_message,"%-80s Expected = %-30s, Actual = %-10s... %s\n",message,expected,errno_string,final_result);	\
		}																													\
																															\
		log_message(final_message);																							\
	} while(0)

#define LOG_MESSAGE(message) 								\
	do {													\
		char final_message[MAX_LENGTH];						\
		sprintf(final_message,"%s\n",message);				\
		log_message(final_message);							\
	} while(0)

#define PATH_BUILDER(path,filename)							\
	char path[100]; 										\
	sprintf(path,"%s%s",PATH_PREFIX,filename)

int CALCULATE_ITERATIONS(int size)
{
	int iterations = size / ONE_PAGE_BUFFER_LENGTH;
	if(size % ONE_PAGE_BUFFER_LENGTH != 0)
		iterations++;

	return iterations;
}

#define RUN_TEST(test,title,ok_message,fail_message)	\
	do {												\
		int status;										\
		int child_pid = fork();							\
		if(!child_pid) 									\
		{												\
			int result = test(); 						\
			while( wait(&status) != -1);				\
			return result;								\
		}												\
		else 											\
		{												\
			waitpid(child_pid,&status,0);				\
			if(WEXITSTATUS(status))						\
				printf("%s%s\n",title,ok_message);		\
			else										\
				printf("%s%s\n",title,fail_message);	\
		}												\
	} while(0)

#define OPEN_VSF(filename,fd,mode,expected,condition,passed_tests,total_tests)								\
	do {																						\
		PATH_BUILDER(path,filename);															\
		fd = open(path,mode);																	\
		char message[MAX_LENGTH];																\
		if(mode == O_WRONLY)																	\
			sprintf(message,"Trying to open a VSF named '%s', using O_WRONLY flag",filename);	\
		else if(mode == O_RDONLY)																\
			sprintf(message,"Trying to open a VSF named '%s', using O_RDONLY flag",filename);	\
		else																					\
			sprintf(message,"Trying to open a VSF named '%s', using other flags",filename);		\
		total_tests++;																			\
		LOG_RESULT(message,expected,fd,passed_tests,condition);									\
	} while(0)

#define CREATE_VSF(fd_controller,minor_for_read,minor_for_write,expected,condition,passed_tests,total_tests)									\
	do {																															\
		struct vsf_command_parameters bind_args =																					\
		{																															\
			.read_minor = minor_for_read,																							\
			.write_minor = minor_for_write,																							\
		};																															\
		int result = ioctl(fd_controller, VSF_CREATE, &bind_args);																	\
		char message[MAX_LENGTH];																									\
		sprintf(message,"Trying to create a VSF device on read_minor = %d and write_minor = %d",minor_for_read,minor_for_write);	\
		total_tests++;																												\
		LOG_RESULT(message,expected,result,passed_tests,condition);																	\
	} while(0)

#define FREE_VSF(fd_controller,minor_for_read,minor_for_write,expected,condition,passed_tests,total_tests)										\
	do {																															\
		struct vsf_command_parameters bind_args =																					\
		{																															\
			.read_minor = minor_for_read,																							\
			.write_minor = minor_for_write,																							\
		};																															\
		int result = ioctl(fd_controller, VSF_FREE, &bind_args);																	\
		char message[MAX_LENGTH];																									\
		sprintf(message,"Trying to delete a VSF device on read_minor = %d and write_minor = %d",minor_for_read,minor_for_write);	\
		total_tests++;																												\
		LOG_RESULT(message,expected,result,passed_tests,condition);																	\
	} while(0)

#define CREATE_VSF_NULL_PARAM(fd_controller,expected,condition,passed_tests,total_tests)												\
	do {																													\
		int result = ioctl(fd_controller, VSF_CREATE, NULL);																\
		char message[MAX_LENGTH];																							\
		sprintf(message,"Trying to create a VSF device with NULL command_parameters");										\
		total_tests++;																										\
		LOG_RESULT(message,expected,result,passed_tests,condition);															\
	} while(0)

#define CREATE_VSF_INVALID_ADDRESS(fd_controller,expected,condition,passed_tests,total_tests)												\
	do {																													\
		int result = ioctl(fd_controller, VSF_CREATE, (void *)4);																\
		char message[MAX_LENGTH];																							\
		sprintf(message,"Trying to create a VSF device with NULL command_parameters");										\
		total_tests++;																										\
		LOG_RESULT(message,expected,result,passed_tests,condition);															\
	} while(0)

#define FREE_VSF_NULL_PARAM(fd_controller,expected,condition,passed_tests,total_tests) 													\
	do {																													\
		int result = ioctl(fd_controller, VSF_FREE, NULL);																	\
		char message[MAX_LENGTH];																							\
		sprintf(message,"Trying to delete a VSF device with NULL command_parameters");										\
		total_tests++;																										\
		LOG_RESULT(message,expected,result,passed_tests,condition);															\
	} while(0)

#define VSF_LSEEK(fd,offset,whence,expected,condition,passed_tests,total_tests)								\
	do {																						\
		int result = lseek(fd,offset,whence);													\
		char message[MAX_LENGTH];																\
		sprintf(message,"Trying to call lseek() on file descriptor = %d",fd);					\
		total_tests++;																			\
		LOG_RESULT(message,expected,result,passed_tests,condition);								\
	} while(0)


#define READ_VSF(fd,size,expected_result,expected_iterations,expected_string,condition,passed_tests,total_tests)			\
	do {																										\
		char message[MAX_LENGTH];																				\
		char buffer[size];																						\
		nullify_buffer(buffer,size);																			\
		int bytes_read = 0;																						\
		int iterations = 0;																						\
		int result;																								\
		int bytes = size;																						\
		while(bytes_read < bytes)																				\
		{																										\
			result = read(fd,buffer + bytes_read, bytes - bytes_read);											\
			if(result < 0)																						\
				break;																							\
			else																								\
				bytes_read += result;																			\
			iterations++;																						\
		}																										\
		char iterations_string[MAX_LENGTH];																		\
		sprintf(iterations_string,"%d",expected_iterations);													\
		sprintf(message,"Trying to read from VSF with file descriptor = %d",fd);								\
		LOG_RESULT(message,expected_result,result,passed_tests,condition);										\
		sprintf(message,"Checking the number of iterations of the last read() call");							\
		LOG_RESULT(message,iterations_string,iterations,passed_tests,(iterations == expected_iterations));		\
		sprintf(message,"Comparing write() string with the read() string");										\
		int cmp_result = strcmp(buffer,expected_string);														\
		LOG_RESULT(message,"0",cmp_result,passed_tests,cmp_result == 0);										\
		total_tests += 3;																						\
	} while(0)

#define WRITE_VSF(fd,buffer,size,expected_result,expected_iterations,condition,passed_tests,total_tests)		\
	do {																										\
		char message[MAX_LENGTH];																				\
		generate_random_data(buffer,size);																		\
		int bytes_wrote = 0;																					\
		int iterations = 0;																						\
		int result;																								\
		int bytes = size;																						\
		while(bytes_wrote < bytes)																				\
		{																										\
			result = write(fd,buffer + bytes_wrote, bytes - bytes_wrote);										\
			if(result < 0)																						\
				break;																							\
			else																								\
				bytes_wrote += result;																			\
			iterations++;																						\
		}																										\
		char iterations_string[MAX_LENGTH];																		\
		sprintf(iterations_string,"%d",expected_iterations);													\
		sprintf(message,"Trying to write to VSF with file descriptor = %d",fd);									\
		LOG_RESULT(message,expected_result,result,passed_tests,condition);										\
		sprintf(message,"Checking the number of iterations of the last write() call");							\
		LOG_RESULT(message,iterations_string,iterations,passed_tests,(iterations == expected_iterations));		\
		total_tests += 2;																						\
	} while(0)

#define WRITE_READ(fd_read,fd_write,size,passed_tests,total_tests)																				\
	do {																																		\
		char buffer[size];																														\
		generate_random_data(buffer,size);																										\
		int fork_res = fork();																													\
		if(fork_res)																															\
		{																																		\
			int status;																															\
			WRITE_VSF(fd_write,buffer,size,"Some number above zero",CALCULATE_ITERATIONS(size),(result > 0),passed_tests,total_tests);			\
			waitpid(fork_res,&status,0);																										\
			passed_tests += WEXITSTATUS(status);																								\
			total_tests += 3;																													\
		}																																		\
		else																																	\
		{																																		\
			int child_passed_tests = 0;																											\
			READ_VSF(fd_read,size,"Some number above zero",CALCULATE_ITERATIONS(size),buffer,(result > 0),child_passed_tests,total_tests);		\
			return child_passed_tests;																											\
		}																																		\
	} while(0)

#define READ_WRITE(fd_read,fd_write,size,passed_tests,total_tests)																				\
	do {																																		\
		char buffer[size];																														\
		char iterations_string[MAX_LENGTH];																										\
		generate_random_data(buffer,size);																										\
		int fork_res = fork();																													\
		if(fork_res)																															\
		{																																		\
			int status;																															\
			READ_VSF(fd_read,size,"Some number above zero",CALCULATE_ITERATIONS(size),buffer,(result > 0),passed_tests,total_tests);			\
			waitpid(fork_res,&status,0);																										\
			passed_tests += WEXITSTATUS(status);																								\
			total_tests += 2;																													\
		}																																		\
		else																																	\
		{																																		\
			int child_passed_tests = 0;																											\
			WRITE_VSF(fd_write,buffer,size,"Some number above zero",CALCULATE_ITERATIONS(size),(result > 0),child_passed_tests,total_tests);	\
			return child_passed_tests;																											\
		}																																		\
	} while(0)

#define INVALID_WRITE_READ(fd_read,fd_write,passed_tests,total_tests)											\
	do {																										\
		struct sched_param param;																				\
		int result;																								\
		param.sched_priority = 10;																				\
		sched_setscheduler(getpid(),SCHED_FIFO,&param);															\
		fork_res = fork();																						\
		if(fork_res)																							\
		{																										\
			int status;																							\
			char message[MAX_LENGTH];																			\
			char buffer[1000];																					\
			result = write(fd_write,NULL, 1000);																\
			sprintf(message,"Trying to write a null buffer to VSF with file descriptor = %d",fd_write);			\
			LOG_RESULT(message,"EINVAL",result,passed_tests,(result < 0) && (errno == EINVAL));					\
			result = write(fd_write,buffer, 1000);																\
			sprintf(message,"Trying to write a valid buffer to VSF with file descriptor = %d",fd_write);		\
			LOG_RESULT(message,"1000",result,passed_tests,(result == 1000));									\
			waitpid(fork_res,&status,0);																		\
			passed_tests += WEXITSTATUS(status);																\
			total_tests += 3;																					\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);													\
		}																										\
		else																									\
		{																										\
			int child_passed_tests = 0;																			\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);													\
			sched_yield();																						\
			char message[MAX_LENGTH];																			\
			char buffer[1000];																					\
			result = read(fd_read,buffer, 1000);																\
			sprintf(message,"Trying to read a valid buffer from VSF with file descriptor = %d",fd_read);		\
			LOG_RESULT(message,"1000",result,child_passed_tests,(result == 1000));								\
			return child_passed_tests;																			\
		}																										\
	} while(0)


#define INVALID_READ_WRITE(fd_read,fd_write,passed_tests,total_tests)											\
	do {																										\
		struct sched_param param;																				\
		int result;																								\
		param.sched_priority = 10;																				\
		sched_setscheduler(getpid(),SCHED_FIFO,&param);															\
		fork_res = fork();																						\
		if(fork_res)																							\
		{																										\
			int status;																							\
			char message[MAX_LENGTH];																			\
			char buffer[1000];																					\
			result = read(fd_read,NULL, 1000);																	\
			sprintf(message,"Trying to read a null buffer from VSF with file descriptor = %d",fd_read);			\
			LOG_RESULT(message,"EINVAL",result,passed_tests,(result < 0) && (errno == EINVAL));					\
			result = read(fd_read,buffer, 1000);																\
			sprintf(message,"Trying to read a valid buffer from VSF with file descriptor = %d",fd_read);		\
			LOG_RESULT(message,"1000",result,passed_tests,(result == 1000));									\
			waitpid(fork_res,&status,0);																		\
			passed_tests += WEXITSTATUS(status);																\
			total_tests += 3;																					\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);													\
		}																										\
		else																									\
		{																										\
			int child_passed_tests = 0;																			\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);													\
			sched_yield();																						\
			char message[MAX_LENGTH];																			\
			char buffer[1000];																					\
			result = write(fd_write,buffer, 1000);																\
			sprintf(message,"Trying to write a valid buffer to VSF with file descriptor = %d",fd_write);		\
			LOG_RESULT(message,"1000",result,child_passed_tests,(result == 1000));								\
			return child_passed_tests;																			\
		}																										\
	} while(0)

#define INTERRUPT_READ(fd,size,passed_tests,total_tests)										\
	do {																						\
		struct sched_param param;																\
		param.sched_priority = 10;																\
		sched_setscheduler(getpid(),SCHED_FIFO,&param);											\
		fork_res = fork();																		\
		if(fork_res)																			\
		{																						\
			READ_VSF(fd,size,"EINTR",0,"",result && (errno == EINTR),passed_tests,total_tests);	\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);									\
		}																						\
		else																					\
		{																						\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);									\
			sched_yield();																		\
			kill(getppid(),SIGSTOP);															\
			kill(getppid(),SIGCONT);															\
			return 0;																			\
		}																						\
	} while(0)

#define INTERRUPT_WRITE(fd,size,passed_tests,total_tests)												\
	do {																								\
		struct sched_param param;																		\
		param.sched_priority = 10;																		\
		sched_setscheduler(getpid(),SCHED_FIFO,&param);													\
		fork_res = fork();																				\
		if(fork_res)																					\
		{																								\
			char buffer[size];																			\
			WRITE_VSF(fd,buffer,size,"EINTR",0,result && (errno == EINTR),passed_tests,total_tests);	\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);											\
		}																								\
		else																							\
		{																								\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);											\
			sched_yield();																				\
			kill(getppid(),SIGSTOP);																	\
			kill(getppid(),SIGCONT);																	\
			return 0;																					\
		}																								\
	} while(0)

#define READ_SIMULTANEOUSLY(fd_read,fd_write,size,passed_tests,total_tests)																					\
	do {																															\
		struct sched_param param;																\
		param.sched_priority = 10;																\
		sched_setscheduler(getpid(),SCHED_FIFO,&param);											\
		char buffer[size];																											\
		generate_random_data(buffer,size);																							\
		int fork_res = fork();																										\
		if(fork_res)																												\
		{																															\
			int status;																												\
			READ_VSF(fd_read,size,"Some number above zero",CALCULATE_ITERATIONS(size),buffer,(result > 0),passed_tests,total_tests);			\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);									\
			waitpid(fork_res,&status,0);																							\
			passed_tests += WEXITSTATUS(status);																					\
			total_tests += 5;																	\
		}																															\
		else																														\
		{																															\
			int child_passed_tests = 0;	\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);									\
			sched_yield();																		\
			READ_VSF(fd_read,size,"EBUSY",0,"",result && (errno == EBUSY),child_passed_tests,total_tests);											\
			WRITE_VSF(fd_write,buffer,size,"Some number above zero",CALCULATE_ITERATIONS(size),(result > 0),child_passed_tests,total_tests);			\
			return child_passed_tests;																									\
		}																															\
	} while(0)

#define WRITE_SIMULTANEOUSLY(fd_read,fd_write,size,passed_tests,total_tests)																					\
	do {																															\
		struct sched_param param;																\
		param.sched_priority = 10;																\
		sched_setscheduler(getpid(),SCHED_FIFO,&param);											\
		char buffer[size];																											\
		generate_random_data(buffer,size);																							\
		int fork_res = fork();																										\
		if(fork_res)																												\
		{																															\
			int status;																												\
			WRITE_VSF(fd_write,buffer,size,"Some number above zero",CALCULATE_ITERATIONS(size),(result > 0),passed_tests,total_tests);			\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);																			\
			waitpid(fork_res,&status,0);																								\
			passed_tests += WEXITSTATUS(status);																						\
			total_tests += 5;				\
		}																															\
		else																														\
		{																															\
			int child_passed_tests = 0;																											\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);																					\
			sched_yield();																										\
			WRITE_VSF(fd_write,buffer,size,"EBUSY",0,result && (errno == EBUSY),child_passed_tests,total_tests);										\
			READ_VSF(fd_read,size,"Some number above zero",CALCULATE_ITERATIONS(size),buffer,(result > 0),child_passed_tests,total_tests);			\
			return child_passed_tests;																									\
		}																															\
	} while(0)

#define OPEN_RANDOM_FILE(buffer,fd,number,fork_depth,passed_tests,total_tests)									\
	int fd;																							\
	char buffer[MAX_LENGTH];																		\
	int number = (rand() % fork_depth) + 1;															\
	if(number % 2 == 0)																				\
	{																								\
		sprintf(buffer,"vsf_write%d",number);														\
		OPEN_VSF(buffer,fd,O_WRONLY,"Valid file descriptor",(fd_controller >= 0),passed_tests,total_tests);		\
	}																								\
	else																							\
	{																								\
		sprintf(buffer,"vsf_read%d",number);														\
		OPEN_VSF(buffer,fd,O_RDONLY,"Valid file descriptor",(fd_controller >= 0),passed_tests,total_tests);		\
	}

#define BLOCKING_WRITE_READ(fd_read,fd_write,size,passed_tests,total_tests)																				\
	do {																																		\
		struct sched_param param;																\
		param.sched_priority = 10;																\
		sched_setscheduler(getpid(),SCHED_FIFO,&param);											\
		char buffer[size];																														\
		generate_random_data(buffer,size);																										\
		int fork_res = fork();																													\
		if(fork_res)																															\
		{																																		\
			int status;																															\
			WRITE_VSF(fd_write,buffer,size,"Some number above zero",CALCULATE_ITERATIONS(size),(result > 0),passed_tests,total_tests);			\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);																			\
			waitpid(fork_res,&status,0);																										\
			passed_tests += WEXITSTATUS(status);																								\
			total_tests += 3;																													\
		}																																		\
		else																																	\
		{																																		\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);																					\
			usleep(DELAY);																														\
			sched_yield();																														\
			int child_passed_tests = 0;																											\
			READ_VSF(fd_read,size,"Some number above zero",CALCULATE_ITERATIONS(size),buffer,(result > 0),child_passed_tests,total_tests);		\
			return child_passed_tests;																											\
		}																																		\
	} while(0)

#define BLOCKING_READ_WRITE(fd_read,fd_write,size,passed_tests,total_tests)																				\
	do {																																		\
		struct sched_param param;																\
		param.sched_priority = 10;																\
		sched_setscheduler(getpid(),SCHED_FIFO,&param);											\
		char buffer[size];																														\
		char iterations_string[MAX_LENGTH];																										\
		generate_random_data(buffer,size);																										\
		int fork_res = fork();																													\
		if(fork_res)																															\
		{																																		\
			int status;																															\
			READ_VSF(fd_read,size,"Some number above zero",CALCULATE_ITERATIONS(size),buffer,(result > 0),passed_tests,total_tests);			\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);																			\
			waitpid(fork_res,&status,0);																										\
			passed_tests += WEXITSTATUS(status);																								\
			total_tests += 2;																													\
		}																																		\
		else																																	\
		{																																		\
			sched_setscheduler(getpid(),SCHED_OTHER,&param);																					\
			usleep(DELAY);																														\
			sched_yield();																														\
			int child_passed_tests = 0;																											\
			WRITE_VSF(fd_write,buffer,size,"Some number above zero",CALCULATE_ITERATIONS(size),(result > 0),child_passed_tests,total_tests);	\
			return child_passed_tests;																											\
		}																																		\
	} while(0)

int test_output_fd;
int major;
int max_vsf_devices;
int number_of_stressing_processes;
char letters[10] = {'A','B','C','D','E','F','G','H','I','J'};

void nullify_buffer(char* buffer, int length)
{
	int i;
	for(i = 0; i < length; i++)
		buffer[i] = 0;
}

void generate_random_data(char* buffer, int length)
{
	int i;
	for(i = 0; i < (length - 1); i++)
		buffer[i] = letters[rand() % 10];

	buffer[length - 1] = 0;
}

void log_message(char* buffer)
{
	int bytes_written = 0;
	while(bytes_written < strlen(buffer))
	{
		bytes_written += write(test_output_fd,buffer + bytes_written,strlen(buffer) - bytes_written);
	}
}

void convert_errno_to_string(char* buffer)
{
	switch(errno)
	{
		case EPERM:
			sprintf(buffer,"EPERM");
			break;

		case EINTR:
			sprintf(buffer,"EINTR");
			break;

		case ENOMEM:
			sprintf(buffer,"ENOMEM");
			break;

		case EFAULT:
			sprintf(buffer,"EFAULT");
			break;

		case EINVAL:
			sprintf(buffer,"EINVAL");
			break;

		case EBUSY:
			sprintf(buffer,"EBUSY");
			break;

		case EEXIST:
			sprintf(buffer,"EEXIST");
			break;

		case ENODEV:
			sprintf(buffer,"ENODEV");
			break;

		case ESPIPE:
			sprintf(buffer,"ESPIPE");
			break;

		default:
			sprintf(buffer,"0",errno);
	}
}

int make_node(const char *filename, int major, int minor)
{
	PATH_BUILDER(path,filename);
	dev_t device = makedev(major,minor);
	int result = mknod(path,S_IFCHR,device);

	return result;
}

int remove_file(char* filename)
{
	char buffer[100];
	sprintf(buffer,"rm -f /dev/%s",filename);
	system(buffer);
}

void removeAllFiles()
{
	remove_file("vsf_*");
}

int test_open_and_ioctl_functionality()
{
	int passed_tests = 0;
	int total_tests = 0;
	int result;

	LOG_MESSAGE("LAUNCHING PROCEDURE - test_open_and_ioctl_functionality():");
	LOG_MESSAGE("==========================================================");

	removeAllFiles();

	/**************************************************************************/
	/* Part1: Trying to open a VSF minor before its VSF object been allocated */
	/**************************************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part1 - Trying to open a VSF minor before its VSF object been allocated:");
	LOG_MESSAGE("------------------------------------------------------------------------");

	make_node("vsf_unallocated_minor1",major,1);
	make_node("vsf_unallocated_minor2",major,2);

	int fd1;
	int fd2;
	int fd3;
	int fd4;

	OPEN_VSF("vsf_unallocated_minor1",fd1,O_RDONLY,"ENODEV",(fd1 < 0) && (errno == ENODEV),passed_tests,total_tests);
	OPEN_VSF("vsf_unallocated_minor1",fd2,O_WRONLY,"ENODEV",(fd2 < 0) && (errno == ENODEV),passed_tests,total_tests);
	OPEN_VSF("vsf_unallocated_minor2",fd3,O_RDONLY,"ENODEV",(fd3 < 0) && (errno == ENODEV),passed_tests,total_tests);
	OPEN_VSF("vsf_unallocated_minor2",fd4,O_WRONLY,"ENODEV",(fd4 < 0) && (errno == ENODEV),passed_tests,total_tests);

	close(fd1);
	close(fd2);
	close(fd3);
	close(fd4);

	/******************************/
	/* Part2: Open the controller */
	/******************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part2 - Open the controller:");
	LOG_MESSAGE("-----------------------------");

	make_node("vsf_controller",major,0);
	int fd_controller;
	int fd_controller2;
	int fd_controller3;
	OPEN_VSF("vsf_controller",fd_controller,O_RDONLY,"Valid file descriptor",(fd_controller >= 0),passed_tests,total_tests);
	OPEN_VSF("vsf_controller",fd_controller2,O_WRONLY,"Valid file descriptor",(fd_controller2 >= 0),passed_tests,total_tests);
	OPEN_VSF("vsf_controller",fd_controller3,O_RDWR,"Valid file descriptor",(fd_controller3 >= 0),passed_tests,total_tests);
	/*************************************************************/
	/* Part3: Invalid attempts to create and destroy VSF objects */
	/*************************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part3 - Invalid attempts to create and destroy VSF objects:");
	LOG_MESSAGE("-----------------------------------------------------------");

	make_node("vsf_device5",major,5);
	int fd5;

	CREATE_VSF_NULL_PARAM(fd_controller3,"EFAULT",result && (errno == EFAULT),passed_tests,total_tests);
	CREATE_VSF_INVALID_ADDRESS(fd_controller3,"EFAULT",result && (errno == EFAULT),passed_tests,total_tests);
	CREATE_VSF(fd_controller,5,6,"0",!result,passed_tests,total_tests);
	OPEN_VSF("vsf_device5",fd5,O_RDONLY,"Valid file descriptor",(fd5 >= 0),passed_tests,total_tests);

	CREATE_VSF(fd_controller,5,8,"EINVAL",result && (errno == EINVAL),passed_tests,total_tests);
	CREATE_VSF(fd_controller2,8,8,"EINVAL",result && (errno == EINVAL),passed_tests,total_tests);
	CREATE_VSF(fd_controller2,5,6,"EINVAL",result && (errno == EINVAL),passed_tests,total_tests);
	CREATE_VSF(fd_controller,6,5,"EINVAL",result && (errno == EINVAL),passed_tests,total_tests);

	FREE_VSF(fd_controller,5,8,"EINVAL",result && (errno == EINVAL),passed_tests,total_tests);
	FREE_VSF(fd_controller,8,6,"EINVAL",result && (errno == EINVAL),passed_tests,total_tests);
	FREE_VSF(fd_controller3,8,8,"EINVAL",result && (errno == EINVAL),passed_tests,total_tests);
	FREE_VSF(fd_controller3,6,5,"EINVAL",result && (errno == EINVAL),passed_tests,total_tests);

	/***********************************************************************/
	/* Part4: Trying to create and delete VSF device not by the controller */
	/***********************************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part4 - Trying to create and delete VSF device not by the controller:");
	LOG_MESSAGE("---------------------------------------------------------------------");

	CREATE_VSF(fd5,10,11,"EINVAL",result && (errno == EINVAL),passed_tests,total_tests);
	FREE_VSF(fd5,5,6,"EINVAL",result && (errno == EINVAL),passed_tests,total_tests);

	/*******************/
	/* Part5: Cleanups */
	/*******************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part5 - Cleanups:");
	LOG_MESSAGE("-----------------");

	close(fd5);
	FREE_VSF(fd_controller,5,6,"0",!result,passed_tests,total_tests);

	/***********************************************/
	/* Part6: Valid attempts to create VSF objects */
	/***********************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part6 - Valid attempts to create VSF objects:");
	LOG_MESSAGE("---------------------------------------------");

	make_node("vsf_device20",major,20);
	make_node("vsf_device30",major,30);
	make_node("vsf_device100",major,100);
	make_node("vsf_device150",major,150);

	CREATE_VSF(fd_controller,20,30,"0",!result,passed_tests,total_tests);
	CREATE_VSF(fd_controller2,100,150,"0",!result,passed_tests,total_tests);

	/********************************************************/
	/* Part7: Trying to open VSF objects with invalid flags */
	/********************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part7 - Trying to open VSF objects with invalid flags:");
	LOG_MESSAGE("------------------------------------------------------");

	int fd20;
	int fd30;
	int fd100;
	int fd150;

	OPEN_VSF("vsf_device20",fd20,O_WRONLY,"EPERM",(fd20 < 0) && (errno == EPERM),passed_tests,total_tests);
	OPEN_VSF("vsf_device30",fd30,O_RDONLY,"EPERM",(fd30 < 0) && (errno == EPERM),passed_tests,total_tests);
	OPEN_VSF("vsf_device100",fd100,O_WRONLY,"EPERM",(fd100 < 0) && (errno == EPERM),passed_tests,total_tests);
	OPEN_VSF("vsf_device150",fd150,O_RDONLY,"EPERM",(fd150 < 0) && (errno == EPERM),passed_tests,total_tests);
	close(fd150);
	OPEN_VSF("vsf_device150",fd150,O_RDWR,"EPERM",(fd150 < 0) && (errno == EPERM),passed_tests,total_tests);
	close(fd150);
	OPEN_VSF("vsf_device150",fd150,O_NONBLOCK,"EPERM",(fd150 < 0) && (errno == EPERM),passed_tests,total_tests);
	close(fd20);
	close(fd30);
	close(fd100);
	close(fd150);

	/********************************************************/
	/* Part8: Trying to open VSF objects with valid flags	*/
	/********************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part8 - Trying to open VSF objects with valid flags:");
	LOG_MESSAGE("----------------------------------------------------");

	OPEN_VSF("vsf_device20",fd20,O_RDONLY,"Valid file descriptor",(fd20 >= 0),passed_tests,total_tests);
	OPEN_VSF("vsf_device30",fd30,O_WRONLY,"Valid file descriptor",(fd30 >= 0),passed_tests,total_tests);
	OPEN_VSF("vsf_device100",fd100,O_RDONLY,"Valid file descriptor",(fd100 >= 0),passed_tests,total_tests);
	OPEN_VSF("vsf_device150",fd150,O_WRONLY,"Valid file descriptor",(fd150 >= 0),passed_tests,total_tests);

	close(fd20);
	close(fd30);
	close(fd100);
	close(fd150);

	/***********************************************/
	/* Test9: Valid attempts to delete VSF objects */
	/***********************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part9 - Valid attempts to delete VSF objects:");
	LOG_MESSAGE("---------------------------------------------");

	FREE_VSF(fd_controller,20,30,"0",!result,passed_tests,total_tests);
	FREE_VSF(fd_controller3,100,150,"0",!result,passed_tests,total_tests);

	close(fd_controller);
	close(fd_controller2);
	close(fd_controller3);
	return passed_tests == total_tests;
}

int test_lseek_functionality()
{
	int result;
	int total_tests = 0;
	int passed_tests = 0;

	LOG_MESSAGE("LAUNCHING PROCEDURE - test_lseek_functionality():");
	LOG_MESSAGE("=================================================");

	removeAllFiles();

	/******************************/
	/* Part1: Open the controller */
	/******************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part1 - Open the controller:");
	LOG_MESSAGE("-----------------------------");

	make_node("vsf_controller",major,0);
	int fd_controller;
	OPEN_VSF("vsf_controller",fd_controller,O_RDONLY,"Valid file descriptor",(fd_controller >= 0),passed_tests,total_tests);

	/******************************************************************************/
	/* Part2: Trying to call lseek() on various VSF objects and on the controller */
	/******************************************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part2 - Trying to call lseek() on various VSF objects and on the controller:");
	LOG_MESSAGE("----------------------------------------------------------------------------");

	int fd252;
	int fd253;
	int fd254;
	int fd255;

	make_node("vsf_device252",major,252);
	make_node("vsf_device253",major,253);
	make_node("vsf_device254",major,254);
	make_node("vsf_device255",major,255);

	CREATE_VSF(fd_controller,252,253,"0",!result,passed_tests,total_tests);
	CREATE_VSF(fd_controller,254,255,"0",!result,passed_tests,total_tests);

	OPEN_VSF("vsf_device252",fd252,O_RDONLY,"Valid file descriptor",(fd252 >= 0),passed_tests,total_tests);
	OPEN_VSF("vsf_device253",fd253,O_WRONLY,"Valid file descriptor",(fd253 >= 0),passed_tests,total_tests);
	OPEN_VSF("vsf_device254",fd254,O_RDONLY,"Valid file descriptor",(fd254 >= 0),passed_tests,total_tests);
	OPEN_VSF("vsf_device255",fd255,O_WRONLY,"Valid file descriptor",(fd255 >= 0),passed_tests,total_tests);

	VSF_LSEEK(fd252,1,SEEK_SET,"ESPIPE",result && (errno == ESPIPE),passed_tests,total_tests);
	VSF_LSEEK(fd253,4,SEEK_SET,"ESPIPE",result && (errno == ESPIPE),passed_tests,total_tests);
	VSF_LSEEK(fd254,6,SEEK_SET,"ESPIPE",result && (errno == ESPIPE),passed_tests,total_tests);
	VSF_LSEEK(fd255,10,SEEK_SET,"ESPIPE",result && (errno == ESPIPE),passed_tests,total_tests);
	VSF_LSEEK(fd_controller,12,SEEK_SET,"ESPIPE",result && (errno == ESPIPE),passed_tests,total_tests);

	FREE_VSF(fd_controller,252,253,"0",!result,passed_tests,total_tests);
	FREE_VSF(fd_controller,254,255,"0",!result,passed_tests,total_tests);

	close(fd252);
	close(fd253);
	close(fd254);
	close(fd255);
	close(fd_controller);
	return passed_tests == total_tests;
}

int test_read_and_write_functionality()
{
	int total_tests = 0;
	int passed_tests = 0;
	int fork_res;

	LOG_MESSAGE("LAUNCHING TEST PROCEDURE - test_read_and_write_functionality():");
	LOG_MESSAGE("===============================================================");

	removeAllFiles();

	/************************************************/
	/* Part1: Open controller and create VSF object */
	/************************************************/
	LOG_MESSAGE("");
	LOG_MESSAGE("Part 1 - Open controller and create VSF object:");
	LOG_MESSAGE("-----------------------------------------------");

	make_node("vsf_controller",major,0);
	int fd_controller;
	OPEN_VSF("vsf_controller",fd_controller,O_RDONLY,"Valid file descriptor",(fd_controller >= 0),passed_tests,total_tests);

	make_node("vsf_read",major,1);
	make_node("vsf_write",major,2);

	CREATE_VSF(fd_controller,1,2,"0",!result,passed_tests,total_tests);

	int fd_write;
	int fd_read;
	OPEN_VSF("vsf_read",fd_read,O_RDONLY,"Valid file descriptor",(fd_read >= 0),passed_tests,total_tests);
	OPEN_VSF("vsf_write",fd_write,O_WRONLY,"Valid file descriptor",(fd_write >= 0),passed_tests,total_tests);

	/************************************************************/
	/* Part2: Test read/write blocking, when write called first */
	/************************************************************/
	LOG_MESSAGE("");
	LOG_MESSAGE("Part 2 - Test read/write blocking, when write called first:");
	LOG_MESSAGE("----------------------------------------------------------");

	BLOCKING_WRITE_READ(fd_read,fd_write,10,passed_tests,total_tests);
	BLOCKING_WRITE_READ(fd_read,fd_write,10,passed_tests,total_tests);

	/***********************************************************/
	/* Part3: Test read/write blocking, when read called first */
	/***********************************************************/
	LOG_MESSAGE("");
	LOG_MESSAGE("Part 3 - Test read/write blocking, when read called first:");
	LOG_MESSAGE("---------------------------------------------------------");

	BLOCKING_READ_WRITE(fd_read,fd_write,10,passed_tests,total_tests);
	BLOCKING_READ_WRITE(fd_read,fd_write,10,passed_tests,total_tests);

	/***************************************************/
	/* Part4: Test read/write with invalid read buffer */
	/***************************************************/
	LOG_MESSAGE("");
	LOG_MESSAGE("Part 4 - Test read/write with invalid read buffer:");
	LOG_MESSAGE("--------------------------------------------------");
	INVALID_READ_WRITE(fd_read,fd_write,passed_tests,total_tests);

	/****************************************************/
	/* Part5: Test read/write with invalid write buffer */
	/****************************************************/
	LOG_MESSAGE("");
	LOG_MESSAGE("Part 5 - Test read/write with invalid write buffer:");
	LOG_MESSAGE("---------------------------------------------------");
	INVALID_WRITE_READ(fd_read,fd_write,passed_tests,total_tests);

	/***********************************/
	/* Part6: Trying to interrupt read */
	/***********************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part 6 - Trying to interrupt read:");
	LOG_MESSAGE("----------------------------------");
	INTERRUPT_READ(fd_read,400,passed_tests,total_tests);
	INTERRUPT_READ(fd_read,400,passed_tests,total_tests);
	INTERRUPT_READ(fd_read,400,passed_tests,total_tests);
	INTERRUPT_READ(fd_read,400,passed_tests,total_tests);

	/************************************/
	/* Part7: Trying to interrupt write */
	/************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part 7 - Trying to interrupt write:");
	LOG_MESSAGE("-----------------------------------");
	INTERRUPT_WRITE(fd_write,400,passed_tests,total_tests);
	INTERRUPT_WRITE(fd_write,400,passed_tests,total_tests);
	INTERRUPT_WRITE(fd_write,400,passed_tests,total_tests);
	INTERRUPT_WRITE(fd_write,400,passed_tests,total_tests);

	/****************************************/
	/*  Part8: Trying to read simultaneously */
	/****************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part 8 - Trying to read simultaneously:");
	LOG_MESSAGE("---------------------------------------");

	READ_SIMULTANEOUSLY(fd_read,fd_write,1000,passed_tests,total_tests);
	READ_SIMULTANEOUSLY(fd_read,fd_write,1000,passed_tests,total_tests);
	READ_SIMULTANEOUSLY(fd_read,fd_write,1000,passed_tests,total_tests);
	READ_SIMULTANEOUSLY(fd_read,fd_write,1000,passed_tests,total_tests);
	READ_SIMULTANEOUSLY(fd_read,fd_write,1000,passed_tests,total_tests);
	/*****************************************/
	/* Part9: Trying to write simultaneously */
	/*****************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part 9 - Trying to write simultaneously:");
	LOG_MESSAGE("----------------------------------------");

	WRITE_SIMULTANEOUSLY(fd_read,fd_write,1000,passed_tests,total_tests);
	WRITE_SIMULTANEOUSLY(fd_read,fd_write,1000,passed_tests,total_tests);
	WRITE_SIMULTANEOUSLY(fd_read,fd_write,1000,passed_tests,total_tests);
	WRITE_SIMULTANEOUSLY(fd_read,fd_write,1000,passed_tests,total_tests);
	WRITE_SIMULTANEOUSLY(fd_read,fd_write,1000,passed_tests,total_tests);

	/**********************************************************/
	/* Part10: Trying to transfer a packet of size 4KB exactly */
	/**********************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part 10 - Trying to transfer a packet of size 4KB exactly:");
	LOG_MESSAGE("----------------------------------------------------------");
	WRITE_READ(fd_read,fd_write,ONE_PAGE_BUFFER_LENGTH,passed_tests,total_tests);

	/**************************************************************/
	/* Part11: Trying to transfer a packet of size 4KB + 1 exactly */
	/**************************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part 11 - Trying to transfer a packet of size 4KB + 1 exactly:");
	LOG_MESSAGE("--------------------------------------------------------------");
	WRITE_READ(fd_read,fd_write,ONE_PAGE_BUFFER_LENGTH + 1,passed_tests,total_tests);

	/***********************************************************/
	/* Part12: Trying to transfer packets much bigger than 4KB */
	/***********************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part 12 - Trying to transfer packets much bigger than 4KB:");
	LOG_MESSAGE("----------------------------------------------------------");
	WRITE_READ(fd_read,fd_write,10000,passed_tests,total_tests);
	READ_WRITE(fd_read,fd_write,30000,passed_tests,total_tests);
	WRITE_READ(fd_read,fd_write,100000,passed_tests,total_tests);
	READ_WRITE(fd_read,fd_write,500000,passed_tests,total_tests);
	WRITE_READ(fd_read,fd_write,1000000,passed_tests,total_tests);
	READ_WRITE(fd_read,fd_write,2000000,passed_tests,total_tests);

	/*******************/
	/* Part13: Cleanup */
	/*******************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part 13 - Cleanup:");
	LOG_MESSAGE("------------------");

	FREE_VSF(fd_controller,1,2,"0",!result,passed_tests,total_tests);

	close(fd_read);
	close(fd_write);
	close(fd_controller);

	return passed_tests == total_tests;
}

int test_multiple_open_functionality()
{
	int total_tests = 0;
	int passed_tests = 0;
	int fork_res;
	char files[NUMBER_OF_DEVICES*2 + 1][MAX_LENGTH];
	int fds1[NUMBER_OF_DEVICES*2];
	int fds2[NUMBER_OF_DEVICES*2];
	int fds3[NUMBER_OF_DEVICES*2];
	int fds4[NUMBER_OF_DEVICES*2];
	int fds5[NUMBER_OF_DEVICES*2];
	int fds6[NUMBER_OF_DEVICES*2];

	LOG_MESSAGE("LAUNCHING TEST PROCEDURE - test_max_devices():");
	LOG_MESSAGE("==============================================");

	removeAllFiles();

	int i;
	/******************************/
	/* Part1: Open the controller */
	/******************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part1 - Create files and open the controller:");
	LOG_MESSAGE("---------------------------------------------");

	for(i = 0; i < (NUMBER_OF_DEVICES*2 + 1); i++)
	{
		sprintf(files[i],"vsf_file%d",i);
		make_node(files[i],major,i);
	}

	int fd_controller1;
	int fd_controller2;
	int fd_controller3;
	int fd_controller4;
	int fd_controller5;
	OPEN_VSF(files[0],fd_controller1,O_RDONLY,"Valid file descriptor",(fd_controller1 >= 0),passed_tests,total_tests);
	OPEN_VSF(files[0],fd_controller2,O_WRONLY,"Valid file descriptor",(fd_controller2 >= 0),passed_tests,total_tests);
	OPEN_VSF(files[0],fd_controller3,O_RDWR,"Valid file descriptor",(fd_controller3 >= 0),passed_tests,total_tests);
	OPEN_VSF(files[0],fd_controller4,O_RDONLY,"Valid file descriptor",(fd_controller4 >= 0),passed_tests,total_tests);
	OPEN_VSF(files[0],fd_controller5,O_WRONLY,"Valid file descriptor",(fd_controller5 >= 0),passed_tests,total_tests);
	/*****************************************************************************/
	/* Part2: Trying to create and delete VSF objects with different controllers */
	/*****************************************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part2 - Trying to create and delete VSF objects with different controllers:");
	LOG_MESSAGE("---------------------------------------------------------------------------");

	for(i = 1; i <= NUMBER_OF_DEVICES;i++)
	{
		CREATE_VSF(fd_controller1,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	for(i = 1; i <= NUMBER_OF_DEVICES;i++)
	{
		FREE_VSF(fd_controller1,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	for(i = 1; i <= NUMBER_OF_DEVICES;i++)
	{
		CREATE_VSF(fd_controller2,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	for(i = 1; i <= NUMBER_OF_DEVICES;i++)
	{
		FREE_VSF(fd_controller2,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	for(i = 1; i <= NUMBER_OF_DEVICES;i++)
	{
		CREATE_VSF(fd_controller3,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	for(i = 1; i <= NUMBER_OF_DEVICES;i++)
	{
		FREE_VSF(fd_controller3,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	for(i = 1; i <= NUMBER_OF_DEVICES;i++)
	{
		CREATE_VSF(fd_controller4,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	for(i = 1; i <= NUMBER_OF_DEVICES;i++)
	{
		FREE_VSF(fd_controller4,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	for(i = 1; i <= NUMBER_OF_DEVICES;i++)
	{
		CREATE_VSF(fd_controller5,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	/**********************************************************/
	/* Part3: Opening twelve file objects for each VSF object */
	/**********************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part3 - Opening twelve file objects for each VSF object:");
	LOG_MESSAGE("--------------------------------------------------------");

	for(i = 1; i <= (NUMBER_OF_DEVICES*2);i++)
	{
		if(i % 2 != 0)
		{
			OPEN_VSF(files[i],fds1[i-1],O_RDONLY,"Valid file descriptor",(fds1[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds2[i-1],O_RDONLY,"Valid file descriptor",(fds2[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds3[i-1],O_RDONLY,"Valid file descriptor",(fds3[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds4[i-1],O_RDONLY,"Valid file descriptor",(fds4[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds5[i-1],O_RDONLY,"Valid file descriptor",(fds3[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds6[i-1],O_RDONLY,"Valid file descriptor",(fds4[i-1] >= 0),passed_tests,total_tests);

		}
		else
		{
			OPEN_VSF(files[i],fds1[i-1],O_WRONLY,"Valid file descriptor",(fds1[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds2[i-1],O_WRONLY,"Valid file descriptor",(fds2[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds3[i-1],O_WRONLY,"Valid file descriptor",(fds3[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds4[i-1],O_WRONLY,"Valid file descriptor",(fds4[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds5[i-1],O_WRONLY,"Valid file descriptor",(fds5[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds6[i-1],O_WRONLY,"Valid file descriptor",(fds6[i-1] >= 0),passed_tests,total_tests);
		}
	}

	/**********************************/
	/* Part4: Trying to transfer data */
	/**********************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part4 - Trying to transfer data:");
	LOG_MESSAGE("--------------------------------");

	for(i = 0; i < NUMBER_OF_DEVICES; i++)
	{
		READ_WRITE(fds1[2*i],fds1[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds1[2*i],fds1[2*i + 1],200000,passed_tests,total_tests);
		READ_WRITE(fds2[2*i],fds2[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds2[2*i],fds2[2*i + 1],200000,passed_tests,total_tests);
		READ_WRITE(fds3[2*i],fds3[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds3[2*i],fds3[2*i + 1],200000,passed_tests,total_tests);
		READ_WRITE(fds4[2*i],fds4[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds4[2*i],fds4[2*i + 1],200000,passed_tests,total_tests);
		READ_WRITE(fds5[2*i],fds5[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds5[2*i],fds5[2*i + 1],200000,passed_tests,total_tests);
		READ_WRITE(fds6[2*i],fds6[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds6[2*i],fds6[2*i + 1],200000,passed_tests,total_tests);
	}

	/***************************************************/
	/* Part5: Closing all file descriptors and cleanup */
	/***************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part5 - Closing first group of file descriptors, and trying to transfer data again:");
	LOG_MESSAGE("-----------------------------------------------------------------------------------");

	for(i = 0; i < 2*NUMBER_OF_DEVICES; i++)
	{
		close(fds1[i]);
		close(fds2[i]);
		close(fds3[i]);
		close(fds4[i]);
		close(fds5[i]);
		close(fds6[i]);
	}

	for(i = 1; i <= NUMBER_OF_DEVICES;i++)
	{
		FREE_VSF(fd_controller4,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	close(fd_controller1);
	close(fd_controller2);
	close(fd_controller3);
	close(fd_controller4);
	close(fd_controller5);

	return passed_tests == total_tests;
}

int test_vsf_active_after_free()
{
	int total_tests = 0;
	int passed_tests = 0;
	int fork_res;
	char files[NUMBER_OF_DEVICES*2 + 1][MAX_LENGTH];
	int fds1[NUMBER_OF_DEVICES*2];
	int fds2[NUMBER_OF_DEVICES*2];
	int fds3[NUMBER_OF_DEVICES*2];
	int fds4[NUMBER_OF_DEVICES*2];

	LOG_MESSAGE("LAUNCHING TEST PROCEDURE - test_max_devices():");
	LOG_MESSAGE("==============================================");

	removeAllFiles();

	int i;
	/******************************/
	/* Part1: Open the controller */
	/******************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part1 - Create files and open the controller:");
	LOG_MESSAGE("---------------------------------------------");

	for(i = 0; i < (NUMBER_OF_DEVICES*2 + 1); i++)
	{
		sprintf(files[i],"vsf_file%d",i);
		make_node(files[i],major,i);
	}

	int fd_controller;
	OPEN_VSF(files[0],fd_controller,O_RDONLY,"Valid file descriptor",(fd_controller >= 0),passed_tests,total_tests);

	/**********************************************************/
	/* Part2: Trying to create VSF object for all minor slots */
	/**********************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part2 - Trying to create VSF object for all minor slots:");
	LOG_MESSAGE("--------------------------------------------------------");

	for(i = 1; i <= NUMBER_OF_DEVICES;i++)
	{
		CREATE_VSF(fd_controller,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	/*********************************************************/
	/* Part3: Opening eight file objects for each VSF object */
	/*********************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part3 - Opening eight file objects for each VSF object:");
	LOG_MESSAGE("-------------------------------------------------------");

	for(i = 1; i <= (NUMBER_OF_DEVICES*2);i++)
	{
		if(i % 2 != 0)
		{
			OPEN_VSF(files[i],fds1[i-1],O_RDONLY,"Valid file descriptor",(fds1[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds2[i-1],O_RDONLY,"Valid file descriptor",(fds2[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds3[i-1],O_RDONLY,"Valid file descriptor",(fds3[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds4[i-1],O_RDONLY,"Valid file descriptor",(fds4[i-1] >= 0),passed_tests,total_tests);
		}
		else
		{
			OPEN_VSF(files[i],fds1[i-1],O_WRONLY,"Valid file descriptor",(fds1[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds2[i-1],O_WRONLY,"Valid file descriptor",(fds2[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds3[i-1],O_WRONLY,"Valid file descriptor",(fds3[i-1] >= 0),passed_tests,total_tests);
			OPEN_VSF(files[i],fds4[i-1],O_WRONLY,"Valid file descriptor",(fds4[i-1] >= 0),passed_tests,total_tests);
		}
	}

	/**********************************/
	/* Part4: Trying to transfer data */
	/**********************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part4 - Trying to transfer data:");
	LOG_MESSAGE("--------------------------------");

	for(i = 0; i < NUMBER_OF_DEVICES; i++)
	{
		READ_WRITE(fds1[2*i],fds1[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds1[2*i],fds1[2*i + 1],200000,passed_tests,total_tests);
		READ_WRITE(fds2[2*i],fds2[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds2[2*i],fds2[2*i + 1],200000,passed_tests,total_tests);
		READ_WRITE(fds3[2*i],fds3[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds3[2*i],fds3[2*i + 1],200000,passed_tests,total_tests);
		READ_WRITE(fds4[2*i],fds4[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds4[2*i],fds4[2*i + 1],200000,passed_tests,total_tests);
	}

	/***********************************************/
	/* Part5: Calling VSF_FREE for all VSF objects */
	/***********************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part5 - Calling VSF_FREE for all VSF objects:");
	LOG_MESSAGE("---------------------------------------------");

	for(i = 1; i <= NUMBER_OF_DEVICES;i++)
	{
		FREE_VSF(fd_controller,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	/****************************************/
	/* Part6: Trying to transfer data again */
	/****************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part6 - Trying to transfer data again:");
	LOG_MESSAGE("--------------------------------------");

	for(i = 0; i < NUMBER_OF_DEVICES; i++)
	{
		READ_WRITE(fds1[2*i],fds1[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds1[2*i],fds1[2*i + 1],200000,passed_tests,total_tests);
		READ_WRITE(fds2[2*i],fds2[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds2[2*i],fds2[2*i + 1],200000,passed_tests,total_tests);
		READ_WRITE(fds3[2*i],fds3[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds3[2*i],fds3[2*i + 1],200000,passed_tests,total_tests);
		READ_WRITE(fds4[2*i],fds4[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds4[2*i],fds4[2*i + 1],200000,passed_tests,total_tests);
	}

	/*************************************************************************************/
	/* Part7: Closing first group of file descriptors, and trying to transfer data again */
	/*************************************************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part7 - Closing first group of file descriptors, and trying to transfer data again:");
	LOG_MESSAGE("-----------------------------------------------------------------------------------");

	for(i = 0; i < 2*NUMBER_OF_DEVICES; i++)
	{
		close(fds1[i]);
	}

	for(i = 0; i < NUMBER_OF_DEVICES; i++)
	{
		READ_WRITE(fds2[2*i],fds2[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds2[2*i],fds2[2*i + 1],200000,passed_tests,total_tests);
		READ_WRITE(fds3[2*i],fds3[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds3[2*i],fds3[2*i + 1],200000,passed_tests,total_tests);
		READ_WRITE(fds4[2*i],fds4[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds4[2*i],fds4[2*i + 1],200000,passed_tests,total_tests);
	}

	/**************************************************************************************/
	/* Part8: Closing second group of file descriptors, and trying to transfer data again */
	/**************************************************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part8 - Closing second group of file descriptors, and trying to transfer data again:");
	LOG_MESSAGE("------------------------------------------------------------------------------------");

	for(i = 0; i < 2*NUMBER_OF_DEVICES; i++)
	{
		close(fds2[i]);
	}

	for(i = 0; i < NUMBER_OF_DEVICES; i++)
	{
		READ_WRITE(fds3[2*i],fds3[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds3[2*i],fds3[2*i + 1],200000,passed_tests,total_tests);
		READ_WRITE(fds4[2*i],fds4[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds4[2*i],fds4[2*i + 1],200000,passed_tests,total_tests);
	}

	/*************************************************************************************/
	/* Part9: Closing third group of file descriptors, and trying to transfer data again */
	/*************************************************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part9 - Closing third group of file descriptors, and trying to transfer data again:");
	LOG_MESSAGE("-----------------------------------------------------------------------------------");

	for(i = 0; i < 2*NUMBER_OF_DEVICES; i++)
	{
		close(fds3[i]);
	}

	for(i = 0; i < NUMBER_OF_DEVICES; i++)
	{
		READ_WRITE(fds4[2*i],fds4[2*i + 1],200000,passed_tests,total_tests);
		WRITE_READ(fds4[2*i],fds4[2*i + 1],200000,passed_tests,total_tests);
	}

	/***************************************************************************************/
	/* Part10: Closing fourth group of file descriptors, and trying to transfer data again */
	/***************************************************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part10 - Closing fourth group of file descriptors, and trying to transfer data again:");
	LOG_MESSAGE("-------------------------------------------------------------------------------------");

	for(i = 0; i < NUMBER_OF_DEVICES; i++)
	{
		close(fds4[i]);
	}

	/************************************************/
	/* Part11: Trying to free the VSF devices again */
	/************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part11 - Trying to free the VSF devices again:");
	LOG_MESSAGE("----------------------------------------------");

	for(i = 1; i <= NUMBER_OF_DEVICES;i++)
	{
		FREE_VSF(fd_controller,2*i - 1,2*i,"EINVAL",result && (errno == EINVAL),passed_tests,total_tests);
	}

	close(fd_controller);

	return passed_tests == total_tests;
}

int test_max_devices()
{
	int total_tests = 0;
	int passed_tests = 0;
	int fork_res;
	char files[256][MAX_LENGTH];
	int fds1[256];
	int fds2[256];

	LOG_MESSAGE("LAUNCHING TEST PROCEDURE - test_max_devices():");
	LOG_MESSAGE("==============================================");

	removeAllFiles();

	int i;
	/******************************/
	/* Part1: Open the controller */
	/******************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part1 - Create files and open the controller:");
	LOG_MESSAGE("---------------------------------------------");

	for(i = 0; i < 256; i++)
	{
		sprintf(files[i],"vsf_file%d",i);
		make_node(files[i],major,i);
	}

	int fd_controller;
	OPEN_VSF(files[0],fd_controller,O_RDONLY,"Valid file descriptor",(fd_controller >= 0),passed_tests,total_tests);

	/**********************************************************/
	/* Part2: Trying to create VSF object for all minor slots */
	/**********************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part2 - Trying to create VSF object for all minor slots:");
	LOG_MESSAGE("--------------------------------------------------------");

	for(i = 1; i <= 127;i++)
	{
		CREATE_VSF(fd_controller,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	CREATE_VSF(fd_controller,255,1,"EINVAL",result && (errno == EINVAL),passed_tests,total_tests);


	/*******************************/
	/* Part3: Open all VSF objects */
	/*******************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part3 - Open all VSF objects:");
	LOG_MESSAGE("-----------------------------");

	for(i = 1; i <= 254;i++)
	{
		if(i % 2)
		{
			OPEN_VSF(files[i],fds1[i],O_RDONLY,"Valid file descriptor",(fds1[i] >= 0),passed_tests,total_tests);
		}
		else
		{
			OPEN_VSF(files[i],fds1[i],O_WRONLY,"Valid file descriptor",(fds1[i] >= 0),passed_tests,total_tests);
		}
	}

	/**********************************/
	/* Part4: Deletes all VSF objects */
	/**********************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part4 - Deletes all VSF objects:");
	LOG_MESSAGE("--------------------------------");

	for(i = 1; i <= 127;i++)
	{
		FREE_VSF(fd_controller,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	/****************************************************************/
	/* Part5: Trying to create VSF object for all minor slots again */
	/****************************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part5 - Trying to create VSF object for all minor slots again:");
	LOG_MESSAGE("--------------------------------------------------------------");

	for(i = 1; i <= 127;i++)
	{
		CREATE_VSF(fd_controller,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	CREATE_VSF(fd_controller,255,1,"EPERM",result && (errno == EPERM),passed_tests,total_tests);

	/*************************************/
	/* Part6: Open all VSF objects again */
	/*************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part6 - Open all VSF objects again:");
	LOG_MESSAGE("-----------------------------------");

	for(i = 1; i <= 254;i++)
	{
		if(i % 2)
		{
			OPEN_VSF(files[i],fds2[i],O_RDONLY,"Valid file descriptor",(fds2[i] >= 0),passed_tests,total_tests);
		}
		else
		{
			OPEN_VSF(files[i],fds2[i],O_WRONLY,"Valid file descriptor",(fds2[i] >= 0),passed_tests,total_tests);
		}
	}

	/****************************************/
	/* Part7: Deletes all VSF objects again */
	/****************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part7 - Deletes all VSF objects again:");
	LOG_MESSAGE("--------------------------------------");

	for(i = 1; i <= 127;i++)
	{
		FREE_VSF(fd_controller,2*i - 1,2*i,"0",!result,passed_tests,total_tests);
	}

	/**********************************************************************************************/
	/* Part8: Trying to create VSF object for all minor slots again, it should exceed the maximum */
	/**********************************************************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part8 - Trying to create VSF object for all minor slots again, it should exceed the maximum:");
	LOG_MESSAGE("--------------------------------------------------------------------------------------------");

	for(i = 1; i <= 127;i++)
	{
		CREATE_VSF(fd_controller,2*i - 1,2*i,"EPERM",result && (errno == EPERM),passed_tests,total_tests);
	}

	CREATE_VSF(fd_controller,255,1,"EPERM",result && (errno == EPERM),passed_tests,total_tests);

	for(i = 1; i <= 254;i++)
	{
		close(fds1[i]);
		close(fds2[i]);
	}

	/****************************************************************************/
	/* Part9: Deletes all VSF objects again, they all should be already deleted */
	/****************************************************************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part9 - Deletes all VSF objects again, they all should be already deleted:");
	LOG_MESSAGE("--------------------------------------------------------------------------");

	for(i = 1; i <= 127;i++)
	{
		FREE_VSF(fd_controller,2*i - 1,2*i,"EINVAL",result && (errno == EINVAL),passed_tests,total_tests);
	}

	close(fd_controller);
	return passed_tests == total_tests;
}

typedef struct vsf_info {
	int write_minor;
	int read_minor;
	int fd_read;
	int fd_write;
} vsf_info_t;

int stress_test()
{
	int total_tests = 0;
	int passed_tests = 0;
	int fork_res;
	char files[256][MAX_LENGTH];
	vsf_info_t vsf_info[127];
	int fd_controller;
	int i;

	LOG_MESSAGE("LAUNCHING TEST PROCEDURE - stress_test():");
	LOG_MESSAGE("=========================================");

	removeAllFiles();

	/*************************/
	/* Part1: Initialization */
	/*************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part1 - Initialization:");
	LOG_MESSAGE("-----------------------");

	for(i = 1; i <= 254; i++)
	{
		sprintf(files[i],"vsf_file%d",i);
		make_node(files[i],major,i);
	}

	make_node("vsf_controller",major,0);
	OPEN_VSF("vsf_controller",fd_controller,O_RDONLY,"Valid file descriptor",(fd_controller >= 0),passed_tests,total_tests);

	for(i = 0; i < 127;i++)
	{
		CREATE_VSF(fd_controller,2*(i+1) - 1,2*(i+1),"0",!result,passed_tests,total_tests);
		vsf_info[i].read_minor = 2*(i+1) - 1;
		vsf_info[i].write_minor = 2*(i+1);
	}

	/*******************************/
	/* Part2: Open all VSF objects */
	/*******************************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part2 - Open all VSF objects:");
	LOG_MESSAGE("-----------------------------");

	for(i = 0; i < 127;i++)
	{
		OPEN_VSF(files[2*(i+1) - 1],vsf_info[i].fd_read,O_RDONLY,"Valid file descriptor",(vsf_info[i].fd_read >= 0),passed_tests,total_tests);
		OPEN_VSF(files[2*(i+1)],vsf_info[i].fd_write,O_WRONLY,"Valid file descriptor",(vsf_info[i].fd_write >= 0),passed_tests,total_tests);
	}

	/********************/
	/* Part3: Stressing */
	/********************/

	LOG_MESSAGE("");
	LOG_MESSAGE("Part3 - Stressing:");
	LOG_MESSAGE("------------------");

	for(i = 0; i < number_of_stressing_processes; i++)
	{
		fork_res = fork();
		if(!fork_res)
		{
			int j;
			for(j = 0; j < NUMBER_OF_STRESSING_ITERATIONS; j++)
			{
				READ_WRITE(vsf_info[i].fd_read,vsf_info[i].fd_write,200000,passed_tests,total_tests);
				WRITE_READ(vsf_info[i].fd_read,vsf_info[i].fd_write,200000,passed_tests,total_tests);
			}
			return;
		}
	}

	int status;
	while( wait(&status) != -1);

	return passed_tests == total_tests;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	int result;
	if(argc != 4)
	{
		printf("Invalid command arguments\n");
		return 0;
	}
	printf("Running test - it may take few moments:\n");
	printf("---------------------------------------\n");
	test_output_fd = open(TEST_OUTPUT, O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR | S_IWUSR);
	number_of_stressing_processes = atoi(argv[1]);
	major = atoi(argv[2]);
	max_vsf_devices = atoi(argv[3]);

	RUN_TEST(test_open_and_ioctl_functionality,	"Testing open() and ioctl() functionality........................ ","OK.","FAIL.");
	LOG_MESSAGE("");
	RUN_TEST(test_lseek_functionality,			"Testing lseek() functionality................................... ","OK.","FAIL.");
	LOG_MESSAGE("");
	RUN_TEST(test_read_and_write_functionality,	"Testing read() and write() functionality........................ ","OK.","FAIL.");
	LOG_MESSAGE("");
	RUN_TEST(test_multiple_open_functionality,	"Testing multiple calls for open() functionality................. ","OK.","FAIL.");
	LOG_MESSAGE("");
	RUN_TEST(test_vsf_active_after_free,		"Testing VSF object functionality after calling VSF_FREE......... ","OK.","FAIL.");
	LOG_MESSAGE("");
	RUN_TEST(test_max_devices,					"Testing creation of more than 'max_vsf_devices' VSF objects..... ","OK.","FAIL.");
	LOG_MESSAGE("");
	RUN_TEST(stress_test,						"Stressing module................................................ ","OK.","FAIL.");
	close(test_output_fd);
	removeAllFiles();
	return 0;
}
