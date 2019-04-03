/**
 * History:
 * ================================================================
 * 2019-02-28 qing.zou created
 *
 */

#include "librtmp/rtmp.h"
#include "tima_rtmp_packager.h"

typedef struct _PrivInfo
{

} PrivInfo;

static int rtmp_body_length(int length) 
{ 
	return length + 5 + RTMP_MAX_HEADER_SIZE; 
}


static void* rtmp_data_pack(void *p, const char* data, int length)
{
	RTMPPacket *packet = (RTMPPacket *)p;
	char *buf = packet + 1;
	char *body = buf + RTMP_MAX_HEADER_SIZE;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nChannel = 0x04;
    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = length - 5;
    packet->m_body = body;

    *(body++) = 0xAF;
    *(body++) = 0x01;
    memcpy(body, data + 7, length - 7);

    return packet;
}

static void* rtmp_metadata(void *p, const char* data, int length)
{
	RTMPPacket *packet = (RTMPPacket *)p;
	char *buf = packet + 1;
    char *body = buf + RTMP_MAX_HEADER_SIZE;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nChannel = 0x04;
    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = length + 2;
    packet->m_body = body;

    *(body++) = 0xAF;
    *(body++) = 0x00;
    memcpy(body, data, length);

    return packet;
}

static void		rtmp_packager_destory(TimaRTMPPackager* thiz)
{
	if (thiz)
		TIMA_FREE(thiz);
}

TimaRTMPPackager* tima_aac_rtmp_create(void)
{
	TimaRTMPPackager* thiz = TIMA_CALLOC(1, sizeof(TimaRTMPPackager)/* + sizeof(PrivInfo)*/);
	if (thiz)
	{
		thiz->body_len		= rtmp_body_length;
		thiz->meta_pack		= rtmp_metadata;
		thiz->data_pack		= rtmp_data_pack;
		thiz->destory		= rtmp_packager_destory;
	}

	return thiz;
}

