#include "hw2_syscalls.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <linux/errno.h>
extern int errno;

#define ARRAY_SIZE 30000

#define CHECK_FOR_ERROR(command){ 										\
	if (command == -1){  												\
		printf("command in line %d fell with ", __LINE__);				\
		EXPLAIN_ERROR(errno);											\
	}																	\
	else{ 																\
		printf("command in line %d is passed\n", __LINE__); 			\
	}																	\
}

#define EXPLAIN_ERROR(errno) {											\
	if (errno == 1) printf("EPERM - Operation not permitted\n");		\
	if (errno == 22) printf("EINVAL -  Invalid argument\n");			\
	if (errno == 38) printf("ENOSYS - Function not implemented\n"); }

/* use this only on short or overdue process
 * short process should have policy = 4 otherwise it is considered as overdue
 * PARAMS
 * ARR_SIZE - the size of the array - limits the time this calculation
 * can take, but as default it should be long enought
 * req_time - the time in milisec that your processor will work on this
 * long compute, should be accurate but can miss sometimes in 1 milisec */
void long_compute(int ARR_SIZE, int req_time, int policy){
	int arr[ARR_SIZE,ARR_SIZE];
	int i,j;
	int change = 0;
	int start_time = 0;
	//int policy = sched_getscheduler(getpid());
	if (policy == 4){
		start_time=short_query_remaining_time(getpid());
		/* should become overdue after this run */
		if (req_time > start_time){change=1;}
	}
	else{
		start_time=short_query_overdue_time(getpid());
	}

	/* lot's of calculations */
	for (i=0; i<ARR_SIZE; i++){
		for (j=0; j<ARR_SIZE; j++){
			arr[i,j]=rand() % 10000;
			if (policy == 4){
				if (change==0){
					if (start_time - short_query_remaining_time(getpid()) > req_time){
							return;
					}
				}
				else{
					if (short_query_overdue_time(getpid()) > (req_time-start_time)){
							return;
					}
				}
			}
			else{
				if(short_query_overdue_time(getpid()) - start_time > req_time){
					return;
				}
			}
		}
	}
}
int main() {
	printf("\n\n---------- You are running sched_tester ----------\n\n");
	struct sched_param mysched;
	/* if a process becoming overdue during fork we should do the forking
	 * according to overdue rules */
	mysched.sched_priority = 1;
	CHECK_FOR_ERROR(sched_setscheduler(getpid(), 4, &mysched));
	if (fork()!=0){
		printf("Check the father policy : %d (should be 4)\n",sched_getscheduler(getpid()));
    fflush(stdout);
		wait();
		printf("Check the father policy : %d\n",sched_getscheduler(getpid()));
    fflush(stdout);
	}
	else{
		long_compute(ARRAY_SIZE,5,3);
		printf("Check the child policy : %d (should be 3)\n",sched_getscheduler(getpid()));
    fflush(stdout);
		return 0;
	}

	/* Let's check changes in sched_setscheduler from sched.c */
	printf("\n\n-------------- check policy change -------------\n\n");
  fflush(stdout);
	mysched.sched_priority = 500;
	/* policy */
	CHECK_FOR_ERROR(sched_setscheduler(getpid(), 4, &mysched));
	CHECK_FOR_ERROR(sched_setscheduler(getpid(), 45, &mysched));
	int policy = sched_getscheduler(getpid());
	printf("Policy of current process is %d\n", policy);
	printf("The mil remain to the process as short: %d\n", short_query_remaining_time(getpid()));

	/* priority */
	printf("\n\n------------- check priority changes ------------\n\n");
	mysched.sched_priority = 30001;
	CHECK_FOR_ERROR(sched_setscheduler(getpid(), 4, &mysched));
	mysched.sched_priority = 400;
	CHECK_FOR_ERROR(sched_setscheduler(getpid(), 4, &mysched));
	CHECK_FOR_ERROR(short_query_overdue_time(getpid()));
	printf("The result of short_query_overdue_time(getpid()) is %d\n", short_query_overdue_time(getpid()));
	CHECK_FOR_ERROR(short_query_remaining_time(getpid()));
	printf("The result of short_query_remaining_time(getpid()) is %d\n", short_query_remaining_time(getpid()));
	mysched.sched_priority = 500;
	CHECK_FOR_ERROR(sched_setscheduler(getpid(), 4, &mysched));
	mysched.sched_priority = 1000;
	CHECK_FOR_ERROR(sched_setscheduler(getpid(), 4, &mysched));
	mysched.sched_priority = 1000;
	CHECK_FOR_ERROR(sched_setscheduler(getpid(), 3, &mysched));
	CHECK_FOR_ERROR(sched_getparam(getpid(), &mysched));
	printf("Priority of current process is %d\n", mysched.sched_priority);

	/* changing policy from short to short_overdue by long run*/
	printf("\n\n--------- changing policy short 2 overdue --------\n\n");
	printf("The remaining time of this process is %d\n", short_query_remaining_time(getpid()));
	long_compute(ARRAY_SIZE,1700,4);
	printf("After the long compute the time remainnig as short: %d\n", short_query_remaining_time(getpid()));
	printf("and the time as short_overdue is: %d\n", short_query_overdue_time(getpid()));

	/* changing policy from short_overdue to short */
	printf("\n\n------- changing policy from overdue 2 short  -------\n\n");
	printf("Let's change it back to short:\n");
	mysched.sched_priority = 2000;
	CHECK_FOR_ERROR(sched_setscheduler(getpid(), 4, &mysched));
	policy = sched_getscheduler(getpid());
	printf("Policy of current process is %d\n", policy);
	printf("and the time remainnig as short: %d\n", short_query_remaining_time(getpid()));
	printf("Another long compute :\n");
	long_compute(ARRAY_SIZE,2000,4);
	policy = sched_getscheduler(getpid());
	printf("Policy of current process is %d\n", policy);
	printf("and the time as overdue: %d\n", short_query_overdue_time(getpid()));

	/* check forking */
	printf("\n\n-------------- checking overdue fork ----------------\n\n");
	/* forking overdue process and checking the check max_overdue_time calculation*/
	if (fork()!=0){
		printf("Now the father have %d mil as overdue\n",short_query_overdue_time(getpid()));
		printf("The father makes long compute to enlarge his overdure time\n");
		long_compute(ARRAY_SIZE,300,3);
		printf("Now the father have %d mil as overdue\n",short_query_overdue_time(getpid()));
		wait();
	}
	else{
		/* we'll send the son wait while you choose your prefered char and while
		 * you realising you need to enter a char and hit ENTER
		 * this process will go for wait and the father will do what he have to
		 * before we fork again, that way we can check if the max_overdue_time
		 * works fine */
		printf("This is the child... Enter Char : \n");
		char ch = getchar();
		printf("The char is %c\n",ch);
		printf("the child overdue time is %d \n",short_query_overdue_time(getpid()));
		if (fork()!=0){
			printf("Second fork!! father has %d mil as overdue\n",short_query_overdue_time(getpid()));
		}
		else{
			printf("This is the child after the second fork - doing nothing ..\n");
		}
		printf("Process number %d finished\n",getpid());
		return 0;
	}


	/* forking short process*/
	/* make the process short again to check short forking */
	printf("\n\n-------------- checking short fork ----------------\n\n");
	mysched.sched_priority = 5000;
	CHECK_FOR_ERROR(sched_setscheduler(getpid(), 4, &mysched));
	int father_pid = getpid();
	printf("current process pid is : %d\n",getpid());
	printf("before the fork this process left %d mil to run as short\n",short_query_remaining_time(getpid()));
	if (fork()!=0){
		printf("after fork the father pid is : %d\n",getpid());
		printf("and now the father left %d mil to run as short\n",short_query_remaining_time(getpid()));
		wait();
	}
	else{
		printf("after fork the son pid is : %d\n",getpid());
		printf("the son should be 0.49 mil of the father: %d\n",short_query_remaining_time(getpid()));
	}
	printf("Process number %d finished\n",getpid());
	return 0;
}
