/**
 * History:
 * ================================================================
 * 2018-02-05 qing.zou created
 *
 */

#include "bll_rtmp_stream_demo.h"

#include "tima_support.h"

#include "tima_h264.h"
#include "tima_rtmp_packager.h"
#include "tima_rtmp_publisher.h"


typedef struct _PrivInfo
{
	DemoDataReq			req;
	DemoDataRep			rep;

	int					id;

	TimaRTMPPackager* packager;
	TimaRTMPPublisher* publisher;

	int					started;
	h264_meta_t			meta_data;


	TimaBuffer			buffer;


} PrivInfo;

//static int tima_buffer_should_realign(tpc_evbuffer_chain *chain, size_t datlen)
//{
//	return chain->buffer_len - chain->off >= datlen &&
//		(chain->off < chain->buffer_len / 2) &&
//		(chain->off <= TPC_MAX_TO_REALIGN_IN_EXPAND);
//}
//
//static int is_key_frame(char* data) { return ((data[0] & 0x1f) == 0x05); }
//static int is_sps(char* data) { return ((data[0] & 0x1f) == 0x07); }
//static int is_pps(char* data) { return ((data[0] & 0x1f) == 0x08); }
//static int is_dpc(char* data) { return ((data[0] & 0x1f) == 0x03); }
//static int is_aud(char* data) { return ((data[0] & 0x1f) == 0x09); }

static PrivInfo *priv;
static char *chunk_buffer = NULL;

void bll_demo_init(void)
{
	chunk_buffer = malloc(1<<20);
	priv = calloc(1, sizeof(PrivInfo));
	tima_buffer_init(&priv->buffer, 128<<10);


	priv->packager = tima_h264_rtmp_create();

	//priv->publisher = tima_rtmp_create("rtmp://localhost/live");
	//priv->publisher = tima_rtmp_create("rtmp://172.20.25.209:2019/timalive/test1");
	priv->publisher = tima_rtmp_create("rtmp://172.20.25.47:1935/hls/1");
	//priv->publisher = tima_rtmp_create("rtmp://172.20.25.47:1936/live/1");
	tima_rtmp_connect(priv->publisher);
}


int bll_demo_proc(const char* buf, size_t size, long long timestamp)
{
	size_t pos = 0, len;
	const char *data = buf;
	size_t length = size;
	PrivInfo *thiz = priv;

	//int i = 0;
	//for (i = 0; i < 32; i++)
	//	printf("%02x ", (unsigned char)data[i]);
	//printf("\n");

	tima_buffer_strdup(&thiz->buffer, data, length, 1);
	data = tima_buffer_data(&thiz->buffer, 0);
	length = tima_buffer_used(&thiz->buffer);

	//size_t s = tima_buffer_size(&thiz->buffer);
	//if (s > 64 << 10)
	//	printf("buffer total size: %d\n", s);

	if (!thiz->started) {

		int i = 0;
		for (i = 0; i < 32; i++)
			printf("%02x ", (unsigned char)data[i]);
		printf("\n");

		pos = h264_metadata_get(data, length, 0, &thiz->meta_data);
		if (pos == 0) {
			VMP_LOGE("stream parse error at metadata");
			return -1;
		}
		thiz->started = 1;

		RTMPPacket meta = thiz->packager->meta_pack(chunk_buffer, thiz->meta_data.data, thiz->meta_data.size);
		tima_rtmp_send(thiz->publisher, &meta, timestamp);
	}
	
	do 
	{
		h264_nalu_t nalu;
		len = h264_nalu_read(data, length, pos, &nalu);
		if (len != 0) {
			pos += len;
			printf("## nalu type(%d) len(%d)\n", nalu.nalu_type, nalu.nalu_len);
			
			//send to server
			if (nalu.nalu_type == 0x05) {

				//RTMPPacket packet = {0};
				//tima_rtmp_send(thiz->publisher, &packet, 0);
				RTMPPacket meta = thiz->packager->meta_pack(chunk_buffer, thiz->meta_data.data, thiz->meta_data.size);
				tima_rtmp_send(thiz->publisher, &meta, timestamp);
			} else if (nalu.nalu_type == 0x07 || nalu.nalu_type == 0x08 || nalu.nalu_type == 0x06) {
				continue;
			}
			RTMPPacket packet = thiz->packager->data_pack(chunk_buffer, nalu.nalu_data, nalu.nalu_len);
			tima_rtmp_send(thiz->publisher, &packet, timestamp);
		}
	} while (len);
	
	tima_buffer_align(&thiz->buffer, pos);

	return 0;
}
