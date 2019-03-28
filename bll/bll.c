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
#include "bll_rtmp_stream_demo.h"
#include "context.h"
#include "ThreadPool.h"
#include "tcpserver.h"
#include "tima.h"
#include "cache.h"


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

int on_tima_get_property_callback(void* p, int msg, void* arg)
{
	TimaGetPropertyRsp* res = (TimaGetPropertyRsp*)arg;

	if ( msg != NODE_SUCCESS)
	{
		TIMA_LOGW("do_get_property_callback fail");

		return -1;
	}

	TIMA_LOGI("do_get_property_callback msg=%d, url=%s, property=%s", msg, res->url, res->property);

	return 0;
}


static int do_tima_get_property(void* ss)
{
	node* p = NodeCreate(TIMA_GET_PROPERTY_CLASS);

	TimaGetPropertyReq req = {0};
	strcpy(req.simNo, "013800000000");
	req.chNo= 1;
	strcpy(req.url, "192.168.1.118:1935");
	p->pfnCb	= (nodecb)on_tima_get_property_callback;
	p->parent	=  NULL;
	p->pfnSet(p, 0, &req, sizeof(TimaGetPropertyReq));
	p->pfnStart(p);

	return 0;
}



void bll_init(char* pConf)
{
	tima_log_init(0);
	context_init(pConf);
	cache_init();
	tcpserver_init();
	tima_init();

	node* cache = NodeCreate(CACHE_CLASS);
	cache->pfnStart(cache);
	CacheNetworkConfig cfg = {0};
	cache->pfnGet(cache, CACHE_TIMA_NETWORK, &cfg, sizeof(CacheNetworkConfig));

	bll_demo_init();
	do_tima_get_property(NULL);
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
	tima_done();
	tcpserver_done();
	cache_done();
	context_done();
}

