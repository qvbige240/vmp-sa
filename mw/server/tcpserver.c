/*
============================================================================
Name        : tcpserver.c
Author      : wison.wei
Copyright   : 2019(c) Timanetworks Company
Description :
============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
//#include <netinet/tcp.h>
#include "context.h"
#include "ThreadPool.h"


static unsigned char pre_buf[102400] = {0};
static size_t audio_buf_len;
static int flag_audio = 0;

size_t hander_packet(const unsigned char* packet, int copy_len);
void save_h264_bitstream (const unsigned char *data_buf, int len);
extern int bll_demo_proc(const char* buf, size_t size, long long timestamp);

void server_on_event(struct bufferevent* bev, short event, void* arg)
{
	if( event & (BEV_EVENT_EOF))
	{
		printf("EOF %d\n", arg);
	}
	else if( event & BEV_EVENT_ERROR)
	{
		printf("ERROR %d\n", arg);
	}
	else if( event & BEV_EVENT_TIMEOUT)
	{
		printf("TIMEOUT %d\n", arg);
	}
	else if (event & BEV_EVENT_CONNECTED)
	{
		printf("CONNECTED %d\n", arg);
		return;
	}
	bufferevent_free(bev);
}

static int vpk_file_save(const char* filename, void* data, size_t size)
{
	FILE* fp = 0;
	size_t ret = 0;
	//return_val_if_fail(filename != NULL && data != NULL, -1);

	fp = fopen(filename, "a+");
	if (fp != NULL && data)
	{
		ret = fwrite(data, 1, size, fp);
		fclose(fp);
	}
	if (ret != size)
		printf("fwrite size(%d != %d) incorrect!", ret, size);

	return ret;
}

#define BCD_PARSE(c)		(((c & 0xf0) >> 4) * 10 + (c & 0x0f))

static unsigned long long simno_get(unsigned char *bcd)
{
	unsigned long long num = 0;
	unsigned char *ptr = bcd;
	int i = 0;
	//printf("bcd: ");
	for (i = 0; i < 6; i++)
	{
		unsigned digital = BCD_PARSE(*(ptr+i));
		num = num * 100 + digital;
		//printf("%02x ", *(ptr+i));
	}
	//printf("\nparsed: %lld\n", num);

	return num;
}


#if 0
static void server_on_read(struct bufferevent* bev,void* arg)
{
	size_t copy_len = 0, ret_len = 0;
	struct evbuffer* input = bufferevent_get_input(bev);
	//size_t len = 0;
	//len = evbuffer_get_length(input);

	if (flag_audio)
	{
		if (audio_buf_len > 0)
		{
			copy_len = evbuffer_copyout(input,pre_buf, audio_buf_len);
			vpk_file_save("raw_data.audio", pre_buf, copy_len);
			vpk_file_save("raw_data.media", pre_buf, copy_len);
			ret_len = evbuffer_remove(input, pre_buf,copy_len);
			printf("audio_buf_len=%d, copy_len=%d, ret=%d\n", audio_buf_len, copy_len, ret_len);
			printf("audio_buf_len: %d - %d = %d\n", audio_buf_len, ret_len, audio_buf_len-ret_len);
			audio_buf_len -= ret_len;
			return;
		}
		else
		{
			flag_audio = 0;
		}
	}

	copy_len = evbuffer_copyout(input,pre_buf, 1024);
	size_t packet_len = hander_packet(pre_buf, copy_len);
	if (packet_len == 0)
	{
		//printf("#packet_len == 0\n");
		packet_len = copy_len > 980 ? 980 : copy_len;
		evbuffer_remove(input, pre_buf,packet_len); // clear the buffer
		vpk_file_save("raw_data.media", pre_buf, packet_len);
		return ;
	}


	long long timestamp = pre_buf[16] << 56 | pre_buf[16+1] << 48
		| pre_buf[16+2] << 40 | pre_buf[16+3] << 32
		| pre_buf[16+4] << 24 | pre_buf[16+5] << 16
		| pre_buf[16+6] << 8 | pre_buf[16+7];

	printf("timestamp: %lld, ", timestamp);
	//int ii = 0;
	//for (ii = 0; ii < 8; ii++)
	//{
	//    printf("%02x ", pre_buf[16+ii]);
	//}

	printf("#channelid=%d, type[15]=%02x, [28:29]=%02x %02x, packet_len=%d, copy_len=%d\n",
		pre_buf[14], pre_buf[15], pre_buf[28],pre_buf[29], packet_len, copy_len);


	if ((pre_buf[15]& 0xf0) == 0x30)
	{
		flag_audio = 1;
		audio_buf_len = packet_len;
		if (packet_len > 980)
		{

			vpk_file_save("raw_data.audio", pre_buf, 980);
			vpk_file_save("raw_data.media", pre_buf, 980);
			ret_len = evbuffer_remove(input, pre_buf, 980);
			printf("audio_buf_len: %d - %d = %d\n", audio_buf_len, 950, audio_buf_len-950);
			audio_buf_len -= 950;
			//printf("audio_buf_len=%d\n", audio_buf_len);
		}
		else
		{
			vpk_file_save("raw_data.media", pre_buf, packet_len);
			evbuffer_remove(input, pre_buf, packet_len);
			printf("audio_buf_len: %d - %d = %d\n", audio_buf_len, (packet_len - 30), audio_buf_len-(packet_len - 30));
			audio_buf_len = audio_buf_len - (packet_len - 30);
		}
		return;
	}

	if (packet_len > 980)
	{
		packet_len = 980;
		printf("#packet_len > 980\n");
	}

	evbuffer_remove(input, pre_buf,packet_len); // clear the buffer

	vpk_file_save("raw_data.media", pre_buf, packet_len);
	//save_h264_bitstream(pre_buf, packet_len);

	return;
}
#elif 0
static void server_on_read(struct bufferevent* bev,void* arg)
{
	struct evbuffer* input = bufferevent_get_input(bev);
	size_t len = evbuffer_get_length(input);

	if (flag_audio)
	{
		if (audio_buf_len > 0)
		{
			size_t ret_len = evbuffer_remove(input, pre_buf, audio_buf_len);
			audio_buf_len -= ret_len;
			printf("audio_buf_len=%d, ret_len=%d\n", audio_buf_len, ret_len);
			return;
		}
		else
		{
			flag_audio = 0;
		}
	}

	do 	 
	{
		size_t copy_len = evbuffer_copyout(input, pre_buf, 1024);
		size_t packet_len = hander_packet(pre_buf, copy_len);

		long long timestamp = pre_buf[16] << 56 | pre_buf[16+1] << 48
			| pre_buf[16+2] << 40 | pre_buf[16+3] << 32
			| pre_buf[16+4] << 24 | pre_buf[16+5] << 16
			| pre_buf[16+6] << 8 | pre_buf[16+7];

		// for audio
		if ((pre_buf[15]& 0xf0) == 0x30)
		{
			flag_audio = 1;
			audio_buf_len = packet_len;
			if (packet_len > 980)
			{
				evbuffer_remove(input, pre_buf, 980);
				audio_buf_len -= 950;
			}
			else
			{
				evbuffer_remove(input, pre_buf, packet_len);
				audio_buf_len = audio_buf_len - packet_len - 30;
			}
			return;
		}

		if (packet_len > 980)
		{
			packet_len = 980;
			printf("#packet_len > 980\n");
		}

		if (copy_len < packet_len) break;
		evbuffer_remove(input, pre_buf, packet_len); // clear the buffer
		if (pre_buf[14] == 1)
			bll_demo_proc(pre_buf+30, packet_len-30, timestamp);
		len -= packet_len;
	} while (len > 0);
	return;
}
#else


static void server_on_read(struct bufferevent* bev,void* arg)
{
	struct evbuffer* input = bufferevent_get_input(bev);
	size_t len = evbuffer_get_length(input);

	do 	 
	{
		size_t copy_len = evbuffer_copyout(input, pre_buf, 1024);
		size_t packet_len = hander_packet(pre_buf, copy_len);

		unsigned long long sim_num = simno_get(pre_buf+8);

		long long timestamp = pre_buf[16] << 56 | pre_buf[16+1] << 48
			| pre_buf[16+2] << 40 | pre_buf[16+3] << 32
			| pre_buf[16+4] << 24 | pre_buf[16+5] << 16
			| pre_buf[16+6] << 8 | pre_buf[16+7];

		if (packet_len > 980)
		{
			packet_len = 980;
			printf("#packet_len > 980\n");
		}

		if (copy_len < packet_len)
			break;

		evbuffer_remove(input, pre_buf, packet_len); // clear the buffer

		// for audio
		if ((pre_buf[15]& 0xf0) == 0x30)
		{
			parser_h264_bitstream(pre_buf, packet_len);
		}
		else 
		{
			if (pre_buf[14] == 1)
				bll_demo_proc(pre_buf+30, packet_len-30, sim_num);

		}

		len -= packet_len;


	} while (len > 0);

	return;
}
#endif

void server_on_accept(struct evconnlistener* listener,evutil_socket_t fd,struct sockaddr *address,int socklen,void *arg)
{
	printf("accept a client : %d\n", fd);

	// set TCP_NODELAY
	//int enable = 1;
	//if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&enable, sizeof(enable)) < 0)
	//   printf("Consensus-side: TCP_NODELAY SETTING ERROR!\n");

	struct event_base *base = evconnlistener_get_base(listener);
	struct bufferevent* new_buff_event = bufferevent_socket_new(base,fd,BEV_OPT_CLOSE_ON_FREE);
	// bufferevent_setwatermark(new_buff_event, EV_READ, 72, 0);
	bufferevent_setcb(new_buff_event,server_on_read,NULL,server_on_event,fd);
	// set a read timeout of 1000 us
	// struct timeval tv = {0, 10000};
	// bufferevent_set_timeouts(new_buff_event, &tv, NULL);
	bufferevent_enable(new_buff_event,EV_READ|EV_WRITE);

}

void* tcpserver_thread(void* arg)
{
	printf("tcpserver_thread begin\n");

	int port = 9999;
	struct sockaddr_in my_address;
	memset(&my_address, 0, sizeof(my_address));
	my_address.sin_family = AF_INET;
	//my_address.sin_addr.s_addr = htonl(0x7f000001); // 127.0.0.1
	my_address.sin_addr.s_addr = htonl(0x0000000); // 0.0.0.0
	my_address.sin_port = htons(port);

	struct event_base* base = event_base_new();
	struct evconnlistener* listener =
		evconnlistener_new_bind(base,server_on_accept,
		NULL,LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,-1,
		(struct sockaddr*)&my_address,sizeof(my_address));

	if(!listener)
	{
		printf("tcpserver_thread  listener==NULL\n");
	}

	event_base_dispatch(base);
	evconnlistener_free(listener);
	event_base_free(base);

	printf("tcpserver_thread end\n");
}

void tcpserver_init(void)
{
	printf("tcpserver_init\n");

	context* ctx = Context();

	ThreadPoolJob job;
	TPJobInit( &job, ( start_routine) tcpserver_thread, NULL);
	TPJobSetFreeFunction( &job, ( free_routine ) NULL );
	TPJobSetPriority( &job, HIGH_PRIORITY );
	ThreadPoolAddPersistent( ctx->tp, &job, &job.jobId);
	int id = job.jobId;
}

void tcpserver_done(void)
{
	tp_done();
}


static int channel[8] = {1,2,3,4,5,6,7,8};
static char* file_path[8] = {
	"./media_data1",
	"./media_data2",
	"./media_data3",
	"./media_data4",
	"./media_data5",
	"./media_data6",
	"./media_data7",
	"./media_data8"
};

void save_h264_bitstream (const unsigned char *data_buf, int len)
{
	FILE *fp;
	int i;
	for (i = 0; i <8 ; ++i)
	{
		if (data_buf[14] == channel[i])
		{
			fp = fopen(file_path[i], "a+");
			unsigned char* sub_buf = (unsigned char *)malloc(len-30);
			memcpy(sub_buf, data_buf+30, len-30);
			fwrite(sub_buf, len-30, 1, fp);
			fclose (fp);
			free(sub_buf);
		}
	}
}

size_t hander_packet(const unsigned char* packet, int copy_len)
{
	size_t data_len = 0;
	if (packet[0] == 0x30 && packet[1] == 0x31 && packet[2] == 0x63 && packet[3] == 0x64)
	{
		int is_audio = ((packet[15]& 0xf0) == 0x30);
		if (is_audio)
			data_len = packet[28] * 2;
		else
			data_len = packet[29] | packet[28] << 8;
		return data_len + 30;
	}
	else
	{
		int ii = 0;
		for (ii = 0; ii < 32; ii++)
		{
			printf("%02x ", packet[ii]);
		}
		printf("... data packet exception\n");
		return 0;
	}
}

