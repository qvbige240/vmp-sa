/*
 ============================================================================
 Name        : bll.c
 Author      : wison.wei
 Copyright   : 2019(c) Timanetworks Company
 Description : 
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tima_support.h"

#include "context.h"
#include "ThreadPool.h"

#include "bll.h"
#include "server_listener.h"
#include "bll_core.h"
#include "bll_h264_stream.h"

static void PrintThreadPoolStats(void)
{
	ThreadPoolStats stats;
	ThreadPool *tp = context_get()->tp;
	
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
	server_listener_init();
	bll_core_init();
	bll_h264_init();

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
	context_done();
}
