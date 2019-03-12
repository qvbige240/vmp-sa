/**
 * History:
 * ================================================================
 * 2019-03-11 qing.zou created
 *
 */

#include "context.h"
#include "ThreadPool.h"

#include "bll_h264_stream.h"

typedef struct _PrivInfo
{
	H264StreamReq		req;
	H264StreamRep		rep;

	int					id;
} PrivInfo;

int bll_h264_delete(vmp_node_t* p);

int h264_stream_callback(void* p, int msg, void* arg)
{
	vmp_node_t* demo = ((vmp_node_t*)p)->parent;
	if ( msg != NODE_SUCCESS)
	{
		VMP_LOGW("h264_stream_callback fail");
		bll_h264_delete(demo);
		return -1;
	}

	bll_h264_delete(demo);
	return 0;
}

static void *h264_stream_thread(void* arg)
{
	TIMA_LOGD("h264_stream_thread");

	vmp_node_t* p = (vmp_node_t*)arg;




	return NULL;
}

int bll_h264_get(vmp_node_t* p, int id, void* data, int size)
{
	return 0;
}

int bll_h264_set(vmp_node_t* p, int id, void* data, int size)
{	
	PrivInfo* thiz = p->private;
	thiz->req = *((H264StreamReq*)data);
	
	return 0;
}

void* bll_h264_start(vmp_node_t* p)
{
	VMP_LOGD("bll h264 stream start");

	PrivInfo* thiz = p->private;

	context * ctx = context_get();
	ThreadPoolJob job;

	TPJobInit( &job, ( start_routine) h264_stream_thread, p);
	TPJobSetFreeFunction( &job, ( free_routine ) NULL );
	TPJobSetStopFunction( &job, ( stop_routine ) NULL );
	TPJobSetPriority( &job, MED_PRIORITY );
	ThreadPoolAdd( ctx->tp, &job, &job.jobId );
	thiz->id = job.jobId;
	
	return NULL;
}

int bll_h264_stop(vmp_node_t* p)
{
	return 0;
}

vmp_node_t* bll_h264_create(void)
{
	vmp_node_t* p = NULL;

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));
		
		p = (vmp_node_t*)malloc(sizeof(vmp_node_t));
		memset(p, 0, sizeof(vmp_node_t));
		p->nclass		= BLL_H264STREAM_CLASS;
		p->pfn_get		= (nodeget)bll_h264_get;
		p->pfn_set		= (nodeset)bll_h264_set;
		p->pfn_start	= (nodestart)bll_h264_start;
		p->pfn_stop		= (nodestop)bll_h264_stop;
		p->pfn_callback = NULL;
		p->private		= thiz;

		return p;
	} while(0);
	
	return p;
}

int bll_h264_delete(vmp_node_t* p)
{
	//VMP_LOGD("bll_h264_delete");

	PrivInfo* thiz = (PrivInfo*)p->private;
	if(thiz != NULL)
	{
		free(thiz);
		p->private = NULL;
	}

	free(p);
	
	return 0;
}

static const nodedef node_h264_stream = 
{	
	sizeof(PrivInfo),
	BLL_H264STREAM_CLASS,
	(nodecreate)bll_h264_create,
	(nodedelete)bll_h264_delete
};

void bll_h264_init(void)
{
	VMP_LOGD("bll_h264_init");
	
	NODE_CLASS_REGISTER(node_h264_stream);
}

void bll_h264_done(void)
{
	VMP_LOGD("bll_h264_done");

	NODE_CLASS_UNREGISTER(BLL_H264STREAM_CLASS);
}
