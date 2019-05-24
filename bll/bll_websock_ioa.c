/**
 * History:
 * ================================================================
 * 2018-05-22 qing.zou created
 *
 */

#include "context.h"
#include "ThreadPool.h"

#include "bll_websock_ioa.h"

typedef struct _PrivInfo
{
	WebsockIOAReq		req;
	WebsockIOARep		rep;

	int					id;
} PrivInfo;

static int bll_websockioa_delete(vmp_node_t* p);

static int task_websockioa_callback(void* p, int msg, void* arg)
{
	vmp_node_t* demo = ((vmp_node_t*)p)->parent;
	if ( msg != NODE_SUCCESS)
	{
		VMP_LOGW("task_websockioa_callback fail");
		bll_websockioa_delete(demo);
		return -1;
	}

	bll_websockioa_delete(demo);
	return 0;
}

static int bll_websockioa_get(vmp_node_t* p, int id, void* data, int size)
{
	return 0;
}

static int bll_websockioa_set(vmp_node_t* p, int id, void* data, int size)
{	
	PrivInfo* thiz = p->private;
	thiz->req = *((WebsockIOAReq*)data);
	
	return 0;
}


static int ioa_on_message(void *client, tima_wsmessage_t *msg)
//static int ioa_on_message(void *client, void *d)
{
	//tima_wsmessage_t *msg = d;
	//fprintf(stderr, "Received message from client: %d\n", state->sockfd);
	//fprintf(stderr, "Message opcode: %d\n", msg->opcode);
	//fprintf(stderr, "Payload Length: %llu\n", msg->payload_len);
	//fprintf(stderr, "[%p]Payload: %s\n", (void*)pthread_self(), msg->payload);
	TIMA_LOGD("[%p]Payload: %s\n", (void*)pthread_self(), msg->payload);
	//now let's send it back.

	//libwebsock_send_text(state, msg->payload);
	
	//tima_websock_send_text(client, msg->payload);
	tima_websock_send_binary(client, msg->payload, msg->payload_len);

	return 0;
}

static int ioa_on_onpong(void *client)
{
	return 0;
}

static int ioa_on_close(void *client)
{
	int fd = tima_websock_fd_get(client);

	TIMA_LOGI("websock client close fd: %d", fd);

	return 0;
}

static void* bll_websockioa_start(vmp_node_t* p)
{
	VMP_LOGD("bll_websockioa_start");

	PrivInfo* thiz = p->private;

	TimaWebsockFunc sock = {0};
	sock.onmessage	= ioa_on_message;
	sock.onclose	= ioa_on_close;
	sock.onpong		= ioa_on_onpong;

	tima_websock_callback_set(thiz->req.client, &sock);
	
	return NULL;
}

static int bll_websockioa_stop(vmp_node_t* p)
{
	return 0;
}

static vmp_node_t* bll_websockioa_create(void)
{
	vmp_node_t* p = NULL;

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));
		
		p = (vmp_node_t*)malloc(sizeof(vmp_node_t));
		memset(p, 0, sizeof(vmp_node_t));
		p->nclass		= BLL_WEBSOCK_IOA_CLASS;
		p->pfn_get		= (nodeget)bll_websockioa_get;
		p->pfn_set		= (nodeset)bll_websockioa_set;
		p->pfn_start	= (nodestart)bll_websockioa_start;
		p->pfn_stop		= (nodestop)bll_websockioa_stop;
		p->pfn_callback = NULL;
		p->private		= thiz;

		return p;
	}while(0);
	
	return p;
}

static int bll_websockioa_delete(vmp_node_t* p)
{
	//VMP_LOGD("bll_websockioa_delete");

	PrivInfo* thiz = (PrivInfo*)p->private;
	if(thiz != NULL)
	{
		free(thiz);
		p->private = NULL;
	}

	free(p);
	
	return 0;
}

static const nodedef node_websockioa = 
{	
	sizeof(PrivInfo),
	BLL_WEBSOCK_IOA_CLASS,
	(nodecreate)bll_websockioa_create,
	(nodedelete)bll_websockioa_delete
};

void bll_websockioa_init(void)
{
	VMP_LOGI("bll_websockioa_init");

	NODE_CLASS_REGISTER(node_websockioa);
}

void bll_websockioa_done(void)
{
	VMP_LOGI("bll_websockioa_done");

	NODE_CLASS_UNREGISTER(BLL_WEBSOCK_IOA_CLASS);
}
