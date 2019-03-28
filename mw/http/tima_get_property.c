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

#include "tima_get_property.h"
#include "tima_buffer.h"

typedef struct _PrivInfo
{
	TimaGetPropertyReq	req;
	TimaGetPropertyRsp	rsp;

	TimaBuffer		buffer;

	int				id;
	char*			post_data;
} PrivInfo;


static void _tima_get_property_json_create(node* p)
{
	char *param = NULL;
	PrivInfo* thiz = p->private;
	TimaGetPropertyReq* req = &thiz->req;

	unsigned int data_len = 0, len = 0;
	char chNo[8] = {0};
	sprintf(chNo, "%d", req->chNo);

	data_len += strlen("?simNo=");
	data_len += strlen(req->simNo);
	data_len += strlen("&number=");
	data_len += strlen(chNo);
	data_len += strlen("&url=");
	data_len += strlen(req->url);

	tima_buffer_init(&thiz->buffer, data_len << 1);
	size_t total = tima_buffer_size(&thiz->buffer);

	param = "?simNo=";
	tima_buffer_strdup(&thiz->buffer, param, strlen(param), 1);
	tima_buffer_strdup(&thiz->buffer, req->simNo, strlen(req->simNo), 1);

	param = "&number=";
	tima_buffer_strdup(&thiz->buffer, param, strlen(param), 1);
	tima_buffer_strdup(&thiz->buffer, chNo, strlen(chNo), 1);

	param = "&url=";
	tima_buffer_strdup(&thiz->buffer, param, strlen(param), 1);
	tima_buffer_strdup(&thiz->buffer, req->url, strlen(req->url), 1);

	thiz->post_data = tima_buffer_data(&thiz->buffer, 0);
	len = tima_buffer_used(&thiz->buffer);
	total = tima_buffer_size(&thiz->buffer);

	TIMA_LOGD("get property param(len=%d/%d): %s\n", len, total, thiz->post_data);
}

static bool _tima_get_property_json_parse(node* p, char* data, int len)
{
#if 1
	data = "{  \
    		\"playAddress\": \"playAddress/1/2\",  \
    		\"property\": \"12343423432432\", \
    		\"returnErrCode\": \"ERR_CODE\", \
    		\"returnErrMsg\": \"ERR_MSG\", \
    		\"returnSuccess\":false}";
    	len = strlen(data);
#endif

	PrivInfo* thiz = (PrivInfo*)p->private;
	TimaGetPropertyRsp* rsp = &thiz->rsp;
	char* err_msg = NULL;
	char* err_code = NULL;
	json_t* json_root = json_loads(data, 0, NULL);
	if (json_root)
	{
		json_t *jobject;
		jobject = json_object_get(json_root, "playAddress");
		if (!jobject) goto json_parse_end;
		char* playAddress = (char*)json_string_value(jobject);
		if (!playAddress || strlen(playAddress) < 2)
		{
			goto json_parse_end;
		}
		strcpy(rsp->url, playAddress);

		jobject = json_object_get(json_root, "property");
		if (!jobject) goto json_parse_end;
		char* property = (char*)json_string_value(jobject);
		if (!property || strlen(property) < 2)
		{
			goto json_parse_end;
		}
		strcpy(rsp->property, property);

		json_decref(json_root);
		return true;

json_parse_end:
		TIMA_LOGE("\n ======= videoUrlAndProperty error!!!! =======\n");
		jobject = json_object_get(json_root, "returnErrCode");
		if (jobject) {
			err_code = (char*)json_string_value(jobject);
			if (err_code) {
				TIMA_LOGE("ERROR: return error at %s", err_code);
			}
		}
		jobject = json_object_get(json_root, "returnErrMsg");
		if (jobject) {
			err_msg = (char*)json_string_value(jobject);
			if (err_msg) {
				TIMA_LOGE("ERROR: return error msg %s", err_msg);
			}
		}
		json_decref(json_root);
	}

	return false;
}

static void _tima_get_property_json_free(node* p)
{
	PrivInfo* thiz = p->private;

	if (thiz && thiz->post_data)
	{
		tima_buffer_clean(&thiz->buffer);
		thiz->post_data = NULL;
	}
}


static int tima_get_property_delete(node* p);

void tima_get_property_callback(HttpRsp* rsp)
{
	TIMA_LOGI("tima_get_property_callback %d %s %d %d %s", rsp->id, rsp->reason, rsp->size, rsp->status, rsp->data);

	node* p = (node*)rsp->pPriv;
	PrivInfo* thiz = (PrivInfo*)p->private;

	bool ret = _tima_get_property_json_parse(p, rsp->data, rsp->size);
	if(ret == true)
	{
		if(p->pfnCb) 
		{
			TimaGetPropertyRsp* rsp = &thiz->rsp;
			p->pfnCb(p, NODE_SUCCESS, rsp);
		}
	}
	else
	{
		TIMA_LOGE("videoUrlAndProperty failed!");
		if(p->pfnCb) 
		{
			p->pfnCb(p, NODE_FAIL, NULL);
		}
	}

	tima_get_property_delete(p);
}

static int tima_get_property_get(node* p, int id, void* data, int size)
{
	return 0;
}

static int tima_get_property_set(node* p, int id, void* data, int size)
{
	PrivInfo* thiz = p->private;
	thiz->req = *((TimaGetPropertyReq*)data);

	return 0;
}

static void* tima_get_property_start(node* p)
{
	TIMA_LOGD("tima_get_property_start");
	
	CacheNetworkConfig cfg;
	node* cache = Context()->cache;
	cache->pfnGet(cache, CACHE_TIMA_NETWORK, &cfg, sizeof(CacheNetworkConfig));
	
	PrivInfo* thiz = p->private;
	HttpUri uri = {0};
	uri.type	= HTTP_REQ_GET;
	uri.ip	= cfg.http_ip;
	uri.port	= cfg.http_port;
	uri.path	= TIMA_GETPROPERTY_URL;

	_tima_get_property_json_create(p);
	tmHttpPost(&uri, thiz->post_data, p, tima_get_property_callback, 3, NULL, &thiz->id);
	_tima_get_property_json_free(p);

	return NULL;
}

static int tima_get_property_stop(node* p)
{
	TIMA_LOGD("tima_get_property_stop");

	PrivInfo* thiz = p->private;
	tmHttpCancel(thiz->id);

	return 0;
}

static node* tima_get_property_create(void)
{
	node* p = NULL;

	TIMA_LOGD("tima_get_property_create");

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));
		thiz->post_data = "";
		p = (node*)malloc(sizeof(node));
		memset(p, 0, sizeof(node));
		p->nClass	= TIMA_GET_PROPERTY_CLASS;
		p->pfnGet	= (nodeget)tima_get_property_get;
		p->pfnSet	= (nodeset)tima_get_property_set;
		p->pfnStart = (nodestart)tima_get_property_start;
		p->pfnStop	= (nodestop)tima_get_property_stop;
		p->pfnCb	= NULL;
		p->private	= thiz;

	} while(0);

	TIMA_LOGD("tima_token_create end");
	return p;
}

static int tima_get_property_delete(node* p)
{
	//TIMA_LOGD("tima_get_property_delete");

	PrivInfo* thiz = (PrivInfo*)p->private;
	if (thiz != NULL)
	{
		free(thiz);
		p->private = NULL;
	}

	free(p);

	return 0;
}

static const nodedef node_get_property = 
{
	sizeof(PrivInfo),
	TIMA_GET_PROPERTY_CLASS,
	(nodecreate)tima_get_property_create,
	(nodedelete)tima_get_property_delete,
};


void tima_get_property_init(void)
{
	TIMA_LOGD("tima_get_property_init");

	NodeRegisterClass(&node_get_property);
}

void tima_get_property_done(void)
{
	TIMA_LOGD("tima_get_property_done");

	NodeUnregisterClass(TIMA_GET_PROPERTY_CLASS);
}
