
#include "tima_support.h"

#include "context.h"
#include "cache.h"


typedef struct _PrivInfo
{
	vmp_node_t*			tima;

} PrivInfo;


static int cache_get(void* p, int id, void* data, int size)
{
	vmp_node_t* cache = (vmp_node_t*)p;
	PrivInfo* thiz = cache->private;
	
	//tima
	if(id > CACHE_TIMA_BEGIN && id < CACHE_TIMA_END)
	{
		vmp_node_t* tima = thiz->tima;
		tima->pfn_get(tima, id, data, size);
	}
	
	return 0;
}

static int cache_set(void* p, int id, const void* data, int size)
{
	vmp_node_t* cache = (vmp_node_t*)p;
	PrivInfo* thiz = cache->private;
	
	//tima
	if(id > CACHE_TIMA_BEGIN && id < CACHE_TIMA_END)
	{
		vmp_node_t* tima = thiz->tima;
		tima->pfn_set(tima, id, data, size);
	}

	return 0;
}

static int cache_start(void* p)
{
	PrivInfo* thiz = (PrivInfo*)((vmp_node_t*)p)->private;

	if (thiz->tima == NULL)
	{
		context* ctx = context_get();
		vmp_node_t* tima = node_create(CACHE_TIMA_CLASS, ctx->vector_node);
		tima->pfn_start(tima);
		thiz->tima = tima;
	}

	return 0;
}

static int cache_stop(void* p)
{
	PrivInfo* thiz = (PrivInfo*)((vmp_node_t*)p)->private;

	if (thiz->tima != NULL)
	{
		vmp_node_t* tima = thiz->tima;
		tima->pfn_stop(tima);
	}

	return 0;
}

static vmp_node_t* cache_create(void)
{
	vmp_node_t* p = NULL;
	TIMA_LOGI("cache_create");

	do 
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		
		memset(thiz, 0x00, sizeof(PrivInfo));

		p = (vmp_node_t*)malloc(sizeof(vmp_node_t));
		memset(p, 0, sizeof(vmp_node_t));
		p->nclass		= CACHE_CLASS;
		p->pfn_get		= (nodeget)cache_get;
		p->pfn_set		= (nodeset)cache_set;
		p->pfn_start	= (nodestart)cache_start;
		p->pfn_stop		= (nodestop)cache_stop;
		p->pfn_callback	= NULL;
		p->private		= thiz;

		context_get()->cache = p;

	} while (0);

	return p;
}

static int cache_delete(vmp_node_t* p)
{
	TIMA_LOGD("cache_delete");

	PrivInfo* thiz = (PrivInfo*)p->private;
	if (thiz != NULL) 
	{
		free(thiz);
		p->private = NULL;
	}

	free(p);

	return 0;
}

static const nodedef NODE_CACHE =
{
	sizeof(PrivInfo),
	CACHE_CLASS,
	(nodecreate)cache_create,
	(nodedelete)cache_delete,
};

void cache_init()
{
	TIMA_LOGD("cache_init");

	NODE_CLASS_REGISTER(NODE_CACHE);
	
	cache_tima_init();
}

void cache_done()
{
	TIMA_LOGD("cache_done");

	NODE_CLASS_UNREGISTER(CACHE_CLASS);

	cache_tima_done();
}
