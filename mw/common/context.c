/*
 ============================================================================
 Name        : context.C
 Author      : wison.wei
 Copyright   : 2017(c) Timanetworks Company
 Description : 
 ============================================================================
 */
#define LOG_TAG    "context"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "context.h"
#include "tp.h"

#include "rtmp_publish.h"


static context* g_context = NULL;

context* context_get(void)
{
	return g_context;
}
void context_set(context* p)
{
	g_context = p;
}

void context_init(void)
{
	context* p = malloc(sizeof(context));
	if(!p) return;
	
	memset(p, 0, sizeof(context));
	//p->version = CONTEXT_VERSION;
	
	context_set(p);
	
	node_init((void**)&p->vector_node);
	tp_init();
	rtmp_publish_init();
}

void context_done(void)
{
	context* p = g_context;

	rtmp_publish_done();
	tp_done();
	node_done((void**)&p->vector_node);

	if(g_context != NULL)
	{
		free(g_context);
		g_context = NULL;
	}	
		
}
