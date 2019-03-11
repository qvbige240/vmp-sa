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


#define TP_MAX_THREADS 			100
#define TP_MIN_THREADS 			0
#define TP_JOBS_PER_THREAD 		1
#define TP_THREAD_IDLE_TIME 		5000
#define TP_MAX_JOBS_TOTAL 		100

ThreadPool* tp_create()
{
	int nRet = -1;
	ThreadPool* pTP = NULL;

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
	}

	return pTP;
}

void tp_delete(ThreadPool* tp)
{
	if(tp)
	{
		ThreadPool* pTP = (ThreadPool*)tp;
		ThreadPoolShutdown(pTP);
		free(tp);
	}
}

void tp_init(void)
{
	context* p = Context();
	int nRet = -1;

	printf("TP_Init begin\n");

	p->tp = tp_create();
	p->tp_connect = tp_create();
	p->tp_transcode = tp_create();
	p->tp_push = tp_create();


	printf("TP_Init end\n");
}

void tp_done(void)
{
	context* p = Context();

	if(p->tp)
	{
		tp_delete(p->tp);
		p->tp = NULL;
	}

	if(p->tp_connect)
	{
		tp_delete(p->tp_connect);
		p->tp_connect= NULL;
	}

	if(p->tp_transcode)
	{
		tp_delete(p->tp_transcode);
		p->tp_transcode= NULL;
	}

	if(p->tp_push)
	{
		tp_delete(p->tp_push);
		p->tp_push = NULL;
	}
}

