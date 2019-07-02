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

	int					channel;
	unsigned long long	sim;
	int					state;

	VmpSocketIOA		*sock;

	unsigned char		buff[1024];

	//list_t				nalu_head;
	//int					list_size;
	//pthread_mutex_t		list_mutex;
	//pthread_cond_t		cond_empty;
} PrivInfo;

static int bll_sockioa_delete(vmp_node_t* p);


static int sockioa_close_socket(vmp_node_t* p)
{
	PrivInfo* thiz = (PrivInfo*)p->private;
	evutil_closesocket(thiz->req.client.fd);
	thiz->req.client.fd = 0;
	return 0;
}

static int sockioa_input_release(vmp_node_t* p, int status)
{
	if (p) 
	{
		PrivInfo* thiz = (PrivInfo*)p->private;

		if (p->pfn_callback) {
			p->pfn_callback(thiz->req.ss, status, NULL);
		}

		if (thiz->req.client.bev) {
			bufferevent_free(thiz->req.client.bev);
			thiz->req.client.bev = NULL;
		}
		
		//event_free(thiz->sock->read_event);
		if (thiz->sock) {
			vmp_socket_release(thiz->sock);
		}

		bll_sockioa_delete(p);
	}

	return 0;
}

//static int socket_input_callback(void* p, int msg, void* arg)
//{
//	vmp_node_t* n = (vmp_node_t*)p;
//	if ( msg != NODE_SUCCESS)
//	{
//		VMP_LOGW("socket_input_callback fail");
//		sockioa_input_release(n);
//		//bll_sockioa_delete(n);
//		return -1;
//	}
//
//	sockioa_input_release(n);
//	//bll_sockioa_delete(n);
//	return 0;
//}
//
//static int client_connection_close(vmp_socket_t *sock, int immediately)
//{
//	vmp_node_t* p = (vmp_node_t*)sock->priv;
//	PrivInfo* thiz = (PrivInfo*)p->private;
//	struct bufferevent *bev = sock->bev;
//
//	bufferevent_flush(bev, EV_READ|EV_WRITE, BEV_FLUSH); 
//	bufferevent_disable(bev, EV_READ|EV_WRITE);
//
//	thiz->cond = 0;
//	usleep(5000);
//
//	while(thiz->running) {
//		usleep(50000);
//	}
//
//	TIMA_LOGW("[%ld] Connection closed. (%lld_%d fd %d)", 
//		thiz->req.flowid, thiz->sim, thiz->channel, sock->fd);
//
//	TIMA_LOGD("================ closed fd %d ================", sock->fd);
//	sockioa_input_release(p);
//
//	return 0;
//}
//
//static int bll_sockioa_stream_close(vmp_node_t* p)
//{
//	PrivInfo* thiz = (PrivInfo*)p->private;
//
//	client_connection_close(&thiz->req.client, 0);
//
//	return 0;
//}

static void stream_socket_eventcb(struct bufferevent* bev, short event, void* arg)
{
	vmp_socket_t *sock = (vmp_socket_t*)arg;
	vmp_node_t* p = (vmp_node_t*)sock->priv;
	PrivInfo* thiz = (PrivInfo*)p->private;

	if( event & (BEV_EVENT_EOF)) {
		TIMA_LOGW("[%ld] Connection closed. (%lld_%d fd %d) EOF (0x%2x)", 
			thiz->req.flowid, thiz->sim, thiz->channel, thiz->req.client.fd, event);
	} else if( event & BEV_EVENT_ERROR) {
		TIMA_LOGW("[%ld] (%lld_%d fd %d) socket ERROR (0x%2x)\n", 
			thiz->req.flowid, thiz->sim, thiz->channel, thiz->req.client.fd, event);
	} else if( event & BEV_EVENT_TIMEOUT) {
		TIMA_LOGW("[%ld] (%lld_%d fd %d) socket TIMEOUT (0x%2x)\n", 
			thiz->req.flowid, thiz->sim, thiz->channel, thiz->req.client.fd, event);
	} else if (event & BEV_EVENT_CONNECTED) {
		TIMA_LOGI("[%ld] stream_socket_eventcb CONNECTED (0x%2x)\n", thiz->req.flowid, event);
		return;
	} else {
		TIMA_LOGW("[%ld] (%lld_%d fd %d) stream_socket_eventcb event = 0x%2x\n", 
			thiz->req.flowid, thiz->sim, thiz->channel, thiz->req.client.fd, event);
	}

	bufferevent_flush(bev, EV_READ|EV_WRITE, BEV_FLUSH); 
	bufferevent_disable(bev, EV_READ|EV_WRITE); 
	//bufferevent_free(bev);

	thiz->cond = 0;
	sockioa_input_release(p, NODE_SUCCESS);
}


static int socket_input_proc(vmp_node_t* p, const char* buf, size_t size)
{
	size_t len;
	//size_t length = size;
	//const char *data = buf;
	PrivInfo* thiz = (PrivInfo*)p->private;

	len = vpk_udp_send(thiz->sock->abs.fd, &thiz->sock->dest_addr, buf, size);
	TIMA_LOGD("socket relay send len = %d", len);

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

static int socket_match_client(vmp_node_t* p, stream_header_t *head)
{
	VmpSocketIOA *wsock = NULL;
	PrivInfo* thiz = (PrivInfo*)p->private;
	vmp_server_t *server = thiz->req.ss;

	char tmp[16] = {0};
	sprintf(tmp, "%012lld", head->simno);

	if (thiz->state == SOCK_MATCH_STATE_GET) {
		wsock = tima_ioamaps_get_type(server->map, tmp, MAPS_SOCK_WEBSKT);
	}

	if (thiz->sim == (unsigned long long)-1) {
		thiz->sim = head->simno;
		TIMA_LOGI("[%ld] %p fd=%d sim no. [%lld]: %d", 
			thiz->req.flowid, vmp_thread_get_id(), thiz->req.client.fd, thiz->sim, head->channel);

		RelaySocketIO* relay_sock = tima_ioamaps_put(server->map, tmp, thiz->sock, MAPS_SOCK_STREAM);
		if (!relay_sock) {
			TIMA_LOGE("sock map put failed.");
			return -1;
		}

		thiz->state = SOCK_MATCH_STATE_GET;
		wsock = tima_ioamaps_exist(relay_sock, MAPS_SOCK_WEBSKT);
	}

	if (wsock) {
		vpk_sockaddr_set_port(&thiz->sock->dest_addr, wsock->src_port);
		if (vpk_sockaddr_get_port(&thiz->sock->dest_addr) < 1) {
			TIMA_LOGE("set dest addr port[%d] failed", wsock->src_port);
			thiz->state = SOCK_MATCH_STATE_ERROR;
			return -1;
		}

		thiz->sock->dst_port = wsock->src_port;
		thiz->state = SOCK_MATCH_STATE_SUCCESS;
		TIMA_LOGI("socket match success [%d <-> %d].", thiz->sock->src_port, wsock->src_port);
	}
	return 0;
}

static int media_stream_proc(vmp_node_t* p, struct bufferevent *bev/*, vmp_socket_t *sock*/)
{
	int ret = 0;
	PrivInfo* thiz = (PrivInfo*)p->private;
	struct evbuffer* input = bufferevent_get_input(bev);

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

			if (ret > JT1078_STREAM_PACKAGE_SIZE) {
				TIMA_LOGD("=====flowid[%d] %lld[fd %d], ret = %d", thiz->req.flowid, thiz->sim, thiz->req.client.fd, ret);
				TIMA_LOGE("[%ld] JT/T 1078-2016 parse failed, ret = %d", thiz->req.flowid, ret);
				ret = JT1078_STREAM_PACKAGE_SIZE;
				goto parse_end;
			}

			if (thiz->state != SOCK_MATCH_STATE_SUCCESS) {
				socket_match_client(p, &head);
				goto parse_end;
			}
			
			if ((head.mtype & 0xf0) == 0x30) {	// audio

				thiz->channel = head.channel;

#if 1
				//socket_input_proc(p, (const char*)stream, head.bodylen);
				socket_input_proc(p, (const char*)thiz->buff, head.bodylen+30);
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
	vmp_socket_t *sock = (vmp_socket_t*)arg;
	vmp_node_t* p = (vmp_node_t*)sock->priv;
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
			break;
		}

	} while (len > 30 && thiz->cond);

	thiz->running = 0;
}

static int client_connection_register(vmp_launcher_t *e, vmp_socket_t *sock)
{
	sock->bev = bufferevent_socket_new(sock->event_base, sock->fd, VMP_BUFFEREVENTS_OPTIONS);
	bufferevent_setcb(sock->bev, stream_input_handler, NULL, stream_socket_eventcb, sock);
	//bufferevent_setwatermark(sock->bev, EV_READ, 0, VOI_BUFFEREVENT_HIGH_WATERMARK);
	//bufferevent_settimeout(sock->bev, 60, 0);
	bufferevent_enable(sock->bev, EV_READ|EV_WRITE); /* Start reading. */
	return 0;
}


static void socket_input_init(PrivInfo* thiz)
{
	//tima_buffer_init(&thiz->channel.buffer, 1<<10);
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

static char cmsg[2048] = {0};
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

		ret = bufferevent_write(thiz->req.client.bev, recv_buffer, ret);

	}

	if (try_again) {
		goto try_start;
	}

	return ;
}
static int socket_relay_init(vmp_node_t* p)
{
	int ret = 0;
	VmpSocketIOA *s = NULL;
	PrivInfo* thiz = p->private;
	vmp_server_t *server = thiz->req.ss;

	ret = vmp_relay_socket_create(server, VPK_APPTYPE_SOCKET_RELAY, &thiz->sock);
	if (ret < 0) {
		VMP_LOGE("relay socket create failed.");
		return -1;
	}

	s = thiz->sock;
	s->read_event = event_new(server->event_base, s->abs.fd, EV_READ|EV_PERSIST, relay_input_handler, p);
	event_add(s->read_event, NULL);

	return 0;
}

void* bll_sockioa_start(vmp_node_t* p)
{
	VMP_LOGD("bll_sockioa_start");

	int ret = 0;
	PrivInfo* thiz = p->private;
	thiz->req.client.priv = p;

	ret = socket_relay_init(p);
	if (ret < 0) {
		sockioa_close_socket(p);
		sockioa_input_release(p, NODE_FAIL);
		return NULL;
	}

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

		//pthread_mutex_init(&thiz->list_mutex, NULL);
		//pthread_cond_init(&thiz->cond_empty, NULL);

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
	VMP_LOGI("bll_sockioa_delete %p\n", vmp_thread_get_id());

	PrivInfo* thiz = (PrivInfo*)p->private;
	if(thiz != NULL)
	{
		//pthread_mutex_destroy(&thiz->list_mutex);
		//pthread_cond_destroy(&thiz->cond_empty);
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
