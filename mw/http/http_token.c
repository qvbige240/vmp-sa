/**
 * History:
 * ================================================================
 * 2019-03-29 qing.zou created
 *
 */

#include "context.h"
#include "tima_http.h"
#include "tima_support.h"

#include "http_token.h"


typedef struct _PrivInfo
{
	HttpTokenReq		req;
	HttpTokenRep		rep;

	TimaBuffer			buffer;

	int					id;
	char*				post_data;
} PrivInfo;

static int http_token_delete(vmp_node_t* p);

static void token_json_create(vmp_node_t* p)
{
	char *param = NULL;
	PrivInfo* thiz = p->private;
	HttpTokenReq* req = &thiz->req;

	unsigned int data_len = 0, len = 0;

	data_len += strlen("?deviceType=");
	data_len += strlen(thiz->req.devtype);
	data_len += strlen("&deviceSeriesNo=");
	data_len += strlen(thiz->req.seriesno);

	TIMA_LOGD("data_len: %d\n", data_len);

	tima_buffer_init(&thiz->buffer, data_len << 1);
	size_t total = tima_buffer_size(&thiz->buffer);
	TIMA_LOGD("data_len: %d, total: %d\n", data_len, total);

	param = "?deviceType=";
	tima_buffer_strdup(&thiz->buffer, param, strlen(param), 1);
	tima_buffer_strdup(&thiz->buffer, req->devtype, strlen(req->devtype), 1);

	param = "&deviceSeriesNo=";
	tima_buffer_strdup(&thiz->buffer, param, strlen(param), 1);
	tima_buffer_strdup(&thiz->buffer, req->seriesno, strlen(req->seriesno), 1);

	thiz->post_data = tima_buffer_data(&thiz->buffer, 0);
	len = tima_buffer_used(&thiz->buffer);
	total = tima_buffer_size(&thiz->buffer);

	TIMA_LOGD("get token param(len=%d/%d): %s\n", len, total, thiz->post_data);
}

static void token_json_free(vmp_node_t* p)
{
	PrivInfo* thiz = p->private;

	if (thiz && thiz->post_data)
	{
		tima_buffer_clean(&thiz->buffer);
		//free(thiz->post_data);
		thiz->post_data = NULL;
	}
}

//static int task_demo_callback(void* p, int msg, void* arg)
//{
//	vmp_node_t* demo = ((vmp_node_t*)p)->parent;
//	if ( msg != NODE_SUCCESS)
//	{
//		VMP_LOGW("task_demo_callback fail");
//		http_token_delete(demo);
//		return -1;
//	}
//
//	http_token_delete(demo);
//	return 0;
//}

static int http_token_get(vmp_node_t* p, int id, void* data, int size)
{
	return 0;
}

static int http_token_set(vmp_node_t* p, int id, void* data, int size)
{	
	PrivInfo* thiz = p->private;
	thiz->req = *((HttpTokenReq*)data);
	
	return 0;
}

#define TIMA_GETOKEN_URL			"/carNet/sc/tg/token"
static void* http_token_start(vmp_node_t* p)
{
	VMP_LOGD("http_token_start");

	PrivInfo* thiz = p->private;
	TimaUri tima_uri = {0};
	tima_uri.type	= HTTP_REQ_GET;
	tima_uri.ip		= "tg.test.timanetwork.cn";	//HTTP_IP;
	tima_uri.port	= 8114;		//PORT_TG;
	tima_uri.path	= TIMA_GETOKEN_URL;

	token_json_create(p);
	//tmHttpPost(&tima_uri, thiz->post_data, p, tima_token_callback, 3, NULL, &thiz->id);
	tima_http_post(&tima_uri, thiz->post_data, p, NULL, 3, &thiz->id);
	token_json_free(p);

	return NULL;
}

static int http_token_stop(vmp_node_t* p)
{
	return 0;
}

static vmp_node_t* http_token_create(void)
{
	vmp_node_t* p = NULL;

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));
		
		p = (vmp_node_t*)malloc(sizeof(vmp_node_t));
		memset(p, 0, sizeof(vmp_node_t));
		p->nclass		= HTTP_TOKEN_CLASS;
		p->pfn_get		= (nodeget)http_token_get;
		p->pfn_set		= (nodeset)http_token_set;
		p->pfn_start	= (nodestart)http_token_start;
		p->pfn_stop		= (nodestop)http_token_stop;
		p->pfn_callback = NULL;
		p->private		= thiz;

		return p;
	}while(0);
	
	return p;
}

static int http_token_delete(vmp_node_t* p)
{
	//VMP_LOGD("http_token_delete");

	PrivInfo* thiz = (PrivInfo*)p->private;
	if(thiz != NULL)
	{
		free(thiz);
		p->private = NULL;
	}

	free(p);
	
	return 0;
}

static const nodedef node_demo = 
{	
	sizeof(PrivInfo),
	HTTP_TOKEN_CLASS,
	(nodecreate)http_token_create,
	(nodedelete)http_token_delete
};

void http_token_init(void)
{
	VMP_LOGI("http_token_init");

	NODE_CLASS_REGISTER(node_demo);
}

void http_token_done(void)
{
	VMP_LOGI("http_token_done");

	NODE_CLASS_UNREGISTER(HTTP_TOKEN_CLASS);
}
