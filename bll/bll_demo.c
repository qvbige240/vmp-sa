/**
 * History:
 * ================================================================
 * 2018-02-05 qing.zou created
 *
 */

//#include "context.h"
//#include "ThreadPool.h"


typedef struct _PrivInfo
{
	DemoDataReq			req;
	DemoDataRep			rep;

	int					id;
} PrivInfo;

int bll_demo_delete(vmp_node_t* p);

int task_demo_callback(void* p, int msg, void* arg)
{
	vmp_node_t* demo = ((vmp_node_t*)p)->parent;
	if ( msg != NODE_SUCCESS)
	{
		VMP_LOGW("task_demo_callback fail");
		bll_demo_delete(demo);
		return -1;
	}

	bll_demo_delete(demo);
	return 0;
}

int bll_demo_get(vmp_node_t* p, int id, void* data, int size)
{
	return 0;
}

int bll_demo_set(vmp_node_t* p, int id, void* data, int size)
{	
	PrivInfo* thiz = p->private;
	thiz->req = *((DemoDataReq*)data);
	
	return 0;
}

void* bll_demo_start(vmp_node_t* p)
{
	VMP_LOGD("bll_demo_start");

	PrivInfo* thiz = p->private;

	
	return NULL;
}

int bll_demo_stop(vmp_node_t* p)
{
	return 0;
}

vmp_node_t* bll_demo_create(void)
{
	vmp_node_t* p = NULL;

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));
		
		p = (vmp_node_t*)malloc(sizeof(vmp_node_t));
		memset(p, 0, sizeof(vmp_node_t));
		p->nclass		= BLL_DEMO_CLASS;
		p->pfn_get		= (nodeget)bll_demo_get;
		p->pfn_set		= (nodeset)bll_demo_set;
		p->pfn_start	= (nodestart)bll_demo_start;
		p->pfn_stop		= (nodestop)bll_demo_stop;
		p->pfn_callback = NULL;
		p->private		= thiz;

		return p;
	}while(0);
	
	return p;
}

int bll_demo_delete(vmp_node_t* p)
{
	//VMP_LOGD("bll_demo_delete");

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
	BLL_DEMO_CLASS,
	(nodecreate)bll_demo_create,
	(nodedelete)bll_demo_delete
};

void bll_demo_init(void)
{
	VMP_LOGI("bll_demo_init");

	NODE_CLASS_REGISTER(node_demo);
}

void bll_demo_done(void)
{
	VMP_LOGI("bll_demo_done");

	NODE_CLASS_UNREGISTER(BLL_DEMO_CLASS);
}
