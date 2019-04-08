/**
 * History:
 * ================================================================
 * 2018-04-03 qing.zou created
 *
 */

#include <event2/event.h>
#include <event2/event_struct.h>

#include "bll_http_base.h"

#include "context.h"
#include "ThreadPool.h"

typedef struct _PrivInfo
{
	int				id;
	int				cond;

	struct event	timeout;
} PrivInfo;


static int bll_hbase_delete(vmp_node_t* p);

static int bll_hbase_get(vmp_node_t* p, int id, void* data, int size)
{
	return 0;
}

static int bll_hbase_set(vmp_node_t* p, int id, void* data, int size)
{	
	return 0;
}

static void timeout_cb(evutil_socket_t fd, short event, void *arg)
{
	//struct event *timeout = arg;
	//struct timeval tv;
	//evutil_timerclear(&tv);
	//tv.tv_sec = 500;
	//event_add(timeout, &tv);
}

static void *bll_hbase_thread(void* arg)
{
	vmp_node_t* p = (vmp_node_t*)arg;
	PrivInfo* thiz = NULL;
	return_val_if_fail(p != NULL, NULL);

	evthread_use_pthreads();

	thiz = (PrivInfo*)p->private;

	context* ctx = context_get();
	struct event_base **hbase = (struct event_base **)&ctx->hbase;
	*hbase = event_base_new();

	TIMA_LOGD("bll_hbase_thread start...");

	while (thiz->cond) {
		//int pipe_fd[2];
		//pipe(pipe_fd);

		//vpk_event_assign(&thiz->event, base, pipe_fd[0], VPK_EV_READ|VPK_EV_PERSIST, pipe_callback, NULL);
		//vpk_event_add(&thiz->event, NULL);

		//struct event timeout;
		struct timeval tv;
		int flags = EV_PERSIST;
		//int flags = 0;
		/* Initalize one event */
		event_assign(&thiz->timeout, *hbase, -1, flags, timeout_cb, (void*) &thiz->timeout);

		evutil_timerclear(&tv);
		tv.tv_sec = 60000;
		event_add(&thiz->timeout, &tv);

		event_base_dispatch(*hbase);
		event_base_free(*hbase);

		TIMA_LOGE("================= http base end ===================\n");
	}

	*hbase = NULL;

	bll_hbase_delete(p);

	return NULL;
}

static void* bll_hbase_start(vmp_node_t* p)
{
	VMP_LOGD("bll_hbase_start");

	PrivInfo* thiz = p->private;

	context* ctx = context_get();

	ThreadPoolJob job;
	TPJobInit(&job, (start_routine) bll_hbase_thread, p);
	TPJobSetFreeFunction(&job, (free_routine) NULL);
	TPJobSetPriority( &job, HIGH_PRIORITY );
	ThreadPoolAddPersistent(ctx->tp, &job, &job.jobId);
	thiz->id = job.jobId;

	return NULL;
}

static int bll_hbase_stop(vmp_node_t* p)
{
	return 0;
}

static vmp_node_t* bll_hbase_create(void)
{
	vmp_node_t* p = NULL;

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));

		thiz->cond = 1;
		
		p = (vmp_node_t*)malloc(sizeof(vmp_node_t));
		memset(p, 0, sizeof(vmp_node_t));
		p->nclass		= BLL_HTTPBASE_CLASS;
		p->pfn_get		= (nodeget)bll_hbase_get;
		p->pfn_set		= (nodeset)bll_hbase_set;
		p->pfn_start	= (nodestart)bll_hbase_start;
		p->pfn_stop		= (nodestop)bll_hbase_stop;
		p->pfn_callback = NULL;
		p->private		= thiz;

		return p;
	} while(0);
	
	return p;
}

static int bll_hbase_delete(vmp_node_t* p)
{
	PrivInfo* thiz = (PrivInfo*)p->private;
	if(thiz != NULL)
	{
		free(thiz);
		p->private = NULL;
	}

	free(p);
	
	return 0;
}

static const nodedef node_hbase = 
{	
	sizeof(PrivInfo),
	BLL_HTTPBASE_CLASS,
	(nodecreate)bll_hbase_create,
	(nodedelete)bll_hbase_delete
};

void bll_hbase_init(void)
{
	VMP_LOGI("bll_hbase_init");

	NODE_CLASS_REGISTER(node_hbase);
}

void bll_hbase_done(void)
{
	VMP_LOGI("bll_hbase_done");

	NODE_CLASS_UNREGISTER(BLL_HTTPBASE_CLASS);
}
