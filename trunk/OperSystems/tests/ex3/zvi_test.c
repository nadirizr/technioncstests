#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mp_interface.h"

/*
 * This program generates <SENDERS> threads to write messages, and <READERS> threads to read messages.
 * Each message is a string describing its type (message/broadcast), its flags, and its sender's id.
 * Most arguments are random - the number of messages, the message's target, and the message's type & flags.
 * The program utilizes all of the library functions, including the mp_barrier calls.
 * As much as I tested this program, it works well with a correctly implemented mplib library,
 * So if the program gets stuck or kicked, it probably originats in the library's code.
 *
 * don't forget to link to the libraries:
 * gcc test.c -L"." -lpthread -lmp -o test
 *
 * usage:
 * ./test
 */


#define SENDERS 2		/* you can change these parameters as you wish */
#define READERS 5		/* more readers mean much more messages */

#define BUF_SIZE 50

pthread_t reader[READERS];
pthread_t sender[SENDERS];
pthread_t final_message_sender;

context_t* con;
barrier_t* register_barrier;
barrier_t* senders_barrier;

char *final = "Final Message";

void mpsend(void* v) {

	char buf[BUF_SIZE];
	char* message_type[4] = { "REGULAR" , "URGENT" , "SYNCED" , "SYNCED URGENT" };

	int retval,i;
	int is_broadcast;
	int number_of_messages;
	int message_target;
	int flags;

	int tid = pthread_self();
	char stid[10];
	sprintf(stid,"%d",tid);

	number_of_messages = abs(rand()) % (READERS * READERS);				/* more READERS ==> much more messages */

	printf("@ Sender %d registering...\n",tid);
  fflush(stdout);
	retval = mp_register(con);										/* register */
	printf("@ Sender %d registered with return value %d\n",tid,retval);
  fflush(stdout);

	mp_barrier(con,register_barrier);								/*  wait for all others threads to register */

	for (i = 0; i < number_of_messages; i++) {

		is_broadcast = !(abs(rand()) % READERS);							 /* is a broadcast with 1/READERS probability */
		message_target = abs(rand()) % READERS;
		flags = abs(rand()) % 4;
		if (is_broadcast && (SENDERS > 1) && (flags & SEND_SYNC))  	 /* this will probably result with a deadlock */
			flags &= 0xFD;											 /* turn the SEND_SYNC flag off */

		strcpy(buf,message_type[flags]);							 /* construct the message */
		if (is_broadcast) {
			strcat(buf," Broadcast from ");							 /* construct the message */
			strcat(buf,stid);										 /* construct the message */
			retval = mp_broadcast(con,buf,strlen(buf)+1,flags);
			printf("@ Sender %d sent %s BROADCAST with return value %d\n",tid,message_type[flags],retval);
      fflush(stdout);
		}
		else {
			strcat(buf," Message from ");							 /* construct the message */
			strcat(buf,stid);										 /* construct the message */
			retval = mp_send(con,&reader[message_target],buf,strlen(buf)+1,flags);
			printf("@ Sender %d sent %s MESSAGE to Reader %d with return value %d\n",tid,message_type[flags],reader[message_target],retval);
      fflush(stdout);
		}
	}

	mp_barrier(con,senders_barrier);								 /* wait for all sending threads to finish */
	if (tid == final_message_sender) {								 /* only one of the sending threads will issue a final message */
		retval = mp_broadcast(con,final,strlen(final)+1,0);
		printf("@ Sender %d sent FINAL BROADCAST with return value %d\n",tid,retval);
    fflush(stdout);
		if (retval < 0) {
			printf("\n@ Error in sending FINAL BROADCAST - terminating all threads...\n");
      fflush(stdout);
			exit(1);
		}
	}

	mp_unregister(con);												 /* unregister */
	printf("@ Sender %d unregistered\n",tid);
  fflush(stdout);
	
	pthread_exit(0);
}


void mpread(void* v) {

	int retval;
	int len;
	char buf[BUF_SIZE];
	int tid = pthread_self();

	printf("@ Reader %d registering...\n",tid);
  fflush(stdout);
	retval = mp_register(con);											/* register */
	printf("@ Reader %d registered with return value %d\n",tid,retval);
  fflush(stdout);
	mp_barrier(con,register_barrier);									/* wait for all others threads to register */

	while (1) {															/* loop and read messages with RECV_SYNC on */
		retval = mp_recv(con,buf,BUF_SIZE,&len,RECV_SYNC);
		printf("@ Message '%s' (length=%d) read by %d with return value %d\n",buf,len,tid,retval);
    fflush(stdout);
		if (!strcmp(buf,final))											/* check if final message arrived */
			break;														/* and if so quit */
	}

	mp_unregister(con);													/* unregister */
	printf("@ Reader %d unregistered\n",tid);
  fflush(stdout);
	pthread_exit(0);
}

int main(void) {

	int rnum,snum;

	srand(time(NULL));

	con = mp_init();
  if (con == NULL) {
    printf("@ Context could not be initialized.\n");
    fflush(stdout);
    return 1;
  }
	register_barrier = mp_initbarrier(con,READERS + SENDERS);
  if (register_barrier == NULL) {
    printf("@ register_barrier could not be initialized.\n");
    fflush(stdout);
    return 1;
  }
	senders_barrier = mp_initbarrier(con,SENDERS);
  if (senders_barrier == NULL) {
    printf("@ senders_barrier could not be initialized.\n");
    fflush(stdout);
    return 1;
  }

	/* create threads */
	for (snum = 0; snum < SENDERS; snum++)
		pthread_create(&sender[snum],NULL,(void*)&mpsend,NULL);
	for (rnum = 0; rnum < READERS; rnum++)
		pthread_create(&reader[rnum],NULL,(void*)&mpread,NULL);

	//final_message_sender = sender[abs(rand()) % SENDERS];
	final_message_sender = sender[0];

	/* wait for threads to finish */
	for (snum = 0; snum < SENDERS; snum++)
		pthread_join(sender[snum],NULL);
	for (rnum = 0; rnum < READERS; rnum++)
		pthread_join(reader[rnum],NULL);

	mp_destroybarrier(con,register_barrier);
	mp_destroybarrier(con,senders_barrier);
	mp_destroy(con);

	printf("\n@ All threads terminated successfully\n");
  fflush(stdout);

	return 0;
}
