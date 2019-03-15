/**
 * History:
 * ================================================================
 * 2019-03-11 qing.zou created
 *
 */

#include <pthread.h>

#include "context.h"
#include "ThreadPool.h"

#include "bll_h264_stream.h"

typedef struct _PrivInfo
{
	H264StreamReq		req;
	H264StreamRep		rep;

	int					id;
	int					cond;

	unsigned char		buff[1024];
} PrivInfo;

int bll_h264_delete(vmp_node_t* p);

static unsigned long get_thread_id(void)
{
	union {
		pthread_t thr;
		ev_uint64_t id;
	} r;
	memset(&r, 0, sizeof(r));
	r.thr = pthread_self();
	return (unsigned long)r.id;
}

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


void stream_socket_eventcb(struct bufferevent* bev, short event, void* arg)
{
	vmp_socket_t *s = (vmp_socket_t*)arg;
	vmp_node_t* p = (vmp_node_t*)s->priv;
	PrivInfo* thiz = (PrivInfo*)p->private;
	//printf("stream_socket_eventcb\n");

	if( event & (BEV_EVENT_EOF))
	{
		TIMA_LOGW("[%ld]Connection closed %d. EOF", thiz->req.flowid, thiz->req.client.fd);
	}
	else if( event & BEV_EVENT_ERROR)
	{
		TIMA_LOGW("stream_socket_eventcb ERROR\n");
	}
	else if( event & BEV_EVENT_TIMEOUT)
	{
		TIMA_LOGW("stream_socket_eventcb TIMEOUT\n");
	}
	else if (event & BEV_EVENT_CONNECTED)
	{
		TIMA_LOGI("stream_socket_eventcb CONNECTED\n");
		return;
	}

	TIMA_LOGW("stream_socket_eventcb event = 0x%2x\n", event);

	//bll_h264_delete(p);
	thiz->cond = 0;
}

//static size_t tima_packet_parser();

static void stream_input_handler(struct bufferevent *bev, void* arg)
{
	vmp_socket_t *s = (vmp_socket_t*)arg;
	vmp_node_t* p = (vmp_node_t*)s->priv;
	PrivInfo* thiz = (PrivInfo*)p->private;
	struct evbuffer* input = bufferevent_get_input(bev);
	size_t len = evbuffer_get_length(input);

	do 	 
	{
		size_t copy_len = evbuffer_copyout(input, thiz->buff, 1024);
		TIMA_LOGD("[%lld] %d recv[fd %d]: %s", 
			get_thread_id(), thiz->req.flowid, thiz->req.client.fd, thiz->buff);
		//size_t packet_len = hander_packet(thiz->buff, copy_len);


		//if (packet_len > 980)
		//{
		//	packet_len = 980;
		//	printf("#packet_len > 980\n");
		//}

		//if (copy_len < packet_len)
		//	break;

		evbuffer_remove(input, thiz->buff, copy_len); // clear the buffer

		len -= copy_len;

	} while (len > 0);
}

int client_connection_register(vmp_launcher_t *e, vmp_socket_t *s)
{
	//struct event_base* base = event_base_new();
	//s->event_base = base;
	s->bev = bufferevent_socket_new(e->event_base, s->fd, VMP_BUFFEREVENTS_OPTIONS);
	//debug_ptr_add(s->bev);
	bufferevent_setcb(s->bev, stream_input_handler, NULL, stream_socket_eventcb, s);
	//bufferevent_setwatermark(s->bev, EV_READ|EV_WRITE, 0, BUFFEREVENT_HIGH_WATERMARK);
	//bufferevent_setwatermark(s->bev, EV_WRITE, BUFFEREVENT_LOW_WATERMARK, BUFFEREVENT_HIGH_WATERMARK);
	bufferevent_enable(s->bev, EV_READ|EV_WRITE); /* Start reading. */
	return 0;
}

static void *h264_stream_thread(void* arg)
{
	TIMA_LOGD("h264_stream_thread %lld", get_thread_id());

	vmp_node_t* p = (vmp_node_t*)arg;
	PrivInfo* thiz = p->private;

	thiz->req.client.priv = p;

	TIMA_LOGD("========== flowid[%d] client[fd %d] register ==========", thiz->req.flowid, thiz->req.client.fd);
	client_connection_register(thiz->req.e, &thiz->req.client);

	//event_base_dispatch(thiz->req.client.event_base);

	while (1) {
		thiz->cond = 1;


		while (thiz->cond) {

			sleep(1);
			//thiz->cond = 0;
		}

		sleep(1);

		if (thiz->cond == 0)
			break;
	}

	TIMA_LOGD("========== flowid[%d] client[fd %d] end ==========", thiz->req.flowid, thiz->req.client.fd);

	bll_h264_delete(p);

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
	VMP_LOGD("bll_h264_start");

	PrivInfo* thiz = p->private;

	context * ctx = context_get();
	ThreadPoolJob job;

	TPJobInit( &job, ( start_routine) h264_stream_thread, p);
	TPJobSetFreeFunction( &job, ( free_routine ) NULL );
	TPJobSetStopFunction( &job, ( stop_routine ) NULL );
	//TPJobSetPriority( &job, MED_PRIORITY );
	//ThreadPoolAdd( ctx->tp, &job, &job.jobId );
	TPJobSetPriority( &job, HIGH_PRIORITY );
	ThreadPoolAddPersistent( ctx->tp, &job, &job.jobId);
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
	VMP_LOGD("bll_h264_delete");

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
	VMP_LOGI("bll_h264_init");
	
	NODE_CLASS_REGISTER(node_h264_stream);
}

void bll_h264_done(void)
{
	VMP_LOGI("bll_h264_done");

	NODE_CLASS_UNREGISTER(BLL_H264STREAM_CLASS);
}
