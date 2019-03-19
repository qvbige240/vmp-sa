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

#include "tima_jt1078_parser.h"
#include "tima_h264.h"
#include "rtmp_publish.h"

#define JT1078_STREAM_PACKAGE_SIZE		980

typedef void (*node_destroy)(void* ctx, void* data);

typedef struct nal_node_s {
	list_t				node;

	stream_object();
	//int					cid;			/* channel id */			
	//int					mtype;		/* media type video/audio */	
	//int					seq;		/* sequence */			
	//long				size;		/* package size */			
	//unsigned long long	sim;		/* sim number */
	//unsigned char*		package;

	node_destroy		destroy;
} nal_node_t;

typedef struct StreamChannel
{
	int					id;
	TimaBuffer			buffer;

	int					started;
	h264_meta_t			meta_data;

} StreamChannel;

typedef struct _PrivInfo
{
	H264StreamReq		req;
	H264StreamRep		rep;

	int					id;
	int					cond;

	unsigned long long	sim;
	StreamChannel		channel[8];

	void				*publish;
	void				*packager;

	unsigned char		buff[1024];

	list_t				nalu_head;
	int					list_size;
	pthread_mutex_t		list_mutex;
	//pthread_cond_t		cond_full;
	pthread_cond_t		cond_empty;
} PrivInfo;

static int bll_h264_delete(vmp_node_t* p);
static int rtmp_push_start(vmp_node_t* p, unsigned long long sim);


static void pkt_node_release(void *ctx, void *data)
{
	nal_node_t *node = data;
	if (node) {
		if (node->package)
			free(node->package);
		node->package = NULL;
		free(node);
	}
}
static nal_node_t* pkt_node_create(void *package, int size, int cid, int seq)
{
	nal_node_t* pkt = (nal_node_t*)calloc(1, sizeof(nal_node_t));
	//return_val_if_fail(pkt && size > 0, -1);

	if (pkt) {
		INIT_LIST_HEAD(&pkt->node);

		pkt->seq		= seq;
		pkt->cid		= cid;
		pkt->size		= size;
		pkt->package	= package;
		pkt->destroy	= pkt_node_release;
		//pkt->package	= (char*)pkt + sizeof(nal_node_t);
		//memcpy(pkt->buffer, buf, size);
	} else {
		TIMA_LOGE("node create failed, malloc null");
	}

	return pkt;
}

static int list_add_nalu(void* p, void* data)
{
	PrivInfo* thiz = NULL;
	nal_node_t* nalu = (nal_node_t*)data;

	thiz = ((vmp_node_t*)p)->private;

	pthread_mutex_lock(&thiz->list_mutex);

	list_add_tail(&nalu->node, &thiz->nalu_head);
	if (thiz->list_size++ == 0)
		pthread_cond_broadcast(&thiz->cond_empty);

	pthread_mutex_unlock(&thiz->list_mutex);

	return 0;
}

static void* list_del_nalu(void* p)
{
	PrivInfo* thiz = NULL;
	nal_node_t* nalu = NULL;

	thiz = ((vmp_node_t*)p)->private;

	pthread_mutex_lock(&thiz->list_mutex);

	if (list_empty(&thiz->nalu_head)) {
		pthread_cond_wait(&thiz->cond_empty, &thiz->list_mutex);
	}

	nalu = container_of(thiz->nalu_head.next, nal_node_t, node);
	list_del(thiz->nalu_head.next);
	thiz->list_size--;

	pthread_mutex_unlock(&thiz->list_mutex);

	return nalu;
}

static int list_traverse(vmp_node_t* p, pub_callback proc, void* ctx)
{
	int ret = -1;
	return_val_if_fail(p && proc != NULL && ctx, -1);
	//PrivInfo* thiz = (PrivInfo*)p->private;

	nal_node_t *nal_node = list_del_nalu(p);

	if (nal_node) {
		ret = proc(ctx, ((void*)nal_node + sizeof(list_t)), NULL);
		if (ret < 0) {
			TIMA_LOGE("nalu process failed");
			// retry...
		}

		nal_node->destroy(NULL, nal_node);
	} else {
		TIMA_LOGE("nalu node get failed");
		usleep(20000);
	}

	return ret;
}

#if 0
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

static int h264_stream_callback(void* p, int msg, void* arg)
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

#endif

static void stream_socket_eventcb(struct bufferevent* bev, short event, void* arg)
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


static int h264_stream_proc(vmp_node_t* p, const char* buf, size_t size, StreamChannel *channel)
{
	size_t pos = 0, len;
	size_t length = size;
	const char *data = buf;
	PrivInfo* thiz = (PrivInfo*)p->private;

	void *pkt = NULL;
	nal_node_t *nalu_node = NULL;

	//int i = 0;
	//for (i = 0; i < 32; i++)
	//	printf("%02x ", (unsigned char)data[i]);
	//printf("\n");

	tima_buffer_strdup(&channel->buffer, data, length, 1);
	data = tima_buffer_data(&channel->buffer, 0);
	length = tima_buffer_used(&channel->buffer);

	//size_t s = tima_buffer_size(&thiz->buffer);
	//if (s > 64 << 10)
	//	printf("buffer total size: %d\n", s);

	if (!channel->started) {

		int i = 0;
		for (i = 0; i < 32; i++)
			printf("%02x ", (unsigned char)data[i]);
		printf("\n");

		pos = h264_metadata_get(data, length, 0, &channel->meta_data);
		if (pos == 0) {
			VMP_LOGE("stream parse error at metadata");
			return -1;
		}
		channel->started = 1;

		//void* rtmp_data_pack(void* packager, const char* data, int length)
		pkt = rtmp_meta_pack(thiz->packager, channel->meta_data.data, channel->meta_data.size);
		if (!pkt) {
			TIMA_LOGE("rtmp_meta_pack failed");
			return -1;
		}
		nalu_node = pkt_node_create(pkt, 0, channel->id, 0);
		list_add_nalu(p, nalu_node);
		//RTMPPacket meta = thiz->packager->meta_pack(chunk_buffer, channel->meta_data.data, thiz->meta_data.size);
		//tima_rtmp_send(thiz->publisher, &meta, timestamp);
	}

	do 
	{
		h264_nalu_t nalu;
		len = h264_nalu_read(data, length, pos, &nalu);
		if (len != 0) {
			pos += len;
			TIMA_LOGD("## nalu type(%d) len(%d)", nalu.nalu_type, nalu.nalu_len);

			//send to server
			if (nalu.nalu_type == 0x05) {
				//RTMPPacket meta = thiz->packager->meta_pack(chunk_buffer, thiz->meta_data.data, thiz->meta_data.size);
				//tima_rtmp_send(thiz->publisher, &meta, timestamp);
				pkt = rtmp_meta_pack(thiz->packager, channel->meta_data.data, channel->meta_data.size);
				if (!pkt) {
					TIMA_LOGE("rtmp_meta_pack failed");
					return -1;
				}
				nalu_node = pkt_node_create(pkt, 0, channel->id, 0);
				list_add_nalu(p, nalu_node);
			} else if (nalu.nalu_type == 0x07 || nalu.nalu_type == 0x08 || nalu.nalu_type == 0x06) {
				continue;
			}

			//RTMPPacket packet = thiz->packager->data_pack(chunk_buffer, nalu.nalu_data, nalu.nalu_len);
			//tima_rtmp_send(thiz->publisher, &packet, timestamp);

			pkt = rtmp_data_pack(thiz->packager, nalu.nalu_data, nalu.nalu_len);
			if (!pkt) {
				TIMA_LOGE("rtmp_data_pack failed");
				return -1;
			}
			nalu_node = pkt_node_create(pkt, 0, channel->id, 0);
			list_add_nalu(p, nalu_node);
		}
	} while (len);

	tima_buffer_align(&channel->buffer, pos);

	return 0;
}

static int media_stream_proc(vmp_node_t* p, struct bufferevent *bev/*, vmp_socket_t *s*/)
{
	int ret = 0;
	PrivInfo* thiz = (PrivInfo*)p->private;
	struct evbuffer* input = bufferevent_get_input(bev);
	size_t len = evbuffer_get_length(input);

	if (input) {
		unsigned char *stream = NULL;
		stream_header_t head = {0};
		ev_ssize_t clen = evbuffer_copyout(input, thiz->buff, JT1078_STREAM_PACKAGE_SIZE);
		if (clen > 0) {
			if (clen > JT1078_STREAM_PACKAGE_SIZE)
				clen = JT1078_STREAM_PACKAGE_SIZE;

			ret = packet_jt1078_parse(thiz->buff, clen, &head, &stream);
			if (ret == 0)
				return 0;

			if (ret < 0)
				TIMA_LOGE("JT/T 1078-2016 parse failed");

			//int ii = 0;
			//for (ii = 0; ii < 8; ii++)
			//{
			//    printf("%02x ", thiz->buff[16+ii]);
			//}

			//printf("[len=%5ld]#sim=%lld, channelid=%d, type[15]=%02x, [28:29]=%02x %02x, copy len=%ld, body len=%d, parsed=%d\n",
			//	len, head.simno, thiz->buff[14], thiz->buff[15], thiz->buff[28], thiz->buff[29], clen, head.bodylen, ret);



			if (ret > JT1078_STREAM_PACKAGE_SIZE) {
				ret = JT1078_STREAM_PACKAGE_SIZE;
				TIMA_LOGE("JT/T 1078-2016 parse failed");
			}

			if (thiz->sim == (unsigned long long)-1) {
				thiz->sim = head.simno;
				TIMA_LOGI("sim no. [%lld]", thiz->sim);

				rtmp_push_start(p, thiz->sim);
			}
			
			if ((head.mtype & 0xf0) != 0x30) {	// audio

			} else if ((head.mtype & 0xf0) != 0x40) {	// transparent

			} else {	// video
				h264_stream_proc(p, (const char*)stream, head.bodylen, &thiz->channel[head.channel]);
			}

			//evbuffer_remove(input, thiz->buff, ret);

		} else if (clen < 0) {
			TIMA_LOGE("buffer copy out failed, maybe closed");
			ret = -1;
		}
	} else {
		TIMA_LOGE("socket input failed, socket to be closed");
		ret = -1;
	}
	
	return ret;
}

static void stream_input_handler(struct bufferevent *bev, void* arg)
{
	int ret = 0;
	vmp_socket_t *s = (vmp_socket_t*)arg;
	vmp_node_t* p = (vmp_node_t*)s->priv;
	PrivInfo* thiz = (PrivInfo*)p->private;

	struct evbuffer* input = bufferevent_get_input(bev);
	size_t len = evbuffer_get_length(input);

	do
	{
		ret = media_stream_proc(p, bev);

		if (ret > 0) {

			evbuffer_remove(input, thiz->buff, ret);
			len -= ret;
		} else {
			//usleep(110000);
			break;
		}


	} while (len > 0);
}

int client_connection_register(vmp_launcher_t *e, vmp_socket_t *s)
{
	//struct event_base* base = event_base_new();
	//s->event_base = base;
	s->bev = bufferevent_socket_new(e->event_base, s->fd, VMP_BUFFEREVENTS_OPTIONS);
	bufferevent_setcb(s->bev, stream_input_handler, NULL, stream_socket_eventcb, s);
	//bufferevent_setwatermark(s->bev, EV_READ|EV_WRITE, 0, BUFFEREVENT_HIGH_WATERMARK);
	//bufferevent_setwatermark(s->bev, EV_WRITE, BUFFEREVENT_LOW_WATERMARK, BUFFEREVENT_HIGH_WATERMARK);
	bufferevent_enable(s->bev, EV_READ|EV_WRITE); /* Start reading. */
	return 0;
}


static int rtmp_push_start(vmp_node_t* p, unsigned long long sim)
{
	context* ctx = context_get();
	vmp_node_t* pub = node_create(RTMP_PUBLISH_CLASS, ctx->vector_node);

	RtmpPublishReq req = {0};
	req.sim			= sim;
	req.traverse	= list_traverse;
	pub->parent	= p;
	pub->pfn_set(pub, 0, &req, sizeof(RtmpPublishReq));
	pub->pfn_start(pub);

	return 0;
}

static void h264_stream_init(PrivInfo* thiz)
{
	int i = 0;
	for (i = 0; i < 8; i++) {
		thiz->channel[i].id = i;
		tima_buffer_init(&thiz->channel[i].buffer, 128<<10);
	}
	thiz->sim = (unsigned long long)-1;

	TIMA_LOGD("========== flowid[%d] client[fd %d] register ==========", thiz->req.flowid, thiz->req.client.fd);
	client_connection_register(thiz->req.e, &thiz->req.client);
}

//static void *h264_stream_thread(void* arg)
//{
//	TIMA_LOGD("h264_stream_thread");
//
//	vmp_node_t* p = (vmp_node_t*)arg;
//	PrivInfo* thiz = p->private;
//
//	thiz->req.client.priv = p;
//
//	//TIMA_LOGD("========== flowid[%d] client[fd %d] register ==========", thiz->req.flowid, thiz->req.client.fd);
//	//client_connection_register(thiz->req.e, &thiz->req.client);
//	h264_stream_init(thiz);
//
//	while (1) {
//		thiz->cond = 1;
//
//
//		while (thiz->cond) {
//
//			sleep(1);
//			//thiz->cond = 0;
//		}
//
//		sleep(1);
//
//		if (thiz->cond == 0)
//			break;
//	}
//
//	TIMA_LOGD("========== flowid[%d] client[fd %d] end ==========", thiz->req.flowid, thiz->req.client.fd);
//
//	bll_h264_delete(p);
//
//	return NULL;
//}

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
	thiz->req.client.priv = p;

	//int i = 0;
	//for (i = 0; i < 8; i++) {
	//	thiz->channel[i].id = i;
	//	tima_buffer_init(&thiz->channel[i].buffer, 128<<10);
	//}
	//thiz->sim = (unsigned long long)-1;

	//TIMA_LOGD("========== flowid[%d] client[fd %d] register ==========", thiz->req.flowid, thiz->req.client.fd);
	//client_connection_register(thiz->req.e, &thiz->req.client);

#if 1

	h264_stream_init(thiz);
#else
	context * ctx = context_get();
	ThreadPoolJob job;

	TPJobInit( &job, ( start_routine) h264_stream_thread, p);
	TPJobSetFreeFunction( &job, ( free_routine ) NULL );
	TPJobSetStopFunction( &job, ( stop_routine ) NULL );
	TPJobSetPriority( &job, HIGH_PRIORITY );
	ThreadPoolAddPersistent( ctx->tp, &job, &job.jobId);
	thiz->id = job.jobId;

#endif
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
