/**
 * History:
 * ================================================================
 * 2017-07-26 qing.zou created
 *
 */
	 

#include <stdbool.h>

#include "jansson.h"

#include "cache.h"

#include "context.h"
#include "tima_http.h"
#include "tima_support.h"

#include "http_network.h"
#include "tima_get_property.h"

typedef struct _PrivInfo
{
	TimaGetPropertyReq	req;
	TimaGetPropertyRsp	rsp;

	TimaBuffer		buffer;

	int				id;
	char*			post_data;
} PrivInfo;


static void _tima_get_property_json_create(vmp_node_t* p)
{
	PrivInfo* thiz = p->private;
	TimaGetPropertyReq* req = &thiz->req;

	CacheNetworkConfig cfg;
	vmp_node_t* cache = context_get()->cache;
	cache->pfn_get(cache, CACHE_TIMA_NETWORK, &cfg, sizeof(CacheNetworkConfig));

	//char rtmpUrl[MAX_LEN] = {0};
	//sprintf(rtmpUrl, "rtmp://%s:%s/live", cfg.ss_ip, cfg.ss_rtmp_port);
	//
	//char hlsUrl[MAX_LEN] = {0};
	//sprintf(hlsUrl, "http://%s:%s/live", cfg.ss_ip, cfg.ss_http_port);

	//char flvUrl[MAX_LEN] = {0};
	//sprintf(flvUrl, "http://%s:%s/live", cfg.ss_ip, cfg.ss_http_port);

	char chNo[8] = {0};
	sprintf(chNo, "%d", req->chNo);

	json_t* json_root = NULL;
	json_root = json_object();
	
	json_object_set_new(json_root, "simNo", json_string(req->simNo));
	json_object_set_new(json_root, "number", json_string(chNo));
	//json_object_set_new(json_root, "rtmpUrl", json_string(rtmpUrl));
	//json_object_set_new(json_root, "hlsUrl", json_string(hlsUrl));
	//json_object_set_new(json_root, "flvUrl", json_string(flvUrl));
	
	char* data_dump = json_dumps(json_root, 0);

	thiz->post_data = malloc(strlen(data_dump) + 1);
	if (thiz->post_data)
		strcpy(thiz->post_data, data_dump);
	else
		TIMA_LOGE("_tima_get_property_json_create thiz->post_data==NULL.");

	free(data_dump);
	json_decref(json_root);

	TIMA_LOGD("get property [%d]: %s\n", strlen(thiz->post_data), thiz->post_data);
}

static bool _tima_get_property_json_parse(vmp_node_t* p, char* data, int len)
{
#if 0
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
		if (property)
		{
			strcpy(rsp->property, property);				
		}
		
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

static void _tima_get_property_json_free(vmp_node_t* p)
{
	PrivInfo* thiz = p->private;

	if (thiz && thiz->post_data)
	{
		//tima_buffer_clean(&thiz->buffer);
		free(thiz->post_data);
		thiz->post_data = NULL;
	}
}


static int tima_get_property_delete(vmp_node_t* p);

void tima_get_property_callback(TimaHttpRsp* rsp)
{
	TIMA_LOGI("http_token_callback %d %s %d %d %s", rsp->id, rsp->reason, rsp->size, rsp->status, rsp->data);

	bool ret = false;
	vmp_node_t* p = (vmp_node_t*)rsp->priv;
	PrivInfo* thiz = (PrivInfo*)p->private;
	if (rsp->status == 200) {
		ret = _tima_get_property_json_parse(p, rsp->data, rsp->size);
	}

	if(ret == true)
	{
		if(p->pfn_callback) 
		{
			TimaGetPropertyRsp* rsp = &thiz->rsp;
			p->pfn_callback(p, NODE_SUCCESS, rsp);
		}
	}
	else
	{
		TIMA_LOGE("video url failed!");
		if(p->pfn_callback) 
		{
			p->pfn_callback(p, NODE_FAIL, NULL);
		}
	}

	tima_get_property_delete(p);
}


static int tima_get_property_get(vmp_node_t* p, int id, void* data, int size)
{
	return 0;
}

static int tima_get_property_set(vmp_node_t* p, int id, void* data, int size)
{
	PrivInfo* thiz = p->private;
	thiz->req = *((TimaGetPropertyReq*)data);

	return 0;
}

static void* tima_get_property_start(vmp_node_t* p)
{
	TIMA_LOGD("tima_get_property_start");
	
	CacheNetworkConfig cfg;
	vmp_node_t* cache = context_get()->cache;
	cache->pfn_get(cache, CACHE_TIMA_NETWORK, &cfg, sizeof(CacheNetworkConfig));
	
	PrivInfo* thiz = p->private;
	TimaUri uri = {0};
	uri.type	= HTTP_REQ_POST;
	uri.ip		= cfg.http_ip;
	uri.port	= cfg.http_port;
	uri.path	= TIMA_GETPROPERTY_URL;

	_tima_get_property_json_create(p);
	tima_http_post(&uri, thiz->post_data, p, tima_get_property_callback, 3, &thiz->id);
	_tima_get_property_json_free(p);

	return NULL;
}

static int tima_get_property_stop(vmp_node_t* p)
{
	TIMA_LOGD("tima_get_property_stop");

	//PrivInfo* thiz = p->private;
	//tmHttpCancel(thiz->id);

	return 0;
}

static vmp_node_t* tima_get_property_create(void)
{
	vmp_node_t* p = NULL;

	TIMA_LOGD("tima_get_property_create");

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));
		thiz->post_data = "";
		p = (vmp_node_t*)malloc(sizeof(vmp_node_t));
		memset(p, 0, sizeof(vmp_node_t));
		p->nclass		= TIMA_GET_PROPERTY_CLASS;
		p->pfn_get		= (nodeget)tima_get_property_get;
		p->pfn_set		= (nodeset)tima_get_property_set;
		p->pfn_start	= (nodestart)tima_get_property_start;
		p->pfn_stop		= (nodestop)tima_get_property_stop;
		p->pfn_callback	= NULL;
		p->private		= thiz;

	} while(0);

	return p;
}

static int tima_get_property_delete(vmp_node_t* p)
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

	NODE_CLASS_REGISTER(node_get_property);
}

void tima_get_property_done(void)
{
	TIMA_LOGD("tima_get_property_done");

	NODE_CLASS_UNREGISTER(TIMA_GET_PROPERTY_CLASS);
}
