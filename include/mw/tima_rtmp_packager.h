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

typedef int		(*TimaRTMPBodyLength)(int length);
//typedef RTMPPacket	(*TimaRTMPDataPack)(/*TimaRTMPPackager* thiz, */char* buf, const char* data, int length);
//typedef RTMPPacket	(*TimaRTMPMetaPack)(/*TimaRTMPPackager* thiz, */char* buf, const char* data, int length);
typedef void*	(*TimaRTMPDataPack)(void *p, const char* data, int length);
typedef void*	(*TimaRTMPMetaPack)(void *p, const char* data, int length);
typedef void	(*TimaRTMPPackagerDestory)(TimaRTMPPackager* thiz);

struct TimaRTMPPackager
{
	TimaRTMPBodyLength			body_len;
	TimaRTMPMetaPack			meta_pack;
	TimaRTMPDataPack			data_pack;
	TimaRTMPPackagerDestory		destory;

	char priv[ZERO_LEN_ARRAY];
};

TimaRTMPPackager* tima_h264_rtmp_create(void);

TIMA_END_DELS

#endif //TIMA_RTMP_PACKAGER_H
