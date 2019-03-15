/**
 * History:
 * ================================================================
 * 2019-03-15 qing.zou created
 *
 */

#include "context.h"
#include "ThreadPool.h"

#include "rtmp_publish.h"

#include "tima_rtmp_packager.h"
#include "tima_rtmp_publisher.h"



typedef struct _PrivInfo
{
	RtmpPublishReq		req;
	RtmpPublishRep		rep;

	int					id;
	int					cond;

} PrivInfo;

int rtmp_publish_delete(vmp_node_t* p);



int rtmp_publish_callback(void* p, int msg, void* arg)
{
	vmp_node_t* demo = ((vmp_node_t*)p)->parent;
	if ( msg != NODE_SUCCESS)
	{
		VMP_LOGW("rtmp_publish_callback fail");
		rtmp_publish_delete(demo);
		return -1;
	}

	rtmp_publish_delete(demo);
	return 0;
}


static void *rtmp_publish_thread(void* arg)
{
	TIMA_LOGD("rtmp_publish_thread");

	vmp_node_t* p = (vmp_node_t*)arg;
	PrivInfo* thiz = p->private;

	while (1) {
		thiz->cond = 1;


		while (thiz->cond) {

			sleep(1);
			//thiz->cond = 0;
		}


		if (thiz->cond == 0)
			break;
	}


	rtmp_publish_delete(p);

	return NULL;
}

int rtmp_publish_get(vmp_node_t* p, int id, void* data, int size)
{
	return 0;
}

int rtmp_publish_set(vmp_node_t* p, int id, void* data, int size)
{	
	PrivInfo* thiz = p->private;
	thiz->req = *((RtmpPublishReq*)data);
	
	return 0;
}

void* rtmp_publish_start(vmp_node_t* p)
{
	VMP_LOGD("rtmp_publish_start");

	PrivInfo* thiz = p->private;

	context * ctx = context_get();
	ThreadPoolJob job;

	TPJobInit( &job, ( start_routine) rtmp_publish_thread, p);
	TPJobSetFreeFunction( &job, ( free_routine ) NULL );
	TPJobSetStopFunction( &job, ( stop_routine ) NULL );
	TPJobSetPriority( &job, HIGH_PRIORITY );
	ThreadPoolAddPersistent( ctx->tp, &job, &job.jobId);
	thiz->id = job.jobId;
	
	return NULL;
}

int rtmp_publish_stop(vmp_node_t* p)
{
	return 0;
}

vmp_node_t* rtmp_publish_create(void)
{
	vmp_node_t* p = NULL;

	do
	{
		PrivInfo* thiz = (PrivInfo*)malloc(sizeof(PrivInfo));
		memset(thiz, 0, sizeof(PrivInfo));
		
		p = (vmp_node_t*)malloc(sizeof(vmp_node_t));
		memset(p, 0, sizeof(vmp_node_t));
		p->nclass		= RTMP_PUBLISH_CLASS;
		p->pfn_get		= (nodeget)rtmp_publish_get;
		p->pfn_set		= (nodeset)rtmp_publish_set;
		p->pfn_start	= (nodestart)rtmp_publish_start;
		p->pfn_stop		= (nodestop)rtmp_publish_stop;
		p->pfn_callback = NULL;
		p->private		= thiz;

		return p;
	} while(0);
	
	return p;
}

int rtmp_publish_delete(vmp_node_t* p)
{
	VMP_LOGD("rtmp_publish_delete");

	PrivInfo* thiz = (PrivInfo*)p->private;
	if(thiz != NULL)
	{
		free(thiz);
		p->private = NULL;
	}

	free(p);
	
	return 0;
}

static const nodedef node_rtmp_publish = 
{	
	sizeof(PrivInfo),
	RTMP_PUBLISH_CLASS,
	(nodecreate)rtmp_publish_create,
	(nodedelete)rtmp_publish_delete
};

void rtmp_publish_init(void)
{
	VMP_LOGI("rtmp_publish_init");
	
	NODE_CLASS_REGISTER(node_rtmp_publish);
}

void rtmp_publish_done(void)
{
	VMP_LOGI("rtmp_publish_done");

	NODE_CLASS_UNREGISTER(RTMP_PUBLISH_CLASS);
}

#if 1

void* rtmp_meta_pack(void* packager, const char* data, int length)
{
	return NULL;
}
void* rtmp_data_pack(void* packager, const char* data, int length)
{
	return NULL;
}
#else
void* rtmp_data_pack(void* packager, const char* data, int length)
{
	TimaRTMPPackager* p = packager;
	//TimaRTMPPackager* packager;	//...
	int size = sizeof(RTMPPacket) + p->body_len(length);
	RTMPPacket *packet = calloc(1, size);
	if (!packet) {
		TIMA_LOGE("malloc failed");
		return NULL;
	}
	data_pack(packet, data, length);

	return packet;
}

void* data_pack(RTMPPacket *packet, const char* data, int length)
{
	//int size = length + 5 + 4 + RTMP_MAX_HEADER_SIZE + sizeof(RTMPPacket);
	//RTMPPacket *packet = calloc(1, size);	
	char *buf = packet + 1;
	char *body = buf + RTMP_MAX_HEADER_SIZE;

	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nChannel = 0x04;
	packet->m_hasAbsTimestamp = 0;
#ifndef USE_H264_RAW
	packet->m_nBodySize = length + 5 + 4;
#else
	packet->m_nBodySize = length + 5;
#endif
	packet->m_body = body;

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


void* rtmp_meta_pack(void* packager, const char* data, int length)
{
	//TimaRTMPPackager* packager;	//...
	int size = sizeof(RTMPPacket) + length + 8;
	RTMPPacket *packet = calloc(1, size);
	if (!packet) {
		TIMA_LOGE("malloc failed");
		return NULL;
	}
	meta_pack(packet, data, length);

	return packet;
}
void* meta_pack(RTMPPacket *packet, const char* data, int length)
{
	char *buf = packet + 1;
	char *body = buf + RTMP_MAX_HEADER_SIZE;

	//RTMPPacket packet;
	packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nChannel = 0x04;	//StreamID = (ChannelID-4)/5+1
	packet->m_hasAbsTimestamp = 0;
	packet->m_nBodySize = 8 + length;
	packet->m_body = body;

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
#endif
