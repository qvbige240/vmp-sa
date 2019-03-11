/*
c-pthread-queue - c implementation of a bounded buffer queue using posix threads
Copyright (C) 2008  Matthew Dickinson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <pthread.h>
#include "tmQueue.h"

void tmQueueEnqueue(tmQueue queue, void *value)
{
	//int size = tmQueueSize(queue);
	pthread_mutex_lock(&(queue->mutex));
	while (queue->size == queue->capacity)
		pthread_cond_wait(&(queue->cond_full), &(queue->mutex));
	//printf("enqueue %d size=%d\n", *(int *)value, size);
	queue->buffer[queue->in] = value;
	++ queue->size;
	++ queue->in;
	queue->in %= queue->capacity;
	pthread_mutex_unlock(&(queue->mutex));
	pthread_cond_broadcast(&(queue->cond_empty));
}

void *tmQueueDequeue(tmQueue queue)
{
	//int size = tmQueueSize(queue);
	pthread_mutex_lock(&(queue->mutex));
	while (queue->size == 0){
		pthread_cond_wait(&(queue->cond_empty), &(queue->mutex));
	}
	void *value = queue->buffer[queue->out];
	//printf("dequeue %d size=%d\n", *(int *)value, size);
	-- queue->size;
	++ queue->out;
	queue->out %= queue->capacity;
	pthread_mutex_unlock(&(queue->mutex));
	pthread_cond_broadcast(&(queue->cond_full));
	return value;
}

int tmQueueSize(tmQueue queue)
{
	pthread_mutex_lock(&(queue->mutex));
	int size = queue->size;
	pthread_mutex_unlock(&(queue->mutex));
	return size;
}

int tmQueueClear(tmQueue queue)
{
	pthread_mutex_lock(&(queue->mutex));
	queue->size		= 0;
	queue->in		= 0;
	queue->out		= 0;
	pthread_mutex_unlock(&(queue->mutex));
	return 0;
}
