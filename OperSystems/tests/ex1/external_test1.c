/*
 * tst1-a.c
 *
 *  Created on: Mar 26, 2011
 *      Author: root
 */
#include "syscall_tags.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int main()
{
// this test should be run after reboot
// becouse it checks the syscalls get and set tags in
// there initial values;


int errCode,i,j,k;
int swap,init,currentProc,childProc;
int currentTag,childTag;
int tagTest = 23;


printf("This test was created by the Mamramnic \n");
printf("phase 1 !!! \n \n");
printf("checking initial values of the init and swapper processes\n");
printf("both values should be 0\n");
printf("test 1\n");
swap = gettag(0);
init = gettag(1);
if (swap == 0) {
	printf("The swapper initial tag is %d and it is FINE\n",swap);
} else {
	printf("The swapper initial tag is %d and it is BAD\n",swap);
	if (swap == -1) {
		printf("the errno code is %d\n", errno);
	} else {
		printf("an unknown tag was returned BAD\n");
	}
}
printf("test 2\n");
if (init == 0) {
	printf("The init initial tag is %d and it is FINE\n",swap);
} else {
	printf("The init initial tag is %d and it is BAD\n",swap);
	if (swap == -1) {
		printf("the errno code is %d\n BAD", errno);
	} else {
		printf("an unknown tag was returned BAD\n");
	}
}

printf("\ntest 3");
printf("\nchecking errno error code for the gettag system call\n");
printf("checking process id : 31999. the proc shouldnt exist \n");
errCode = gettag(31999);

if (errCode == -1) {
	if (errno == ESRCH) {
		printf("the correct error code ESRCH was sent FINE\n");
	} else {
		printf("an error code %d was sent and it is BAD\n",errno);
	}
}


printf("\n\nphase 2 !!! \n");
printf("checking set tag system call\n");
printf("test 4\n");
currentProc = getpid();
currentTag = gettag(currentProc);
printf("the current Process PID is %d and it's tag is %d\n",currentProc,currentTag);
printf("checking seting tag of current Proc system call, setting it to tagTest=%d\n",tagTest);

errCode = settag(currentProc,tagTest);
if (errCode != 0) {
	printf("the return value was %d, and it is BAD\n",errCode);
} else {
	printf("the return value was %d, and it is FINE\n",errCode);
	currentTag = gettag(currentProc);
	if (currentTag == tagTest) {
		printf("the tag value of the current Proc is %d and it FINE\n", currentTag);
	} else {
		printf("the tag value of the current Proc is %d and it BAD\n", currentTag);
	}
}
printf("\ntest 5");
printf("\nchecking errno error code for the settag system call\n");
printf("setting process id 31999 to tag=500. the proc shouldnt exist \n");

errCode = settag(31999,500);
if (errCode == -1) {
	if (errno == ESRCH) {
		printf("the correct error code ESRCH was sent FINE\n");
	} else {
		printf("an error code %d was sent and it is BAD\n",errno);
	}
}
printf("test 6\n");
printf("setting process id -5 to tag=500. the proc Really shouldnt exist \n");

errCode = settag(-5,500);
if (errCode == -1) {
	if (errno == ESRCH) {
		printf("the correct error code ESRCH was sent FINE\n");
	} else {
		printf("an error code %d was sent and it is BAD\n",errno);
	}
}
printf("test 7\n");
printf("setting process id -5 to tag=-500. the proc Really shouldnt exist \n");

errCode = settag(-5,-500);
if (errCode == -1) {
	if (errno == ESRCH) {
		printf("the correct error code ESRCH was sent FINE\n");
	} else {
		printf("an error code %d was sent and it is BAD\n",errno);
	}
}
printf("test 8\n");
printf("setting process id currentProc to tag=-500.  \n");

errCode = settag(currentProc,-500);
if (errCode == -1) {
	if (errno == EINVAL) {
		printf("the correct error code EINVAL was sent FINE\n");
	} else {
		printf("an error code %d was sent and it is BAD\n",errno);
	}
}
printf("test 9\n");
printf("setting process id INIT to tag=500.  \n");
errCode = settag(1,500);
if (errCode == -1) {
	if (errno == EINVAL) {
		printf("the correct error code EINVAL was sent FINE\n");
	} else {
		printf("an error code %d was sent and it is BAD\n",errno);
	}
}

printf("\n\nphase 3 !!! \n");
printf("checking if we can spawn a process with tag=%d/2 and set it\n",tagTest);

childProc = fork();
if (childProc == 0) { //a big loop
	for (i = 0; i <1000; i++) {
		for (j=0; j< 1000; j++) {
			for (k=0 ;k< 1000; k++) {}
		}
	}
} else { //orignal father code
	printf("test 10\n");
	childTag = gettag(childProc);
	if (childTag == (tagTest/2) ) {
		printf("the childProc tag is %d and it is half the FatherProc value and FINE\n", childTag);
	} else {
		if (childTag < 0)
			printf("an error code %d was sent and it is BAD\n",childTag);
		else {
			printf("the childProc tag is %d and it is not half father's tag BAD\n", childTag);
		}
	}
	printf("test 11\n");
	printf("trying to change son tag to %d\n", tagTest+400);
	errCode = settag(childProc,tagTest+400);
	if (errCode == -1) {
		printf("an error code %d was sent and it is BAD\n",errno);
		} else {
			childTag = gettag(childProc);
			if (childTag == tagTest+400) {
				printf("the childProc tag was changed to %d, and it's FINE\n", tagTest+400);
			} else {
				printf("a bad tag value was returned, tag=%d and it's BAD\n, ",childTag);
		}
	}

	printf("\nEnd of the Tests - if you got a sentence with BAD in it Check it out \n");
}

return 0;
}


