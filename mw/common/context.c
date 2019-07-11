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
#include "tima_rtmp_packager.h"

#include "http_token.h"
#include "tima_get_property.h"


static context* g_context = NULL;

context* context_get(void)
{
	return g_context;
}
void context_set(context* p)
{
	g_context = p;
}

void context_init(void *conf)
{
	context* p = malloc(sizeof(context));
	if(!p) return;
	
	memset(p, 0, sizeof(context));
	//p->version = CONTEXT_VERSION;
	p->conf = conf;
	p->packager = tima_h264_rtmp_create();

	p->porter = vmp_ports_create(10000, 20000);

	p->workqueue = vpk_workqueue_create(2);

	context_set(p);
	
	node_init((void**)&p->vector_node);
	tp_init();
	rtmp_publish_init();

	http_token_init();
	tima_get_property_init();
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
