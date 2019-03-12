
#ifndef VMP_NODE_H
#define VMP_NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vmp_log.h"

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_LEN		256

#define TYPE_BOOL	1
#define TYPE_INT	2

#define FOURCCLE(a,b,c,d)	\
	(((unsigned char) (a) << 0) | ((unsigned char) (b) << 8) | \
	((unsigned char) (c) << 16) | ((unsigned char) (d) << 24))

#define NODE_CLASS		FOURCCLE('N','O','D','E')

#define NODE_SUCCESS	0
#define NODE_FAIL		1


typedef int (*nodeget)(void* p, int id, void* data, int size);
typedef int (*nodeset)(void* p, int id, const void* data, int size);
typedef int (*nodestart)(void* p);
typedef int (*nodestop)(void* p);
typedef int (*nodecb)(void* p, int msg, void* arg);


typedef struct vmp_node_s
{
	int			nclass;
	nodeget		pfn_get;
	nodeset		pfn_set;
	nodestart	pfn_start;
	nodestop	pfn_stop;
	nodecb		pfn_callback;

	void*		private;
	void*		parent;
} vmp_node_t;


typedef vmp_node_t* (*nodecreate)(void);
typedef int (*nodedelete)(vmp_node_t* p);

typedef struct nodedef
{
	int			nflags;
	int			nclass;
	nodecreate	pfn_create;
	nodedelete	pfn_delete;
	
} nodedef;


void node_init(void);
void node_done(void);

void node_register_class(const nodedef* def);
void node_unregister_class(int nclass);

vmp_node_t* node_create(int nclass);
void node_delete(vmp_node_t*);

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // VMP_NODE_H
