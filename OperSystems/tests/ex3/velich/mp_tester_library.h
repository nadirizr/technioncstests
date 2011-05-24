/*
 * mp_tester_message_list.h
 *
 *  Created on: May 19, 2011
 *      Author: Roy
 */

#ifndef MP_TESTER_MESSAGE_LIST_H_
#define MP_TESTER_MESSAGE_LIST_H_

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "mp_interface.h"

#define MAX_CONSUMERS 		20
#define MAX_PRODUCERS 		20
#define MAX_ENTRIES			100

#define WAIT_AT_START 1

typedef struct mp_producer* mp_producer_ptr;
typedef struct mp_producer mp_producer_t;

typedef struct mp_consumer* mp_consumer_ptr;
typedef struct mp_consumer mp_consumer_t;

typedef struct mp_test_message* mp_test_message_ptr;
typedef struct mp_test_message mp_test_message_t;

typedef struct mp_map* mp_map_ptr;
typedef struct mp_map mp_map_t;

typedef struct mp_test_message_list* mp_test_message_list_ptr;
typedef struct mp_test_message_list mp_test_message_list_t;

typedef struct mp_barrier_tester* mp_barrier_tester_ptr;
typedef struct mp_barrier_tester mp_barrier_tester_t;

typedef struct mp_recv_tester* mp_recv_tester_ptr;
typedef struct mp_recv_tester mp_recv_tester_t;

struct mp_recv_tester {
	barrier_t* bar;
	context_t* con;
	pthread_t destination;
};

struct mp_barrier_tester {
	barrier_t* bar;
	context_t* con;
};

struct mp_map {
	int next_id;
	pthread_mutex_t mutex;
	pthread_t entries[MAX_ENTRIES];
};

struct mp_producer {
	pthread_t thread_id;
	barrier_t* bar;
	context_t* con;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	mp_consumer_ptr consumers[MAX_CONSUMERS];
	mp_map_ptr map;
	int messages_to_produce;
	int dependencies;
	int next_consumer;
	int flags;
	int broadcast;
	int messages_produced;
	int wait_on_start;
	int id;
	unsigned long iterations;
};

struct mp_consumer {
	pthread_t thread_id;
	barrier_t* bar;
	context_t* con;
	mp_test_message_list_ptr message_list;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	mp_producer_ptr producers[MAX_PRODUCERS];
	mp_map_ptr map;
	int messages_to_consume;
	int dependencies;
	int complete;
	int wait_on_start;
	int next_producer;
	int flags;
	int messages_consumed;
	int id;
	unsigned long iterations;
};

struct mp_test_message {
	int message_tag;
	int message_type;
	int dest_id;
	int src_id;
	pthread_t message_source;
	mp_test_message_ptr next;
};

struct mp_test_message_list {
	mp_test_message_ptr head;
	mp_test_message_ptr tail;
};

/****************************************/
/*				MESSAGE					*/
/****************************************/
mp_test_message_ptr mp_test_message_create(pthread_t message_source, int message_tag, int message_type, int src_id,int dest_id);
mp_test_message_list_ptr mp_test_message_list_create();
void mp_test_message_list_destroy(mp_test_message_list_ptr message_list);
void mp_test_message_list_add(mp_test_message_list_ptr message_list,mp_test_message_ptr message);

/****************************************/
/*				MAP						*/
/****************************************/
mp_map_ptr mp_map_create();
void mp_map_destroy(mp_map_ptr map);
int mp_map_register(mp_map_ptr map,pthread_t thread_id);
pthread_t mp_map_get_by_index(mp_map_ptr map, int index);

/****************************************/
/*				CONSUMER				*/
/****************************************/
mp_consumer_ptr mp_consumer_create(context_t* con,barrier_t* bar, mp_map_ptr map,int flags,int messages_to_consume,int wait_on_start);
void mp_consumer_destroy(mp_consumer_ptr consumer);
void mp_consumer_add_to_dependencies(mp_consumer_ptr consumer, int offset);
int mp_consumer_get_dependencies(mp_consumer_ptr consumer);
void mp_consumer_increase_iterations(mp_consumer_ptr consumer);
int mp_consumer_get_iterations(mp_consumer_ptr consumer);
void mp_consumer_register_producer(mp_consumer_ptr consumer, mp_producer_ptr producer);
void mp_consumer_signal(mp_consumer_ptr consumer);
int mp_consumer_get_id(mp_consumer_ptr consumer);
void mp_consumer_set_id(mp_consumer_ptr consumer, int id);


/****************************************/
/*				PRODUCER				*/
/****************************************/
mp_producer_ptr mp_producer_create(context_t* con,barrier_t* bar,mp_map_ptr map,int broadcast,int flags,int messages_to_produce,int wait_on_start);
void mp_producer_register_consumer(mp_producer_ptr producer, mp_consumer_ptr consumer);
void mp_producer_destroy(mp_producer_ptr producer);
void mp_producer_add_to_dependencies(mp_producer_ptr producer, int offset);
int mp_producer_get_dependencies(mp_producer_ptr producer);
void mp_producer_increase_iterations(mp_producer_ptr producer);
int mp_producer_get_iterations(mp_producer_ptr producer);
void mp_producer_signal(mp_producer_ptr producer);
int mp_producer_get_id(mp_producer_ptr producer);
void mp_producer_set_id(mp_producer_ptr producer, int id);

#endif /* MP_TESTER_MESSAGE_LIST_H_ */
