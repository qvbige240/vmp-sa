/**
 * History:
 * ================================================================
 * 2019-03-15 qing.zou created
 *
 */

#include <pthread.h>

#include "librtmp/rtmp.h"

#include "context.h"
#include "ThreadPool.h"

#include "rtmp_publish.h"

#include "tima_rtmp_packager.h"
#include "tima_rtmp_publisher.h"

typedef struct stream_attribute_s
{
	stream_object();
	//int					cid;		/* channel id */			
	//int					mtype;		/* media type video/audio */	
	//int					seq;		/* sequence */			
	//long					size;		/* package size */			
	//unsigned long long	sim;		/* sim number */
	//unsigned char*		package;
} stream_attribute_t;

typedef struct PublishInfo
{

	int					connected;
	TimaRTMPPublisher	*publisher;
} PublishInfo;


typedef struct _PrivInfo
{
	RtmpPublishReq		req;
	RtmpPublishRep		rep;

	int					id;
	int					cond;

	//PublishInfo			pub[8];
	PublishInfo			pub;
	//TimaRTMPPublisher	*publisher[8];
} PrivInfo;

static int rtmp_publish_delete(vmp_node_t* p);
static int rtmp_publish_connect(vmp_node_t* p);
static int rtmp_publish_disconnect(vmp_node_t* p);

#if 0
static int rtmp_publish_callback(void* p, int msg, void* arg)
{
	vmp_node_t* demo = ((vmp_node_t*)p)->parent;
	if ( msg != NODE_SUCCESS)
	{
		VMP_LOGW("rtmp_publish_callback fail");
		rtmp_publish_delete(demo);
		return -1;
	}

	rtmp_publish_delete(demo);
	return 0;
}
#endif

#if 1
static unsigned long long get_thread_id(void)
{
	union {
		pthread_t thr;
		unsigned long long id;
	} r;
	memset(&r, 0, sizeof(r));
	r.thr = pthread_self();
	return (unsigned long long)r.id;
}
#endif

static int rtmp_publish_release(vmp_node_t* p)
{
	PrivInfo* thiz = p->private;

	nodecb	pfn_callback;
	void*	pfn_ctx;

	if(thiz->req.pfncb) {
		pfn_callback = thiz->req.pfncb;
		pfn_ctx	= p->parent;
	}

	rtmp_publish_disconnect(p);
	rtmp_publish_delete(p);

	if (pfn_callback)
		pfn_callback(pfn_ctx, NODE_SUCCESS, NULL);

	return 0;
}

static int rtmp_stream_pub(void* ctx, void* data, void* result)
{
	int ret = 0;
	PrivInfo* thiz = NULL;
	vmp_node_t* p = (vmp_node_t*)ctx;
	stream_attribute_t* stream = data;
	return_val_if_fail(ctx && data, -1);

	thiz = p->private;

	//tima_rtmp_send(thiz->pub[stream->cid-1].publisher, stream->package, timestamp);
	ret = tima_rtmp_send(thiz->pub.publisher, (RTMPPacket*)stream->package, 0);
	if (ret < 0)
		TIMA_LOGW("Thread %p %lld tima_rtmp_send, ret = %d", get_thread_id(), thiz->req.sim, ret);

	return ret;
}

static void rtma_publish_proc(vmp_node_t* p)
{
	PrivInfo* thiz = p->private;
	RtmpPublishReq* req = &thiz->req;

	if (req->traverse) {
		req->traverse(p->parent, rtmp_stream_pub, p);
	} else {
		TIMA_LOGE("list_traverse_callback unregister");
		//sleep(2);
		thiz->cond = 0;
	}
}

static void *rtmp_publish_thread(void* arg)
{
	TIMA_LOGD("rtmp_publish_thread");

	vmp_node_t* p = (vmp_node_t*)arg;
	PrivInfo* thiz = p->private;

	rtmp_publish_connect(p);

	while (1) 
	{		
		TIMA_LOGD("start rtma_publish_proc");

		while (thiz->cond) {

			rtma_publish_proc(p);

		}

		if (thiz->cond == 0)
			break;
	}

	rtmp_publish_release(p);
	//rtmp_publish_delete(p);

	return NULL;
}

static int rtmp_publish_get(vmp_node_t* p, int id, void* data, int size)
{
	return 0;
}

static int rtmp_publish_set(vmp_node_t* p, int id, void* data, int size)
{	
	PrivInfo* thiz = p->private;
	
	thiz->id = id;
	switch (id)
	{
	case RTMP_PUB_STATE_TYPE_START:
		thiz->req = *((RtmpPublishReq*)data);
		break;
	case RTMP_PUB_STATE_TYPE_TIMEOUT:
	case RTMP_PUB_STATE_TYPE_EOF:
	case RTMP_PUB_STATE_TYPE_ERROR:
		thiz->cond = 0;
		break;
	}

	return 0;
}

static int rtmp_publish_connect(vmp_node_t* p)
{
	int ret;
	char url[256] = {0};
	PrivInfo* thiz = p->private;
	//const char *uri = "rtmp://172.20.25.47:1935/hls/";
	//const char *uri = "rtmp://172.20.25.47:1935/live/";
	//const char *uri = "rtmp://192.168.1.113:1935/live/";
	//const char *uri = "rtmp://192.168.1.118:1935/live/";
	const char *uri = "rtmp://172.20.25.228:1935/live/";
	//const char *uri = "rtmp://127.0.0.1:1935/live/";

	//for (i = 0; i < _countof(thiz->pub); i++)
	//{
	//	//sprintf(url, "%s%d", uri, i+1);
	//	snprintf(url, sizeof(url), "%s%lld_%d", uri, thiz->req.sim, i+1);
	//	TIMA_LOGI("connect to %s", url);
	//	thiz->pub[i].publisher = tima_rtmp_create(url);
	//	ret = tima_rtmp_connect(thiz->pub[i].publisher);
	//	if (ret < 0) {
	//		TIMA_LOGW("tima_rtmp_connect failed");
	//	} else {
	//		thiz->pub[i].connected = 1;
	//	}
	//}
	snprintf(url, sizeof(url), "%s%lld_%d", uri, thiz->req.sim, thiz->req.channel);
	TIMA_LOGI("%p connect to %s", get_thread_id(), url);
	thiz->pub.publisher = tima_rtmp_create(url);
	ret = tima_rtmp_connect(thiz->pub.publisher);
	if (ret < 0) {
		TIMA_LOGW("tima_rtmp_connect failed");
	} else {
		thiz->pub.connected = 1;
	}
	return 0;
}

static int rtmp_publish_disconnect(vmp_node_t* p)
{
	PrivInfo* thiz = p->private;
	if (thiz->pub.connected) {
		tima_rtmp_destory(thiz->pub.publisher);
		thiz->pub.publisher = NULL;
		thiz->pub.connected = 0;
	}

	return 0;
}

static void* rtmp_publish_start(vmp_node_t* p)
{
	VMP_LOGD("rtmp_publish_start");

	PrivInfo* thiz = p->private;

	context * ctx = context_get();
	ThreadPoolJob job;

	TPJobInit( &job, ( start_routine) rtmp_publish_thread, p);
	TPJobSetFreeFunction( &job, ( free_routine ) NULL );
	TPJobSetStopFunction( &job, ( stop_routine ) NULL );
	TPJobSetPriority( &job, HIGH_PRIORITY );
	ThreadPoolAddPersistent( ctx->tp, &job, &job.jobId);
	thiz->id = job.jobId;
	
	return NULL;
}

static int rtmp_publish_stop(vmp_node_t* p)
{
	return 0;
}

static vmp_node_t* rtmp_publish_create(void)
{
	vmp_node_t* p = NULL;

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));

		thiz->cond = 1;

		p = (vmp_node_t*)malloc(sizeof(vmp_node_t));
		memset(p, 0, sizeof(vmp_node_t));
		p->nclass		= RTMP_PUBLISH_CLASS;
		p->pfn_get		= (nodeget)rtmp_publish_get;
		p->pfn_set		= (nodeset)rtmp_publish_set;
		p->pfn_start	= (nodestart)rtmp_publish_start;
		p->pfn_stop		= (nodestop)rtmp_publish_stop;
		p->pfn_callback = NULL;
		p->private		= thiz;

		return p;
	} while(0);
	
	return p;
}

static int rtmp_publish_delete(vmp_node_t* p)
{
	VMP_LOGD("rtmp_publish_delete %p", get_thread_id());

	PrivInfo* thiz = (PrivInfo*)p->private;
	if(thiz != NULL)
	{
		free(thiz);
		p->private = NULL;
	}

	free(p);
	
	return 0;
}

static const nodedef node_rtmp_publish = 
{	
	sizeof(PrivInfo),
	RTMP_PUBLISH_CLASS,
	(nodecreate)rtmp_publish_create,
	(nodedelete)rtmp_publish_delete
};

void rtmp_publish_init(void)
{
	VMP_LOGI("rtmp_publish_init");
	
	NODE_CLASS_REGISTER(node_rtmp_publish);
}

void rtmp_publish_done(void)
{
	VMP_LOGI("rtmp_publish_done");

	NODE_CLASS_UNREGISTER(RTMP_PUBLISH_CLASS);
}


void* rtmp_data_pack(void* p, const char* data, int length)
{
	TimaRTMPPackager* packager = p;
	int size = sizeof(RTMPPacket) + packager->body_len(length);
	RTMPPacket *packet = calloc(1, size);

	if (!packet) {
		TIMA_LOGE("malloc failed");
		return NULL;
	}
	packager->data_pack(packet, data, length);

	return packet;
}

void* rtmp_meta_pack(void* p, const char* data, int length)
{
	TimaRTMPPackager* packager = p;
	int size = sizeof(RTMPPacket) + length + 8 + RTMP_MAX_HEADER_SIZE;
	RTMPPacket *packet = calloc(1, size);

	if (!packet) {
		TIMA_LOGE("malloc failed");
		return NULL;
	}
	packager->meta_pack(packet, data, length);

	return packet;
}
