/*
 ============================================================================
 Name        : node.c
 Author      : wison.wei
 Copyright   : 2017(c) Timanetworks Company
 Description : 
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tmHttp.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <evhttp.h>
#include <event.h>
#include <evdns.h>
#include "event2/dns.h"
#include "event2/dns_compat.h"
#include "event2/dns_struct.h"

#include <context.h>

typedef struct _HttpContext
{
	struct event_base *base;
	struct evhttp_connection *cn;
	struct evhttp_request *req;
	struct bufferevent* bev;
	struct evhttp_uri *uri;
	struct evdns_base* dnsBase;
	struct evbuffer* buffer;
	tmHttp* http;
	int retry;
	int status;
	bool bStop;
}HttpContext;

static void http_callback(struct evhttp_request *req, void *arg)
{
	HttpContext* pCtx = (HttpContext*)arg;
	struct event_base* base = pCtx->base;
	if (!req)
	{
		TIMA_LOGE("[%d]http_callback, req null, arg: %p, retry = %d ", pCtx->http->id, arg, pCtx->retry);
		pCtx->retry++;
		event_base_loopexit(base, 0);
		return;
	}

	pCtx->status = req->response_code;

	/*  response is ready */
	switch(req->response_code)
	{
	case HTTP_OK:
		evbuffer_add_buffer(pCtx->buffer, req->input_buffer);
		event_base_loopexit(base, 0);
		break;

	default:
		/*  FAILURE */
		TIMA_LOGI("[%d]http_callback, response_code = %d, retry = %d ", pCtx->http->id, req->response_code, pCtx->retry);
		pCtx->retry++;
		event_base_loopexit(base, 0);
		return;
    }
}

static void http_error_callback(enum evhttp_request_error e, void *arg)
{
	TIMA_LOGE("request error type = %d\n\n", e);
}

void Libevent_StopHttp( void* arg)
{
	HttpContext* pCtx = (HttpContext*)arg;
	if (pCtx)
	{
		evhttp_cancel_request(pCtx->req);
		pCtx->status = 506;
		pCtx->bStop = true;
	}
}

void* Libevent_malloc(void)
{
	HttpContext* pCtx = (HttpContext*)malloc(sizeof(HttpContext));
	memset(pCtx, 0, sizeof(HttpContext));
	return pCtx;
}

void Libevent_free(void* pParam)
{
	HttpContext* pCtx = (HttpContext*)pParam;
	if (pCtx)
	{
		free (pCtx);
	}
}

void Libevent_printfData(int nId, char* pData)
{
	TIMA_LOGI("[%d]RESPONSE DUMP: %s", nId, pData);
}

void Libevent_Handle200(tmHttp* pHttp, HttpContext* pCtx)
{
	struct evbuffer* input = pCtx->buffer;
	if (input)
	{
		int nLen = evbuffer_get_length(input);
		if (nLen > 0)
		{
			pHttp->rsp.data = (char*)malloc(nLen+1);
			pHttp->rsp.size = evbuffer_remove(input, pHttp->rsp.data, nLen);
			pHttp->rsp.data[nLen] = '\0';
		}
	}

	Libevent_printfData(pHttp->id, pHttp->rsp.data);

	pHttp->cbHttp(&pHttp->rsp);

	if (pHttp->rsp.data)
	{
		free(pHttp->rsp.data);
		pHttp->rsp.data = NULL;
	}

	evbuffer_free(pCtx->buffer);
	if (pCtx->cn)
		evhttp_connection_free(pCtx->cn);
	if (pCtx->base)
		event_base_free(pCtx->base);
}

void Libevent_HandleOther(tmHttp* pHttp, HttpContext* pCtx)
{
	TIMA_LOGE("[%d]RESPONSE ERROR ==> %d : %s", pHttp->id, pHttp->rsp.status, tmHttp_Code2Reason(pHttp->rsp.status));
	Libevent_printfData(pHttp->id, pHttp->rsp.data);

	pHttp->cbHttp(&pHttp->rsp);

	evbuffer_free(pCtx->buffer);
	if (pCtx->cn)
		evhttp_connection_free(pCtx->cn);
	if (pCtx->base)
		event_base_free(pCtx->base);
}

void Libevent_GetHost(struct evhttp_uri* pUri, char* pHost, int* pPort, char* pPath)
{
	const char* host = evhttp_uri_get_host(pUri);
	printf("1 host=%s\n", host);
	int port = evhttp_uri_get_port(pUri);
	printf("2 port=%d\n", port);
	port = (port != -1 ? port : 8080);
	const char* path = evhttp_uri_get_path(pUri);
	printf("3 path=%s\n", path);

	struct evutil_addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = EVUTIL_AI_PASSIVE|EVUTIL_AI_ADDRCONFIG;

	struct evutil_addrinfo *ai = NULL;
	char strport[32];
	evutil_snprintf(strport, sizeof(strport), "%d", port);
	evutil_getaddrinfo(host, strport, (struct evutil_addrinfo*)&hints, &ai);
	printf("4 host=%s, port=%s\n", host, strport);
	host = inet_ntoa(((struct sockaddr_in*)(ai->ai_addr))->sin_addr);
	printf("5 host=%s, port=%s\n", host, strport);
	
	strcpy(pHost, host);
	*pPort = port;
	strcpy(pPath, path);
}

void Libevent_DnsParse(const char* url, char* host, int* port,  char* path)
{
	struct event_base *base = event_base_new();
	struct evhttp_uri *uri = evhttp_uri_parse(url);
	Libevent_GetHost(uri, host, port, path);
	event_base_free(base);
}

static void
dns_gethostbyname_cb(int result, char type, int count, int ttl,
    void *addresses, void *arg)
{
	printf("result=%d count=%d tll=%d type=%d\n", result, count, ttl, type);

	switch (type) {
	case DNS_IPv4_A: {
		printf("55\n");
		struct in_addr *in_addrs = addresses;
		int i;
		/* a resolution that's not valid does not help */
		if (ttl < 0)
			goto out;
		printf(("%s \n", inet_ntoa(in_addrs[0])));
		for (i = 0; i < count; ++i)
			printf(("%s \n", inet_ntoa(in_addrs[i])));
		break;
	}
	case DNS_PTR:
		/* may get at most one PTR */
		if (count != 1)
			goto out;

		printf(("%s ", *(char **)addresses));
		break;
	default:
		goto out;
	}

out:
	printf("44\n");
	event_base_loopexit((struct event_base *) arg, 0);
}

#if 0
static void dns_gethostbyname(void)
{
	//printf("111\n");
	struct event_base *base = event_base_new();
	struct evdns_base *dnsbase = evdns_base_new(base, 1);
	evdns_base_resolve_ipv4(dnsbase, "http://testcarnet.timanetwork.com:8888", 0, dns_gethostbyname_cb, base);
	//int ret = evdns_base_resolve_ipv4(dnsbase, "testcarnet.timanetwork.com:8888", 0, dns_gethostbyname_cb, NULL);
	
	//printf("222 ret=%d\n", ret);
	event_base_dispatch(base);
	//printf("333 ret=%d\n", ret);
	evdns_base_free(dnsbase, 1);

	event_base_free(base);
}
#endif

void* Libevent_HttpReqThread( void* arg)
{
	tmHttp* pHttp = (tmHttp*)arg;
	HttpContext* pCtx = (HttpContext*)pHttp->stopArg;
	//context* ctx = Context();
	int req_type = EVHTTP_REQ_POST;

	if (pHttp->req.reqtype == HTTP_REQ_GET)
		req_type = EVHTTP_REQ_GET;
	else if (pHttp->req.reqtype == HTTP_REQ_POST)
		req_type = EVHTTP_REQ_POST;
	else if (pHttp->req.reqtype == HTTP_REQ_PUT)
		req_type = EVHTTP_REQ_PUT;

	char* strHost = pHttp->req.ip;
	char* strPort = pHttp->req.port;

	TIMA_LOGI("[%d]--------------------------------------------------------------",pHttp->id);
	TIMA_LOGI("[%d]Start Request[%d]: %s:%s%s", pHttp->id, req_type, pHttp->req.ip, pHttp->req.port, pHttp->req.pUrl);
	if (req_type == EVHTTP_REQ_POST || req_type == EVHTTP_REQ_PUT)
		TIMA_LOGI("[%d]REQUEST DUMP: %s", pHttp->id, pHttp->req.pPostData);

	pCtx->retry = 1;
RETRY:
	TIMA_LOGI("[%d]Try  %d",pHttp->id, pCtx->retry);
	
	pCtx->base = event_base_new();

	pCtx->http = pHttp;
	pCtx->buffer = evbuffer_new();

	//char host[256] = {0};
	//int port = 8080;
	//char path[256] = {0};
	//printf("1111\n");
	//dns_gethostbyname();
	//Libevent_DnsParse("http://testcarnet.timanetwork.com:8888/tcn-web", host, &port, path);
	//printf("%s %d %s\n", host, port, path);
	//strHost = "183.221.250.11";
	//strHost = "127.0.0.1";
	//strHost = host;
	//strHost = "testcarnet.timanetwork.com";
	//strPort = "8888";

	char accept[256] ={0};
	strcpy(accept, "application/json");
	
	//strHost = "127.0.1.11";
	TIMA_LOGI("[%d]HOST : %s:%s", pHttp->id, strHost, strPort);

	pCtx->cn = evhttp_connection_base_new(pCtx->base, NULL, strHost, atoi(strPort));
	pCtx->req = evhttp_request_new(http_callback, pCtx);
	evhttp_request_set_error_cb(pCtx->req, http_error_callback);
	errno == 0;
	evhttp_add_header(pCtx->req->output_headers, "Accept", "*/*");
	evhttp_add_header(pCtx->req->output_headers, "Content-Type", "application/json");
	evhttp_add_header(pCtx->req->output_headers, "Host", strHost);
	if (req_type == EVHTTP_REQ_POST || req_type == EVHTTP_REQ_PUT)
		evbuffer_add(pCtx->req->output_buffer, pHttp->req.pPostData, strlen(pHttp->req.pPostData));
	int ret = evhttp_make_request(pCtx->cn, pCtx->req, req_type, pHttp->req.pUrl);
	TIMA_LOGD("errno = %d, ret = %d", errno, ret);
	if (ret == -1) {
		TIMA_LOGE("Couldn't make request");
	}
	//if(errno != 111 && errno != 110)
	//{
	//	//TODO check errno!=111 is not form operation just for  disconnected
	//	
	//	//TIMA_LOGD("wcs----0 %d ret=%d", pCtx->req->output_headers, ret);
	//	//evhttp_add_header(pCtx->req->output_headers, "Accept", "application/json");
	//	evhttp_add_header(pCtx->req->output_headers, "Accept", "*/*");
	//	//TIMA_LOGD("wcs----0 %d", pCtx->req->output_headers);
	//	evhttp_add_header(pCtx->req->output_headers, "Content-Type", "application/json");
	//	//TIMA_LOGD("wcs----1 %d", pCtx->req->output_headers);
	//	evhttp_add_header(pCtx->req->output_headers, "Host", strHost);
	//	//TIMA_LOGD("wcs----2 %d", pCtx->req->output_headers);
	//}
	
	//evhttp_add_header(ctx.req->output_headers, "User-Agent", "TimaUserAgent");
	//evhttp_add_header(ctx.req->output_headers, "Connection", "keep-alive");
	evhttp_connection_set_timeout(pCtx->cn, HTTP_REQUEST_NONBLOCK_TIMEOUT);
	event_base_dispatch(pCtx->base);

	pHttp->rsp.status = pCtx->status;
	switch (pCtx->status)
	{
	case HTTP_OK:
		Libevent_Handle200(pHttp, pCtx);
		break;

	case HTTP_INTERNAL:
		Libevent_HandleOther(pHttp, pCtx);
		if (pCtx->retry <= HTTP_REQUEST_RETRYTIMES)
			goto RETRY;
		break;
	default:
		if (pCtx->retry <= HTTP_REQUEST_RETRYTIMES) {
			TIMA_LOGI("[%d]test mult-http post.", pHttp->id);
			Libevent_printfData(pHttp->id, pHttp->rsp.data);
			goto RETRY;
		}
		else
		{
			Libevent_HandleOther(pHttp, pCtx);
		}
		break;
	}
	
	TIMA_LOGI("[%d]Request End ", pHttp->id);
	TIMA_LOGI("[%d]--------------------------------------------------------------", pHttp->id);
	
	return NULL;
}
