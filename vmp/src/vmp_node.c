
#include <stdbool.h>

#include "context.h"
#include "vmp_vector.h"
#include "vmp_node.h"

void node_init(void)
{
	context* p = context_get();

	tmVectorCreate(&p->vector_node, sizeof(nodedef), 100);
}
void node_done(void)
{
	context* p = context_get();

	tmVectorFree(&p->vector_node);
}

void node_register_class(const nodedef* def)
{
	context* p = context_get();

	tmVectorAddElement(p->vector_node,def);

	TIMA_LOGD("node_register_class %d", tmVectorSize(p->vector_node));
}

void node_unregister_class(int nclass)
{
	context* p = context_get();
	int i;

	for(i=0; i<tmVectorSize(p->vector_node); i++)
	{
		nodedef def;
		tmVectorGetElement(p->vector_node, &def, i);

		if(def.nclass == nclass)
		{
			tmVectorRemoveElement(p->vector_node,i);
			break;	
		}
	}	
}

bool node_find_class(context* p, nodedef*def,  int nclass)
{
	int i;
	
	for(i=0; i<tmVectorSize(p->vector_node); i++)
	{
		nodedef tmpDef;
		tmVectorGetElement(p->vector_node, &tmpDef, i);

		if(tmpDef.nclass == nclass)
		{
			*def = tmpDef;
			return true;
		}
	}

	return false;
}

vmp_node_t* node_create(int nclass)
{
	context* p = context_get();
	nodedef def;
	bool bfound;
	
	bfound = node_find_class(p, &def, nclass);
	if(bfound == true)
	{
		return def.pfn_create();
	}	

	return NULL;
}

void node_delete(vmp_node_t* p)
{
	
}


