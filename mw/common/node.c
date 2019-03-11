/*
 ============================================================================
 Name        : node.c
 Author      : wison.wei
 Copyright   : 2017(c) Timanetworks Company
 Description : 
 ============================================================================
 */
#include <stdbool.h>

#include "tmVector.h"
#include "context.h"
#include "node.h"

void node_init(void)
{
	context* p = Context();

	tmVectorCreate(&p->vecNodeDef, sizeof(nodedef),100);
}
void node_done(void)
{
	context* p = Context();

	tmVectorFree(&p->vecNodeDef);
}

void NodeRegisterClass(const nodedef* def)
{
	context* p = Context();

	tmVectorAddElement(p->vecNodeDef,def);

	TIMA_LOGD("NodeRegisterClass %d", tmVectorSize(p->vecNodeDef));
}

void NodeUnregisterClass(int nClass)
{
	context* p = Context();
	int i;

	for(i=0; i<tmVectorSize(p->vecNodeDef); i++)
	{
		nodedef def;
		tmVectorGetElement(p->vecNodeDef, &def, i);

		if(def.nClass == nClass)
		{
			tmVectorRemoveElement(p->vecNodeDef,i);
			break;	
		}
	}	
}

bool NodeFindClass(context* p, nodedef*def,  int nClass)
{
	int i;
	
	for(i=0; i<tmVectorSize(p->vecNodeDef); i++)
	{
		nodedef tmpDef;
		tmVectorGetElement(p->vecNodeDef, &tmpDef, i);

		if(tmpDef.nClass == nClass)
		{
			*def = tmpDef;
			return true;
		}
	}

	return false;
}

node* NodeCreate(int nClass)
{
	context* p = Context();
	nodedef def;
	bool bFound;
	
	bFound = NodeFindClass(p, &def, nClass);
	if(bFound == true)
	{
		return def.pfnCreate();
	}	

	return NULL;
}

void NodeDelete(node* p)
{
	
}


