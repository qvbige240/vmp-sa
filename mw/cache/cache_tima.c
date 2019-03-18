

#include "context.h"
#include "cache_tima.h"
#include "node.h"

typedef struct _PrivInfo
{
	CacheNetworkConfig	cfg;
} PrivInfo;


static int cache_tima_get(void* p, int id, void* data, int size)
{
	node* tima = (node*)p;
	PrivInfo* thiz = tima->private;

	switch (id)
	{
	case CACHE_TIMA_NETWORK:
		*((CacheNetworkConfig*)data) = thiz->cfg;
		break;
	default:
		return -1;
	}
	
	return 0;
}

static int cache_tima_set(void* p, int id, const void* data, int size)
{
	node* tima = (node*)p;
	PrivInfo* thiz = tima->private;

	switch (id)
	{
	case CACHE_TIMA_NETWORK:
		thiz->cfg = *((CacheNetworkConfig*)data);
		break;
	
	default:
		return -1;
	}

	return 0;
}

static int cache_tima_start(void* p)
{
	PrivInfo* thiz = (PrivInfo*)((node*)p)->private;
	
	strcpy(thiz->cfg.socket_ip, "115.182.105.71");
	thiz->cfg.socket_port = 8888;
	strcpy(thiz->cfg.http_ip, "tg.test.timanetwork.cn");
	strcpy(thiz->cfg.http_port, "8114");
	strcpy(thiz->cfg.bda_ip, "bdatest.91carnet.com");
	strcpy(thiz->cfg.bda_port, "80");

	return 0;
}

static int cache_tima_stop(void* p)
{
	return 0;
}

static node* cache_tima_create(void)
{
	node* p = NULL;
	TIMA_LOGI("cache_tima_create");

	do 
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));

		memset(thiz, 0x00, sizeof(PrivInfo));

		p = (node*)malloc(sizeof(node));
		memset(p, 0, sizeof(node));
		p->nClass	= CACHE_TIMA_CLASS;
		p->pfnGet	= (nodeget)cache_tima_get;
		p->pfnSet	= (nodeset)cache_tima_set;
		p->pfnStart	= (nodestart)cache_tima_start;
		p->pfnStop	= (nodestop)cache_tima_stop;
		p->pfnCb	= NULL;
		p->private	= thiz;

		p->parent = Context()->cache;

	} while (0);

	return p;
}

static int cache_tima_delete(node* p)
{
	TIMA_LOGD("cache_tima_delete");

	PrivInfo* thiz = (PrivInfo*)p->private;
	if (thiz != NULL) 
	{
		free(thiz);
		p->private = NULL;
	}

	free(p);

	return 0;
}

static const nodedef NODE_CACHE_TIMA =
{
	sizeof(PrivInfo),
	CACHE_TIMA_CLASS,
	(nodecreate)cache_tima_create,
	(nodedelete)cache_tima_delete,
};

void cache_tima_init()
{
	TIMA_LOGD("cache_tima_init");
	NodeRegisterClass(&NODE_CACHE_TIMA);
}

void cache_tima_done()
{
	TIMA_LOGD("cache_tima_done");
	NodeUnregisterClass(CACHE_TIMA_CLASS);
}
