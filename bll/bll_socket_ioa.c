/**
 * History:
 * ================================================================
 * 2019-03-11 qing.zou created
 *
 */

#include <pthread.h>

#include "context.h"
#include "ThreadPool.h"

#include "bll_socket_ioa.h"
#include "server_listener.h"

#include "event2/bufferevent_compat.h"

#include "tima_jt1078_parser.h"

#define JT1078_STREAM_PACKAGE_SIZE		980

//typedef void (*node_destroy)(void* ctx, void* data);
//
//typedef struct nal_node_s {
//	list_t				node;
//
//	stream_object();
//	//int					cid;			/* channel id */			
//	//int					mtype;		/* media type video/audio */	
//	//int					seq;		/* sequence */			
//	//long				size;		/* package size */			
//	//unsigned long long	sim;		/* sim number */
//	//unsigned char*		package;
//
//	node_destroy		destroy;
//} nal_node_t;

typedef struct StreamChannel
{
	int					id;
	TimaBuffer			buffer;

	int					started;
	//h264_meta_t			meta_data;

} StreamChannel;

typedef struct _PrivInfo
{
	SocketIOAReq		req;
	SocketIOARep		rep;

	int					id;
	int					cond;
	volatile int		running;

	//int					wmark;
	//int					wm_time;
	//size_t				wm_count;

	unsigned long long	sim;
	StreamChannel		channel;

	VmpSocketIOA		*sock;

	unsigned char		buff[1024];

	//list_t				nalu_head;
	//int					list_size;
	pthread_mutex_t		list_mutex;
	pthread_cond_t		cond_empty;
} PrivInfo;

static int bll_sockioa_stream_close(vmp_node_t* p);
static int bll_sockioa_delete(vmp_node_t* p);
static int rtmp_push_start(vmp_node_t* p, unsigned long long sim, char channel, char* uri);
static int rtmp_push_end(vmp_node_t* p, int state);


//static int stream_nalu_wmcnt(vmp_node_t* p, size_t size)
//{
//	PrivInfo* thiz = (PrivInfo*)p->private;
//
//	if (thiz->wmark)
//		return 0;
//
//	if (thiz->wm_time++ < 4) {
//		thiz->wm_count += size;
//	} else {
//		thiz->wmark = (thiz->wm_count/thiz->wm_time) > (8 << 10) ? 1 : -1;
//	}
//
//	return 0;
//}
//static inline void stream_set_wartermark(vmp_node_t* p, struct bufferevent *bev)
//{
//	PrivInfo* thiz = (PrivInfo*)p->private;
//	if (thiz->wmark == 1) {
//		bufferevent_setwatermark(bev, EV_READ, VOI_BUFFEREVENT_LOW_WATERMARK, VOI_BUFFEREVENT_HIGH_WATERMARK);
//		thiz->wmark = 2;
//	}
//}
//
//static void pkt_node_release(void *ctx, void *data)
//{
//	nal_node_t *node = data;
//	if (node) {
//		if (node->package)
//			free(node->package);
//		node->package = NULL;
//		free((void*)node);
//	}
//}
//static nal_node_t* pkt_node_create(void *package, int size, int cid, int seq)
//{
//	nal_node_t* pkt = (nal_node_t*)calloc(1, sizeof(nal_node_t));
//	//return_val_if_fail(pkt && size > 0, -1);
//
//	if (pkt) {
//		INIT_LIST_HEAD(&pkt->node);
//
//		pkt->seq		= seq;
//		pkt->cid		= cid;
//		pkt->size		= size;
//		pkt->package	= package;
//		pkt->destroy	= pkt_node_release;
//		//pkt->package	= (char*)pkt + sizeof(nal_node_t);
//		//memcpy(pkt->buffer, buf, size);
//	} else {
//		TIMA_LOGE("node create failed, malloc null");
//	}
//
//	return pkt;
//}
//
//static int list_clear_out(void* p)
//{
//	nal_node_t* nalu = NULL;
//	PrivInfo* thiz = ((vmp_node_t*)p)->private;
//
//	pthread_mutex_lock(&thiz->list_mutex);
//
//	list_t *pos, *n;
//	list_for_each_safe(pos, n, &thiz->nalu_head)
//	{
//		nalu = container_of(pos, nal_node_t, node);
//		if (nalu) {
//			list_del(pos);
//			nalu->destroy(NULL, nalu);
//			thiz->list_size--;
//		}
//	}
//
//	pthread_mutex_unlock(&thiz->list_mutex);
//
//	return 0;
//}
//
//static int list_add_nalu(void* p, void* data)
//{
//	PrivInfo* thiz = NULL;
//	nal_node_t* nalu = (nal_node_t*)data;
//
//	thiz = ((vmp_node_t*)p)->private;
//
//	pthread_mutex_lock(&thiz->list_mutex);
//
//	list_add_tail(&nalu->node, &thiz->nalu_head);
//	if (thiz->list_size++ == 0)
//		pthread_cond_broadcast(&thiz->cond_empty);
//
//	pthread_mutex_unlock(&thiz->list_mutex);
//
//	return 0;
//}
//
//static void* list_del_nalu(void* p)
//{
//	PrivInfo* thiz = NULL;
//	nal_node_t* nalu = NULL;
//
//	thiz = ((vmp_node_t*)p)->private;
//
//	pthread_mutex_lock(&thiz->list_mutex);
//
//	if (list_empty(&thiz->nalu_head)) {
//		pthread_cond_wait(&thiz->cond_empty, &thiz->list_mutex);
//	}
//
//	if (thiz->list_size > 0) {
//		nalu = container_of(thiz->nalu_head.next, nal_node_t, node);
//		list_del(thiz->nalu_head.next);
//		thiz->list_size--;
//		if (!nalu)
//			TIMA_LOGE("nalu node get failed, null");
//		//else
//		if (thiz->list_size > 100)
//			TIMA_LOGI("%lld [%d] list_size: %d", thiz->sim, nalu->cid, thiz->list_size);
//	} else {
//		TIMA_LOGD("pthread_cond_wait end");
//	}
//
//	pthread_mutex_unlock(&thiz->list_mutex);
//
//	return nalu;
//}
//
//static int list_traverse(vmp_node_t* p, pub_callback proc, void* ctx)
//{
//	int ret = -1;
//	return_val_if_fail(p && proc != NULL && ctx, -1);
//	//PrivInfo* thiz = (PrivInfo*)p->private;
//
//	nal_node_t *nal_node = list_del_nalu(p);
//
//	if (nal_node) {
//		ret = proc(ctx, ((void*)nal_node + sizeof(list_t)), NULL);
//		if (ret < 0) {
//			TIMA_LOGE("nalu process failed");
//			bll_sockioa_stream_close(p);
//		}
//
//		nal_node->destroy(NULL, nal_node);
//	} else {
//		//usleep(20000);
//	}
//
//	return ret;
//}

#if 1
static unsigned long long get_thread_id(void)
{
	union {
		pthread_t thr;
		ev_uint64_t id;
	} r;
	memset(&r, 0, sizeof(r));
	r.thr = pthread_self();
	return (unsigned long long)r.id;
}
#endif


static int socket_input_release(vmp_node_t* p)
{
	if (p) 
	{
		PrivInfo* thiz = (PrivInfo*)p->private;

		if (p->pfn_callback) {
			p->pfn_callback(thiz->req.s, NODE_SUCCESS, NULL);
		}

		if (thiz->req.client.bev) {
			bufferevent_free(thiz->req.client.bev);
			thiz->req.client.bev = NULL;
		}

		vmp_socket_release(thiz->sock);

		tima_buffer_clean(&thiz->channel.buffer);
		//if (thiz->channel.meta_data.data) {
		//	free(thiz->channel.meta_data.data);
		//	thiz->channel.meta_data.data = NULL;
		//}

		//list_clear_out(p);

		bll_sockioa_delete(p);
	}

	return 0;
}

static int socket_input_callback(void* p, int msg, void* arg)
{
	vmp_node_t* n = (vmp_node_t*)p;
	if ( msg != NODE_SUCCESS)
	{
		VMP_LOGW("socket_input_callback fail");
		socket_input_release(n);
		//bll_sockioa_delete(n);
		return -1;
	}

	socket_input_release(n);
	//bll_sockioa_delete(n);
	return 0;
}

static int client_connection_close(vmp_socket_t *s, int immediately)
{
	vmp_node_t* p = (vmp_node_t*)s->priv;
	PrivInfo* thiz = (PrivInfo*)p->private;
	struct bufferevent *bev = s->bev;

	bufferevent_flush(bev, EV_READ|EV_WRITE, BEV_FLUSH); 
	bufferevent_disable(bev, EV_READ|EV_WRITE);

	thiz->cond = 0;
	usleep(5000);

	while(thiz->running) {
		usleep(100000);
	}

	TIMA_LOGW("[%ld] Connection closed. (%lld_%d fd %d)", 
		thiz->req.flowid, thiz->sim, thiz->channel.id, s->fd);

	TIMA_LOGD("================ closed fd %d ================", s->fd);
	if (immediately) {
		//bufferevent_free(bev);
		socket_input_release(p);
	} else {
		//if (thiz->publish)
		//	rtmp_push_end(p, RTMP_PUB_STATE_TYPE_EOF);
		//else
			socket_input_release(p);
	}

	return 0;
}

static int bll_sockioa_stream_close(vmp_node_t* p)
{
	PrivInfo* thiz = (PrivInfo*)p->private;

	client_connection_close(&thiz->req.client, 0);

	return 0;
}

static void stream_socket_eventcb(struct bufferevent* bev, short event, void* arg)
{
	vmp_socket_t *s = (vmp_socket_t*)arg;
	vmp_node_t* p = (vmp_node_t*)s->priv;
	PrivInfo* thiz = (PrivInfo*)p->private;

	if( event & (BEV_EVENT_EOF)) {
		TIMA_LOGW("[%ld] Connection closed. (%lld_%d fd %d) EOF (0x%2x)", 
			thiz->req.flowid, thiz->sim, thiz->channel.id, thiz->req.client.fd, event);
	} else if( event & BEV_EVENT_ERROR) {
		TIMA_LOGW("[%ld] (%lld_%d fd %d) socket ERROR (0x%2x)\n", 
			thiz->req.flowid, thiz->sim, thiz->channel.id, thiz->req.client.fd, event);
	} else if( event & BEV_EVENT_TIMEOUT) {
		TIMA_LOGW("[%ld] (%lld_%d fd %d) socket TIMEOUT (0x%2x)\n", 
			thiz->req.flowid, thiz->sim, thiz->channel.id, thiz->req.client.fd, event);
	} else if (event & BEV_EVENT_CONNECTED) {
		TIMA_LOGI("[%ld] stream_socket_eventcb CONNECTED (0x%2x)\n", thiz->req.flowid, event);
		return;
	} else {
		TIMA_LOGW("[%ld] (%lld_%d fd %d) stream_socket_eventcb event = 0x%2x\n", 
			thiz->req.flowid, thiz->sim, thiz->channel.id, thiz->req.client.fd, event);
	}

	bufferevent_flush(bev, EV_READ|EV_WRITE, BEV_FLUSH); 
	bufferevent_disable(bev, EV_READ|EV_WRITE); 
	//bufferevent_free(bev);

	thiz->cond = 0;
	//if (thiz->publish)
	//	rtmp_push_end(p, RTMP_PUB_STATE_TYPE_EOF);
	//else
		socket_input_release(p);
}


static int socket_input_proc(vmp_node_t* p, const char* buf, size_t size, StreamChannel *channel)
{
	size_t pos = 0, len;
	size_t length = size;
	const char *data = buf;
	PrivInfo* thiz = (PrivInfo*)p->private;

	len = vpk_udp_send(thiz->sock->abs.fd, &thiz->sock->dest_addr, buf, size);
	TIMA_LOGD("relay send len = %d", len);

	//void *pkt = NULL;
	//nal_node_t *nalu_node = NULL;

	////int i = 0;
	////for (i = 0; i < 32; i++)
	////	printf("%02x ", (unsigned char)data[i]);
	////printf("\n");

	//tima_buffer_strdup(&channel->buffer, data, length, 1);
	//data = tima_buffer_data(&channel->buffer, 0);
	//length = tima_buffer_used(&channel->buffer);

	////size_t s = tima_buffer_size(&thiz->buffer);
	////if (s > 64 << 10)
	////	printf("buffer total size: %d\n", s);

	//if (!channel->started) {

	//	int i = 0;
	//	for (i = 0; i < 32; i++)
	//		printf("%02x ", (unsigned char)data[i]);
	//	printf("\n");

	//	pos = h264_metadata_get(data, length, 0, &channel->meta_data);
	//	if (pos == 0) {
	//		VMP_LOGE("stream parse error at metadata");
	//		return -1;
	//	}
	//	channel->started = 1;

	//	pkt = rtmp_meta_pack(thiz->packager, channel->meta_data.data, channel->meta_data.size);
	//	if (!pkt) {
	//		TIMA_LOGE("rtmp_meta_pack failed");
	//		return -1;
	//	}
	//	nalu_node = pkt_node_create(pkt, 0, channel->id, 0);
	//	list_add_nalu(p, nalu_node);
	//} else {
	//	if (length > size + 3) {
	//		if (!h264_has_start_code(data, length, length-size-3)) {
	//			//TIMA_LOGD("don't have start code h264_has_start_code");
	//			return 0;
	//		}
	//	}
	//}

	//do 
	//{
	//	h264_nalu_t nalu;
	//	len = h264_nalu_read(data, length, pos, &nalu);
	//	if (len != 0) {
	//		pos += len;
	//		TIMA_LOGD("## %lld nalu type(%d) len(%d)", thiz->sim, nalu.nalu_type, nalu.nalu_len);

	//		if (!thiz->wmark)
	//			stream_nalu_wmcnt(p, nalu.nalu_len);

	//		if (nalu.nalu_type == 0x05) {
	//			pkt = rtmp_meta_pack(thiz->packager, channel->meta_data.data, channel->meta_data.size);
	//			if (!pkt) {
	//				TIMA_LOGE("rtmp_meta_pack failed");
	//				return -1;
	//			}
	//			nalu_node = pkt_node_create(pkt, 0, channel->id, 0);
	//			list_add_nalu(p, nalu_node);

	//		} else if (nalu.nalu_type == 0x07 || nalu.nalu_type == 0x08 || nalu.nalu_type == 0x06) {
	//			continue;
	//		}

	//		pkt = rtmp_data_pack(thiz->packager, nalu.nalu_data, nalu.nalu_len);
	//		if (!pkt) {
	//			TIMA_LOGE("rtmp_data_pack failed");
	//			return -1;
	//		}
	//		nalu_node = pkt_node_create(pkt, 0, channel->id, 0);
	//		list_add_nalu(p, nalu_node);
	//	}
	//} while (len);

	//tima_buffer_align(&channel->buffer, pos);

	return 0;
}

#if 0
static int vpk_file_save(const char* filename, void* data, size_t size)
{
	FILE* fp = 0;
	size_t ret = 0;
	//return_val_if_fail(filename != NULL && data != NULL, -1);

	fp = fopen(filename, "a+");
	if (fp != NULL && data)
	{
		ret = fwrite(data, 1, size, fp);
		fclose(fp);
	}
	if (ret != size)
		printf("fwrite size(%ld != %ld) incorrect!", ret, size);

	return ret;
}
#endif

static int media_stream_proc(vmp_node_t* p, struct bufferevent *bev/*, vmp_socket_t *s*/)
{
	int ret = 0;
	PrivInfo* thiz = (PrivInfo*)p->private;
	struct evbuffer* input = bufferevent_get_input(bev);
	//size_t len = evbuffer_get_length(input);

	if (input) {
		unsigned char *stream = NULL;
		stream_header_t head = {0};
		memset(thiz->buff, 0x00, sizeof(thiz->buff));
		ev_ssize_t clen = evbuffer_copyout(input, thiz->buff, JT1078_STREAM_PACKAGE_SIZE);
		if (clen > 0) {
			if (clen > JT1078_STREAM_PACKAGE_SIZE)
				clen = JT1078_STREAM_PACKAGE_SIZE;

			ret = packet_jt1078_parse(thiz->buff, clen, &head, &stream);
			if (ret == 0)
				return 0;

			if (ret < 0) {
				TIMA_LOGE("[%ld] JT/T 1078-2016 parse failed, ret = %d", thiz->req.flowid, ret);
				TIMA_LOGD("=====flowid[%ld] %lld[fd %d]", thiz->req.flowid, thiz->sim, thiz->req.client.fd);
				goto parse_end;
			}

			//printf("[len=%5ld]#sim=%lld, channelid=%d, type[15]=%02x, [28:29]=%02x %02x, copy len=%ld, body len=%d, parsed=%d\n",
			//	len, head.simno, head.channel, thiz->buff[15], thiz->buff[28], thiz->buff[29], clen, head.bodylen, ret);

			if (ret > JT1078_STREAM_PACKAGE_SIZE) {
				TIMA_LOGD("=====flowid[%d] %lld[fd %d], ret = %d", thiz->req.flowid, thiz->sim, thiz->req.client.fd, ret);
				TIMA_LOGE("[%ld] JT/T 1078-2016 parse failed, ret = %d", thiz->req.flowid, ret);
				ret = JT1078_STREAM_PACKAGE_SIZE;
				goto parse_end;
			}

			if (thiz->sim == (unsigned long long)-1) {
				thiz->sim = head.simno;
				thiz->channel.id = head.channel;
				TIMA_LOGI("[%ld] %p fd=%d sim no. [%lld]: %d", 
					thiz->req.flowid, get_thread_id(), thiz->req.client.fd, thiz->sim, head.channel);

//#ifdef _TEST
//				char uri[256];
//				snprintf(uri, sizeof(uri), "/live/%lld_%d", thiz->sim, head.channel);
//				rtmp_push_start(p, thiz->sim, head.channel, uri);
//#else
//				stream_url_query(p, thiz->sim, head.channel);
//#endif
			}
			
			if ((head.mtype & 0xf0) == 0x30) {	// audio

				thiz->channel.id = head.channel;

#if 1
				//socket_input_proc(p, (const char*)stream, head.bodylen, &thiz->channel);
				socket_input_proc(p, (const char*)thiz->buff, head.bodylen+30, &thiz->channel);
#else	// write back to device
				int ret = bufferevent_write(bev, thiz->buff, head.bodylen+30);
				VMP_LOGD("bufferevent_write len=%d", head.bodylen+30);
#endif

			} else if ((head.mtype & 0xf0) == 0x40) {	// transparent

			} else {	// video

			}

		} else if (clen < 0) {
			TIMA_LOGE("[%ld] buffer copy out failed, maybe closed", thiz->req.flowid);
			ret = -1;
		}
	} else {
		TIMA_LOGE("[%ld] socket input failed, socket to be closed", thiz->req.flowid);
		ret = -1;
	}

parse_end:
	return ret;
}

static void stream_input_handler(struct bufferevent *bev, void* arg)
{
	int ret = 0;
	vmp_socket_t *s = (vmp_socket_t*)arg;
	vmp_node_t* p = (vmp_node_t*)s->priv;
	PrivInfo* thiz = (PrivInfo*)p->private;

	if (!thiz->cond)
		return;

	thiz->running = 1;

	struct evbuffer* input = bufferevent_get_input(bev);
	size_t len = evbuffer_get_length(input);

	//evbuffer_drain(input, len);
	//return;

	//stream_set_wartermark(p, bev);

	do
	{
		ret = media_stream_proc(p, bev);

		if (ret > 0) {

			evbuffer_remove(input, thiz->buff, ret);
			//vpk_file_save("./cif_raw_data.video", thiz->buff, ret);
			len -= ret;
		} else {
			//usleep(110000);
			break;
		}

	} while (len > 30 && thiz->cond);

	thiz->running = 0;
}

static int client_connection_register(vmp_launcher_t *e, vmp_socket_t *s)
{
	s->bev = bufferevent_socket_new(s->event_base, s->fd, VMP_BUFFEREVENTS_OPTIONS);
	bufferevent_setcb(s->bev, stream_input_handler, NULL, stream_socket_eventcb, s);
	//bufferevent_setwatermark(s->bev, EV_READ, 0, VOI_BUFFEREVENT_HIGH_WATERMARK);
	//bufferevent_settimeout(s->bev, 60, 0);
	bufferevent_enable(s->bev, EV_READ|EV_WRITE); /* Start reading. */
	return 0;
}

//
//static int rtmp_push_end(vmp_node_t* p, int state)
//{
//	PrivInfo* thiz = p->private;
//	vmp_node_t* pub = thiz->publish;
//
//	if (pub) {
//		pub->pfn_set(pub, state, NULL, 0);
//		thiz->publish = NULL;
//	}
//
//	//pthread_mutex_lock(&thiz->list_mutex);
//	
//	pthread_cond_broadcast(&thiz->cond_empty);
//
//	//pthread_mutex_unlock(&thiz->list_mutex);
//
//	return 0;
//}
//
//static int rtmp_push_start(vmp_node_t* p, unsigned long long sim, char channel, char* uri)
//{
//	PrivInfo* thiz = p->private;
//	context* ctx = context_get();
//	vmp_node_t* pub = node_create(RTMP_PUBLISH_CLASS, ctx->vector_node);
//
//	RtmpPublishReq req = {0};
//	strncpy(req.uri, uri, sizeof(req.uri));
//	req.flowid		= thiz->req.flowid;
//	req.sim			= sim;
//	req.channel		= channel;
//	req.traverse	= list_traverse;
//	req.pfncb		= socket_input_callback;
//	pub->parent		= p;
//	//pub->pfn_callback	= socket_input_callback;
//	pub->pfn_set(pub, RTMP_PUB_STATE_TYPE_START, &req, sizeof(RtmpPublishReq));
//	pub->pfn_start(pub);
//
//	thiz->publish = pub;
//
//	return 0;
//}
//
//#if 1
//#include "tima_get_property.h"
//static int url_query_callback(void* p, int msg, void* arg)
//{
//	vmp_node_t* n = (vmp_node_t*)p;
//	PrivInfo* thiz = n->private;
//	if ( msg != NODE_SUCCESS)
//	{
//		VMP_LOGW("[%ld] url_query_callback fail", thiz->req.flowid);
//
//		client_connection_close(&thiz->req.client, 1);	//... create thread to free
//		return -1;
//	}
//	TimaGetPropertyRsp *rsp = arg; 
//	TIMA_LOGD("stream uri: %s", rsp->uri);
//
//#ifdef _TEST
//	char url[256] = {0};
//	snprintf(url, sizeof(url), "%s_%lld_%d", rsp->uri, thiz->sim, thiz->channel.id);
//	rtmp_push_start(n, thiz->sim, thiz->channel.id, url);
//#else
//	rtmp_push_start(n, thiz->sim, thiz->channel.id, rsp->uri);
//#endif // _TEST
//
//	return 0;
//}
//
//static int stream_url_query(vmp_node_t* p, unsigned long long sim, char channel)
//{
//	PrivInfo* thiz = p->private;
//	context* ctx = context_get();
//	vmp_node_t* n = node_create(TIMA_GET_PROPERTY_CLASS, ctx->vector_node);
//
//	TimaGetPropertyReq req = {0};
//	req.flowid		= thiz->req.flowid;
//#ifdef _TEST
//	sprintf(req.sim, "%011lld", 16180560371);
//#else
//	sprintf(req.sim, "%011lld", sim);
//#endif
//	req.ch			= channel;
//	req.pfncb		= url_query_callback;
//	n->parent		= p;
//	n->pfn_set(n, 0, &req, sizeof(TimaGetPropertyReq));
//	n->pfn_start(n);
//
//	return 0;
//}
//#endif

static void socket_input_init(PrivInfo* thiz)
{
	tima_buffer_init(&thiz->channel.buffer, 1<<10);
	thiz->sim = (unsigned long long)-1;

	TIMA_LOGD("========== flowid[%d] client[fd %d] register ==========", thiz->req.flowid, thiz->req.client.fd);
	client_connection_register(thiz->req.e, &thiz->req.client);
}

static int bll_sockioa_get(vmp_node_t* p, int id, void* data, int size)
{
	return 0;
}

static int bll_sockioa_set(vmp_node_t* p, int id, void* data, int size)
{	
	PrivInfo* thiz = p->private;
	thiz->req = *((SocketIOAReq*)data);
	
	return 0;
}

static char cmsg[1024] = {0};
static char recv_buffer[2048] = {0};

static void relay_input_handler(evutil_socket_t fd, short what, void* arg)
{
	int ret = 0;
	int try_again = 0;
	int ttl = TTL_IGNORE;
	int tos = TOS_IGNORE;
	vpk_sockaddr raddr = {0};

	if (!(what & EV_READ))
		return;

	if(!arg) {
		return;
	}

	vmp_node_t* p = arg;
	PrivInfo* thiz = p->private;
	VmpSocketIOA *s = thiz->sock;

	if(!s) {
		return;
	}

try_start:
	try_again = 0;

	ret = vpk_udp_recvfrom(fd, &raddr, &s->local_addr, recv_buffer, 1024, &ttl, &tos, cmsg, 0, NULL);
	if (ret > 0) {
		try_again = 1;
		VMP_LOGD("recvfrom websock[len=%d]: %s", ret, recv_buffer);

		//tima_websock_send_binary(thiz->req.client, recv_buffer, ret);
		ret = bufferevent_write(thiz->req.client.bev, recv_buffer, ret);
		VMP_LOGD("websock bufferevent_write ret=%d", ret);

	}

	if (try_again) {
		goto try_start;
	}

	return ;
}
static void socket_relay_init(vmp_node_t* p)
{
	int ret = 0;
	VmpSocketIOA *s = NULL;
	PrivInfo* thiz = p->private;
	vmp_server_t *relay = thiz->req.s;

	ret = vmp_relay_socket_create(relay, VPK_APPTYPE_SOCKET_RELAY, &thiz->sock);
	if (ret < 0) {
		VMP_LOGE("relay socket create failed.");

		return;
	}

	s = thiz->sock;
	s->read_event = event_new(relay->event_base, s->abs.fd, EV_READ|EV_PERSIST, relay_input_handler, p);
	event_add(s->read_event, NULL);
}

void* bll_sockioa_start(vmp_node_t* p)
{
	VMP_LOGD("bll_sockioa_start");

	PrivInfo* thiz = p->private;
	//context * ctx = context_get();
	thiz->req.client.priv = p;

	socket_relay_init(p);

	socket_input_init(thiz);

	return NULL;
}

int bll_sockioa_stop(vmp_node_t* p)
{
	return 0;
}

vmp_node_t* bll_sockioa_create(void)
{
	vmp_node_t* p = NULL;

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));

		pthread_mutex_init(&thiz->list_mutex, NULL);
		pthread_cond_init(&thiz->cond_empty, NULL);

		thiz->cond	= 1;

		//INIT_LIST_HEAD(&thiz->nalu_head);
		
		p = (vmp_node_t*)malloc(sizeof(vmp_node_t));
		memset(p, 0, sizeof(vmp_node_t));
		p->nclass		= BLL_SOCKETIOA_CLASS;
		p->pfn_get		= (nodeget)bll_sockioa_get;
		p->pfn_set		= (nodeset)bll_sockioa_set;
		p->pfn_start	= (nodestart)bll_sockioa_start;
		p->pfn_stop		= (nodestop)bll_sockioa_stop;
		p->pfn_callback = NULL;
		p->private		= thiz;

		return p;
	} while(0);
	
	return p;
}

int bll_sockioa_delete(vmp_node_t* p)
{
	VMP_LOGI("bll_sockioa_delete %p\n", get_thread_id());

	PrivInfo* thiz = (PrivInfo*)p->private;
	if(thiz != NULL)
	{
		pthread_mutex_destroy(&thiz->list_mutex);
		pthread_cond_destroy(&thiz->cond_empty);
		free(thiz);
		p->private = NULL;
	}

	free(p);
	
	return 0;
}

static const nodedef node_sock_ioa = 
{	
	sizeof(PrivInfo),
	BLL_SOCKETIOA_CLASS,
	(nodecreate)bll_sockioa_create,
	(nodedelete)bll_sockioa_delete
};

void bll_sockioa_init(void)
{
	VMP_LOGI("bll_sockioa_init");
	
	NODE_CLASS_REGISTER(node_sock_ioa);
}

void bll_sockioa_done(void)
{
	VMP_LOGI("bll_sockioa_done");

	NODE_CLASS_UNREGISTER(BLL_SOCKETIOA_CLASS);
}
