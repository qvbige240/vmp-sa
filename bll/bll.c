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
#include "cache.h"

#include "bll.h"
#include "server_listener.h"
#include "server_websock.h"
#include "bll_core.h"
#include "bll_h264_stream.h"
#include "bll_http_base.h"
#include "bll_voice_task.h"
#include "bll_websock_ioa.h"
#include "bll_socket_ioa.h"

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

void bll_init(const char *conf)
{
	tima_log_init(0, conf);
	vmp_use_pthreads_init();
	context_init((void*)conf);
	cache_init();
	server_listener_init();
	server_websock_init();
	bll_h264_init();
	bll_hbase_init();
	bll_voice_init();
	bll_websockioa_init();
	bll_sockioa_init();

	bll_core_init();	// start
}

int bll_cond(void)
{
	return 1;
}

void bll_idle(void)
{
	PrintThreadPoolStats();
}

void bll_done(void)
{
	context_done();
}
