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

int bll_demo_delete(node* p);

int task_demo_callback(void* p, int msg, void* arg)
{
	node* demo = ((node*)p)->parent;
	if ( msg != NODE_SUCCESS)
	{
		VMP_LOGW("task_demo_callback fail");
		bll_demo_delete(demo);
		return -1;
	}

	bll_demo_delete(demo);
	return 0;
}

int bll_demo_get(node* p, int id, void* data, int size)
{
	return 0;
}

int bll_demo_set(node* p, int id, void* data, int size)
{	
	PrivInfo* thiz = p->private;
	thiz->req = *((DemoDataReq*)data);
	
	return 0;
}

void* bll_demo_start(node* p)
{
	VMP_LOGD("bll demo start");

	PrivInfo* thiz = p->private;

	
	return NULL;
}

int bll_demo_stop(node* p)
{
	return 0;
}

node* bll_demo_create(void)
{
	node* p = NULL;

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));
		
		p = (node*)malloc(sizeof(node));
		memset(p, 0, sizeof(node));
		p->nClass = BLL_DEMO_CLASS;
		p->pfnGet = (nodeget)bll_demo_get;
		p->pfnSet = (nodeset)bll_demo_set;
		p->pfnStart = (nodestart)bll_demo_start;
		p->pfnStop = (nodestop)bll_demo_stop;
		p->pfnCb = NULL;
		p->private = thiz;

		return p;
	}while(0);
	
	return p;
}

int bll_demo_delete(node* p)
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
	VMP_LOGD("bll_demo_init");

	NodeRegisterClass(&node_demo);
}

void bll_demo_done(void)
{
	VMP_LOGD("bll_demo_done");

	NodeUnregisterClass(BLL_DEMO_CLASS);
}
