/**
 * History:
 * ================================================================
 * 2019-03-15 qing.zou created
 *
 */

#ifndef RTMP_PUBLISH_H
#define RTMP_PUBLISH_H

#include "vmp.h"
#include "tima_typedef.h"
#include "tima_support.h"

#pragma pack(1)

TIMA_BEGIN_DELS

#define RTMP_PUBLISH_CLASS		FOURCCLE('R','T','P','U')

typedef enum _RtmpPubStateType
{
	RTMP_PUB_STATE_TYPE_START = 1,
	RTMP_PUB_STATE_TYPE_TIMEOUT,
	RTMP_PUB_STATE_TYPE_EOF,
	RTMP_PUB_STATE_TYPE_ERROR,
} RtmpPubStateType;

#define stream_object()		\
	int					cid;		/* channel id */			\
	int					mtype;		/* media type video/audio */	\
	int					seq;		/* sequence */			\
	long				size;		/* package size */			\
	unsigned long long	sim;		/* sim number */		\
	unsigned char*		package;

typedef int (*pub_callback)(void* ctx, void* data, void* result);

typedef int (*list_traverse_callback)(vmp_node_t* p, pub_callback proc, void* ctx);

typedef struct
{
	char					channel;
	unsigned long long		sim;

	char					uri[MAX_PATH_SIZE+1];

	nodecb					pfncb;
	list_traverse_callback	traverse;

} RtmpPublishReq;

typedef struct
{
	
} RtmpPublishRep;

void rtmp_publish_init(void);
void rtmp_publish_done(void);


void* rtmp_meta_pack(void* packager, const char* data, int length);
void* rtmp_data_pack(void* packager, const char* data, int length);

#pragma pack()

TIMA_END_DELS

#endif // RTMP_PUBLISH_H
