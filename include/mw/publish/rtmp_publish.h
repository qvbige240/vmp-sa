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


typedef struct
{

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
