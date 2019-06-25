/**
 * History:
 * ================================================================
 * 2019-03-11 qing.zou created
 *
 */

#include <netinet/in.h>

#include <event2/event.h>
#include <event2/listener.h>

#include <event2/thread.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "server_listener.h"

#include "context.h"
#include "ThreadPool.h"

struct listener_server {
	struct event_base		*event_base;
	vmp_launcher_t			*e;
	struct evconnlistener	*lis;
	vmp_server_t			*server;
	vmp_connection_t		conn;

	server_new_connection_handler on_connect;

	void					*parent;
};

typedef struct _PrivInfo
{
	ServerListenerReq		req;
	ServerListenerRep		rep;

	int						id;

	struct listener_server *listener;

	vmp_server_t			*main_server;

	vmp_server_t			**stream_server;
} PrivInfo;

#define MAX_STREAM_SERVER_NUM	8

static void setup_server(vmp_node_t* p, vmp_server_t *server, void *base);
static void run_events(struct event_base *base, vmp_launcher_t *e);

static INLINE void stream_server_setup(vmp_node_t* p, vmp_server_t *server, void *base);

static void timeout_cb(evutil_socket_t fd, short event, void *arg) 
{
}
static void stream_server_task_set(void *base)
{
	//struct event timeout;
	struct timeval tv;
	int flags = EV_PERSIST;
	/* Initalize one event */
	struct event *timeout = event_new(base, -1, flags, timeout_cb, NULL);
	//event_assign(&timeout, base, -1, flags, timeout_cb, (void*) &timeout);

	evutil_timerclear(&tv);
	tv.tv_sec = 60000;
	event_add(timeout, &tv);
}

static void *stream_server_thread(void *arg)
{
	vmp_server_t *server = (vmp_server_t *)arg;
	vmp_node_t* p = server->priv;
	
	evthread_use_pthreads();
	
	stream_server_setup(p, server, NULL);
	stream_server_task_set(server->event_base);

	run_events(server->event_base, NULL);

	TIMA_LOGE("stream server thread %p exit", pthread_self());
	pthread_exit(0);

	return arg;
}

static void stream_server_general(vmp_node_t* p, int num)
{
	int ret = 0, i = 0;
	PrivInfo* thiz = p->private;

	vmp_server_t **server = calloc(1, sizeof(vmp_server_t*) * num);
	thiz->stream_server = server;

	for (i = 0; i < num; i++)
	{
		server[i] = calloc(1, sizeof(vmp_server_t));
		server[i]->id	= i+1;
		server[i]->priv = p;

		ret = pthread_create(&(server[i]->pth_id), NULL, stream_server_thread, (void*)server[i]);
		if (ret != 0)
			TIMA_LOGE("create thread \'%p\' failed", &(server[i]->pth_id));

		pthread_detach(server[i]->pth_id);
	}
}

static INLINE void stream_server_setup(vmp_node_t* p, vmp_server_t *server, void *base)
{
	if (base)
		setup_server(p, server, base);
	else
		setup_server(p, server, event_base_new());
}



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

static void setup_server(vmp_node_t* p, vmp_server_t *server, void *base)
{
	vmp_launcher_t *e = NULL;
	struct bufferevent *pair[2];
	PrivInfo* thiz = p->private;
	context* ctx = context_get();

	if (!ctx->service) {
		struct event_base* base = event_base_new();
		ctx->service = vmp_stream_service_create(/*mem*/ NULL, base);
	}

	e = (vmp_launcher_t *)ctx->service;

	server->porter			= ctx->porter;
	server->e				= e;
	server->core			= p->parent;
	if (base)
		server->event_base	= base;
	else
		server->event_base	= e->event_base;

	bufferevent_pair_new(server->event_base, VMP_BUFFEREVENTS_OPTIONS, pair);
	server->in_buf = pair[0];
	server->out_buf = pair[1];
	bufferevent_setcb(server->in_buf, thiz->req.read_cb, NULL, NULL, server);
	bufferevent_enable(server->in_buf, EV_READ|EV_PERSIST);
}

static void server_accept_handler(struct evconnlistener *l, evutil_socket_t fd,
				struct sockaddr *address, int socklen, void *arg)
{
	vmp_server_t *server = NULL;
	struct listener_server *ls = (struct listener_server*) arg;
	vmp_node_t* p = ls->parent;
	PrivInfo* thiz = p->private;
	TIMA_LOGD("tcp connected to fd: %d", fd);

	if(!(ls->on_connect)) {
		close(fd);
		return;
	}

	if (MAX_STREAM_SERVER_NUM == 0) {
		server = thiz->main_server;
	} else {
		unsigned int index = hash_int32(fd) % MAX_STREAM_SERVER_NUM;
		server = thiz->stream_server[index];
	}

	vpk_bcopy(address, &(ls->conn.sock.peer_addr), socklen);
	ls->conn.sock.fd			= fd;
	ls->conn.sock.e				= server->e;
	ls->conn.sock.event_base	= server->event_base;
	ls->conn.sock.server		= server;
	ls->conn.stream_server		= server;

	int rc = ls->on_connect(ls->e, &ls->conn);
	if (rc < 0)
		TIMA_LOGE("Cannot create tcp session");
}

static struct listener_server *create_tcp_listener_server(vmp_addr *addr, vmp_launcher_t *e,
				vmp_server_t *server, server_new_connection_handler send_socket)
{
	vmp_node_t* p = server->priv;
	struct listener_server *listener = calloc(1, sizeof(struct listener_server));
	if(!listener)
	{
		TIMA_LOGE("create_tcp_listener_server listener null\n");
		return NULL;
	}
	listener->server		= server;
	listener->event_base	= server->event_base;
	listener->on_connect	= send_socket;
	listener->e				= e;
	listener->parent		= p;

	listener->lis = evconnlistener_new_bind(listener->event_base, server_accept_handler, 
						listener, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,	
						(struct sockaddr*)addr, sizeof(vmp_addr));
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

static void setup_tcp_listener_server(vmp_node_t* p, vmp_server_t *server)
{
	PrivInfo* thiz = p->private;

	struct in_addr	sin_addr;
	if (!inet_aton("0.0.0.0", &sin_addr))
		TIMA_LOGE("inet_aton error!");

	//int port = 9999;
	struct sockaddr_in my_address = {0};
	my_address.sin_family = AF_INET;
	my_address.sin_addr.s_addr = sin_addr.s_addr;
	my_address.sin_port = htons(thiz->req.port);

	thiz->listener = create_tcp_listener_server((vmp_addr *)&my_address, server->e, server, send_socket_to_core);
}

static void run_events(struct event_base *base, vmp_launcher_t *e)
{
	if (!base && e)
		base = e->event_base;

	if (!base)
		return;

	event_base_dispatch(base);
}

static void run_listener_server(struct listener_server *listener)
{
	run_events(listener->event_base, listener->e);

	evconnlistener_free(listener->lis);
	event_base_free(listener->event_base);
}

static void* server_listener_thread(void* arg)
{
	vmp_server_t *server = NULL;
	vmp_node_t *p = (vmp_node_t*)arg;
	PrivInfo* thiz = p->private;
	evthread_use_pthreads();

	VMP_LOGI("server_listener_thread begin\n");

	server = calloc(1, sizeof(vmp_server_t));
	thiz->main_server = server;
	server->priv = p;

	init_server();
	if (thiz->req.base)
		setup_server(p, server, thiz->req.base);
	else
		setup_server(p, server, NULL);
	stream_server_general(p, MAX_STREAM_SERVER_NUM);
	setup_tcp_listener_server(p, server);

	//barrier_wait();
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
	return NULL;
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

	NODE_CLASS_REGISTER(node_server_listener);
}

void server_listener_done(void)
{
	VMP_LOGD("server_listener_done");

	NODE_CLASS_UNREGISTER(SERVER_LISTENER_CLASS);
}
