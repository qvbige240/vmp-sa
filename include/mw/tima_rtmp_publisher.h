/**
 * History:
 * ================================================================
 * 2019-02-28 qing.zou created
 *
 */
#ifndef TIMA_RTMP_PUBLISHER_H
#define TIMA_RTMP_PUBLISHER_H

#include "vmp.h"
#include "librtmp/rtmp.h"

TIMA_BEGIN_DELS

struct TimaRTMPPublisher;
typedef struct TimaRTMPPublisher TimaRTMPPublisher;


struct TimaRTMPPublisher
{
	int		id;
	char	*url;

	char priv[ZERO_LEN_ARRAY];
};

TimaRTMPPublisher* tima_rtmp_create(const char *url);
int tima_rtmp_connect(TimaRTMPPublisher *publisher);
int tima_rtmp_send(TimaRTMPPublisher *publisher, RTMPPacket *packet, unsigned int timestamp);
void tima_rtmp_destory(TimaRTMPPublisher *publisher);

TIMA_END_DELS

#endif //TIMA_RTMP_PUBLISHER_H
