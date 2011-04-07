/*
 * test1.c
 *
 *  Created on: Mar 29, 2011
 *      Author: ophir-PC
 */

// this is REV 2

// the test is needs to run on a booted system
// when all tags are set to 0

#include "syscall_tags.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <errno.h>

#define FOREVER 3600

int blood_line(int generations, int tag);
int blood_line_aux(int generations);

int task_init();
int check_get_set();
int check_swapper();
int check_getgood(int offspr, int tag);
int check_makegood(int offspr, int tag);

int main(void) {
	int status = -1;

	status = check_get_set();
	switch (status) {
	case (1):
	case (2):
		printf("get tag error - reference %d\n", status);
		break;
	case (3):
	case (4):
	case (5):
	case (6):
		printf("set tag error - reference %d\n", status);
		break;
	case (7):
		printf("error - reference %d\n", status);
		break;
	default:
		printf("gettag() and settag() test - OK\n");
	}

	// task_init works when all the proccesses's tags are 0
	// means the system has to be rebooted before.
	status = task_init();
	switch (status) {
	case (0):
		printf("init test - OK\n");
		break;
	case (1):
		printf("init failed\n");
		break;
	default:
		printf("test init needs reboot\n");
	}

	status = blood_line(10, 256);
	switch (status) {
	case (0):
		printf("fork() tag handling test - OK\n");
		break;
	case (1):
		printf("fork() tag handling failed\n");
		break;
	default:
		printf("test failed\n");
	}

	status = check_getgood(5, 10);
	switch (status) {
	case (0):
		printf("getgoodprocesses() test - OK\n");
		break;
	case (-1):
		printf("test for getgoodprocesses needs reboot\n");
		break;
	default:
		printf("test failed - references %d\n", status);
	}

		status = check_makegood(5, 10);
		switch (status) {
		case (0):
			printf("makegoodprocesses() test - OK\n");
			break;
		case (-1):
			printf("test init needs reboot\n");
			break;
		default:
			printf("test failed - references %d\n", status);
		}
	//	int counter = makegoodprocesses();
	//	printf("tag of pid %d is %d\n", getppid(), gettag(getppid()));
	//	printf("makegoodprocesses returnd %d\n", counter);
	//	printf("\n");

	return 0;
}

/*
 * simulate inheritance process's tags
 * return values:
 * 		0 - test success
 * 		1 - test failed
 * 		-1 - set or get tag problem
 */
int blood_line(int generations, int tag) {

	int status = 0, t_org = gettag(getpid());
	if (settag(getpid(), tag) == -1)
		return -1;

	status = blood_line_aux(generations);

	if (settag(getpid(), t_org) == -1 || gettag(getpid()) != t_org)
		return -1;

	return status;
}

/*
 * return values:
 * 		0 - test success
 * 		1 - test failed
 */
int blood_line_aux(int generations) {
	int iterator = 1, org_pid = getpid();
	pid_t ret_val = -1, status = 0;
	for (; iterator < generations; ++iterator) {
		status = fork();
		if (status != 0) {
			waitpid(status, &ret_val, 0);
			//			printf("%d ", WEXITSTATUS(ret_val));
			if (getpid() != org_pid){
				if (WEXITSTATUS(ret_val) != 0){
					exit (1);
				}
				break;
			}
			else {
				if (WEXITSTATUS(ret_val) != 0){
					return 1;
				}
				return 0;
			}

		}
//		if (gettag(getpid()) != (gettag(getppid())) / 2)
//			exit (1);
//		exit(0);
	}
	if (gettag(getpid()) != (gettag(getppid())) / 2)
		exit (1);
	exit(0);
}

/*
 * return values:
 * 		0 - test success
 * 		1 - test failed
 * 		-1 - system need reboot
 */
int task_init() {
	if (gettag(1) == 0) {
		if (gettag(0) != 0) {
			return 1;
		}

		return 0;
	}

	return -1;
}

/*
 *  return values -
 *  	1-2 - gettag errors
 *  	3-6 - settag errors
 *  	7 - other error
 *  	8 - error return type, return wrong error type or not at all
 */
int check_get_set() {
	if (gettag(getpid()) < 0)
		return 1;
	// gettag failed

	if (gettag(35000) != -1 || gettag(-1) != -1)
		return 2;
	// no such number

	int t_org = gettag(getpid());
	if (settag(getpid(), 5) != 0)
		return 3;
	// settag failed

	if (settag(getpid(), -1) == 0)
		return 4;
	// tag must be greater then 0

	if (gettag(getpid()) != 5)
		return 5;
	// settag problem

	settag(getpid(), t_org);

	if (settag(-1, 2) != -1 || settag(35000, 2) != -1)
		return 6;
	// no such number

	settag(35000, -12);
	if (errno != ESRCH){
		return 8;
	}

	settag(0 , -12);
	if (errno != EINVAL){
		return 8;
	}

	settag(getppid() , 1);
	if (errno != EINVAL){
		return 8;
	}

	int ret_val = 0;
	pid_t status = fork();
	if (status != 0) {
		waitpid(status, &ret_val, 0);
		if (WEXITSTATUS(ret_val) != 0)
			return WEXITSTATUS(ret_val);
	} else {
		if (settag(getppid(), 1) != -1)
			exit(1);
		exit(0);
	}

	status = fork();
	ret_val = 0;
	if (status == 0){
		pause();
	}
	if (settag(status, 1) != 0)
		ret_val = 1;
	if (gettag(getpid()) != t_org)
		ret_val = 1;
	kill(status, SIGKILL);
	while (wait(&t_org) != -1);
	if (settag(status, 2) != -1)
		ret_val = 7;
	return ret_val;
}

/*
 * return values:
 * 		0 - test success
 * 		1 - test failed on good array pointer
 * 		2 - test failed on bad array pointer
 * 		3 - set or get tag problem
 * 		4 - array not sorted
 * 		-1 - system need reboot
 */
int check_getgood(int offspr, int tag) {
	if (task_init() != 0)
		return task_init();
	int t_org = gettag(getpid());
	if (settag(getpid(), tag) == -1)
		return 3;

	pid_t status = 0;
	pid_t* pid_table = (pid_t*) calloc(offspr, sizeof(pid_t));
	int* procs = (int*) calloc(offspr + 1, sizeof(int));
	if (!pid_table || !procs)
		return 3;

	int i = 0, counter = 0, ret_val = 1;
	for (; i < offspr; ++i) {
		status = fork();
		if (status == 0){
			pause();
			exit(0);
		}
		else
			*(pid_table + i) = status;
	}

	for (i = 0; i < offspr; ++i) {
		counter += gettag(*(pid_table + i));
	}

	if (counter != (gettag(getpid()) / 2) * offspr)
		ret_val = 1;
	status = getgoodprocesses(procs, offspr + 1);
	if (counter >= gettag(getpid()) && status > offspr)
		ret_val = 1;
	else if (counter < gettag(getpid()) && status < offspr)
		ret_val = 1;
	else {
		counter = 0;
		for (i = 0; i < offspr + 1; ++i) {
			int j = 0;
			for (; j < offspr; ++j) {
				if (*(pid_table + j) == *(procs + i)) {
					counter++;
				}
			}
		}
		if (counter == offspr)
			ret_val = 0;
	}
	int* not_an_array = NULL;
	if (getgoodprocesses(not_an_array, offspr) != -1) {
		ret_val = 2;
	}

	for (i = 0; i < offspr; ++i) {
		kill(*(pid_table + i), SIGKILL);
	}
	while (wait(&status) != -1);
	free(pid_table);

	for (i = 0; i < offspr; ++i) {
		int j = i;
		for (; j < offspr; ++j) {
			if (*(procs + i) > *(procs + j))
				ret_val = 4;
		}
	}
	free(procs);
	if (settag(getpid(), t_org) == -1 || gettag(getpid()) != t_org)
		return 3;

	return ret_val;
}

/*
 * return values:
 * 		0 - test success
 * 		1 - test failed
 * 		2 - settag, gettag or allocation problem
 * 		3 - test failed - wrong height delta between son and parent
 * 	   -1 - test need's a system reboot
 */
int check_makegood(int offspr, int tag) {
	//	if (task_init() != 0)
	//		return task_init();
	int t_org = gettag(getpid());
	if (settag(getpid(), tag) == -1)
		return 2;

	pid_t status = 0;
	pid_t* pid_table = (pid_t*) calloc(offspr, sizeof(pid_t));
	if (!pid_table)
		return 2;

	int i = 0, ret_val = 0;
	for (; i < offspr; ++i) {
		status = fork();
		if (status == 0){
			pause();
			exit(0);
		}
		else
			*(pid_table + i) = status;
	}

	settag(getpid(), tag * offspr);
	makegoodprocesses();
	if (gettag(getpid()) != tag * offspr)
		return 3;

	settag(getpid(), tag);
	makegoodprocesses();

	if (gettag(getpid()) != (offspr*(tag/2))+1)
		return 1;

	for (i = 0; i < offspr; ++i) {
		kill(*(pid_table + i), SIGKILL);
	}
	while (wait(&status) != -1);
	free(pid_table);
	if (settag(getpid(), t_org) == -1 || gettag(getpid()) != t_org)
		return 2;

	int org_pid = getpid(), temp_val =0;
	for (i=0 ; i < offspr; ++i) {
		status = fork();
		if (status != 0) {
			waitpid(status, &temp_val, 0);

			if (getpid() != org_pid){
				exit WEXITSTATUS(temp_val);
			}
			else
				break;

		}

		if (i == offspr-1){
			exit (makegoodprocesses());
		}
	}
	if (WEXITSTATUS(temp_val) - makegoodprocesses() != offspr)
		ret_val = 3;

	return ret_val;
}
