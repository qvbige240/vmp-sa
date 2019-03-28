/*
 ============================================================================
 Name        : context.h
 Author      : wison.wei
 Copyright   : 2017(c) Timanetworks Company
 Description : 
 ============================================================================
 */

#ifndef TM_CONTEXT_H
#define TM_CONTEXT_H

#include "tmVector.h"


#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct context
{
	tmVector vecNodeDef;
	char* conf;
	
	void* tp;
	void* tp_connect;
	void* tp_transcode;
	void* tp_push;
	
	void* cache;
	
	void* bll;

} context;

void context_init(char* pConf);
void context_done(void);
context* Context(void);

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // TM_CONTEXT_H

