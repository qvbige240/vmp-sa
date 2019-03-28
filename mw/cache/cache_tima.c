

#include "context.h"
#include "cache_tima.h"
#include "node.h"
#include "dictionary.h"

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

static int _cache_tima_load_conf(void* p)
{
	PrivInfo* thiz = (PrivInfo*)((node*)p)->private;
	char* conf = Context()->conf;
	dictionary*   ini = NULL ;

    	const char  *   s ;

    	ini = iniparser_load(conf);
    	if (ini==NULL) {
        		TIMA_LOGE("---------------------cannot parse file: %s", conf);
        		return -1 ;
    	}
    	iniparser_dump(ini, NULL);

    	/* Get network attributes */
    	s = iniparser_getstring(ini, "network:http_ip", NULL);
	if(s != NULL && strlen(s) >0)
	{
		strcpy(thiz->cfg.http_ip, s);
		TIMA_LOGD("http_ip: [%s]", s ? s : "UNDEF");
	}
	else
	{
		TIMA_LOGE("http_ip: [%s]", s ? s : "UNDEF");
	}
	
	s = iniparser_getstring(ini, "network:http_port", NULL);
	if(s != NULL && strlen(s) >0)
	{
		strcpy(thiz->cfg.http_port, s);
		TIMA_LOGD("http_port: [%s]", s ? s : "UNDEF");
	}
	else
	{
		TIMA_LOGE("http_port: [%s]", s ? s : "UNDEF");
	}
	
	s = iniparser_getstring(ini, "network:ss_ip", NULL);
	if(s != NULL && strlen(s) >0)
	{
		strcpy(thiz->cfg.ss_ip, s);
		TIMA_LOGD("ss_ip: [%s]", s ? s : "UNDEF");
	}
	else
	{
		TIMA_LOGE("ss_ip: [%s]", s ? s : "UNDEF");
	}

	s = iniparser_getstring(ini, "network:ss_rtmp_port", NULL);
	if(s != NULL && strlen(s) >0)
	{
		strcpy(thiz->cfg.ss_rtmp_port, s);
		TIMA_LOGD("ss_rtmp_port: [%s]", s ? s : "UNDEF");
	}
	else
	{
		TIMA_LOGE("ss_rtmp_port: [%s]", s ? s : "UNDEF");
	}

	s = iniparser_getstring(ini, "network:ss_http_port", NULL);
	if(s != NULL && strlen(s) >0)
	{
		strcpy(thiz->cfg.ss_http_port, s);
		TIMA_LOGD("ss_http_port: [%s]", s ? s : "UNDEF");
	}
	else
	{
		TIMA_LOGE("ss_http_port: [%s]", s ? s : "UNDEF");
	}

    	iniparser_freedict(ini);
}

static int cache_tima_start(void* p)
{
	PrivInfo* thiz = (PrivInfo*)((node*)p)->private;
	
	strcpy(thiz->cfg.http_ip, "tg.test.timanetwork.cn");
	strcpy(thiz->cfg.http_port, "8114");

	strcpy(thiz->cfg.ss_ip, "127.0.0.1");
	strcpy(thiz->cfg.ss_rtmp_port, "1935");
	strcpy(thiz->cfg.ss_http_port, "8090");

	_cache_tima_load_conf(p);
	
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
