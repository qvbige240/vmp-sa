/*
 ============================================================================
 Name        : bll.c
 Author      : wison.wei
 Copyright   : 2019(c) Timanetworks Company
 Description : 
 ============================================================================
 */
#define LOG_TAG    "bll"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vmp.h"
#include "tima_support.h"

#include "bll.h"
#include "context.h"
#include "ThreadPool.h"
#include "tcpserver.h"


static void PrintThreadPoolStats(void)
{
	ThreadPoolStats stats;
	ThreadPool *tp = Context()->tp;
	
	//ThreadPoolGetStats(tp, &stats);
	//printf("job(%d, %d, %d)\t"
	//	"wait(%lf, %lf, %lf)\t"
	//	"thread(%d, %d, %d, %d, %d)\t"
	//	"time(%lf, %lf)\n",
	//	stats.currentJobsHQ,
	//	stats.currentJobsMQ,
	//	stats.currentJobsLQ,
	//	stats.avgWaitHQ,
	//	stats.avgWaitMQ,
	//	stats.avgWaitLQ,
	//	stats.maxThreads,
	//	stats.workerThreads,
	//	stats.persistentThreads,
	//	stats.idleThreads,
	//	stats.totalThreads,
	//	stats.totalWorkTime,
	//	stats.totalIdleTime);

	ThreadPoolGetStats(tp, &stats);
	VMP_LOGN("job(%d, %d, %d)  "
		"wait(%lf, %lf, %lf)  "
		"thread(%d, %d, %d, %d, %d)  "
		"time(%lf, %lf)",
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


void bll_init(void)
{
	tima_log_init(0);
	context_init();
	tcpserver_init();
}

int bll_cond(void)
{
	return 1;
}

void bll_idle(void)
{
	PrintThreadPoolStats();
	sleep(10);
}

void bll_done(void)
{
	tcpserver_done();
	context_done();
}

