/**
 * History:
 * ================================================================
 * 2019-02-28 qing.zou created
 *
 */

#include "tima_rtmp_packager.h"

typedef struct _PrivInfo
{

} PrivInfo;

static int rtmp_body_length(int length) 
{ 
	return length + 5 + 4 + RTMP_MAX_HEADER_SIZE; 
}

static RTMPPacket	rtmp_data_pack(/*TimaRTMPPackager* thiz, */char* buf, const char* data, int length)
{
	char *body = buf + RTMP_MAX_HEADER_SIZE;

	RTMPPacket packet;
	packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet.m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet.m_nChannel = 0x04;
	packet.m_hasAbsTimestamp = 0;
#ifndef USE_H264_RAW
	packet.m_nBodySize = length + 5 + 4;
#else
	packet.m_nBodySize = length + 5;
#endif
	packet.m_body = body;

	*(body++) = (data[4] & 0x1f) == 0x05 ? 0x17 : 0x27;
	*(body++) = 0x01;
	*(body++) = 0x00;
	*(body++) = 0x00;
	*(body++) = 0x00;
#ifndef USE_H264_RAW
	// NALUs
	*(body++) = length >> 24 & 0xff;
	*(body++) = length >> 16 & 0xff;
	*(body++) = length >> 8 & 0xff;
	*(body++) = length & 0xff;
#endif
	memcpy(body, data, length);

	return packet;
}

static RTMPPacket	rtmp_metadata(/*TimaRTMPPackager* thiz, */char* buf, const char* data, int length)
{
	char *body = buf + RTMP_MAX_HEADER_SIZE;

	RTMPPacket packet;
	packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	packet.m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet.m_nChannel = 0x04;	//StreamID = (ChannelID-4)/5+1
	packet.m_hasAbsTimestamp = 0;
	packet.m_nBodySize = 8 + length;
	packet.m_body = body;

	*(body++) = 0x17; // 1-keyframe, 7-AVC
	*(body++) = 0x00;
	*(body++) = 0x00;
	*(body++) = 0x00;
	*(body++) = 0x00;

	// AVCDecoderConfigurationRecord

	*(body++) = 0x01;		// configurationVersion
	*(body++) = data[5];	// AVCProfileIndication
	*(body++) = data[6];	// profile_compatibility
	*(body++) = data[7];	// AVCLevelIndication
	*(body++) = 0xff;		// 111111(reserved) + lengthSizeMinusOne

	int len = (data[2] << 8) | data[3];

	*(body++) = 0xe1;		// 111(reserved) + numOfSequenceParameterSets
	memcpy(body, data + 2, len + 2);

	body += (len + 2);
	*(body++) = 0x01;		// numOfPictureParameterSets
	memcpy(body, data + len + 6, length - len - 6);

	return packet;
}

static void		rtmp_packager_destory(TimaRTMPPackager* thiz)
{
	if (thiz)
		TIMA_FREE(thiz);
}

TimaRTMPPackager* tima_h264_rtmp_create(void)
{
	TimaRTMPPackager* thiz = TIMA_CALLOC(1, sizeof(TimaRTMPPackager)/* + sizeof(PrivInfo)*/);
	if (thiz)
	{
		//DECL_PRIV(thiz, priv);

		thiz->body_len		= rtmp_body_length;
		thiz->meta_pack		= rtmp_metadata;
		thiz->data_pack		= rtmp_data_pack;
		thiz->destory		= rtmp_packager_destory;
	}

	return thiz;
}
