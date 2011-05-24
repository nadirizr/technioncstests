#include "mp_tester_library.h"

/****************************************/
/*			TEST MESSAGE LIST			*/
/****************************************/
mp_test_message_ptr mp_test_message_create(pthread_t message_source, int message_tag, int message_type, int src_id, int dest_id)
{
	mp_test_message_ptr message = (mp_test_message_ptr)malloc(sizeof(mp_test_message_t));
	message->next = NULL;
	message->message_source = message_source;
	message->message_tag = message_tag;
	message->message_type = message_type;
	message->dest_id = dest_id;
	message->src_id = src_id;
	return message;
}

mp_test_message_list_ptr mp_test_message_list_create()
{
	mp_test_message_list_ptr message_list = (mp_test_message_list_ptr)malloc(sizeof(mp_test_message_list_t));
	message_list->head = NULL;
	message_list->tail = NULL;

	return message_list;
}

void mp_test_message_list_destroy(mp_test_message_list_ptr message_list)
{
	if(message_list)
	{
		mp_test_message_ptr current = message_list->head;
		mp_test_message_ptr next;
		while(current)
		{
			next = current->next;
			free(current);
			current = next;
		}
		free(message_list);
	}
}

void mp_test_message_list_add(mp_test_message_list_ptr message_list,mp_test_message_ptr message)
{
	if(message_list)
	{
		if(message_list->tail == NULL)
		{
			message_list->head = message;
			message_list->tail = message;
		}
		else
		{
			message_list->tail->next = message;
			message_list->tail = message;
			message->next = NULL;
		}

	}
}

/****************************************/
/*				MAP						*/
/****************************************/
mp_map_ptr mp_map_create()
{
	mp_map_ptr map = (mp_map_ptr)malloc(sizeof(mp_map_t));
	map->next_id = 0;
	pthread_mutex_init(&map->mutex, NULL);
	return map;
}

void mp_map_destroy(mp_map_ptr map)
{
	if(map)
	{
		pthread_mutex_destroy(&map->mutex);
		free(map);
	}
}

int mp_map_register(mp_map_ptr map,pthread_t thread_id)
{
	if(map)
	{
		pthread_mutex_lock(&map->mutex);
		map->entries[map->next_id] = thread_id;
		int id = map->next_id++;
		pthread_mutex_unlock(&map->mutex);
		return id;
	}

	return -1;
}

pthread_t mp_map_get_by_index(mp_map_ptr map, int index)
{
	if(map)
	{
		pthread_mutex_lock(&map->mutex);
		pthread_t thread_id = map->entries[index];
		pthread_mutex_unlock(&map->mutex);
		return thread_id;
	}

	return -1;
}

/****************************************/
/*				CONSUMER				*/
/****************************************/
mp_consumer_ptr mp_consumer_create(context_t* con,barrier_t* bar,mp_map_ptr map,int flags,int messages_to_consume,int wait_on_start)
{
	mp_consumer_ptr consumer = (mp_consumer_ptr)malloc(sizeof(mp_consumer_t));
	consumer->con = con;
	consumer->bar = bar;
	consumer->messages_to_consume = messages_to_consume;
	consumer->iterations = 0;
	consumer->dependencies = 0;
	consumer->wait_on_start = wait_on_start;
	consumer->next_producer = 0;
	consumer->message_list = mp_test_message_list_create();
	consumer->flags = flags;
	consumer->map = map;
	consumer->messages_consumed = 0;
	pthread_mutex_init(&consumer->mutex, NULL);
	pthread_cond_init(&consumer->cond, NULL);

	int i;
	for(i = 0; i < MAX_PRODUCERS; i++)
	{
		consumer->producers[i] = NULL;
	}

	return consumer;
}

void mp_consumer_destroy(mp_consumer_ptr consumer)
{
	if(consumer)
	{
		pthread_mutex_destroy(&consumer->mutex);
		pthread_cond_destroy(&consumer->cond);
		mp_test_message_list_destroy(consumer->message_list);
		free(consumer);
	}
}

void mp_consumer_add_to_dependencies(mp_consumer_ptr consumer, int offset)
{
	if(consumer)
	{
		pthread_mutex_lock(&consumer->mutex);
		consumer->dependencies += offset;
		pthread_mutex_unlock(&consumer->mutex);
	}
}

int mp_consumer_get_dependencies(mp_consumer_ptr consumer)
{
	if(consumer)
	{
		pthread_mutex_lock(&consumer->mutex);
		int dependencies = consumer->dependencies;
		pthread_mutex_unlock(&consumer->mutex);
		return dependencies;
	}

	return -1;
}

void mp_consumer_increase_iterations(mp_consumer_ptr consumer)
{
	if(consumer)
	{
		pthread_mutex_lock(&consumer->mutex);
		consumer->iterations++;
		pthread_mutex_unlock(&consumer->mutex);
	}
}

int mp_consumer_get_iterations(mp_consumer_ptr consumer)
{
	if(consumer)
	{
		pthread_mutex_lock(&consumer->mutex);
		int iterations = consumer->iterations;
		pthread_mutex_unlock(&consumer->mutex);
		return iterations;
	}
	return -1;
}

void mp_consumer_signal(mp_consumer_ptr consumer)
{
	if(consumer)
	{
		pthread_mutex_lock(&consumer->mutex);
		consumer->wait_on_start = 0;
		pthread_cond_signal(&consumer->cond);
		pthread_mutex_unlock(&consumer->mutex);
	}
}

void mp_consumer_register_producer(mp_consumer_ptr consumer, mp_producer_ptr producer)
{
	if(producer && consumer)
	{
		if(consumer->next_producer < MAX_PRODUCERS)
		{
			consumer->producers[consumer->next_producer] = producer;
			consumer->next_producer++;
			mp_producer_add_to_dependencies(producer,1);
		}
	}
}

int mp_consumer_get_id(mp_consumer_ptr consumer)
{
	if(consumer)
	{
		pthread_mutex_lock(&consumer->mutex);
		int id = consumer->id;
		pthread_mutex_unlock(&consumer->mutex);
		return id;
	}
	return -1;
}
void mp_consumer_set_id(mp_consumer_ptr consumer, int id)
{
	if(consumer)
	{
		pthread_mutex_lock(&consumer->mutex);
		consumer->id = id;
		pthread_mutex_unlock(&consumer->mutex);
	}
}

/****************************************/
/*				PRODUCER				*/
/****************************************/
mp_producer_ptr mp_producer_create(context_t* con,barrier_t* bar,mp_map_ptr map,int broadcast,int flags,int messages_to_produce,int wait_on_start)
{
	mp_producer_ptr producer = (mp_producer_ptr)malloc(sizeof(mp_producer_t));
	producer->con = con;
	producer->bar = bar;
	producer->messages_to_produce = messages_to_produce;
	producer->iterations = 0;
	producer->next_consumer = 0;
	producer->flags = flags;
	producer->map = map;
	producer->wait_on_start = wait_on_start;
	producer->dependencies = 0;
	producer->messages_produced = 0;
	producer->broadcast = broadcast;
	pthread_mutex_init(&producer->mutex, NULL);
	pthread_cond_init(&producer->cond, NULL);

	int i;
	for(i = 0; i < MAX_CONSUMERS; i++)
	{
		producer->consumers[i] = NULL;
	}

	return producer;
}

void mp_producer_register_consumer(mp_producer_ptr producer, mp_consumer_ptr consumer)
{
	if(producer && consumer)
	{
		if(producer->next_consumer < MAX_CONSUMERS)
		{
			producer->consumers[producer->next_consumer] = consumer;
			producer->next_consumer++;
			mp_consumer_add_to_dependencies(consumer,1);
		}
	}
}

void mp_producer_destroy(mp_producer_ptr producer)
{
	if(producer)
	{
		pthread_mutex_destroy(&producer->mutex);
		pthread_cond_destroy(&producer->cond);
		free(producer);
	}
}

void mp_producer_add_to_dependencies(mp_producer_ptr producer, int offset)
{
	if(producer)
	{
		pthread_mutex_lock(&producer->mutex);
		producer->dependencies += offset;
		pthread_mutex_unlock(&producer->mutex);
	}
}

int mp_producer_get_dependencies(mp_producer_ptr producer)
{
	if(producer)
	{
		pthread_mutex_lock(&producer->mutex);
		int dependencies = producer->dependencies;
		pthread_mutex_unlock(&producer->mutex);
		return dependencies;
	}

	return -1;
}

void mp_producer_increase_iterations(mp_producer_ptr producer)
{
	if(producer)
	{
		pthread_mutex_lock(&producer->mutex);
		producer->iterations++;
		pthread_mutex_unlock(&producer->mutex);
	}
}

int mp_producer_get_iterations(mp_producer_ptr producer)
{
	if(producer)
	{
		pthread_mutex_lock(&producer->mutex);
		int iterations = producer->iterations;
		pthread_mutex_unlock(&producer->mutex);
		return iterations;
	}

	return -1;
}

void mp_producer_signal(mp_producer_ptr producer)
{
	if(producer)
	{
		pthread_mutex_lock(&producer->mutex);
		producer->wait_on_start = 0;
		pthread_cond_signal(&producer->cond);
		pthread_mutex_unlock(&producer->mutex);
	}
}

int mp_producer_get_id(mp_producer_ptr producer)
{
	if(producer)
	{
		pthread_mutex_lock(&producer->mutex);
		int id = producer->id;
		pthread_mutex_unlock(&producer->mutex);
		return id;
	}

	return -1;
}
void mp_producer_set_id(mp_producer_ptr producer, int id)
{
	if(producer)
	{
		pthread_mutex_lock(&producer->mutex);
		producer->id = id;
		pthread_mutex_unlock(&producer->mutex);
	}
}
