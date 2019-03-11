/*
 ============================================================================
 Name        : node.h
 Author      : wison.wei
 Copyright   : 2017(c) Timanetworks Company
 Description : 
 ============================================================================
 */

#ifndef TM_NODE_H
#define TM_NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tima_log.h"

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_LEN	256

#define TYPE_BOOL	1
#define TYPE_INT	2

#define FOURCCLE(a,b,c,d)	\
	(((unsigned char) (a) << 0) | ((unsigned char) (b) << 8) | \
	((unsigned char) (c) << 16) | ((unsigned char) (d) << 24))

#define NODE_CLASS	FOURCCLE('N','O','D','E')

#define NODE_SUCCESS	0
#define NODE_FAIL		1


typedef int (*nodeget)(void* p, int id, void* data, int size);
typedef int (*nodeset)(void* p, int id, const void* data, int size);
typedef int (*nodestart)(void* p);
typedef int (*nodestop)(void* p);
typedef int (*nodecb)(void* p, int msg, void* arg);


typedef struct node
{
	int nClass;
	nodeget	pfnGet;
	nodeset pfnSet;
	nodestart pfnStart;
	nodestop pfnStop;
	nodecb pfnCb;

	void* private;
	void* parent;
} node;


typedef node* (*nodecreate)(void);
typedef int (*nodedelete)(node* p);

typedef struct nodedef
{
	int nFlags;
	int nClass;
	nodecreate pfnCreate;
	nodedelete pfnDelete;
	
} nodedef;


void node_init(void);
void node_done(void);

void NodeRegisterClass(const nodedef* def);
void NodeUnregisterClass(int nClass);

node* NodeCreate(int nClass);
void NodeDelete(node*);

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // TM_CONTEXT_H

