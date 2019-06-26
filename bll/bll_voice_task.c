/**
 * History:
 * ================================================================
 * 2018-02-05 qing.zou created
 *
 */

#include "context.h"
#include "ThreadPool.h"

#include "server_websock.h"
#include "server_listener.h"

#include "bll_voice_task.h"
#include "bll_websock_ioa.h"
#include "bll_socket_ioa.h"


typedef struct _PrivInfo
{
	VoiceTaskReq		req;
	VoiceTaskRep		rep;

	unsigned long		flowid;
	int					id;
} PrivInfo;

static int bll_voice_delete(vmp_node_t* p);

static int task_voice_callback(void* p, int msg, void* arg)
{
	vmp_node_t* demo = ((vmp_node_t*)p)->parent;
	if ( msg != NODE_SUCCESS)
	{
		VMP_LOGW("task_voice_callback fail");
		bll_voice_delete(demo);
		return -1;
	}

	bll_voice_delete(demo);
	return 0;
}

static int bll_voice_get(vmp_node_t* p, int id, void* data, int size)
{
	return 0;
}

static int bll_voice_set(vmp_node_t* p, int id, void* data, int size)
{	
	PrivInfo* thiz = p->private;
	thiz->req = *((VoiceTaskReq*)data);
	
	return 0;
}

/** client **/
static int on_connect(void *client, void *ws)
{
	int fd = tima_websock_fd_get(client);

	TIMA_LOGI("websock server handle fd: %d", fd);

	context* ctx = context_get();
	vmp_node_t* p = node_create(BLL_WEBSOCK_IOA_CLASS, ctx->vector_node);

	WebsockIOAReq req = {0};
	req.client	= client;
	req.ws		= ws;
	//p->parent	= thiz;
	p->pfn_set(p, 0, &req, sizeof(WebsockIOAReq));
	p->pfn_start(p);


	//TIMA_LOGI("[%ld] server[%d] handle fd: %d, count: %d  (%s : %u)", 
		//thiz->flowid, ss->id, sock->fd, ss->client_cnt, ip, vpk_sockaddr_get_port(&sock->peer_addr));
	return 0;
}

static void* voi_websock_server(vmp_node_t* p)
{
	VMP_LOGD("voi_websock_start");

	PrivInfo* thiz = p->private;

	context* ctx = context_get();
	vmp_node_t* server = node_create(SERVER_WEBSOCK_CLASS, ctx->vector_node);

	ServerWebsockReq req = {0};
	req.port		= 9002;
	req.ctx			= thiz;
	req.on_connect	= on_connect;
	server->parent	= thiz;
	server->pfn_set(server, 0, &req, sizeof(ServerWebsockReq));
	server->pfn_start(server);

	return NULL;
}

/** device **/
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

static int handle_relay_connection(vmp_server_t *ss, vmp_socket_t *sock)
{
	context* ctx = context_get();

	vmp_node_t* p = node_create(BLL_SOCKETIOA_CLASS, ctx->vector_node);

	PrivInfo* thiz = (PrivInfo*)ss->core;
	thiz->flowid++;


	char ip[INET_ADDRSTRLEN] = {0};
	vpk_inet_ntop(AF_INET, vpk_sockaddr_get_addr(&sock->peer_addr), ip, sizeof(ip));
	printf("ip: %s:%u\n", ip, vpk_sockaddr_get_port(&sock->peer_addr));

	//ss->client_cnt++;	// need lock and -- at release
	TIMA_LOGI("[%ld] relay server[%d] handle fd: %d, count: %d  (%s : %u)", 
		thiz->flowid, ss->id, sock->fd, ss->client_cnt, ip, vpk_sockaddr_get_port(&sock->peer_addr));
	//inet_ntoa(*(struct in_addr*)vpk_sockaddr_get_addr(&sock->peer_addr)));


	SocketIOAReq req = {0};
	req.flowid		= thiz->flowid;
	req.e			= ss->e;
	req.s			= ss;
	memcpy(&req.client, sock, sizeof(vmp_socket_t));
	p->parent		= ss->core;
	p->pfn_callback = handle_message_callback;
	p->pfn_set(p, 0, &req, sizeof(SocketIOAReq));
	p->pfn_start(p);

	return 0;
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

		handle_relay_connection(ss, &session.sock);
	}
}

static int voi_device_server(PrivInfo* thiz)
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


static void* bll_voice_start(vmp_node_t* p)
{
	VMP_LOGD("bll_voice_start");

	PrivInfo* thiz = (PrivInfo*)p->private;

	voi_device_server(thiz);
	voi_websock_server(p);

	return NULL;
}

static int bll_voice_stop(vmp_node_t* p)
{
	return 0;
}

static vmp_node_t* bll_voice_create(void)
{
	vmp_node_t* p = NULL;

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));
		
		p = (vmp_node_t*)malloc(sizeof(vmp_node_t));
		memset(p, 0, sizeof(vmp_node_t));
		p->nclass		= BLL_VOICE_TASK_CLASS;
		p->pfn_get		= (nodeget)bll_voice_get;
		p->pfn_set		= (nodeset)bll_voice_set;
		p->pfn_start	= (nodestart)bll_voice_start;
		p->pfn_stop		= (nodestop)bll_voice_stop;
		p->pfn_callback = NULL;
		p->private		= thiz;

		return p;
	}while(0);
	
	return p;
}

static int bll_voice_delete(vmp_node_t* p)
{
	//VMP_LOGD("bll_voice_delete");

	PrivInfo* thiz = (PrivInfo*)p->private;
	if(thiz != NULL)
	{
		free(thiz);
		p->private = NULL;
	}

	free(p);
	
	return 0;
}

static const nodedef node_voice_task = 
{	
	sizeof(PrivInfo),
	BLL_VOICE_TASK_CLASS,
	(nodecreate)bll_voice_create,
	(nodedelete)bll_voice_delete
};

void bll_voice_init(void)
{
	VMP_LOGI("bll_voice_init");

	NODE_CLASS_REGISTER(node_voice_task);
}

void bll_voice_done(void)
{
	VMP_LOGI("bll_voice_done");

	NODE_CLASS_UNREGISTER(BLL_VOICE_TASK_CLASS);
}
