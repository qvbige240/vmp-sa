/**
 * History:
 * ================================================================
 * 2019-03-29 qing.zou created
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tima_http.h"
#include "tima_http_client.h"

typedef struct tima_http_s
{
	TimaHttpReq			req;
	TimaHttpRsp			rsp;

	TimaHttpCB			http_callback;

	int					retry;

	char				client[0];

} tima_http_t;

static int t_id = 0;
extern struct event_base* http_base;

int tima_http_post(void *uri, void *post_data,	void *node, TimaHttpCB callback, int retry, int *task_id)
{
	tima_http_t *http = malloc(sizeof(tima_http_t) + sizeof(HttpClient));
	if (!http) {
		TIMA_LOGE("tima_http_post failed, malloc null.");
		return -1;
	}

	TimaUri* tima_uri = (TimaUri*)uri;

	http->http_callback = callback;
	http->req.id		= t_id++;
	http->req.reqtype	= tima_uri->type;
	if (tima_uri->type == HTTP_REQ_GET)
	{
		int len1 = strlen(tima_uri->path);
		int len2 = strlen(post_data);
		http->req.url =  (char*)calloc(1, len1+len2+1);
		strncpy(http->req.url, tima_uri->path, len1);
		strncpy(http->req.url+len1, post_data, len2);
	}
	else
	{
		http->req.data = (char*)calloc(1, strlen(post_data)+1);
		strcpy(http->req.data, post_data);
		http->req.url =  (char*)calloc(1, strlen(tima_uri->path)+1);
		strcpy(http->req.url, tima_uri->path);
	}
	strncpy(http->req.host, tima_uri->ip, sizeof(http->req.host));
	http->req.port	= tima_uri->port;
	http->req.retry = retry;
	http->req.priv	= node;
	http->rsp.priv	= node;

	//context* ctx = context_get();
	//struct event_base *base = ctx->http_base;
	tima_http_handle(http->client, &http->req, http_base);

	return 0;
}
