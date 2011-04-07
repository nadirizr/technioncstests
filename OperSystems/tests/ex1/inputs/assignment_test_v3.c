/*
 * assignment_test.c
 *
 *  Created on: 29/03/2011
 *      Author: Amit
 */

/* OPERATING SYSTEMS 234123 ASSIGNMENT 1 TEST */
/* Hello! This file contains a program to test your solution for
 * the first assignment in the Operating System course numbered
 * 234123. I wrote this program in order to test our solution,
 * but I publish this in order to help you test yours.
 *
 * The program tests several aspects of the solution, using
 * random values for testing, random number of commands, and
 * random commands. In fact, this is a rather simple test.
 *
 * However, there might be some mistakes in the program. If
 * that is the case, please contact me via the Computer Science
 * Faculty Facebook group or via e-mail (samitcar@t2.technion.ac.il).
 *
 * HOW TO RUN THE TEST?
 * A simple compile-and-run program, no need for parameters, or
 * something complicated.
 * IMPORTANT! reboot before running. This is IMPORTANT because I
 * alter swapper's tag.
 */

/***************/
/* INCLUSIONS: */
/***************/
#include "syscall_tags.h"
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

/****************/
/* DEFINITIONS: */
/****************/

#define MIN_TESTS 40
#define MAX_TESTS 50

#define MIN_COMMAND 1
#define MAX_COMMAND 4

#define WAIT_TIME 100000000
#define DEFAULT_SESSION 20

#define SWAPPER_PID 0
#define INIT_PID 1

#define N 20

#define DEFAULT_NUM_OF_CHILDREN 3
#define MIN_TAG_TESTED 0
#define MAX_TAG_TESTED 100

#define GETTAG_COMMAND 1
#define SETTAG_COMMAND 2
#define GETGOODPROCESSES_COMMAND 3
#define MAKEGOODPROCESSES_COMMAND 4

#define SUCCESS_VALUE 0
#define FAILURE_VALUE -1

#define ASSERT_EQUALS(x,y,z, command_code) \
		{ if (x == y && !is_child) { \
		printf("\tError! %s has failed; \t errno = %d \n", \
		print_command(command_code), errno); \
		printf("\t\t Expected: %d, \t Recieved: %d \n", z, x); return FAILURE_VALUE; } }
#define ASSERT_NOT_EQUALS(x,y, command_code) \
		{ if (x != y && !is_child) { \
		printf("\tError! %s has failed; \t errno = %d \n", \
		print_command(command_code), errno); \
		printf("\t\t Expected: %d, \t Recieved: %d \n", y, x); return FAILURE_VALUE; } }

int is_child = 0;

int get_random_tag()
{
	srand(time(NULL));
	return MIN_TAG_TESTED + rand() % (MAX_TAG_TESTED - MIN_TAG_TESTED);
}

int get_random_tests()
{
	srand(time(NULL));
	return MIN_TESTS + rand() % (MAX_TESTS - MIN_TESTS);
}

int get_random_command()
{
	srand(time(NULL));
	return MIN_COMMAND + rand() % (MAX_COMMAND - MIN_COMMAND);
}

char* print_command(int command_code)
{
	switch(command_code)
	{
		case GETTAG_COMMAND: { return "gettag command"; }
		case SETTAG_COMMAND: { return "settag command"; }
		case GETGOODPROCESSES_COMMAND: { return "getgoodprocesses command"; }
		case MAKEGOODPROCESSES_COMMAND: { return "makegoodprocesses command"; }
		default: { return "undefined command"; }
	}
}

void busy_wait(int amount)
{
	int time = 0;
	int session = 0;

	while (time < WAIT_TIME)
	{ time += 1; }
	session += 1;
	if (session < amount)
	{ busy_wait(amount-1); }
}

void create_children(int num, int* array_of_children)
{
	int i = 0;
	int result;
	for (; i < num; i += 1)
	{
		result = fork();
		if (result)
		{ array_of_children[i] = result; }
		else
		{ 
			is_child = 1; 
			busy_wait(DEFAULT_SESSION); 
		}
	}
}

int test()
{
	int i = 0;
	int sum = 0;
	int old_tag = 0, new_tag = 0;
	int result = 0;
	int array_of_children[DEFAULT_NUM_OF_CHILDREN];

	printf("\tTESTING: gettag on swapper...\n");
	result = gettag(SWAPPER_PID);
	ASSERT_NOT_EQUALS(result, 0, GETTAG_COMMAND);

	printf("\tTESTING: gettag on current process...\n");
	result = gettag(getpid());
	ASSERT_EQUALS(result, FAILURE_VALUE, 0,  GETTAG_COMMAND);

	old_tag = get_random_tag();

	printf("\tTESTING: settag on swapper...\n");
	result = settag(SWAPPER_PID, old_tag);
	ASSERT_NOT_EQUALS(result, FAILURE_VALUE, SETTAG_COMMAND);
	ASSERT_NOT_EQUALS(errno, EINVAL, SETTAG_COMMAND);

	printf("\tTESTING: settag on current process...\n");
	result = settag(getpid(), old_tag);
	ASSERT_NOT_EQUALS(result, SUCCESS_VALUE, SETTAG_COMMAND);

	new_tag = old_tag + 1;

	printf("\tTESTING: settag on current process...\n");
	result = settag(getpid(), new_tag);
	ASSERT_NOT_EQUALS(result, SUCCESS_VALUE, SETTAG_COMMAND);

	printf("\tTESTING: gettag on current process...\n");
	result = gettag(getpid());
	ASSERT_NOT_EQUALS(result, new_tag,  GETTAG_COMMAND);

	create_children(DEFAULT_NUM_OF_CHILDREN, array_of_children);
	if (!is_child) 
	{
		printf("\tTESTING: gettag on current process' children...\n");
		for (i = 0; i < DEFAULT_NUM_OF_CHILDREN; i += 1)
		{
			result = gettag(array_of_children[i]);
			ASSERT_NOT_EQUALS(result, new_tag/2, GETTAG_COMMAND);
			result = settag(array_of_children[i], old_tag);
			ASSERT_NOT_EQUALS(result, SUCCESS_VALUE, SETTAG_COMMAND);
			result = gettag(array_of_children[i]);
			ASSERT_NOT_EQUALS(result, old_tag, GETTAG_COMMAND);
			sum += result;
		} 
	}

	if (is_child) 
	{
		while(1){ if (getppid() == INIT_PID) { return FAILURE_VALUE; } };
	}
	else 
	{
		int array[N] = {0};
		int old_result;
		int is_current_process_found = 0, is_sorted = 1;
		
		//getgoodprocesses #1
		settag(getpid(), sum+1);
		printf("\tTESTING: getgoodprocesses on current process, while it is good...\n");
		result = getgoodprocesses(array, N);
		for (i = 1; i < result; i += 1)
		{ if (array[i] < array[i-1]) { is_sorted = 0; break; } }
		if (!is_sorted)
		{
			printf("\t\tSPECIAL ERROR: getgoodprocesses should have the array sorted!\n");
			return FAILURE_VALUE;
		}
		else if (result < 1) 
		{ printf("\t\tSPECIAL ERROR: getgoodprocesses did not identiy the current process as a good process\n"); }
		for (i = 0; i < N; i++)
		{ array[i] = 0; }
		
		//getgoodprocesses #2
		settag(getpid(), sum);
		printf("\tTESTING: getgoodprocesses on current process, while it is not good...\n");
		result = getgoodprocesses(array, N);
		is_current_process_found = (array[0] == getpid());
		for (i = 1; i < result; i += 1)
		{
			if (array[i] == getpid()) { is_current_process_found = 1; }
			if (array[i] < array[i-1]) { is_sorted = 0; break; }
		}
		if (is_current_process_found)
		{
			printf("\t\tSPECIAL ERROR: current process is not good, but was returned in getgoodprocesses!\n");
			return FAILURE_VALUE;
		}
		else if (!is_sorted)
		{
			printf("\t\tSPECIAL ERROR: getgoodprocesses should have the array sorted!\n");
			return FAILURE_VALUE;
		}
		for (i = 0; i < N; i++)
		{ array[i] = 0; }
		
		//makegoodprocesses #1
		printf("\tTESTING: makegoodprocesses on swapper, while it is not good...\n");
		old_result = result = makegoodprocesses();
		if (result < 1)
		{
			printf("\t\tSPECIAL ERROR: makegoodprocessess should return	at least 1, because current process is not good!\n");
			return FAILURE_VALUE;
		}
		for (i = 0; i < N; i++)
		{ array[i] = -1; }
		
		//getgoodprocesses #3
		getgoodprocesses(array,N);
		int is_swapper_found = (array[0] == SWAPPER_PID);
		for (i = 1; i < result; i += 1)
		{ if (array[i] == SWAPPER_PID) { is_swapper_found = 1; break; } }
		if (!is_swapper_found)
		{ printf("\t\tSPECIAL ERROR: getgoodprocesses did not return swapper, although it was good!\n"); return FAILURE_VALUE; }
		
		//makegoodprocesses #3
		old_tag = gettag(getpid());
		printf("\tTESTING: makegoodprocesses on current process, while it is good...\n");
		result = makegoodprocesses();
		ASSERT_NOT_EQUALS(gettag(getpid()), old_tag, MAKEGOODPROCESSES_COMMAND);
		ASSERT_NOT_EQUALS(result, old_result, MAKEGOODPROCESSES_COMMAND); 
		
		
	}

	return SUCCESS_VALUE;
}


int main()
{
	int result = test();
	if (!result) { printf("TEST SUCCESSFULLY COMPLETED!\n"); }
	return 0;
}


