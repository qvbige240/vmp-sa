/**
 * History:
 * ================================================================
 * 2018-05-22 qing.zou created
 *
 */

#include "context.h"
#include "ThreadPool.h"
#include "server_websock.h"

#include "event2/event.h"

#include "bll_websock_ioa.h"

typedef struct _PrivInfo
{
	WebsockIOAReq		req;
	WebsockIOARep		rep;

	VmpSocketIOA		*sock;

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

static int websocket_input_proc(vmp_node_t* p, const char* buf, size_t size, void *channel)
{
	size_t pos = 0, len;
	size_t length = size;
	const char *data = buf;
	PrivInfo* thiz = (PrivInfo*)p->private;

	len = vpk_udp_send(thiz->sock->abs.fd, &thiz->sock->dest_addr, buf, size);
	TIMA_LOGD("websock relay send len = %d", len);

	return 0;
}

static int ioa_on_message(void *client, tima_wsmessage_t *msg)
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
	//tima_websock_send_binary(client, msg->payload, msg->payload_len);

	vmp_node_t *p =	tima_websock_priv_get(client);
	websocket_input_proc(p, msg->payload, msg->payload_len, NULL);

	return 0;
}

static int ioa_on_onpong(void *client)
{
	int fd = tima_websock_fd_get(client);

	TIMA_LOGD("[%p]ioa_on_onpong client fd: %d", (void*)pthread_self(), fd);
	return 0;
}

static int ioa_on_close(void *client)
{
	int fd = tima_websock_fd_get(client);

	vmp_node_t *p =	tima_websock_priv_get(client);
	PrivInfo* thiz = p->private;

	//relay_wserver_t *rws = tima_websock_get_relay_server(thiz->req.client);
	//event_free(thiz->sock->read_event);

	vmp_socket_release(thiz->sock);


	TIMA_LOGI("[%p]websock client close fd: %d", (void*)pthread_self(), fd);

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
		VMP_LOGD("recvfrom socket[len=%d]: %s", ret, recv_buffer);

		tima_websock_send_binary(thiz->req.client, recv_buffer, ret);
	}

	if (try_again) {
		goto try_start;
	}


}

static void* bll_websockioa_start(vmp_node_t* p)
{
	VMP_LOGD("bll_websockioa_start");

	int ret = 0;
	PrivInfo* thiz = p->private;

	TimaWebsockFunc sock = {0};
	sock.onmessage	= ioa_on_message;
	sock.onclose	= ioa_on_close;
	sock.onpong		= ioa_on_onpong;

	tima_websock_callback_set(thiz->req.client, &sock);
	tima_websock_priv_set(thiz->req.client, p);

	//relay_wserver_t *rws = tima_websock_get_relay_server(thiz->req.client);
	vmp_wserver_t *ws = thiz->req.ws;

	ret = vmp_relay_socket_create(ws, VPK_APPTYPE_WEBSOCKET_RELAY, &thiz->sock);
	if (ret < 0) {
		VMP_LOGE("relay socket create failed.");
	}

	VmpSocketIOA *s = thiz->sock;
	s->read_event = event_new(ws->event_base, s->abs.fd, EV_READ|EV_PERSIST, relay_input_handler, p);
	event_add(s->read_event, NULL);
	
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
