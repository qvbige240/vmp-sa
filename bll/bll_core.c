/**
 * History:
 * ================================================================
 * 2019-03-11 qing.zou created
 *
 */

#include "context.h"
#include "ThreadPool.h"

#include "bll_core.h"
#include "server_listener.h"

typedef struct _PrivInfo
{
	long			flowid;

	int				cond;
	int				id;

	vmp_server_t	*server;
} PrivInfo;





static void relay_receive_message(struct bufferevent *bev, void *ptr)
{
	vmp_connection_t session;
	int n = 0;
	struct evbuffer *input = bufferevent_get_input(bev);
	//struct stream_server *ss = (struct stream_server *)ptr;

	while ((n = evbuffer_remove(input, &session, sizeof(vmp_connection_t))) > 0) {

		if (n != sizeof(vmp_connection_t)) {
			perror("Weird buffer error\n");
			continue;
		}

		TIMA_LOGD("handle fd: %d", session.sock.fd);

		//handle_relay_message(ss, &session);
	}
}

static int task_server_listener(PrivInfo* thiz)
{
	context* ctx = context_get();
	vmp_node_t* p = node_create(SERVER_LISTENER_CLASS, ctx->vector_node);

	vmp_server_t *server = calloc(1, sizeof(vmp_server_t));
	server->read_cb = relay_receive_message;
	thiz->server = server;

	ServerListenerReq req = {0};
	req.server	= server;
	p->parent	= thiz;
	p->pfn_set(p, 0, &req, sizeof(ServerListenerReq));
	p->pfn_start(p);
	
	return 0;
}



static void* bll_core_thread(void* arg)
{
	PrivInfo* thiz = (PrivInfo*)arg;

	//test...
	task_server_listener(thiz);

	while(thiz->cond)
	{
		//CoreTaskMsg* msg = (CoreTaskMsg*) bll_core_recvmsg(thiz);
		//if(msg)
		//{
		//	bll_core_onmsg(thiz, msg);
		//	free(msg);
		//}
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
