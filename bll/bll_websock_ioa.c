/**
 * History:
 * ================================================================
 * 2018-05-22 qing.zou created
 *
 */

#include "event2/event.h"

#include "context.h"
#include "ThreadPool.h"
#include "server_websock.h"
#include "tima_jt1078_parser.h"

#include "bll_websock_ioa.h"

typedef struct _PrivInfo
{
	WebsockIOAReq		req;
	WebsockIOARep		rep;

	VmpSocketIOA		*sock;
	void				*client;

	unsigned long long	sim;
	int					state;

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


static int websockioa_input_release(vmp_node_t* p, int status, int motivated)
{
	int ret = 0;
	PrivInfo* thiz = p->private;
	
	TIMA_LOGD("websock release %d, cb status %d", thiz->state, status);
	if (p->pfn_callback) {
		p->pfn_callback(thiz->req.ws, status, NULL);
	}

	{
		char key[16] = {0};
		sprintf(key, "%012lld", thiz->sim);
		vmp_wserver_t *wserver = thiz->req.ws;
		tima_ioamaps_clear(wserver->map, key, MAPS_SOCK_WEBSKT);
	}

	if (thiz->sock) {
		vmp_socket_release(thiz->sock);
	}
	
	if (motivated) {
		ret = tima_websock_close(thiz->req.client);
	}

	return ret;
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

static int package_relay_proc(vmp_node_t* p, const char* buf, size_t size)
{
	size_t len;
	PrivInfo* thiz = (PrivInfo*)p->private;

	len = vpk_udp_send(thiz->sock->abs.fd, &thiz->sock->dest_addr, buf, size);
	//TIMA_LOGD("websock relay send len = %d", len);

	return len;
}

static int websocket_match_device(vmp_node_t* p, stream_header_t *head)
{
	VmpSocketIOA *sock = NULL;
	PrivInfo* thiz = (PrivInfo*)p->private;
	vmp_wserver_t *ws = thiz->req.ws;

	char tmp[16] = {0};
	sprintf(tmp, "%012lld", head->simno);

	if (thiz->state == SOCK_MATCH_STATE_GET) {
		sock = tima_ioamaps_get_type(ws->map, tmp, MAPS_SOCK_STREAM);
	}

	if (thiz->sim == (unsigned long long)-1) {
		thiz->sim = head->simno;
		TIMA_LOGI("[%ld] %p fd=%d sim no. [%lld]: %d", 
			thiz->req.flowid, vmp_thread_get_id(), tima_websock_fd_get(thiz->client), thiz->sim, head->channel);

		RelaySocketIO* relay_sock = tima_ioamaps_put(ws->map, tmp, thiz->sock, MAPS_SOCK_WEBSKT);
		if (!relay_sock) {
			TIMA_LOGE("websock map put failed.");
			return -1;
		}

		thiz->state = SOCK_MATCH_STATE_GET;
		sock = tima_ioamaps_exist(relay_sock, MAPS_SOCK_STREAM);
	}

	if (sock) {
		vpk_sockaddr_set_port(&thiz->sock->dest_addr, sock->src_port);
		if (vpk_sockaddr_get_port(&thiz->sock->dest_addr) < 1) {
			TIMA_LOGE("websock set dest addr port[%d] failed", sock->src_port);
			thiz->state = SOCK_MATCH_STATE_ERROR;
			return -1;
		}

		thiz->sock->dst_port = sock->src_port;
		thiz->state = SOCK_MATCH_STATE_SUCCESS;
		TIMA_LOGI("websocket match success [%d <-> %d].", thiz->sock->src_port, sock->src_port);
	}
	return 0;
}
static int websocket_input_proc(vmp_node_t* p, const char* buf, size_t size, void *channel)
{
	int ret = 0;
	size_t length = size;
	const char *data = buf;
	PrivInfo* thiz = (PrivInfo*)p->private;

	unsigned char *stream = NULL;
	stream_header_t head = {0};

	ret = packet_jt1078_parse((unsigned char*)data, length, &head, &stream);
	if (ret == 0)
		return 0;

	if (ret < 0) {
		TIMA_LOGE("[%ld] JT/T 1078-2016 parse failed, ret = %d", thiz->req.flowid, ret);
		TIMA_LOGD("=====flowid[%ld] %lld[fd %d]", thiz->req.flowid, thiz->sim, tima_websock_fd_get(thiz->client));
		goto parse_end;
	}

	if (ret > JT1078_STREAM_PACKAGE_SIZE) {
		TIMA_LOGD("=====flowid[%d] %lld[fd %d], ret = %d", thiz->req.flowid, thiz->sim, tima_websock_fd_get(thiz->client), ret);
		TIMA_LOGE("[%ld] JT/T 1078-2016 parse failed, ret = %d", thiz->req.flowid, ret);
		ret = JT1078_STREAM_PACKAGE_SIZE;
		goto parse_end;
	}

	if (thiz->state != SOCK_MATCH_STATE_SUCCESS) {
		printf("[len=%ld]#sim=%lld, channelid=%d, type[15]=%02x, [28:29]=%02x %02x, body len=%d, parsed=%d, state=%d\n",
			length, head.simno, head.channel, data[15], data[28], data[29], head.bodylen, ret, thiz->state);
		websocket_match_device(p, &head);
	} else {
		package_relay_proc(p, data, head.bodylen+30);
	}

parse_end:
	return ret;
}

static int ioa_on_message(void *client, tima_wsmessage_t *msg)
{
	//tima_wsmessage_t *msg = d;
	//fprintf(stderr, "Received message from client: %d\n", state->sockfd);
	//fprintf(stderr, "Message opcode: %d\n", msg->opcode);
	//fprintf(stderr, "Payload Length: %llu\n", msg->payload_len);
	//fprintf(stderr, "[%p]Payload: %s\n", (void*)pthread_self(), msg->payload);
	//TIMA_LOGD("[%p]Payload(len=%d): %s\n", (void*)pthread_self(), msg->payload_len, msg->payload);
	//now let's send it back.

	//libwebsock_send_text(state, msg->payload);
	
	//tima_websock_send_text(client, msg->payload);
	//tima_websock_send_binary(client, msg->payload, msg->payload_len);

	vmp_node_t *p =	tima_websock_priv_get(client);
	PrivInfo* thiz = (PrivInfo*)p->private;
	thiz->client = client;

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
	vmp_node_t *p =	tima_websock_priv_get(client);
	PrivInfo* thiz = p->private;

	//relay_wserver_t *rws = tima_websock_get_relay_server(thiz->req.client);
	//event_free(thiz->sock->read_event);

	int fd = tima_websock_fd_get(client);
	TIMA_LOGW("[%p]websock client close fd: %d", (void*)pthread_self(), fd);

	if (thiz->state == SOCK_MATCH_STATE_SUCCESS) {
		int ret = 0;
		char end[64] = {0};
		ret = jt1078_make_end(end, "END", 3);
		package_relay_proc(p, end, ret);
		thiz->state = SOCK_MATCH_STATE_DISCONN;
	}

	//vmp_socket_release(thiz->sock);
	websockioa_input_release(p, NODE_SUCCESS, 0);

	return 0;
}

static char cmsg[2048] = {0};
static char recv_buffer[2048] = {0};

static void relay_input_handler(evutil_socket_t fd, short what, void* arg)
{
	int ret = 0, len = 0;
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

	len = vpk_udp_recvfrom(fd, &raddr, &s->local_addr, recv_buffer, 1024, &ttl, &tos, cmsg, 0, NULL);
	if (len > 0) {
		try_again = 1;
		//VMP_LOGD("recvfrom socket[len=%d]: %s", len, recv_buffer);

		unsigned char *stream = NULL;
		stream_header_t head = {0};
		ret = packet_jt1078_parse((unsigned char*)recv_buffer, len, &head, &stream);
		if (ret > 0 && ret < JT1078_STREAM_PACKAGE_SIZE) {
			if (head.channel == 0xff) {
				if (head.mtype == 0xff) {
					VMP_LOGW("END recvfrom socket[len=%d]", len);

					//thiz->state = SOCK_MATCH_STATE_DISCONN;
					tima_websock_close(thiz->req.client);
					return ;
				}
			}

			tima_websock_send_binary(thiz->req.client, recv_buffer, ret);
		} else {
			TIMA_LOGW("[ws]relay 1078 parse error.");
		}

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

	thiz->sim = (unsigned long long)-1;

	tima_websock_callback_set(thiz->req.client, &sock);
	tima_websock_priv_set(thiz->req.client, p);

	//relay_wserver_t *rws = tima_websock_get_relay_server(thiz->req.client);
	vmp_wserver_t *ws = thiz->req.ws;

	ret = vmp_relay_socket_create(ws, VPK_APPTYPE_WEBSOCKET_RELAY, &thiz->sock);
	if (ret < 0) {
		VMP_LOGE("relay socket create failed.");
		//websockioa_input_release(p, NODE_FAIL, 1);
		tima_websock_close(thiz->req.client);		// maybe better to run next callback
		return NULL;
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
