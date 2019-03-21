/**
 * History:
 * ================================================================
 * 2019-03-15 qing.zou created
 *
 */

#include "librtmp/rtmp.h"

#include "context.h"
#include "ThreadPool.h"

#include "rtmp_publish.h"

#include "tima_rtmp_packager.h"
#include "tima_rtmp_publisher.h"

typedef struct stream_attribute_s
{
	stream_object();
	//int					cid;		/* channel id */			
	//int					mtype;		/* media type video/audio */	
	//int					seq;		/* sequence */			
	//long					size;		/* package size */			
	//unsigned long long	sim;		/* sim number */
	//unsigned char*		package;
} stream_attribute_t;

typedef struct PublishInfo
{

	int					connected;
	TimaRTMPPublisher	*publisher;
} PublishInfo;


typedef struct _PrivInfo
{
	RtmpPublishReq		req;
	RtmpPublishRep		rep;

	int					id;
	int					cond;

	//PublishInfo			pub[8];
	PublishInfo			pub;
	//TimaRTMPPublisher	*publisher[8];
} PrivInfo;

static int rtmp_publish_delete(vmp_node_t* p);
static int rtmp_publish_connect(vmp_node_t* p);



static int rtmp_publish_callback(void* p, int msg, void* arg)
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

static int rtmp_stream_pub(void* ctx, void* data, void* result)
{
	int ret = 0;
	PrivInfo* thiz = NULL;
	vmp_node_t* p = (vmp_node_t*)ctx;
	stream_attribute_t* stream = data;
	return_val_if_fail(ctx && data, -1);

	thiz = p->private;

	//tima_rtmp_send(thiz->pub[stream->cid-1].publisher, stream->package, timestamp);
	ret = tima_rtmp_send(thiz->pub.publisher, (RTMPPacket*)stream->package, 0);

	return ret;
}

static void rtma_publish_proc(vmp_node_t* p)
{
	PrivInfo* thiz = p->private;
	RtmpPublishReq* req = &thiz->req;

	if (req->traverse) {
		req->traverse(p->parent, rtmp_stream_pub, p);
	} else {
		TIMA_LOGE("list_traverse_callback unregister");
		//sleep(2);
		thiz->cond = 0;
	}
}

static void *rtmp_publish_thread(void* arg)
{
	TIMA_LOGD("rtmp_publish_thread");

	vmp_node_t* p = (vmp_node_t*)arg;
	PrivInfo* thiz = p->private;

	rtmp_publish_connect(p);

	while (1) {
		thiz->cond = 1;

		TIMA_LOGD("start rtma_publish_proc");

		while (thiz->cond) {

			rtma_publish_proc(p);
			//thiz->cond = 0;
		}


		if (thiz->cond == 0)
			break;
	}


	rtmp_publish_delete(p);

	return NULL;
}

static int rtmp_publish_get(vmp_node_t* p, int id, void* data, int size)
{
	return 0;
}

static int rtmp_publish_set(vmp_node_t* p, int id, void* data, int size)
{	
	PrivInfo* thiz = p->private;
	thiz->req = *((RtmpPublishReq*)data);
	
	return 0;
}

static int rtmp_publish_connect(vmp_node_t* p)
{
	int ret;
	char url[256] = {0};
	PrivInfo* thiz = p->private;
	//const char *uri = "rtmp://172.20.25.47:1935/hls/";
	const char *uri = "rtmp://172.20.25.47:1935/live/";
	//const char *uri = "rtmp://192.168.1.113:1936/live/";

	//for (i = 0; i < _countof(thiz->pub); i++)
	//{
	//	//sprintf(url, "%s%d", uri, i+1);
	//	snprintf(url, sizeof(url), "%s%lld_%d", uri, thiz->req.sim, i+1);
	//	TIMA_LOGI("connect to %s", url);
	//	thiz->pub[i].publisher = tima_rtmp_create(url);
	//	ret = tima_rtmp_connect(thiz->pub[i].publisher);
	//	if (ret < 0) {
	//		TIMA_LOGW("tima_rtmp_connect failed");
	//	} else {
	//		thiz->pub[i].connected = 1;
	//	}
	//}
	snprintf(url, sizeof(url), "%s%lld_%d", uri, thiz->req.sim, thiz->req.channel);
	TIMA_LOGI("connect to %s", url);
	thiz->pub.publisher = tima_rtmp_create(url);
	ret = tima_rtmp_connect(thiz->pub.publisher);
	if (ret < 0) {
		TIMA_LOGW("tima_rtmp_connect failed");
	} else {
		thiz->pub.connected = 1;
	}
	return 0;
}

static int rtmp_publish_disconnect(vmp_node_t* p)
{
	PrivInfo* thiz = p->private;
	if (thiz->pub.connected) {
		tima_rtmp_destory(thiz->pub.publisher);
		thiz->pub.publisher = NULL;
		thiz->pub.connected = 0;
	}
	return 0;
}

static void* rtmp_publish_start(vmp_node_t* p)
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

static int rtmp_publish_stop(vmp_node_t* p)
{
	return 0;
}

static vmp_node_t* rtmp_publish_create(void)
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

static int rtmp_publish_delete(vmp_node_t* p)
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

#if 0

void* rtmp_meta_pack(void* packager, const char* data, int length)
{
	return NULL;
}
void* rtmp_data_pack(void* packager, const char* data, int length)
{
	return NULL;
}
#else
void* rtmp_data_pack(void* p, const char* data, int length)
{
	TimaRTMPPackager* packager = p;
	int size = sizeof(RTMPPacket) + packager->body_len(length);
	RTMPPacket *packet = calloc(1, size);

// 	printf("size = %d\n", size);
// 	RTMPPacket *packet = malloc(size);
// 	memset((void*)packet, 0x00, size);
	if (!packet) {
		TIMA_LOGE("malloc failed");
		return NULL;
	}
	packager->data_pack(packet, data, length);

	return packet;
}

//void* data_pack(void *p, const char* data, int length)
//{
//	//int size = length + 5 + 4 + RTMP_MAX_HEADER_SIZE + sizeof(RTMPPacket);
//	//RTMPPacket *packet = calloc(1, size);	
//	RTMPPacket *packet = (RTMPPacket *)p;
//	char *buf = packet + 1;
//	char *body = buf + RTMP_MAX_HEADER_SIZE;
//
//	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
//	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
//	packet->m_nChannel = 0x04;
//	packet->m_hasAbsTimestamp = 0;
//#ifndef USE_H264_RAW
//	packet->m_nBodySize = length + 5 + 4;
//#else
//	packet->m_nBodySize = length + 5;
//#endif
//	packet->m_body = body;
//
//	*(body++) = (data[4] & 0x1f) == 0x05 ? 0x17 : 0x27;
//	*(body++) = 0x01;
//	*(body++) = 0x00;
//	*(body++) = 0x00;
//	*(body++) = 0x00;
//#ifndef USE_H264_RAW
//	// NALUs
//	*(body++) = length >> 24 & 0xff;
//	*(body++) = length >> 16 & 0xff;
//	*(body++) = length >> 8 & 0xff;
//	*(body++) = length & 0xff;
//#endif
//	memcpy(body, data, length);
//
//	return packet;
//}


void* rtmp_meta_pack(void* p, const char* data, int length)
{
	TimaRTMPPackager* packager = p;
	int size = sizeof(RTMPPacket) + length + 8 + RTMP_MAX_HEADER_SIZE;
	RTMPPacket *packet = calloc(1, size);

	//printf("meta size = %d\n", size);
	//RTMPPacket *packet = malloc(size);
	if (!packet) {
		TIMA_LOGE("malloc failed");
		return NULL;
	}
	packager->meta_pack(packet, data, length);

	return packet;
}
//void* meta_pack(void *p, const char* data, int length)
//{
//	RTMPPacket *packet = (RTMPPacket *)p;
//	char *buf = packet + 1;
//	char *body = buf + RTMP_MAX_HEADER_SIZE;
//
//	//RTMPPacket packet;
//	packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
//	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
//	packet->m_nChannel = 0x04;	//StreamID = (ChannelID-4)/5+1
//	packet->m_hasAbsTimestamp = 0;
//	packet->m_nBodySize = 8 + length;
//	packet->m_body = body;
//
//	*(body++) = 0x17; // 1-keyframe, 7-AVC
//	*(body++) = 0x00;
//	*(body++) = 0x00;
//	*(body++) = 0x00;
//	*(body++) = 0x00;
//
//	// AVCDecoderConfigurationRecord
//
//	*(body++) = 0x01;		// configurationVersion
//	*(body++) = data[5];	// AVCProfileIndication
//	*(body++) = data[6];	// profile_compatibility
//	*(body++) = data[7];	// AVCLevelIndication
//	*(body++) = 0xff;		// 111111(reserved) + lengthSizeMinusOne
//
//	int len = (data[2] << 8) | data[3];
//
//	*(body++) = 0xe1;		// 111(reserved) + numOfSequenceParameterSets
//	memcpy(body, data + 2, len + 2);
//
//	body += (len + 2);
//	*(body++) = 0x01;		// numOfPictureParameterSets
//	memcpy(body, data + len + 6, length - len - 6);
//
//	return packet;
//}
#endif
