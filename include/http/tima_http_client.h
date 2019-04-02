/**
 * History:
 * ================================================================
 * 2019-03-29 qing.zou created
 *
 */
#ifndef TIMA_HTTP_CLIENT_H
#define TIMA_HTTP_CLIENT_H

#include "tima_httpdef.h"

#pragma pack(1)

TIMA_BEGIN_DELS

#define HTTP_REQUEST_RETRYTIMES				(3)
#define HTTP_REQUEST_NONBLOCK_TIMEOUT		(21)

typedef struct _HttpClient
{
	int							id;

	struct event_base			*base;
	struct evhttp_connection	*evcon;
	struct evhttp_request		*req;
	struct bufferevent			*bev;
	struct evbuffer				*input_buffer;
	struct evhttp_uri			*uri;

	int							retries;

	int							status;
} HttpClient;

int tima_http_handle(void *client, TimaHttpReq *req, void *base);

#pragma pack()

TIMA_END_DELS

#endif // TIMA_HTTP_CLIENT_H
