/**
 * History:
 * ================================================================
 * 2019-03-29 qing.zou created
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

#include "context.h"
#include "tima_http.h"
#include "tima_http_client.h"


typedef struct tima_http_s
{
	TimaHttpReq			req;
	TimaHttpRsp			rsp;

	TimaHttpCB			callback;

	int					retry;

	char				client[0];

} tima_http_t;

static int t_id = 0;
//extern struct event_base* http_base;

static int tima_http_feed(void *ctx, void *data)
{
	tima_http_t *http = container_of(ctx, tima_http_t, client);

	if (http->callback)
		http->callback(data);

	return 0;
}
static int tima_http_free(void *ctx)
{
	tima_http_t *http = container_of(ctx, tima_http_t, client);
	
	if (http) {
		if (http->req.url)
			free(http->req.url);

		if (http->req.data)
			free(http->req.data);

		free(http);
	}
	
	return 0;
}

int tima_http_post(void *uri, void *post_data,	void *node, TimaHttpCB callback, int retry, int *task_id)
{
	context* ctx = context_get();
	struct event_base *hbase = ctx->hbase;
	if (hbase == NULL) {
		TIMA_LOGE("hbase is NULL");
		return -1;
	}

	tima_http_t *http = calloc(1, (sizeof(tima_http_t) + sizeof(HttpClient)));
	if (!http) {
		TIMA_LOGE("tima_http_post failed, malloc null.");
		return -1;
	}

	TimaUri* tima_uri = (TimaUri*)uri;

	http->callback		= callback;
	http->req.id		= ++t_id;
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
	http->rsp.id	= t_id;

	HttpClient *client = (HttpClient*)http->client;
	client->ops.write	= tima_http_feed;
	client->ops.free	= tima_http_free;
	client->rsp			= &http->rsp;
	tima_http_handle(http->client, &http->req, hbase);

	return 0;
}
