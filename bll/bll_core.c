/**
 * History:
 * ================================================================
 * 2019-03-11 qing.zou created
 *
 */

#include "context.h"
#include "ThreadPool.h"
#include "cache.h"

#include "bll_core.h"
#include "server_listener.h"
#include "bll_h264_stream.h"
#include "bll_http_base.h"
#include "bll_voice_task.h"

typedef struct _PrivInfo
{
	unsigned long	flowid;

	int				cond;
	int				id;

	//list_t			client_head;
	//test...
	int				count;

	vmp_server_t	*server;
} PrivInfo;


static int handle_message_callback(void* p, int msg, void* arg)
{
	vmp_server_t *ss = p;
	if ( msg != NODE_SUCCESS)
	{
		VMP_LOGW("handle_message_callback fail");

		return -1;
	}

	PrivInfo* thiz = (PrivInfo*)ss->core;
	ss->client_cnt--;	// need lock...
	TIMA_LOGD("[%ld] server[%d] count: %d", thiz->flowid, ss->id, ss->client_cnt);


	return 0;
}

static int handle_message(vmp_server_t *ss, vmp_socket_t *sock)
{
	context* ctx = context_get();
	vmp_node_t* p = node_create(BLL_H264STREAM_CLASS, ctx->vector_node);

	PrivInfo* thiz = (PrivInfo*)ss->core;
	thiz->flowid++;


	char ip[INET_ADDRSTRLEN] = {0};
	vpk_inet_ntop(AF_INET, vpk_sockaddr_get_addr(&sock->peer_addr), ip, sizeof(ip));
	printf("ip: %s:%u\n", ip, vpk_sockaddr_get_port(&sock->peer_addr));

	ss->client_cnt++;	// need lock and -- at release
	TIMA_LOGI("[%ld] server[%d] handle fd: %d, count: %d  (%s : %u)", 
		thiz->flowid, ss->id, sock->fd, ss->client_cnt, ip, vpk_sockaddr_get_port(&sock->peer_addr));
	//inet_ntoa(*(struct in_addr*)vpk_sockaddr_get_addr(&sock->peer_addr)));


	H264StreamReq req = {0};
	req.flowid		= thiz->flowid;
	req.e			= ss->e;
	req.s			= ss;
	memcpy(&req.client, sock, sizeof(vmp_socket_t));
	p->parent		= ss->core;
	p->pfn_callback = handle_message_callback;
	p->pfn_set(p, 0, &req, sizeof(H264StreamReq));
	p->pfn_start(p);

	return 0;
}

void* sockaddr_get_addr(const vmp_addr *addr)
{
	const vmp_addr *a = (const vmp_addr*)addr;

	return_val_if_fail(a->ss.sa_family == AF_INET ||
		a->ss.sa_family == AF_INET6, NULL);

	if (a->ss.sa_family == AF_INET6)
		return (void*) &a->s6.sin6_addr;
	else
		return (void*) &a->s4.sin_addr;
}
unsigned short sockaddr_get_port(const vmp_addr *addr)
{
	const vmp_addr *a = (const vmp_addr*) addr;

	return_val_if_fail(a->ss.sa_family == AF_INET ||
		a->ss.sa_family == AF_INET6, (unsigned short)0xFFFF);

	return ntohs((unsigned short)(a->ss.sa_family == AF_INET6 ?
		a->s6.sin6_port : a->s4.sin_port));
}

static void relay_receive_message(struct bufferevent *bev, void *ptr)
{
	vmp_connection_t session;
	int n = 0;
	struct evbuffer *input = bufferevent_get_input(bev);
	struct stream_server *ss = (struct stream_server *)ptr;

	while ((n = evbuffer_remove(input, &session, sizeof(vmp_connection_t))) > 0) {

		if (n != sizeof(vmp_connection_t)) {
			TIMA_LOGE("Weird buffer error");
			continue;
		}

		//handle_relay_message(ss, &session);
		handle_message(ss, &session.sock);
	}
}

static int task_server_listener(PrivInfo* thiz)
{
	context* ctx = context_get();
	vmp_node_t* p = node_create(SERVER_LISTENER_CLASS, ctx->vector_node);

	//vmp_server_t *server = calloc(1, sizeof(vmp_server_t));
	//server->core	= thiz;
	//server->read_cb = relay_receive_message;
	//thiz->server = server;

	ServerListenerReq req = {0};
	req.port	= 9999;
	req.ctx		= thiz;
	req.read_cb	= relay_receive_message;
	p->parent	= thiz;
	p->pfn_set(p, 0, &req, sizeof(ServerListenerReq));
	p->pfn_start(p);
	
	return 0;
}

static int task_intercom_server_device(PrivInfo* thiz)
{
	context* ctx = context_get();
	vmp_node_t* p = node_create(SERVER_LISTENER_CLASS, ctx->vector_node);

	ServerListenerReq req = {0};
	req.base	= event_base_new();
	req.port	= 9001;
	req.ctx		= thiz;
	req.read_cb	= relay_receive_message;	//...
	p->parent	= thiz;
	p->pfn_set(p, 0, &req, sizeof(ServerListenerReq));
	p->pfn_start(p);

	return 0;
}

static int task_voi_server_client(PrivInfo* thiz)
{
	context* ctx = context_get();
	vmp_node_t* p = node_create(BLL_VOICE_TASK_CLASS, ctx->vector_node);

	VoiceTaskReq req = {0};
	p->parent	= thiz;
	p->pfn_set(p, 0, &req, sizeof(VoiceTaskReq));
	p->pfn_start(p);

	return 0;
}

static void task_hbase_start(PrivInfo* thiz)
{
	context* ctx = context_get();
	vmp_node_t* p = node_create(BLL_HTTPBASE_CLASS, ctx->vector_node);

	p->parent = thiz;
	p->pfn_start(p);
}

#include "http_token.h"
static int DoTimaTokenGetTest(unsigned long long seriesno)
{
	context* ctx = context_get();
	vmp_node_t* p = node_create(HTTP_TOKEN_CLASS, ctx->vector_node);

	HttpTokenReq req = {0};
	strcpy(req.devtype, "TACHOGRAPH");
	//strcpy(req.seriesno, "123456789012345");
	sprintf(req.seriesno, "%lld", seriesno);
	//p->parent	= thiz;
	p->pfn_set(p, 0, &req, sizeof(HttpTokenReq));
	p->pfn_start(p);

	return 0;
}
#if 0
#include "http_token.h"
#include <event2/event.h>
#include <event2/event_struct.h>
static int DoTimaTokenGet(void)
{
	context* ctx = context_get();
	vmp_node_t* p = node_create(HTTP_TOKEN_CLASS, ctx->vector_node);

	HttpTokenReq req = {0};
	strcpy(req.devtype, "TACHOGRAPH");
	strcpy(req.seriesno, "123456789012345");
	//p->parent	= thiz;
	p->pfn_set(p, 0, &req, sizeof(HttpTokenReq));
	p->pfn_start(p);

	return 0;
}
struct event_base* http_base;
static void timeout_cb(evutil_socket_t fd, short event, void *arg)
{
	struct event *timeout = arg;
	struct timeval tv;
	evutil_timerclear(&tv);
	tv.tv_sec = 500;
	event_add(timeout, &tv);

	printf("================= start get token test =================\n");
	DoTimaTokenGet();
}
#endif

static void* bll_core_thread(void* arg)
{
	PrivInfo* thiz = (PrivInfo*)arg;
	
	task_hbase_start(thiz);
	//task_server_listener1(thiz);
	task_server_listener(thiz);
	task_voi_server_client(thiz);


	while(thiz->cond)
	{
		//CoreTaskMsg* msg = (CoreTaskMsg*) bll_core_recvmsg(thiz);
		//if(msg)
		//{
		//	bll_core_onmsg(thiz, msg);
		//	free(msg);
		//}
		while (1)
		{
			sleep(100);
		}

#if 1
		int test = 1;
		unsigned long long sn = 123456789012345;
		while(test--) {
			DoTimaTokenGetTest(sn++);
		}
		
		while(1) {
			sleep(10);
		}
#else

		http_base = event_base_new();
		struct event timeout;
		struct timeval tv;
		//int flags = EV_PERSIST;
		int flags = 0;
		/* Initalize one event */
		event_assign(&timeout, http_base, -1, flags, timeout_cb, (void*) &timeout);

		evutil_timerclear(&tv);
		tv.tv_sec = 2;
		event_add(&timeout, &tv);

		event_base_dispatch(http_base);
		event_base_free(http_base);
		printf("================= end ===================\n");
#endif
	}

	return NULL;
}

int bll_core_start(PrivInfo* thiz)
{
	TIMA_LOGD("bll_core_start");

	context* ctx = context_get();

	ThreadPoolJob job;
	TPJobInit( &job, ( start_routine) bll_core_thread, thiz);
	TPJobSetFreeFunction( &job, ( free_routine ) NULL );
	TPJobSetPriority( &job, HIGH_PRIORITY );
	ThreadPoolAddPersistent( ctx->tp, &job, &job.jobId);
	thiz->id = job.jobId;

	return 0;
}

int bll_core_stop(PrivInfo* thiz)
{
	TIMA_LOGD("bll_core_stop");

	thiz->cond = 0;

	return 0;
}

void bll_core_init(void)
{
	PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
	memset(thiz, 0, sizeof(PrivInfo));

	//CQueue* cque = (CQueue*)malloc(sizeof(CQueue));
	//cque->buffer	= thiz->buffer;
	//cque->capacity	= BLL_QUEUE_SIZE;
	//cque->size		= 0;
	//cque->in		= 0;
	//cque->out		= 0;
	//pthread_mutex_init (&cque->mutex, NULL);
	//pthread_cond_init(&cque->cond_full, NULL);
	//pthread_cond_init(&cque->cond_empty, NULL);


	//thiz->queue			= cque;
	thiz->cond			= 1;
	context_get()->bll	= thiz;

	context* ctx = context_get();
	vmp_node_t* cache = node_create(CACHE_CLASS, ctx->vector_node);
	cache->pfn_start(cache);

	ctx->cache = cache;

	bll_core_start(thiz);

}

void bll_core_done(void)
{
	PrivInfo* thiz = context_get()->bll;
	if(thiz != NULL)
	{
		free(thiz);
	}
}
