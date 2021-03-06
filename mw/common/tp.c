/*
 ============================================================================
 Name        : log.c
 Author      : wison.wei
 Copyright   : 2017(c) Timanetworks Company
 Description : 
 ============================================================================
 */

#define LOG_TAG    "TP"
	 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "context.h"
#include "ThreadPool.h"


#define TP_MAX_THREADS 			6000
#define TP_MIN_THREADS 			0
#define TP_JOBS_PER_THREAD 		1
#define TP_THREAD_IDLE_TIME 		5000
#define TP_MAX_JOBS_TOTAL 		6000


#ifdef TEST

static void 
PrintThreadPoolStats(ThreadPool *tp,
					 const char *msg)
{
	char log[1024]={0};
	ThreadPoolStats stats;

	log_d("PrintThreadPoolStats 1");
	
	ThreadPoolGetStats(tp, &stats);
	log_d("%s"
		"High Jobs pending: %d\n"
		"Med Jobs Pending: %d\n"
		"Low Jobs Pending: %d\n"
		"Average wait in High Q in milliseconds: %lf\n"
		"Average wait in Med Q in milliseconds: %lf\n"
		"Average wait in Low Q in milliseconds: %lf\n"
		"Max Threads Used: %d\n"
		"Worker Threads: %d\n"
		"Persistent Threads: %d\n"
		"Idle Threads: %d\n"
		"Total Threads: %d\n"
		"Total Work Time: %lf\n"
		"Total Idle Time: %lf\n",
		msg,
		stats.currentJobsHQ,
		stats.currentJobsMQ,
		stats.currentJobsLQ,
		stats.avgWaitHQ,
		stats.avgWaitMQ,
		stats.avgWaitLQ,
		stats.maxThreads,
		stats.workerThreads,
		stats.persistentThreads,
		stats.idleThreads,
		stats.totalThreads,
		stats.totalWorkTime,
		stats.totalIdleTime);

}
#endif

void* RunThread( void* param)
{
	do
	{
		printf("begin\n");
		sleep(2);
		printf("end\n");
	}while (0);

	return NULL;
}


void tp_init(void)
{
	context* p = context_get();
	ThreadPool* pTP = NULL;
	int nRet = -1;

	printf("TP_Init begin\n");

	pTP = (ThreadPool*)malloc(sizeof(ThreadPool));
	if(pTP)
	{
		ThreadPoolAttr attr;
		TPAttrInit(&attr);
		TPAttrSetMaxThreads(&attr, TP_MAX_THREADS);
		TPAttrSetMinThreads(&attr, TP_MIN_THREADS);
		TPAttrSetJobsPerThread(&attr, TP_JOBS_PER_THREAD);
		TPAttrSetIdleTime(&attr, TP_THREAD_IDLE_TIME);
		TPAttrSetMaxJobsTotal(&attr, TP_MAX_JOBS_TOTAL);

		nRet = ThreadPoolInit(pTP, &attr);
		if(nRet != 0)
		{
			printf("TP init failed, nRet=%d", nRet);
			return;
		}
		
		p->tp = pTP;
	}
#ifdef TEST
	int i;
	for (i=0; i< 100; i++)
	{
		ThreadPoolJob job;
		TPJobInit( &job, ( start_routine) RunThread, NULL);
		TPJobSetFreeFunction( &job, ( free_routine ) NULL );
		TPJobSetPriority( &job, MED_PRIORITY );
		ThreadPoolAdd( p->tp, &job, NULL );
	}

	while(1)
		{
			context *p = context_get();
			PrintThreadPoolStats(&p->tp, "");
			sleep(1);
		}

	log_d("pTP=%d", pTP);

	int id = 0;
	tmHttpPost("www.baidu.com", "test", p, RunThread, 3, NULL, &id);
	tmHttpPost("www.baidu.com", "test", p, RunThread, 3, NULL, &id);
	tmHttpPost("www.baidu.com", "test", p, RunThread, 3, NULL, &id);
#endif

	printf("TP_Init end\n");
}

void tp_done(void)
{
	context* p = context_get();

	if(p->tp)
	{
		ThreadPool* pTP = (ThreadPool*)p->tp;
		ThreadPoolShutdown(pTP);
		free(p->tp);
	}
}
