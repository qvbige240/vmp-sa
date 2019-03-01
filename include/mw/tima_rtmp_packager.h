/**
 * History:
 * ================================================================
 * 2019-02-28 qing.zou created
 *
 */
#ifndef TIMA_RTMP_PACKAGER_H
#define TIMA_RTMP_PACKAGER_H

#include "vmp.h"
#include "librtmp/rtmp.h"

TIMA_BEGIN_DELS

struct TimaRTMPPackager;
typedef struct TimaRTMPPackager TimaRTMPPackager;

typedef RTMPPacket	(*TimaRTMPDataPack)(TimaRTMPPackager* thiz, char* buf, const char* data, int length);
typedef RTMPPacket	(*TimaRTMPMetaPack)(TimaRTMPPackager* thiz, char* buf, const char* data, int length);
typedef void		(*TimaRTMPPackagerDestory)(TimaRTMPPackager* thiz);

struct TimaRTMPPackager
{
	TimaRTMPDataPack			data_pack;
	TimaRTMPMetaPack			meta_pack;
	TimaRTMPPackagerDestory		destory;

	char priv[ZERO_LEN_ARRAY];
};

TimaRTMPPackager* tima_h264_rtmp_create(void);

TIMA_END_DELS

#endif //TIMA_RTMP_PACKAGER_H
