/**
 * History:
 * ================================================================
 * 2019-03-11 qing.zou created
 *
 */

#include <netinet/in.h>

#include <event2/event.h>
#include <event2/listener.h>

#include "server_listener.h"

#include "context.h"

typedef struct _PrivInfo
{
	ServerListenerReq		req;
	ServerListenerRep		rep;

	int						id;

	struct listener_server *listener;

} PrivInfo;

vmp_launcher_t *vmp_stream_service_create(void *mem, struct event_base *base)
{
	vmp_launcher_t *service = (vmp_launcher_t*)calloc(1, sizeof(vmp_launcher_t));
	if (base)
		service->event_base = base;
	else {
		service->event_base = event_base_new();
	}

	return service;
}


void init_server(void)
{
	struct event_base* base = NULL;
	context* ctx = context_get();
	if (!ctx->service) {
		base = event_base_new();
		ctx->service = vmp_stream_service_create(/*mem*/ NULL, base);
	}
}

void setup_server(vmp_server_t *server)
{
	struct bufferevent *pair[2];
	struct event_base* base = NULL;
	context* ctx = context_get();
	if (!ctx->service) {
		base = event_base_new();
		ctx->service = vmp_stream_service_create(/*mem*/ NULL, base);
	} else {
		vmp_launcher_t *e = (vmp_launcher_t *)ctx->service;
		base = e->event_base;
	}
	
	server->event_base	= base;
	server->e			= ctx->service;

	bufferevent_pair_new(server->event_base, VMP_BUFFEREVENTS_OPTIONS, pair);
	server->in_buf = pair[0];
	server->out_buf = pair[1];
	bufferevent_setcb(server->in_buf, server->read_cb/*relay_receive_message*/, NULL, NULL, server);
	bufferevent_enable(server->in_buf, EV_READ);
}


static void server_accept_handler(struct evconnlistener *l, evutil_socket_t fd,
				struct sockaddr *address, int socklen, void *arg)
{
	struct listener_server *server = (struct listener_server*) arg;
	TIMA_LOGD("tcp connected to fd: %d", fd);

	if(!(server->on_connect)) {
		close(fd);
		return;
	}

	vpk_bcopy(address, &(server->conn.sock.peer_addr), socklen);
	server->conn.sock.fd = fd;
	server->conn.stream_server = server->server;

	int rc = server->on_connect(server->e, &server->conn);
	if (rc < 0)
		TIMA_LOGE("Cannot create tcp session");
}

static struct listener_server *create_tcp_listener_server(vmp_addr *addr, vmp_launcher_t *e,
				vmp_server_t *server, server_new_connection_handler send_socket)
{
	struct listener_server *listener = calloc(1, sizeof(struct listener_server));
	if(!listener)
	{
		TIMA_LOGE("create_tcp_listener_server listener null\n");
		return NULL;
	}
	listener->server		= server;
	listener->on_connect	= send_socket;
	listener->e				= e;

	listener->lis = evconnlistener_new_bind(listener->e->event_base, server_accept_handler, 
						listener, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,	
						(struct sockaddr*)addr->ss, sizeof(vmp_addr));
	if(!listener->lis)
	{
		TIMA_LOGE("create_tcp_listener_server listener->lis null\n");
		free(listener);
		return NULL;
	}

	return listener;
}

static int send_socket_to_core(vmp_launcher_t *e, vmp_connection_t *sm)
{
	vmp_server_t *server = sm->stream_server;
	if (!server) {
		TIMA_LOGE("stream server is null");
		return -1;
	}

	vmp_connection_t *conn = sm;
	conn->t = VMP_STREAM_JT1078;

	struct evbuffer *output = NULL;
	int success = 0;

	output = bufferevent_get_output(server->out_buf);

	if(output) {
		if(evbuffer_add(output, conn, sizeof(vmp_connection_t)) < 0) {
			TIMA_LOGE("%s: Cannot add message to relay output buffer\n", __FUNCTION__);
		} else {
			success = 1;
		}
	}

	if (!success) {


		//close(fd);
		return -1;
	}

	return 0;
}

static void setup_tcp_listener_server(PrivInfo *thiz, vmp_launcher_t *e, vmp_server_t *server)
{
	//struct listener_server *listener = NULL;

	struct in_addr	sin_addr;
	if (!inet_aton("0.0.0.0", &sin_addr))
		TIMA_LOGE("inet_aton error!");

	int port = 9999;
	struct sockaddr_in my_address = {0};
	my_address.sin_family = AF_INET;
	my_address.sin_addr.s_addr = sin_addr.s_addr;
	my_address.sin_port = htons(port);

	thiz->listener = create_tcp_listener_server((vmp_addr *)&my_address, e, server, send_socket_to_core);
}

static void run_events(struct event_base *base, vmp_launcher_t *e)
{
	if (!base && e)
		base = e->event_base;

	if (!base)
		return;

	event_base_dispatch(base);
}

static void run_listener_server(struct listener_server *lis)
{
	run_events(lis->event_base, lis->e);

	evconnlistener_free(lis);
	event_base_free(lis->event_base);
}

static void* server_listener_thread(void* arg)
{
	vmp_server_t *server = NULL;
	PrivInfo* thiz = (PrivInfo*)arg;
	evthread_use_pthreads();

	VMP_LOGI("server_listener_thread begin\n");

	server = thiz->req.server;
	init_server();
	setup_server(server);
	setup_tcp_listener_server(thiz, server->e, server);

	run_listener_server(thiz->listener);

	//struct in_addr	sin_addr;
	//if (!inet_aton("0.0.0.0", &sin_addr))
	//	TIMA_LOGE("inet_aton error!");

	//int port = 9999;
	//struct sockaddr_in my_address = {0};
	//my_address.sin_family = AF_INET;
	////my_address.sin_addr.s_addr = htonl(0x0000000); // 0.0.0.0
	//my_address.sin_addr.s_addr = sin_addr.s_addr;
	//my_address.sin_port = htons(port);

	//struct event_base* base = event_base_new();
	//struct evconnlistener* listener =
	//	evconnlistener_new_bind(base, server_on_accept,
	//	NULL, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
	//	(struct sockaddr*)&my_address, sizeof(my_address));

	//if(!listener)
	//{
	//	printf("tcpserver_thread  listener==NULL\n");
	//}

	//event_base_dispatch(base);
	//evconnlistener_free(listener);
	//event_base_free(base);

	VMP_LOGI("server_listener_thread end\n");
}


int server_listener_get(vmp_node_t* p, int id, void* data, int size)
{
	return 0;
}

int server_listener_set(vmp_node_t* p, int id, void* data, int size)
{	
	PrivInfo* thiz = p->private;
	thiz->req = *((ServerListenerReq*)data);

	return 0;
}

void* server_listener_start(vmp_node_t* p)
{
	VMP_LOGD("server_listener_start");

	PrivInfo* thiz = p->private;

	context * ctx = context_get();
	ThreadPoolJob job;

	TPJobInit( &job, ( start_routine) server_listener_thread, p);
	TPJobSetFreeFunction( &job, ( free_routine ) NULL );
	TPJobSetStopFunction( &job, ( stop_routine ) NULL );
	TPJobSetPriority( &job, HIGH_PRIORITY );
	ThreadPoolAddPersistent( ctx->tp, &job, &job.jobId);
	thiz->id = job.jobId;

	return NULL;
}

int server_listener_stop(vmp_node_t* p)
{
	return 0;
}

vmp_node_t* server_listener_create(void)
{
	vmp_node_t* p = NULL;

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));

		p = (vmp_node_t*)malloc(sizeof(vmp_node_t));
		memset(p, 0, sizeof(vmp_node_t));
		p->nclass		= SERVER_LISTENER_CLASS;
		p->pfn_get		= (nodeget)server_listener_get;
		p->pfn_set		= (nodeset)server_listener_set;
		p->pfn_start	= (nodestart)server_listener_start;
		p->pfn_stop		= (nodestop)server_listener_stop;
		p->pfn_callback = NULL;
		p->private		= thiz;

		return p;
	} while(0);

	return p;
}

int server_listener_delete(vmp_node_t* p)
{
	VMP_LOGD("server_listener_delete");

	PrivInfo* thiz = (PrivInfo*)p->private;
	if(thiz != NULL)
	{
		free(thiz);
		p->private = NULL;
	}

	free(p);

	return 0;
}

static const nodedef node_server_listener = 
{	
	sizeof(PrivInfo),
	SERVER_LISTENER_CLASS,
	(nodecreate)server_listener_create,
	(nodedelete)server_listener_delete
};

void server_listener_init(void)
{
	VMP_LOGD("server_listener_init");

	node_register_class(&node_server_listener);
}

void server_listener_done(void)
{
	VMP_LOGD("server_listener_done");

	node_unregister_class(SERVER_LISTENER_CLASS);
}
