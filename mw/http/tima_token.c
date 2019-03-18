/**
 * History:
 * ================================================================
 * 2017-07-26 qing.zou created
 *
 */
	 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "node.h"
#include "http_define.h"
#include "tmHttp.h"
#include "context.h"
#include "jansson.h"
#include "cache.h"

#include "tima_token.h"

typedef struct _PrivInfo
{
	TimaTokenReq	req;
	TimaTokenRes	rs;

	int				id;
	char*			post_data;
} PrivInfo;

static int tima_token_delete(node* p);

void tima_token_callback(HttpRsp* rsp)
{
	TIMA_LOGI("tima_token_callback %d %s %d %d %s", rsp->id, rsp->reason, rsp->size, rsp->status, rsp->data);

	node* p = (node*)rsp->pPriv;
#if 0
	PrivInfo* thiz = (PrivInfo*)p->private;

	bool ret = token_json_parse(p, rsp->data, rsp->size);
	if(ret == true)
	{
		if(p->pfnCb) 
		{
			TimaTokenRes* rs = &thiz->rs;
			p->pfnCb(p, NODE_SUCCESS, rs);
		}
	}
	else
	{
		TIMA_LOGE("get_token failed!");
		if(p->pfnCb) 
		{
			p->pfnCb(p, NODE_FAIL, NULL);
		}
	}
#endif
	tima_token_delete(p);
}

static int tima_token_get(node* p, int id, void* data, int size)
{
	return 0;
}

static int tima_token_set(node* p, int id, void* data, int size)
{
	PrivInfo* thiz = p->private;
	thiz->req = *((TimaTokenReq*)data);

	return 0;
}

static void* tima_token_start(node* p)
{
	TIMA_LOGD("tima_token_start");
	
	CacheNetworkConfig cfg;
	node* cache = Context()->cache;
	cache->pfnGet(cache, CACHE_TIMA_NETWORK, &cfg, sizeof(CacheNetworkConfig));

	TIMA_LOGD("tima_token_start 1");
	TIMA_LOGD("tima_token_start %s %s", cfg.http_ip, cfg.http_port);

	
	PrivInfo* thiz = p->private;
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

static int tima_token_stop(node* p)
{
	TIMA_LOGD("tima_token_stop");

	PrivInfo* thiz = p->private;
	tmHttpCancel(thiz->id);

	return 0;
}

static node* tima_token_create(void)
{
	node* p = NULL;

	TIMA_LOGD("tima_token_create");

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));
		thiz->post_data = "";
		p = (node*)malloc(sizeof(node));
		memset(p, 0, sizeof(node));
		p->nClass	= TIMA_TOKEN_CLASS;
		p->pfnGet	= (nodeget)tima_token_get;
		p->pfnSet	= (nodeset)tima_token_set;
		p->pfnStart = (nodestart)tima_token_start;
		p->pfnStop	= (nodestop)tima_token_stop;
		p->pfnCb	= NULL;
		p->private	= thiz;

	} while(0);

	TIMA_LOGD("tima_token_create end");
	return p;
}

static int tima_token_delete(node* p)
{
	//TIMA_LOGD("tima_token_delete");

	PrivInfo* thiz = (PrivInfo*)p->private;
	if (thiz != NULL)
	{
		free(thiz);
		p->private = NULL;
	}

	free(p);

	return 0;
}

static const nodedef node_get_token = 
{
	sizeof(PrivInfo),
	TIMA_TOKEN_CLASS,
	(nodecreate)tima_token_create,
	(nodedelete)tima_token_delete,
};


void tima_token_init(void)
{
	TIMA_LOGD("tima_token_init");

	NodeRegisterClass(&node_get_token);
}

void tima_token_done(void)
{
	TIMA_LOGD("tima_token_done");

	NodeUnregisterClass(TIMA_TOKEN_CLASS);
}
