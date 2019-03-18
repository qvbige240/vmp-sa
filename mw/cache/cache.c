#include "context.h"
#include "node.h"
#include "cache.h"


typedef struct _PrivInfo
{
	node*			tima;

} PrivInfo;


static int cache_get(void* p, int id, void* data, int size)
{
	node* cache = (node*)p;
	PrivInfo* thiz = cache->private;
	
	//tima
	if(id > CACHE_TIMA_BEGIN && id < CACHE_TIMA_END)
	{
		node* tima = thiz->tima;
		tima->pfnGet(tima, id, data, size);
	}
	
	return 0;
}

static int cache_set(void* p, int id, const void* data, int size)
{
	node* cache = (node*)p;
	PrivInfo* thiz = cache->private;
	
	//tima
	if(id > CACHE_TIMA_BEGIN && id < CACHE_TIMA_END)
	{
		node* tima = thiz->tima;
		tima->pfnSet(tima, id, data, size);
	}

	return 0;
}

static int cache_start(void* p)
{
	PrivInfo* thiz = (PrivInfo*)((node*)p)->private;

	if (thiz->tima == NULL)
	{
		node* tima = NodeCreate(CACHE_TIMA_CLASS);
		tima->pfnStart(tima);
		thiz->tima = tima;
	}

	return 0;
}

static int cache_stop(void* p)
{
	PrivInfo* thiz = (PrivInfo*)((node*)p)->private;

	if (thiz->tima != NULL)
	{
		node* tima = thiz->tima;
		tima->pfnStop(tima);
	}

	return 0;
}

static node* cache_create(void)
{
	node* p = NULL;
	TIMA_LOGI("cache_create");

	do 
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		
		memset(thiz, 0x00, sizeof(PrivInfo));

		p = (node*)malloc(sizeof(node));
		memset(p, 0, sizeof(node));
		p->nClass	= CACHE_CLASS;
		p->pfnGet	= (nodeget)cache_get;
		p->pfnSet	= (nodeset)cache_set;
		p->pfnStart	= (nodestart)cache_start;
		p->pfnStop	= (nodestop)cache_stop;
		p->pfnCb	= NULL;
		p->private	= thiz;

		Context()->cache = p;

	} while (0);

	return p;
}

static int cache_delete(node* p)
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
	NodeRegisterClass(&NODE_CACHE);
	
	cache_tima_init();
}

void cache_done()
{
	TIMA_LOGD("cache_done");
	NodeUnregisterClass(CACHE_CLASS);

	cache_tima_done();
}
