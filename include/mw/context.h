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

#include "vmp.h"

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

#define CONTEXT_VERSION		0x100

typedef struct context
{
	//int nVersion;

	VmpVector	vector_node;

	void*		tp;
	void*		service;
	void*		bll;

} context;

void context_init(void);
void context_done(void);
context* context_get(void);

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // TM_CONTEXT_H

