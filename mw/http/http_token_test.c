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



static void* tima_token_start(node* p)
{
	TIMA_LOGD("tima_token_start");

	//CacheNetworkConfig cfg;
	//node* cache = Context()->cache;
	//cache->pfnGet(cache, CACHE_TIMA_NETWORK, &cfg, sizeof(CacheNetworkConfig));

	TIMA_LOGD("tima_token_start 1");
	//TIMA_LOGD("tima_token_start %s %s", cfg.http_ip, cfg.http_port);


	//PrivInfo* thiz = p->private;
	HttpUri uri = {0};
	uri.type	= HTTP_REQ_GET;
	uri.ip	= cfg.http_ip;
	uri.port	= cfg.http_port;
	uri.path	= TIMA_GETOKEN_URL;

	TIMA_LOGD("tima_token_start 2");

	tmHttpPost(&uri, thiz->post_data, p, tima_token_callback, 3, NULL, &thiz->id);

	TIMA_LOGD("tima_token_start end");
	return NULL;
}