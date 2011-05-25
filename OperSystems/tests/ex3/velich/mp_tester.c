#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/file.h>

#include "mp_interface.h"
#include "mp_tester_library.h"

#define BUFFER_SIZE 200

#define TEST_SYNC_BROADCAST_MESSAGES_TO_BROADCAST 100
#define TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME 20
#define TEST_ASYNC_BROADCAST_DELAY 100000

#define TEST_SYNC_SEND_MESSAGES_TO_SEND 100
#define TEST_ASYNC_SEND_MESSAGES_TO_CONSUME 20
#define TEST_ASYNC_SEND_DELAY 100000

#define TEST_SYNC_RECV_MESSAGES_TO_RECV 200
#define TEST_ASYNC_RECV_MESSAGES_TO_SEND 30
#define TEST_ASYNC_RECV_DELAY 400000

#define TEST_URGENT_VS_REGULAR_MESSAGES_TO_SEND 4

int fd;

pthread_t create_thread(void* (*start_routine)(void*),void* arg)
{
	pthread_t thread_id;
	pthread_create(&thread_id,NULL,start_routine,arg);
	return thread_id;
}

void write_to_file(char* buffer)
{
	int bytes_written = 0;
	while(bytes_written < strlen(buffer))
	{
		bytes_written += write(fd,buffer + bytes_written,strlen(buffer) - bytes_written);
	}
}

void write_message_to_file(mp_test_message_ptr message)
{
	if(message)
	{
		char buffer[BUFFER_SIZE];
		if(message->message_type & SEND_URGENT)
			sprintf(buffer,"URGENT 	message from virtual ID=%d with TAG=%d to virtual ID=%d\n",message->src_id,message->message_tag,message->dest_id);
		else
			sprintf(buffer,"REGULAR message from virtual ID=%d with TAG=%d to virtual ID=%d\n",message->src_id,message->message_tag,message->dest_id);

		write_to_file(buffer);
	}
}

void* produce_messages(void* arg)
{
	int i;

	mp_producer_ptr producer = (mp_producer_ptr)arg;
	mp_register(producer->con);
	int id = mp_map_register(producer->map,pthread_self());
	mp_producer_set_id(producer,id);
	mp_barrier(producer->con,producer->bar);

	pthread_mutex_lock(&producer->mutex);
	while(producer->wait_on_start)
		pthread_cond_wait(&producer->cond,&producer->mutex);
	pthread_mutex_unlock(&producer->mutex);

	int tag = 0;
	while(1)
	{
		if(!mp_producer_get_dependencies(producer) && !producer->messages_to_produce)
			break;

		if(!producer->broadcast)
		{
			for(i = 0;i < MAX_CONSUMERS;i++)
			{
				if(producer->consumers[i])
				{
					int index = mp_consumer_get_id(producer->consumers[i]);
					mp_test_message_ptr message = mp_test_message_create(pthread_self(), tag, producer->flags & SEND_URGENT,producer->id,index);
					pthread_t thread_id = mp_map_get_by_index(producer->map,index);
					mp_send(producer->con,&thread_id,(char *)message,sizeof(mp_test_message_t),producer->flags);
					free(message);
				}
			}
		}
		else
		{
			mp_test_message_ptr message = mp_test_message_create(pthread_self(), tag, producer->flags & SEND_URGENT,producer->id,-1);
			mp_broadcast(producer->con,(char *)message,sizeof(mp_test_message_t),producer->flags);
			free(message);
		}

		tag++;
		producer->iterations++;
		if(producer->messages_to_produce)
		{
			if(producer->iterations == producer->messages_to_produce)
			{
				for(i = 0;i < MAX_CONSUMERS;i++)
				{
					if(producer->consumers[i])
						mp_consumer_add_to_dependencies(producer->consumers[i],-1);
				}

				break;
			}
		}
	}
	mp_unregister(producer->con);
	return NULL;
}

void* consume_messages(void* arg)
{
	int i;
	mp_consumer_ptr consumer = (mp_consumer_ptr)arg;
	mp_register(consumer->con);
	int id = mp_map_register(consumer->map,pthread_self());
	mp_consumer_set_id(consumer,id);
	mp_barrier(consumer->con,consumer->bar);

	pthread_mutex_lock(&consumer->mutex);
	while(consumer->wait_on_start)
		pthread_cond_wait(&consumer->cond,&consumer->mutex);
	pthread_mutex_unlock(&consumer->mutex);

	while(1)
	{
		int msglen;
		if(!mp_consumer_get_dependencies(consumer) && !consumer->messages_to_consume)
			break;

		mp_test_message_ptr message = (mp_test_message_ptr)malloc(sizeof(mp_test_message_t));
		mp_recv(consumer->con,(char *)message,sizeof(mp_test_message_t),&msglen,consumer->flags);
		consumer->iterations++;
		if(msglen)
		{
			mp_test_message_list_add(consumer->message_list,message);
			consumer->messages_consumed++;
			if(consumer->messages_to_consume)
			{
				if(consumer->messages_consumed == consumer->messages_to_consume)
				{

					for(i = 0;i < MAX_CONSUMERS;i++)
					{
						if(consumer->producers[i])
							mp_producer_add_to_dependencies(consumer->producers[i],-1);
					}
					break;
				}
			}
		}
		else
			free(message);
	}
	mp_unregister(consumer->con);
	return NULL;
}

mp_test_message_ptr validate_sequence(mp_test_message_ptr root,int urgent, int *success)
{
	int src_id = root->src_id;
	int tag = root->message_tag;
	mp_test_message_ptr current = root->next;
	*success = 1;
	write_message_to_file(root);
	while(current)
	{
		if(current->src_id != src_id)
			break;
		write_message_to_file(current);
		if(((current->message_tag + 1 == tag) && urgent) || ((current->message_tag - 1 == tag) && !urgent))
		{
			tag = current->message_tag;
			current = current->next;
		}
		else
		{
			*success = 0;
			break;
		}
	}

	return current;
}

int validate_message_list(mp_test_message_list_ptr message_list)
{
	int success = 1;
	if(message_list)
	{
		if(message_list->head)
		{
			mp_test_message_ptr current = message_list->head;
			int urgent = (current->message_type & SEND_URGENT) ? 1 : 0;
			while(current)
			{
				current = validate_sequence(current,urgent,&success);
				if(success && current)
				{
					if(!urgent && (current->message_type & SEND_URGENT))
						return 0;

					urgent = (current->message_type & SEND_URGENT) ? 1 : 0;
				}
				else
					break;
			}
		}
		else
			success = 0;
	}

	return success;
}

/*************************************/
/*		ASUNCHRONOUS BROADCAST		 */
/*************************************/
int test_async_broadcast()
{
	char buffer[BUFFER_SIZE];
	context_t* con = mp_init();
	barrier_t* bar = mp_initbarrier(con,6);
	mp_map_ptr map = mp_map_create();

	mp_producer_ptr producer1 = mp_producer_create(con,bar,map,1,0,0,0);
	mp_consumer_ptr consumer1 = mp_consumer_create(con,bar,map,0,TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME,WAIT_AT_START);
	mp_consumer_ptr consumer2 = mp_consumer_create(con,bar,map,0,TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME,WAIT_AT_START);
	mp_consumer_ptr consumer3 = mp_consumer_create(con,bar,map,0,TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME,WAIT_AT_START);
	mp_consumer_ptr consumer4 = mp_consumer_create(con,bar,map,0,TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME,WAIT_AT_START);
	mp_consumer_ptr consumer5 = mp_consumer_create(con,bar,map,0,TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME,WAIT_AT_START);


	mp_consumer_register_producer(consumer1,producer1);
	mp_consumer_register_producer(consumer2,producer1);
	mp_consumer_register_producer(consumer3,producer1);
	mp_consumer_register_producer(consumer4,producer1);
	mp_consumer_register_producer(consumer5,producer1);

	pthread_t consumer1_id = create_thread(consume_messages,consumer1);
	pthread_t consumer2_id = create_thread(consume_messages,consumer2);
	pthread_t consumer3_id = create_thread(consume_messages,consumer3);
	pthread_t consumer4_id = create_thread(consume_messages,consumer4);
	pthread_t consumer5_id = create_thread(consume_messages,consumer5);
	pthread_t producer1_id = create_thread(produce_messages,producer1);

	usleep(TEST_ASYNC_BROADCAST_DELAY);

	mp_consumer_signal(consumer1);
	mp_consumer_signal(consumer2);
	mp_consumer_signal(consumer3);
	mp_consumer_signal(consumer4);
	mp_consumer_signal(consumer5);

	pthread_join(consumer1_id,NULL);
	pthread_join(consumer2_id,NULL);
	pthread_join(consumer3_id,NULL);
	pthread_join(consumer4_id,NULL);
	pthread_join(consumer5_id,NULL);
	pthread_join(producer1_id,NULL);

	int result1 = (consumer1->messages_consumed == TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME);
	int result2 = (consumer2->messages_consumed == TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME);
	int result3 = (consumer3->messages_consumed == TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME);
	int result4 = (consumer4->messages_consumed == TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME);
	int result5 = (consumer5->messages_consumed == TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME);
	int result6 = producer1->iterations > TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME;

	sprintf(buffer,"Consumer 1	:	Messages consumed=%d	(should be %d)\n",consumer1->messages_consumed,TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 2	:	Messages consumed=%d	(should be %d)\n",consumer2->messages_consumed,TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 3	:	Messages consumed=%d	(should be %d)\n",consumer3->messages_consumed,TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 4	:	Messages consumed=%d	(should be %d)\n",consumer4->messages_consumed,TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 5	:	Messages consumed=%d	(should be %d)\n",consumer5->messages_consumed,TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME);
	write_to_file(buffer);
	sprintf(buffer,"Producer	:	Iterations=%d			(should be much more than %d)\n",producer1->iterations,TEST_ASYNC_BROADCAST_MESSAGES_TO_CONSUME);
	write_to_file(buffer);

	mp_producer_destroy(producer1);
	mp_consumer_destroy(consumer1);
	mp_consumer_destroy(consumer2);
	mp_consumer_destroy(consumer3);
	mp_consumer_destroy(consumer4);
	mp_consumer_destroy(consumer5);
	mp_map_destroy(map);
	mp_destroybarrier(con,bar);
	mp_destroy(con);
	return result1 && result2 && result3 && result4 && result5 && result6;
}


/*************************************/
/*		ASUNCHRONOUS SEND		 	 */
/*************************************/
int test_async_send()
{
	char buffer[BUFFER_SIZE];
	context_t* con = mp_init();
	barrier_t* bar = mp_initbarrier(con,10);
	mp_map_ptr map = mp_map_create();
	mp_producer_ptr producer1 = mp_producer_create(con,bar,map,0,0,0,0);
	mp_producer_ptr producer2 = mp_producer_create(con,bar,map,0,SEND_URGENT,0,0);
	mp_producer_ptr producer3 = mp_producer_create(con,bar,map,0,0,0,0);
	mp_producer_ptr producer4 = mp_producer_create(con,bar,map,0,SEND_URGENT,0,0);
	mp_producer_ptr producer5 = mp_producer_create(con,bar,map,0,SEND_URGENT,0,0);

	mp_consumer_ptr consumer1 = mp_consumer_create(con,bar,map,0,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME,WAIT_AT_START);
	mp_consumer_ptr consumer2 = mp_consumer_create(con,bar,map,0,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME,WAIT_AT_START);
	mp_consumer_ptr consumer3 = mp_consumer_create(con,bar,map,0,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME,WAIT_AT_START);
	mp_consumer_ptr consumer4 = mp_consumer_create(con,bar,map,0,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME,WAIT_AT_START);
	mp_consumer_ptr consumer5 = mp_consumer_create(con,bar,map,0,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME,WAIT_AT_START);

	mp_producer_register_consumer(producer1,consumer1);
	mp_consumer_register_producer(consumer1,producer1);

	mp_producer_register_consumer(producer2,consumer2);
	mp_consumer_register_producer(consumer2,producer2);

	mp_producer_register_consumer(producer3,consumer3);
	mp_consumer_register_producer(consumer3,producer3);

	mp_producer_register_consumer(producer4,consumer4);
	mp_consumer_register_producer(consumer4,producer4);

	mp_producer_register_consumer(producer5,consumer5);
	mp_consumer_register_producer(consumer5,producer5);

	pthread_t consumer1_id = create_thread(consume_messages,consumer1);
	pthread_t consumer2_id = create_thread(consume_messages,consumer2);
	pthread_t consumer3_id = create_thread(consume_messages,consumer3);
	pthread_t consumer4_id = create_thread(consume_messages,consumer4);
	pthread_t consumer5_id = create_thread(consume_messages,consumer5);
	pthread_t producer1_id = create_thread(produce_messages,producer1);
	pthread_t producer2_id = create_thread(produce_messages,producer2);
	pthread_t producer3_id = create_thread(produce_messages,producer3);
	pthread_t producer4_id = create_thread(produce_messages,producer4);
	pthread_t producer5_id = create_thread(produce_messages,producer5);

	usleep(TEST_ASYNC_SEND_DELAY);

	mp_consumer_signal(consumer1);
	mp_consumer_signal(consumer2);
	mp_consumer_signal(consumer3);
	mp_consumer_signal(consumer4);
	mp_consumer_signal(consumer5);

	pthread_join(consumer1_id,NULL);
	pthread_join(consumer2_id,NULL);
	pthread_join(consumer3_id,NULL);
	pthread_join(consumer4_id,NULL);
	pthread_join(consumer5_id,NULL);
	pthread_join(producer1_id,NULL);
	pthread_join(producer2_id,NULL);
	pthread_join(producer3_id,NULL);
	pthread_join(producer4_id,NULL);
	pthread_join(producer5_id,NULL);

	int result1 = (consumer1->messages_consumed == TEST_ASYNC_SEND_MESSAGES_TO_CONSUME) && (producer1->iterations > TEST_ASYNC_SEND_MESSAGES_TO_CONSUME);
	int result2 = (consumer2->messages_consumed == TEST_ASYNC_SEND_MESSAGES_TO_CONSUME) && (producer2->iterations > TEST_ASYNC_SEND_MESSAGES_TO_CONSUME);
	int result3 = (consumer3->messages_consumed == TEST_ASYNC_SEND_MESSAGES_TO_CONSUME) && (producer3->iterations > TEST_ASYNC_SEND_MESSAGES_TO_CONSUME);
	int result4 = (consumer4->messages_consumed == TEST_ASYNC_SEND_MESSAGES_TO_CONSUME) && (producer4->iterations > TEST_ASYNC_SEND_MESSAGES_TO_CONSUME);
	int result5 = (consumer5->messages_consumed == TEST_ASYNC_SEND_MESSAGES_TO_CONSUME) && (producer5->iterations > TEST_ASYNC_SEND_MESSAGES_TO_CONSUME);

	sprintf(buffer,"Consumer 1: Messages consumed=%d (should be %d), Producer 1: Iterations=%d (should be much more than %d)\n",consumer1->messages_consumed,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME,producer1->iterations,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 2: Messages consumed=%d (should be %d), Producer 2: Iterations=%d (should be much more than %d)\n",consumer2->messages_consumed,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME,producer2->iterations,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 3: Messages consumed=%d (should be %d), Producer 3: Iterations=%d (should be much more than %d)\n",consumer3->messages_consumed,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME,producer3->iterations,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 4: Messages consumed=%d (should be %d), Producer 4: Iterations=%d (should be much more than %d)\n",consumer4->messages_consumed,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME,producer4->iterations,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 5: Messages consumed=%d (should be %d), Producer 5: Iterations=%d (should be much more than %d)\n",consumer5->messages_consumed,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME,producer5->iterations,TEST_ASYNC_SEND_MESSAGES_TO_CONSUME);
	write_to_file(buffer);

	mp_producer_destroy(producer1);
	mp_producer_destroy(producer2);
	mp_producer_destroy(producer3);
	mp_producer_destroy(producer4);
	mp_producer_destroy(producer5);
	mp_consumer_destroy(consumer1);
	mp_consumer_destroy(consumer2);
	mp_consumer_destroy(consumer3);
	mp_consumer_destroy(consumer4);
	mp_consumer_destroy(consumer5);
	mp_map_destroy(map);
	mp_destroybarrier(con,bar);
	mp_destroy(con);
	return result1 && result2 && result3 && result4 && result5;
}

/*************************************/
/*		SYNCHRONOUS BROADCAST		 */
/*************************************/
int test_sync_broadcast()
{
	char buffer[BUFFER_SIZE];
	context_t* con = mp_init();
	barrier_t* bar = mp_initbarrier(con,6);
	mp_map_ptr map = mp_map_create();
	mp_producer_ptr producer1 = mp_producer_create(con,bar,map,1,SEND_SYNC,TEST_SYNC_BROADCAST_MESSAGES_TO_BROADCAST,0);
	mp_consumer_ptr consumer1 = mp_consumer_create(con,bar,map,0,0,0);
	mp_consumer_ptr consumer2 = mp_consumer_create(con,bar,map,0,0,0);
	mp_consumer_ptr consumer3 = mp_consumer_create(con,bar,map,0,0,0);
	mp_consumer_ptr consumer4 = mp_consumer_create(con,bar,map,0,0,0);
	mp_consumer_ptr consumer5 = mp_consumer_create(con,bar,map,0,0,0);

	mp_producer_register_consumer(producer1,consumer1);
	mp_producer_register_consumer(producer1,consumer2);
	mp_producer_register_consumer(producer1,consumer3);
	mp_producer_register_consumer(producer1,consumer4);
	mp_producer_register_consumer(producer1,consumer5);

	pthread_t consumer1_id = create_thread(consume_messages,consumer1);
	pthread_t consumer2_id = create_thread(consume_messages,consumer2);
	pthread_t consumer3_id = create_thread(consume_messages,consumer3);
	pthread_t consumer4_id = create_thread(consume_messages,consumer4);
	pthread_t consumer5_id = create_thread(consume_messages,consumer5);
	pthread_t producer1_id = create_thread(produce_messages,producer1);

	pthread_join(consumer1_id,NULL);
	pthread_join(consumer2_id,NULL);
	pthread_join(consumer3_id,NULL);
	pthread_join(consumer4_id,NULL);
	pthread_join(consumer5_id,NULL);
	pthread_join(producer1_id,NULL);

	int result1 = (consumer1->messages_consumed == TEST_SYNC_BROADCAST_MESSAGES_TO_BROADCAST);
	int result2 = (consumer2->messages_consumed == TEST_SYNC_BROADCAST_MESSAGES_TO_BROADCAST);
	int result3 = (consumer3->messages_consumed == TEST_SYNC_BROADCAST_MESSAGES_TO_BROADCAST);
	int result4 = (consumer4->messages_consumed == TEST_SYNC_BROADCAST_MESSAGES_TO_BROADCAST);
	int result5 = (consumer5->messages_consumed == TEST_SYNC_BROADCAST_MESSAGES_TO_BROADCAST);
	int result6 = producer1->iterations == TEST_SYNC_BROADCAST_MESSAGES_TO_BROADCAST;

	sprintf(buffer,"Consumer 1	: Messages consumed=%d		(should be %d)\n",consumer1->messages_consumed,TEST_SYNC_BROADCAST_MESSAGES_TO_BROADCAST);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 2	: Messages consumed=%d		(should be %d)\n",consumer2->messages_consumed,TEST_SYNC_BROADCAST_MESSAGES_TO_BROADCAST);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 3	: Messages consumed=%d		(should be %d)\n",consumer3->messages_consumed,TEST_SYNC_BROADCAST_MESSAGES_TO_BROADCAST);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 4	: Messages consumed=%d		(should be %d)\n",consumer4->messages_consumed,TEST_SYNC_BROADCAST_MESSAGES_TO_BROADCAST);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 5	: Messages consumed=%d		(should be %d)\n",consumer5->messages_consumed,TEST_SYNC_BROADCAST_MESSAGES_TO_BROADCAST);
	write_to_file(buffer);
	sprintf(buffer,"Producer	: Iterations=%d			(should be %d)\n",producer1->iterations,TEST_SYNC_BROADCAST_MESSAGES_TO_BROADCAST);
	write_to_file(buffer);

	mp_producer_destroy(producer1);
	mp_consumer_destroy(consumer1);
	mp_consumer_destroy(consumer2);
	mp_consumer_destroy(consumer3);
	mp_consumer_destroy(consumer4);
	mp_consumer_destroy(consumer5);
	mp_map_destroy(map);
	mp_destroybarrier(con,bar);
	mp_destroy(con);
	return result1 && result2 && result3 && result4 && result5 && result6;
}

/*************************************/
/*		SYNCHRONOUS SEND		 	 */
/*************************************/
int test_sync_send()
{
	char buffer[BUFFER_SIZE];
	context_t* con = mp_init();
	barrier_t* bar = mp_initbarrier(con,10);
	mp_map_ptr map = mp_map_create();
	mp_producer_ptr producer1 = mp_producer_create(con,bar,map,0,SEND_SYNC,TEST_SYNC_SEND_MESSAGES_TO_SEND,0);
	mp_producer_ptr producer2 = mp_producer_create(con,bar,map,0,SEND_SYNC | SEND_URGENT,TEST_SYNC_SEND_MESSAGES_TO_SEND,0);
	mp_producer_ptr producer3 = mp_producer_create(con,bar,map,0,SEND_SYNC,TEST_SYNC_SEND_MESSAGES_TO_SEND,0);
	mp_producer_ptr producer4 = mp_producer_create(con,bar,map,0,SEND_SYNC | SEND_URGENT,TEST_SYNC_SEND_MESSAGES_TO_SEND,0);
	mp_producer_ptr producer5 = mp_producer_create(con,bar,map,0,SEND_SYNC | SEND_URGENT,TEST_SYNC_SEND_MESSAGES_TO_SEND,0);

	mp_consumer_ptr consumer1 = mp_consumer_create(con,bar,map,0,0,0);
	mp_consumer_ptr consumer2 = mp_consumer_create(con,bar,map,0,0,0);
	mp_consumer_ptr consumer3 = mp_consumer_create(con,bar,map,0,0,0);
	mp_consumer_ptr consumer4 = mp_consumer_create(con,bar,map,0,0,0);
	mp_consumer_ptr consumer5 = mp_consumer_create(con,bar,map,0,0,0);

	mp_producer_register_consumer(producer1,consumer1);
	mp_producer_register_consumer(producer5,consumer1);

	mp_producer_register_consumer(producer1,consumer2);
	mp_producer_register_consumer(producer2,consumer2);
	mp_producer_register_consumer(producer3,consumer2);

	mp_producer_register_consumer(producer3,consumer3);
	mp_producer_register_consumer(producer4,consumer4);
	mp_producer_register_consumer(producer5,consumer5);

	pthread_t consumer1_id = create_thread(consume_messages,consumer1);
	pthread_t consumer2_id = create_thread(consume_messages,consumer2);
	pthread_t consumer3_id = create_thread(consume_messages,consumer3);
	pthread_t consumer4_id = create_thread(consume_messages,consumer4);
	pthread_t consumer5_id = create_thread(consume_messages,consumer5);
	pthread_t producer1_id = create_thread(produce_messages,producer1);
	pthread_t producer2_id = create_thread(produce_messages,producer2);
	pthread_t producer3_id = create_thread(produce_messages,producer3);
	pthread_t producer4_id = create_thread(produce_messages,producer4);
	pthread_t producer5_id = create_thread(produce_messages,producer5);

	pthread_join(consumer1_id,NULL);
	pthread_join(consumer2_id,NULL);
	pthread_join(consumer3_id,NULL);
	pthread_join(consumer4_id,NULL);
	pthread_join(consumer5_id,NULL);
	pthread_join(producer1_id,NULL);
	pthread_join(producer2_id,NULL);
	pthread_join(producer3_id,NULL);
	pthread_join(producer4_id,NULL);
	pthread_join(producer5_id,NULL);

	int result1 = (consumer1->messages_consumed == 2*TEST_SYNC_SEND_MESSAGES_TO_SEND) && (producer1->iterations == TEST_SYNC_SEND_MESSAGES_TO_SEND);
	int result2 = (consumer2->messages_consumed == 3*TEST_SYNC_SEND_MESSAGES_TO_SEND) && (producer2->iterations == TEST_SYNC_SEND_MESSAGES_TO_SEND);
	int result3 = (consumer3->messages_consumed == TEST_SYNC_SEND_MESSAGES_TO_SEND) && (producer3->iterations == TEST_SYNC_SEND_MESSAGES_TO_SEND);
	int result4 = (consumer4->messages_consumed == TEST_SYNC_SEND_MESSAGES_TO_SEND) && (producer4->iterations == TEST_SYNC_SEND_MESSAGES_TO_SEND);
	int result5 = (consumer5->messages_consumed == TEST_SYNC_SEND_MESSAGES_TO_SEND) && (producer5->iterations == TEST_SYNC_SEND_MESSAGES_TO_SEND);

	sprintf(buffer,"Consumer 1: Messages consumed=%d (should be %d), Producer 1: Iterations=%d (should be %d)\n",consumer1->messages_consumed,2*TEST_SYNC_SEND_MESSAGES_TO_SEND,producer1->iterations,TEST_SYNC_SEND_MESSAGES_TO_SEND);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 2: Messages consumed=%d (should be %d), Producer 2: Iterations=%d (should be %d)\n",consumer2->messages_consumed,3*TEST_SYNC_SEND_MESSAGES_TO_SEND,producer2->iterations,TEST_SYNC_SEND_MESSAGES_TO_SEND);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 3: Messages consumed=%d (should be %d), Producer 3: Iterations=%d (should be %d)\n",consumer3->messages_consumed,TEST_SYNC_SEND_MESSAGES_TO_SEND,producer3->iterations,TEST_SYNC_SEND_MESSAGES_TO_SEND);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 4: Messages consumed=%d (should be %d), Producer 4: Iterations=%d (should be %d)\n",consumer4->messages_consumed,TEST_SYNC_SEND_MESSAGES_TO_SEND,producer4->iterations,TEST_SYNC_SEND_MESSAGES_TO_SEND);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 5: Messages consumed=%d (should be %d), Producer 5: Iterations=%d (should be %d)\n",consumer5->messages_consumed,TEST_SYNC_SEND_MESSAGES_TO_SEND,producer5->iterations,TEST_SYNC_SEND_MESSAGES_TO_SEND);
	write_to_file(buffer);

	mp_producer_destroy(producer1);
	mp_producer_destroy(producer2);
	mp_producer_destroy(producer3);
	mp_producer_destroy(producer4);
	mp_producer_destroy(producer5);
	mp_consumer_destroy(consumer1);
	mp_consumer_destroy(consumer2);
	mp_consumer_destroy(consumer3);
	mp_consumer_destroy(consumer4);
	mp_consumer_destroy(consumer5);
	mp_map_destroy(map);
	mp_destroybarrier(con,bar);
	mp_destroy(con);
	return result1 && result2 && result3 && result4 && result5;
}

/*************************************/
/*		ASYNCHRONOUS RECEIVE		 */
/*************************************/
int test_async_recv()
{
	char buffer[BUFFER_SIZE];
	context_t* con = mp_init();
	barrier_t* bar = mp_initbarrier(con,8);
	mp_map_ptr map = mp_map_create();
	mp_producer_ptr producer1 = mp_producer_create(con,bar,map,0,0,TEST_ASYNC_RECV_MESSAGES_TO_SEND,WAIT_AT_START);
	mp_producer_ptr producer2 = mp_producer_create(con,bar,map,0,SEND_URGENT,TEST_ASYNC_RECV_MESSAGES_TO_SEND,WAIT_AT_START);
	mp_producer_ptr producer3 = mp_producer_create(con,bar,map,0,0,TEST_ASYNC_RECV_MESSAGES_TO_SEND,WAIT_AT_START);
	mp_producer_ptr producer4 = mp_producer_create(con,bar,map,0,SEND_URGENT,TEST_ASYNC_RECV_MESSAGES_TO_SEND,WAIT_AT_START);
	mp_producer_ptr producer5 = mp_producer_create(con,bar,map,0,0,TEST_ASYNC_RECV_MESSAGES_TO_SEND,WAIT_AT_START);

	mp_consumer_ptr consumer1 = mp_consumer_create(con,bar,map,0,TEST_ASYNC_RECV_MESSAGES_TO_SEND,0);
	mp_consumer_ptr consumer2 = mp_consumer_create(con,bar,map,0,2*TEST_ASYNC_RECV_MESSAGES_TO_SEND,0);
	mp_consumer_ptr consumer3 = mp_consumer_create(con,bar,map,0,3*TEST_ASYNC_RECV_MESSAGES_TO_SEND,0);

	mp_producer_register_consumer(producer1,consumer1);
	mp_producer_register_consumer(producer2,consumer2);
	mp_producer_register_consumer(producer4,consumer2);
	mp_producer_register_consumer(producer1,consumer3);
	mp_producer_register_consumer(producer3,consumer3);
	mp_producer_register_consumer(producer5,consumer3);

	pthread_t consumer1_id = create_thread(consume_messages,consumer1);
	pthread_t consumer2_id = create_thread(consume_messages,consumer2);
	pthread_t consumer3_id = create_thread(consume_messages,consumer3);
	pthread_t producer1_id = create_thread(produce_messages,producer1);
	pthread_t producer2_id = create_thread(produce_messages,producer2);
	pthread_t producer3_id = create_thread(produce_messages,producer3);
	pthread_t producer4_id = create_thread(produce_messages,producer4);
	pthread_t producer5_id = create_thread(produce_messages,producer5);

	usleep(TEST_ASYNC_RECV_DELAY);

	mp_producer_signal(producer1);
	mp_producer_signal(producer2);
	mp_producer_signal(producer3);
	mp_producer_signal(producer4);
	mp_producer_signal(producer5);

	pthread_join(consumer1_id,NULL);
	pthread_join(consumer2_id,NULL);
	pthread_join(consumer3_id,NULL);
	pthread_join(producer1_id,NULL);
	pthread_join(producer2_id,NULL);
	pthread_join(producer3_id,NULL);
	pthread_join(producer4_id,NULL);
	pthread_join(producer5_id,NULL);

	sprintf(buffer,"Consumer 1: Messages consumed=%d (should be %d), Iterations=%d (should be much more than %d)\n",consumer1->messages_consumed,TEST_ASYNC_RECV_MESSAGES_TO_SEND,consumer1->iterations,TEST_ASYNC_RECV_MESSAGES_TO_SEND);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 2: Messages consumed=%d (should be %d), Iterations=%d (should be much more than %d)\n",consumer2->messages_consumed,2*TEST_ASYNC_RECV_MESSAGES_TO_SEND,consumer2->iterations,2*TEST_ASYNC_RECV_MESSAGES_TO_SEND);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 3: Messages consumed=%d (should be %d), Iterations=%d (should be much more than %d)\n",consumer3->messages_consumed,3*TEST_ASYNC_RECV_MESSAGES_TO_SEND,consumer3->iterations,3*TEST_ASYNC_RECV_MESSAGES_TO_SEND);
	write_to_file(buffer);

	int result1 = (consumer1->messages_consumed == TEST_ASYNC_RECV_MESSAGES_TO_SEND) && (consumer1->iterations != TEST_ASYNC_RECV_MESSAGES_TO_SEND);
	int result2 = (consumer2->messages_consumed == 2*TEST_ASYNC_RECV_MESSAGES_TO_SEND) && (consumer2->iterations != 2*TEST_ASYNC_RECV_MESSAGES_TO_SEND);
	int result3 = (consumer3->messages_consumed == 3*TEST_ASYNC_RECV_MESSAGES_TO_SEND) && (consumer3->iterations != 3*TEST_ASYNC_RECV_MESSAGES_TO_SEND);

	mp_producer_destroy(producer1);
	mp_producer_destroy(producer2);
	mp_producer_destroy(producer3);
	mp_producer_destroy(producer4);
	mp_producer_destroy(producer5);
	mp_consumer_destroy(consumer1);
	mp_consumer_destroy(consumer2);
	mp_consumer_destroy(consumer3);
	mp_map_destroy(map);
	mp_destroybarrier(con,bar);
	mp_destroy(con);
	return result1 && result2 && result3;
}

/*************************************/
/*		SYNCHRONOUS RECEIVE			 */
/*************************************/
int test_sync_recv()
{
	char buffer[BUFFER_SIZE];
	context_t* con = mp_init();
	barrier_t* bar = mp_initbarrier(con,8);
	mp_map_ptr map = mp_map_create();
	mp_producer_ptr producer1 = mp_producer_create(con,bar,map,0,0,0,0);
	mp_producer_ptr producer2 = mp_producer_create(con,bar,map,0,SEND_URGENT,0,0);
	mp_producer_ptr producer3 = mp_producer_create(con,bar,map,0,0,0,0);
	mp_producer_ptr producer4 = mp_producer_create(con,bar,map,0,SEND_URGENT,0,0);
	mp_producer_ptr producer5 = mp_producer_create(con,bar,map,0,0,0,0);

	mp_consumer_ptr consumer1 = mp_consumer_create(con,bar,map,RECV_SYNC,3*TEST_SYNC_RECV_MESSAGES_TO_RECV,0);
	mp_consumer_ptr consumer2 = mp_consumer_create(con,bar,map,RECV_SYNC,2*TEST_SYNC_RECV_MESSAGES_TO_RECV,0);
	mp_consumer_ptr consumer3 = mp_consumer_create(con,bar,map,RECV_SYNC,TEST_SYNC_RECV_MESSAGES_TO_RECV,0);

	mp_producer_register_consumer(producer1,consumer1);
	mp_consumer_register_producer(consumer1,producer1);

	mp_producer_register_consumer(producer2,consumer2);
	mp_consumer_register_producer(consumer2,producer2);

	mp_producer_register_consumer(producer3,consumer3);
	mp_consumer_register_producer(consumer3,producer3);

	mp_producer_register_consumer(producer4,consumer2);
	mp_consumer_register_producer(consumer2,producer4);

	mp_producer_register_consumer(producer5,consumer1);
	mp_consumer_register_producer(consumer1,producer5);

	pthread_t consumer1_id = create_thread(consume_messages,consumer1);
	pthread_t producer1_id = create_thread(produce_messages,producer1);

	pthread_t consumer2_id = create_thread(consume_messages,consumer2);
	pthread_t consumer3_id = create_thread(consume_messages,consumer3);

	pthread_t producer2_id = create_thread(produce_messages,producer2);
	pthread_t producer3_id = create_thread(produce_messages,producer3);
	pthread_t producer4_id = create_thread(produce_messages,producer4);
	pthread_t producer5_id = create_thread(produce_messages,producer5);

	pthread_join(consumer1_id,NULL);
	pthread_join(consumer2_id,NULL);
	pthread_join(consumer3_id,NULL);
	pthread_join(producer1_id,NULL);
	pthread_join(producer2_id,NULL);
	pthread_join(producer3_id,NULL);
	pthread_join(producer4_id,NULL);
	pthread_join(producer5_id,NULL);

	sprintf(buffer,"Consumer 1: Messages consumed=%d (should be %d), Iterations=%d (should be %d)\n",3*TEST_SYNC_RECV_MESSAGES_TO_RECV,consumer1->messages_consumed,consumer1->iterations,3*TEST_SYNC_RECV_MESSAGES_TO_RECV);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 2: Messages consumed=%d (should be %d), Iterations=%d (should be %d)\n",2*TEST_SYNC_RECV_MESSAGES_TO_RECV,consumer2->messages_consumed,consumer2->iterations,2*TEST_SYNC_RECV_MESSAGES_TO_RECV);
	write_to_file(buffer);
	sprintf(buffer,"Consumer 3: Messages consumed=%d (should be %d), Iterations=%d (should be %d)\n",TEST_SYNC_RECV_MESSAGES_TO_RECV,consumer3->messages_consumed,consumer3->iterations,TEST_SYNC_RECV_MESSAGES_TO_RECV);
	write_to_file(buffer);

	int result1 = (consumer1->messages_consumed == 3*TEST_SYNC_RECV_MESSAGES_TO_RECV) && (consumer1->iterations == 3*TEST_SYNC_RECV_MESSAGES_TO_RECV);
	int result2 = (consumer2->messages_consumed == 2*TEST_SYNC_RECV_MESSAGES_TO_RECV) && (consumer2->iterations == 2*TEST_SYNC_RECV_MESSAGES_TO_RECV);
	int result3 = (consumer3->messages_consumed == TEST_SYNC_RECV_MESSAGES_TO_RECV) && (consumer3->iterations == TEST_SYNC_RECV_MESSAGES_TO_RECV);

	mp_producer_destroy(producer1);
	mp_producer_destroy(producer2);
	mp_producer_destroy(producer3);
	mp_producer_destroy(producer4);
	mp_producer_destroy(producer5);
	mp_consumer_destroy(consumer1);
	mp_consumer_destroy(consumer2);
	mp_consumer_destroy(consumer3);
	mp_map_destroy(map);
	mp_destroybarrier(con,bar);
	mp_destroy(con);
	return result1 && result2 && result3;
}

/*************************************/
/*		URGENT VS. REGULAR			 */
/*************************************/
int test_urgent_vs_regular()
{
	context_t* con = mp_init();
	barrier_t* bar = mp_initbarrier(con,8);
	mp_map_ptr map = mp_map_create();
	mp_producer_ptr producer1 = mp_producer_create(con,bar,map,0,0,TEST_URGENT_VS_REGULAR_MESSAGES_TO_SEND,WAIT_AT_START);
	mp_producer_ptr producer2 = mp_producer_create(con,bar,map,0,SEND_URGENT,TEST_URGENT_VS_REGULAR_MESSAGES_TO_SEND,WAIT_AT_START);
	mp_producer_ptr producer3 = mp_producer_create(con,bar,map,0,0,TEST_URGENT_VS_REGULAR_MESSAGES_TO_SEND,WAIT_AT_START);
	mp_producer_ptr producer4 = mp_producer_create(con,bar,map,0,0,TEST_URGENT_VS_REGULAR_MESSAGES_TO_SEND,WAIT_AT_START);
	mp_producer_ptr producer5 = mp_producer_create(con,bar,map,0,SEND_URGENT,TEST_URGENT_VS_REGULAR_MESSAGES_TO_SEND,WAIT_AT_START);

	mp_consumer_ptr consumer1 = mp_consumer_create(con,bar,map,0,5*TEST_URGENT_VS_REGULAR_MESSAGES_TO_SEND,WAIT_AT_START);
	mp_consumer_ptr consumer2 = mp_consumer_create(con,bar,map,0,5*TEST_URGENT_VS_REGULAR_MESSAGES_TO_SEND,WAIT_AT_START);
	mp_consumer_ptr consumer3 = mp_consumer_create(con,bar,map,0,5*TEST_URGENT_VS_REGULAR_MESSAGES_TO_SEND,WAIT_AT_START);

	mp_producer_register_consumer(producer1,consumer1);
	mp_producer_register_consumer(producer2,consumer1);
	mp_producer_register_consumer(producer3,consumer1);
	mp_producer_register_consumer(producer4,consumer1);
	mp_producer_register_consumer(producer5,consumer1);

	mp_producer_register_consumer(producer1,consumer2);
	mp_producer_register_consumer(producer2,consumer2);
	mp_producer_register_consumer(producer3,consumer2);
	mp_producer_register_consumer(producer4,consumer2);
	mp_producer_register_consumer(producer5,consumer2);

	mp_producer_register_consumer(producer1,consumer3);
	mp_producer_register_consumer(producer2,consumer3);
	mp_producer_register_consumer(producer3,consumer3);
	mp_producer_register_consumer(producer4,consumer3);
	mp_producer_register_consumer(producer5,consumer3);

	pthread_t consumer1_id = create_thread(consume_messages,consumer1);
	pthread_t consumer2_id = create_thread(consume_messages,consumer2);
	pthread_t consumer3_id = create_thread(consume_messages,consumer3);

	pthread_t producer1_id = create_thread(produce_messages,producer1);
	pthread_t producer2_id = create_thread(produce_messages,producer2);
	pthread_t producer3_id = create_thread(produce_messages,producer3);
	pthread_t producer4_id = create_thread(produce_messages,producer4);
	pthread_t producer5_id = create_thread(produce_messages,producer5);

	mp_producer_signal(producer1);
	pthread_join(producer1_id,NULL);
	mp_producer_signal(producer2);
	pthread_join(producer2_id,NULL);
	mp_producer_signal(producer3);
	pthread_join(producer3_id,NULL);
	mp_producer_signal(producer4);
	pthread_join(producer4_id,NULL);
	mp_producer_signal(producer5);
	pthread_join(producer5_id,NULL);
	mp_consumer_signal(consumer1);
	pthread_join(consumer1_id,NULL);
	mp_consumer_signal(consumer2);
	pthread_join(consumer2_id,NULL);
	mp_consumer_signal(consumer3);
	pthread_join(consumer3_id,NULL);

	int result = 	validate_message_list(consumer1->message_list) &&
					validate_message_list(consumer2->message_list) &&
					validate_message_list(consumer3->message_list);

	mp_producer_destroy(producer1);
	mp_producer_destroy(producer2);
	mp_producer_destroy(producer3);
	mp_producer_destroy(producer4);
	mp_producer_destroy(producer5);
	mp_consumer_destroy(consumer1);
	mp_consumer_destroy(consumer2);
	mp_consumer_destroy(consumer3);
	mp_map_destroy(map);
	mp_destroybarrier(con,bar);
	mp_destroy(con);
	return result;
}

/*************************************/
/*		BARRIER TEST				 */
/*************************************/
void* barrier_start(void* arg)
{

	mp_barrier_tester_ptr barrier_tester = arg;
	mp_barrier(barrier_tester->con,barrier_tester->bar);
	return NULL;
}

int test_barrier()
{
	char buffer[BUFFER_SIZE];
	context_t* con = mp_init();
	barrier_t* bar = mp_initbarrier(con,10);
	mp_barrier_tester_t barrier_tester;
	barrier_tester.bar = bar;
	barrier_tester.con = con;
	write_to_file("Launching thread 1...\n");
	pthread_t thread1_id = create_thread(barrier_start,&barrier_tester);
	write_to_file("Launching thread 2...\n");
	pthread_t thread2_id = create_thread(barrier_start,&barrier_tester);
	write_to_file("Launching thread 3...\n");
	pthread_t thread3_id = create_thread(barrier_start,&barrier_tester);
	write_to_file("Launching thread 4...\n");
	pthread_t thread4_id = create_thread(barrier_start,&barrier_tester);
	write_to_file("Launching thread 5...\n");
	pthread_t thread5_id = create_thread(barrier_start,&barrier_tester);
	write_to_file("Launching thread 6...\n");
	pthread_t thread6_id = create_thread(barrier_start,&barrier_tester);
	write_to_file("Launching thread 7...\n");
	pthread_t thread7_id = create_thread(barrier_start,&barrier_tester);
	write_to_file("Launching thread 8...\n");
	pthread_t thread8_id = create_thread(barrier_start,&barrier_tester);
	write_to_file("Launching thread 9...\n");
	pthread_t thread9_id = create_thread(barrier_start,&barrier_tester);
	write_to_file("Launching thread 10...\n");
	pthread_t thread10_id = create_thread(barrier_start,&barrier_tester);

	pthread_join(thread1_id,NULL);
	pthread_join(thread2_id,NULL);
	pthread_join(thread3_id,NULL);
	pthread_join(thread4_id,NULL);
	pthread_join(thread5_id,NULL);
	pthread_join(thread6_id,NULL);
	pthread_join(thread7_id,NULL);
	pthread_join(thread8_id,NULL);
	pthread_join(thread9_id,NULL);
	pthread_join(thread10_id,NULL);

	int result = 1;
	int barrier_result = mp_barrier(con,bar);
	if(barrier_result != -1)
		result = 0;

	sprintf(buffer,"Trying to call mp_barrier() again... returned with result = %d (should be -1)\n",barrier_result);
	write_to_file(buffer);

	mp_destroybarrier(con,bar);
	mp_destroy(con);
	return result;
}

/*************************************/
/*		RECEIVE AND REGISTERATION	 */
/*************************************/
void* test_receive_and_registeration_send_ok(void *arg)
{
	mp_recv_tester_ptr recv_tester = arg;
	mp_register(recv_tester->con);
	char buffer[100];
	sprintf(buffer,"Testing...");
	mp_send(recv_tester->con,&recv_tester->destination,buffer,strlen(buffer),0);
	mp_unregister(recv_tester->con);
	return NULL;
}

void* test_receive_and_registeration_send_too_big(void *arg)
{
	mp_recv_tester_ptr recv_tester = arg;
	mp_register(recv_tester->con);
	char buffer[70];
	sprintf(buffer,"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
	mp_send(recv_tester->con,&recv_tester->destination,buffer,strlen(buffer),0);
	mp_unregister(recv_tester->con);
	return NULL;
}

void* test_receive_and_registeration_keep_registeration(void *arg)
{
	mp_recv_tester_ptr recv_tester = arg;
	mp_register(recv_tester->con);
	return NULL;
}

void* test_receive_and_registeration_dont_keep_registeration(void *arg)
{
	mp_recv_tester_ptr recv_tester = arg;
	mp_register(recv_tester->con);
	mp_unregister(recv_tester->con);
	return NULL;
}

int test_receive_and_registeration()
{
	char text_buffer[BUFFER_SIZE];
	int final_result = 0;
	int len;
	char buffer[20];
	context_t* con = mp_init();
	int result;

	result = mp_register(con);
	if(result == 0)
		final_result++;

	sprintf(text_buffer,"Registering current thread........................................ returned %d (should be 0)\n",result);
	write_to_file(text_buffer);

	result = mp_register(con);
	if(result == -1)
		final_result++;

	sprintf(text_buffer,"Registering current thread again.................................. returned %d (should be -1)\n",result);
	write_to_file(text_buffer);

	result = mp_recv(con,buffer,20,&len,0);
	if((result == 0) && (len == 0))
		final_result++;

	sprintf(text_buffer,"Trying to dequeue an empty queue.................................. returned %d (should be 0), %d bytes read (should be 0)\n",result,len);
	write_to_file(text_buffer);

	mp_recv_tester_t recv_tester;
	recv_tester.con = con;
	recv_tester.destination = pthread_self();
	pthread_t sender1_id = create_thread(test_receive_and_registeration_send_ok,&recv_tester);
	pthread_join(sender1_id,NULL);
	pthread_t sender2_id = create_thread(test_receive_and_registeration_send_too_big,&recv_tester);
	pthread_join(sender2_id,NULL);
	pthread_t sender3_id = create_thread(test_receive_and_registeration_dont_keep_registeration,&recv_tester);
	pthread_t sender4_id = create_thread(test_receive_and_registeration_keep_registeration,&recv_tester);
	pthread_join(sender3_id,NULL);
	pthread_join(sender4_id,NULL);

	result = mp_recv(con,buffer,20,&len,0);
	if(!result && (len == 10))
		final_result++;

	sprintf(text_buffer,"Trying to dequeue a message of 10 bytes into a 20 bytes buffer.... returned %d (should be 0), %d bytes read (should read 10 bytes)\n",result,len);
	write_to_file(text_buffer);

	result = mp_recv(con,buffer,20,&len,0);
	if(result == 36)
		final_result++;

	sprintf(text_buffer,"Trying to dequeue a message of 36 bytes into a 20 bytes buffer.... returned %d (should be 36)\n",result);
	write_to_file(text_buffer);

	result = mp_send(con,&sender3_id,"Testing... 123",14,0);
	if(result == -1)
		final_result++;

	sprintf(text_buffer,"Trying to send a message to an unregistered thread................ returned %d (should be -1)\n",result);
	write_to_file(text_buffer);

	result = mp_send(con,&sender4_id,"Testing... 123",14,0);
	if(!result)
		final_result++;

	sprintf(text_buffer,"Trying to send a message to a registered thread................... returned %d (should be 0)\n",result);
	write_to_file(text_buffer);

	mp_unregister(con);
	mp_destroy(con);
	return (final_result == 7);
}

int main()
{
	fd = open("out", O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR | S_IWUSR);

  fprintf(stderr, "\n");
  fprintf(stderr, "===========================\n");
  fprintf(stderr, "Running Roy Velich's Tests:\n");
  fprintf(stderr, "===========================\n\n");
  fflush(stderr);

	printf("Starting test (It make take some time to complete):\n");
	printf("--------------------------------------------------\n");
  fflush(stdout);

	write_to_file("Barrier test results:\n");
	write_to_file("-------------------------------------------------------\n");
	printf("Testing barrier..................................... ");
  fflush(stdout);
	if(test_barrier())
		printf("OK\n");
	else
		printf("FAILED\n");
  fflush(stdout);

	write_to_file("\n");
	write_to_file("mp_recv() return values and registeration test results:\n");
	write_to_file("-------------------------------------------------------\n");
	printf("Testing mp_recv() return values and registeration... ");
  fflush(stdout);
	if(test_receive_and_registeration())
		printf("OK\n");
	else
		printf("FAILED\n");
  fflush(stdout);

	write_to_file("\n");
	write_to_file("Urgent message vs. regular message logic test results:\n");
	write_to_file("------------------------------------------------------\n");
	printf("Testing urgent message vs. regular message logic.... ");
  fflush(stdout);
	if(test_urgent_vs_regular())
		printf("OK\n");
	else
		printf("FAILED\n");
  fflush(stdout);

	write_to_file("\n");
	write_to_file("Asynchrounous receive test results:\n");
	write_to_file("-----------------------------------\n");
	printf("Testing asynchrounous receive....................... ");
  fflush(stdout);
	if(test_async_recv())
		printf("OK\n");
	else
		printf("FAILED\n");
  fflush(stdout);

	write_to_file("\n");
	write_to_file("Synchrounous receive test results:\n");
	write_to_file("----------------------------------\n");
	printf("Testing synchrounous receive........................ ");
  fflush(stdout);
	if(test_sync_recv())
		printf("OK\n");
	else
		printf("FAILED\n");
  fflush(stdout);

	write_to_file("\n");
	write_to_file("Asynchrounous send test results:\n");
	write_to_file("--------------------------------\n");
	printf("Testing asynchrounous send.......................... ");
  fflush(stdout);
	if(test_async_send())
		printf("OK\n");
	else
		printf("FAILED\n");
  fflush(stdout);

	write_to_file("\n");
	write_to_file("Synchrounous send test results:\n");
	write_to_file("-------------------------------\n");
	printf("Testing synchrounous send........................... ");
  fflush(stdout);
	if(test_sync_send())
		printf("OK\n");
	else
		printf("FAILED\n");
  fflush(stdout);

	write_to_file("\n");
	write_to_file("Asynchrounous broadcast test results:\n");
	write_to_file("-------------------------------------\n");
	printf("Testing asynchrounous broadcast..................... ");
  fflush(stdout);
	if(test_async_broadcast())
		printf("OK\n");
	else
		printf("FAILED\n");
  fflush(stdout);

	write_to_file("\n");
	write_to_file("Synchrounous broadcast test results:\n");
	write_to_file("------------------------------------\n");
	printf("Testing synchrounous broadcast...................... ");
  fflush(stdout);
	if(test_sync_broadcast())
		printf("OK\n");
	else
		printf("FAILED\n");
  fflush(stdout);

	printf("\nDone!\n");

	close(fd);
	return 0;
}
