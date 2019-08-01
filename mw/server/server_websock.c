/**
 * History:
 * ================================================================
 * 2018-05-21 qing.zou created
 *
 */

#include "server_websock.h"

#include "context.h"
#include "ThreadPool.h"

#include "websock/websock.h"


typedef struct _PrivInfo
{
	ServerWebsockReq	req;
	ServerWebsockRep	rep;

	vmp_wserver_t		**wserver;
	int					id;
} PrivInfo;

static int server_websock_delete(vmp_node_t* p);

//static int task_demo_callback(void* p, int msg, void* arg)
//{
//	vmp_node_t* demo = ((vmp_node_t*)p)->parent;
//	if ( msg != NODE_SUCCESS)
//	{
//		VMP_LOGW("task_demo_callback fail");
//		server_websock_delete(demo);
//		return -1;
//	}
//
//	server_websock_delete(demo);
//	return 0;
//}

static int ws_onmessage(libwebsock_client_state *state, libwebsock_message *msg)
{
	fprintf(stderr, "Received message from client: %d\n", state->sockfd);
	fprintf(stderr, "Message opcode: %d\n", msg->opcode);
	fprintf(stderr, "Payload Length: %llu\n", msg->payload_len);
	fprintf(stderr, "[%p]Payload: %s\n", (void*)pthread_self(), msg->payload);
	//now let's send it back.
	libwebsock_send_text(state, msg->payload);
	return 0;
}

static int ws_onopen(libwebsock_client_state *state)
{
	libwebsock_context *websock = state->ctx;
	vmp_node_t *p = (vmp_node_t*)websock->user_data;
	PrivInfo* thiz = p->private;

	fprintf(stderr, "ws_onopen: %d\n", state->sockfd);

	vmp_wserver_t *wserver = NULL;
	if (thiz->wserver && state->server) {
		unsigned char offset = thiz->wserver[0]->id;
		wserver = thiz->wserver[state->server->id - offset];
	}

	ServerWebsockRep rep = {0};
	rep.client	= state;
	rep.ws		= wserver;

	if (thiz->req.on_connect)
		thiz->req.on_connect(thiz->req.ctx, (void*)&rep);

	return 0;
}

static int ws_onclose(libwebsock_client_state *state)
{
	fprintf(stderr, "ws_onclose: %d\n", state->sockfd);
	return 0;
}

static vmp_wserver_t** ws_server_general(libwebsock_context *ws, int num)
{
	int i = 0;
	ws_server_t **server = libwebsock_server_general(ws, num);
	vmp_wserver_t **wserver = calloc(1, sizeof(vmp_wserver_t*) * num);

	context* ctx = context_get();

	for (i = 0; i < num; i++)
	{
		wserver[i] = calloc(1, sizeof(vmp_wserver_t));
		wserver[i]->event_base = server[i]->event_base;
		wserver[i]->id = server[i]->id;
		wserver[i]->porter = ctx->porter;
	}
	
	return wserver;
}

static void* server_websock_thread(void* arg)
{
	//vmp_server_t *server = NULL;
	libwebsock_context *websock = NULL;
	vmp_node_t *p = (vmp_node_t*)arg;
	PrivInfo* thiz = p->private;

	VMP_LOGI("server_websock_thread begin\n");

	websock = libwebsock_init();
	if (websock) {
		char port[8] = {0};
		websock->user_data = p;
		sprintf(port, "%d", thiz->req.port);
		thiz->wserver = ws_server_general(websock, 8);
#if 1
		libwebsock_bind(websock, "0.0.0.0", port);
#else
		//libwebsock_bind_ssl(websock, "0.0.0.0", port, "./privkey.pem", "./cacert.pem");
		libwebsock_bind_ssl(websock, "0.0.0.0", port, "server.key", "server.crt");
		VMP_LOGI("websock listening on port %s, privkey: %s, ca: %s\n", port, "./privkey.pem", "./cacert.pem");
#endif
		websock->onmessage	= ws_onmessage;
		websock->onopen		= ws_onopen;
		websock->onclose	= ws_onclose;

		libwebsock_wait(websock);

	} else {
		VMP_LOGE("Error during libwebsock_init.");
	}

	//perform any cleanup here.
	VMP_LOGI("server_websock_thread end\n");

	return NULL;
}

static int server_websock_get(vmp_node_t* p, int id, void* data, int size)
{
	return 0;
}

static int server_websock_set(vmp_node_t* p, int id, void* data, int size)
{	
	PrivInfo* thiz = p->private;
	thiz->req = *((ServerWebsockReq*)data);
	
	return 0;
}

static void* server_websock_start(vmp_node_t* p)
{
	VMP_LOGD("server_websock_start");

	PrivInfo* thiz = p->private;

	context * ctx = context_get();
	ThreadPoolJob job;

	TPJobInit( &job, ( start_routine) server_websock_thread, p);
	TPJobSetFreeFunction( &job, ( free_routine ) NULL );
	TPJobSetStopFunction( &job, ( stop_routine ) NULL );
	TPJobSetPriority( &job, HIGH_PRIORITY );
	ThreadPoolAddPersistent( ctx->tp, &job, &job.jobId);
	thiz->id = job.jobId;

	return NULL;
}

static int server_websock_stop(vmp_node_t* p)
{
	return 0;
}

static vmp_node_t* server_websock_create(void)
{
	vmp_node_t* p = NULL;

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));
		
		p = (vmp_node_t*)malloc(sizeof(vmp_node_t));
		memset(p, 0, sizeof(vmp_node_t));
		p->nclass		= SERVER_WEBSOCK_CLASS;
		p->pfn_get		= (nodeget)server_websock_get;
		p->pfn_set		= (nodeset)server_websock_set;
		p->pfn_start	= (nodestart)server_websock_start;
		p->pfn_stop		= (nodestop)server_websock_stop;
		p->pfn_callback = NULL;
		p->private		= thiz;

		return p;
	}while(0);
	
	return p;
}

static int server_websock_delete(vmp_node_t* p)
{
	//VMP_LOGD("server_websock_delete");

	PrivInfo* thiz = (PrivInfo*)p->private;
	if(thiz != NULL)
	{
		free(thiz);
		p->private = NULL;
	}

	free(p);
	
	return 0;
}

static const nodedef node_server_websock = 
{	
	sizeof(PrivInfo),
	SERVER_WEBSOCK_CLASS,
	(nodecreate)server_websock_create,
	(nodedelete)server_websock_delete
};

void server_websock_init(void)
{
	VMP_LOGI("server_websock_init");

	NODE_CLASS_REGISTER(node_server_websock);
}

void server_websock_done(void)
{
	VMP_LOGI("server_websock_done");

	NODE_CLASS_UNREGISTER(SERVER_WEBSOCK_CLASS);
}
