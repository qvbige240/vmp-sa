/**
 * History:
 * ================================================================
 * 2019-03-11 qing.zou created
 *
 */

#ifndef BLL_H264STREAM_H
#define BLL_H264STREAM_H

#include "bll_typedef.h"
#include "tima_server.h"

#pragma pack(1)

TIMA_BEGIN_DELS

#define BLL_H264STREAM_CLASS		FOURCCLE('B','L','H','S')

#define BUFFEREVENT_LOW_WATERMARK	(2 << 10)
#define BUFFEREVENT_HIGH_WATERMARK	(2 << 20)

typedef struct
{
	unsigned long		flowid;
	vmp_launcher_t		*e;
	vmp_socket_t		client;

	void				*s;
} H264StreamReq;

typedef struct
{
	
} H264StreamRep;

void bll_h264_init(void);
void bll_h264_done(void);

#pragma pack()

TIMA_END_DELS

#endif // BLL_H264STREAM_H
