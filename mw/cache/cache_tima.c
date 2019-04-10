
#include "tima_support.h"

#include "context.h"
#include "iniparser.h"
#include "dictionary.h"

#include "cache_tima.h"

typedef struct _PrivInfo
{
	CacheNetworkConfig	cfg;
} PrivInfo;


static int cache_tima_get(void* p, int id, void* data, int size)
{
	vmp_node_t* tima = (vmp_node_t*)p;
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
	vmp_node_t* tima = (vmp_node_t*)p;
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

static int cache_tima_load_conf(void* p)
{
	PrivInfo* thiz = (PrivInfo*)((vmp_node_t*)p)->private;
	char* conf = context_get()->conf;
	dictionary *ini = NULL ;
	const char  *s ;

	ini = iniparser_load(conf);
	if (ini == NULL) {
		TIMA_LOGE("---------------------cannot parse file: %s\n", conf);
		return -1 ;
	}
	iniparser_dump(ini, stderr);

	/* Get network attributes */
	s = iniparser_getstring(ini, "network:http_ip", NULL);
	if (s != NULL && strlen(s) > 0) {
		strncpy(thiz->cfg.http_ip, s, sizeof(thiz->cfg.http_ip));
		TIMA_LOGD("http_ip: [%s]", s ? s : "UNDEF");
	} else {
		strcpy(thiz->cfg.http_ip, "tg.test.timanetwork.cn");
		TIMA_LOGE("cannot parse [network:http_ip] from file: %s\n", conf);
	}

	thiz->cfg.http_port = iniparser_getint(ini, "network:http_port", 8114);


	s = iniparser_getstring(ini, "network:rtmp_ip", NULL);
	if (s != NULL && strlen(s) > 0) {
		strncpy(thiz->cfg.rtmp_ip, s, sizeof(thiz->cfg.rtmp_ip));
		TIMA_LOGD("rtmp_ip: [%s]", s ? s : "UNDEF");
	} else {
		strcpy(thiz->cfg.rtmp_ip, "127.0.0.1");
		TIMA_LOGE("cannot parse [network:rtmp_ip] from file: %s\n", conf);
	}

	thiz->cfg.rtmp_port = iniparser_getint(ini, "network:rtmp_port", 1935);

#if 0
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
#endif

	iniparser_freedict(ini);

	return 0;
}

static int cache_tima_start(void* p)
{
	PrivInfo* thiz = (PrivInfo*)((vmp_node_t*)p)->private;

	strcpy(thiz->cfg.http_ip, "tg.test.timanetwork.cn");
	thiz->cfg.http_port = 8114;

	strcpy(thiz->cfg.rtmp_ip, "127.0.0.1");
	thiz->cfg.rtmp_port = 1935;

	//strcpy(thiz->cfg.ss_ip, "127.0.0.1");
	//strcpy(thiz->cfg.ss_rtmp_port, "1935");
	//strcpy(thiz->cfg.ss_http_port, "8090");

	cache_tima_load_conf(p);

	return 0;
}

static int cache_tima_stop(void* p)
{
	return 0;
}

static vmp_node_t* cache_tima_create(void)
{
	vmp_node_t* p = NULL;
	TIMA_LOGI("cache_tima_create");

	do 
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));

		memset(thiz, 0x00, sizeof(PrivInfo));

		p = (vmp_node_t*)malloc(sizeof(vmp_node_t));
		memset(p, 0, sizeof(vmp_node_t));
		p->nclass		= CACHE_TIMA_CLASS;
		p->pfn_get		= (nodeget)cache_tima_get;
		p->pfn_set		= (nodeset)cache_tima_set;
		p->pfn_start	= (nodestart)cache_tima_start;
		p->pfn_stop		= (nodestop)cache_tima_stop;
		p->pfn_callback	= NULL;
		p->private		= thiz;

		p->parent = context_get()->cache;

	} while (0);

	return p;
}

static int cache_tima_delete(vmp_node_t* p)
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

	NODE_CLASS_REGISTER(NODE_CACHE_TIMA);
}

void cache_tima_done()
{
	TIMA_LOGD("cache_tima_done");

	NODE_CLASS_UNREGISTER(CACHE_TIMA_CLASS);
}
