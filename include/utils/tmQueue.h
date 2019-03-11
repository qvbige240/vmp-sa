/*********************************************************************************
  *Copyright(C),2016-2017,timanetworks.com
  *FileName:  tmQueue.h
  *Author:  wison.wei
  *Description:  c queue
**********************************************************************************/

#ifndef _QUEUE_H
#define _QUEUE_H

#include <pthread.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QUEUE_INITIALIZER(buffer) { buffer, sizeof(buffer) / sizeof(buffer[0]), 0, 0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER }

typedef struct
{
	void **buffer;
	int capacity;
	int size;
	int in;
	int out;
	pthread_mutex_t mutex;
	pthread_cond_t cond_full;
	pthread_cond_t cond_empty;
} CQueue;

typedef CQueue * tmQueue;

void tmQueueEnqueue(tmQueue queue, void *value);
void *tmQueueDequeue(tmQueue queue);
int tmQueueSize(tmQueue queue);
int tmQueueClear(tmQueue queue);

#ifdef __cplusplus
}
#endif

#endif
