/**
 * History:
 * ================================================================
 * 2019-03-11 qing.zou created
 *
 */
#include <stdbool.h>

#include "vmp_vector.h"
#include "vmp_node.h"

void node_init(void** node_vector)
{
	tmVectorCreate((VmpVector*)node_vector, sizeof(nodedef), 100);
}

void node_done(void** node_vector)
{
	tmVectorFree((VmpVector*)node_vector);
}

void node_register_class(const nodedef* def, void *node_vector)
{
	tmVectorAddElement((VmpVector)node_vector, def);	

	VMP_LOGD("node_register_class %d", tmVectorSize((VmpVector)node_vector));
}

void node_unregister_class(int nclass, void *node_vector)
{
	int i;

	for (i = 0; i < tmVectorSize((VmpVector)node_vector); i++)
	{
		nodedef def;
		tmVectorGetElement((VmpVector)node_vector, &def, i);

		if(def.nclass == nclass)
		{
			tmVectorRemoveElement((VmpVector)node_vector, i);
			break;	
		}
	}	
}

bool node_find_class(void *node_vector, nodedef*def, int nclass)
{
	int i;
	
	for (i = 0; i < tmVectorSize((VmpVector)node_vector); i++)
	{
		nodedef tmpdef;
		tmVectorGetElement((VmpVector)node_vector, &tmpdef, i);

		if(tmpdef.nclass == nclass)
		{
			*def = tmpdef;
			return true;
		}
	}

	return false;
}

vmp_node_t* node_create(int nclass, void *node_vector)
{
	nodedef def;
	bool bfound;
	
	bfound = node_find_class(node_vector, &def, nclass);
	if(bfound == true)
	{
		return def.pfn_create();
	}	

	return NULL;
}

void node_delete(vmp_node_t* p)
{
	
}


