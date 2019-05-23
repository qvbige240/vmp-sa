/**
 * History:
 * ================================================================
 * 2018-02-05 qing.zou created
 *
 */

#include "context.h"
#include "ThreadPool.h"

#include "server_websock.h"

#include "bll_voice_task.h"


typedef struct _PrivInfo
{
	VoiceTaskReq		req;
	VoiceTaskRep		rep;

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

static int on_connect(void *client)
{
	int fd = tima_websock_fd_get(client);

	TIMA_LOGI("websock server handle fd: %d", fd);

	//TIMA_LOGI("[%ld] server[%d] handle fd: %d, count: %d  (%s : %u)", 
		//thiz->flowid, ss->id, sock->fd, ss->client_cnt, ip, vpk_sockaddr_get_port(&sock->peer_addr));
	return 0;
}

static void* bll_voice_start(vmp_node_t* p)
{
	VMP_LOGD("bll_voice_start");

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
