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
static unsigned char* tmp_buff;
static int flag_audio = 0;

size_t hander_packet(const unsigned char* packet, int copy_len);
void parser_h264_bitstream (const unsigned char *data_buf, int len);

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

static void server_on_read(struct bufferevent* bev,void* arg)
{
    struct evbuffer* input = bufferevent_get_input(bev);
    size_t len = 0;
    len = evbuffer_get_length(input);

    if (flag_audio)
    {
        if (audio_buf_len > 0)
        {
            size_t ret_len = evbuffer_remove(input, pre_buf,audio_buf_len);
            audio_buf_len -= ret_len;
            printf("audio_buf_len=%d, ret_len=%d\n", audio_buf_len, ret_len);
            return;
        }
        else
        {
            flag_audio = 0;
        }
    }

    size_t copy_len = evbuffer_copyout(input,pre_buf,2048);
    size_t packet_len = hander_packet(pre_buf, copy_len);

    long long timestamp = pre_buf[16] << 56 | pre_buf[16+1] << 48
                          | pre_buf[16+2] << 40 | pre_buf[16+3] << 32
                          | pre_buf[16+4] << 24 | pre_buf[16+5] << 16
                          | pre_buf[16+6] << 8 | pre_buf[16+7];

    printf("timestamp: %lld, ", timestamp);
    int ii = 0;
    for (ii = 0; ii < 8; ii++)
    {
        printf("%02x ", pre_buf[16+ii]);
    }

    printf("#[15]=%02x,[28]=%02x,[29]=%02x,channelid=%d, copy_len=%d, packet_len=%d\n",
           pre_buf[15],pre_buf[28],pre_buf[29],pre_buf[14], copy_len, packet_len);


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

    evbuffer_remove(input, pre_buf,packet_len); // clear the buffer
    parser_h264_bitstream(pre_buf, packet_len);
    return;
}


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
static char* file_path[8] = {"/tmp/datatest_result_full1",
                             "/tmp/datatest_result_full2",
                             "/tmp/datatest_result_full3",
                             "/tmp/datatest_result_full4",
                             "/tmp/datatest_result_full5",
                             "/tmp/datatest_result_full6",
                             "/tmp/datatest_result_full7",
                             "/tmp/datatest_result_full8"
                            };

void parser_h264_bitstream (const unsigned char *data_buf, int len)
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
    if (packet[0] == 0x30 && packet[1] == 0x31 && packet[2] == 0x63 && packet[3] == 0x64)
    {
        size_t data_len = packet[29] | packet[28] << 8;
        return data_len + 30;
    }
    else
    {
        int ii = 0;
        for (ii = 0; ii < 32; ii++)
        {
            printf("%02x ", packet[ii]);
        }
        printf("data packet exception\n");
        return 0;
    }
}

