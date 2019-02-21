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


static context* g_context = NULL;

void Context_SetContext(context* p)
{
	g_context = p;
}

context* Context(void)
{
	return g_context;
}

void context_init(void)
{
	context* p = malloc(sizeof(context));
	if(!p) return;
	
	memset(p, 0, sizeof(context));
	p->nVersion = CONTEXT_VERSION;
	
	Context_SetContext(p);
	
	tp_init();
}

void context_done(void)
{
	tp_done();

	if(g_context != NULL)
	{
		free(g_context);
		g_context = NULL;
	}	
		
}

