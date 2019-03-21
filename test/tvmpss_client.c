
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/time.h>
#include <netinet/tcp.h>


#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "ThreadPool.h"
#include "tvmpss_client.h"


void event_cb(struct bufferevent* bev, short event, void* arg)
{
	StClient* p = (StClient*)arg;
	
	if(event & BEV_EVENT_EOF)
	{
		printf("%d -->EOF\n", p->jobId);
	}
	else if(event & BEV_EVENT_ERROR)
	{
		printf("%d-->ERROR\n", p->jobId);
	}
	else if(event & BEV_EVENT_CONNECTED)
	{
		printf("%d-->CONNECTED\n", p->jobId);
		return;
	}

	bufferevent_free(bev);
}


void* tvmpss_send_thread(void* arg)
{
	StClient* p = (StClient*)arg;
	char* stream_buf  = p->data;
	int  stream_len = p->len;
	
	sleep(1);
	
	size_t cur = 0;
	size_t end = 4;
	int flag = 0;

	char sim[13] = {0};
	sprintf(sim, "%08s%04d", p->begin, p->jobId);

	while(1)
	{
		while(end < stream_len-4)
		{
			int tmpflag0;

			tmpflag0 = (stream_buf[end] == 0x30) && (stream_buf[end+1] == 0x31) && (stream_buf[end+2] == 0x63) && (stream_buf[end+3] == 0x64);
			if(tmpflag0)
			{
				flag = 1;
				break;
			}
			end++;
		}

		if(0 == flag)
		{
			end = stream_len;
		}

		char* data = stream_buf + cur;
		int len = end - cur;

		//sim
		char* pSim = data+8;
		int j;
		for(j=0; j<6; j++)
		{
			char c1 = (sim[j*2]-48) << 4;
			char c2 = (sim[j*2+1]-48);
			pSim[j] = c1+c2;
		}
		//memset(data+8, '0', 6);
		//memcpy(data+8+6-nSim, str, nSim); 
		printf("%d-->[%02x] [%02x] [%02x] [%02x] [%02x] [%02x]\n", p->jobId, pSim[0],pSim[1],pSim[2],pSim[3],pSim[4],pSim[5]);	

		//int bret = bufferevent_write(p->bev, data, len);
		int bret = send(p->fd, data, len,0);
		if(bret < len)
		{
			printf("%d-->[%d] \t%s %d\n", p->jobId, len, sim, bret);	
		}

		if(0 == flag)
		{
			cur = 0;
			end = 3;
			continue;
		}

		cur = end;
		end += 3;
		flag = 0;
		
		usleep(p->delay*1000); 
	}
}


void* tvmpss_client_thread(void* arg)
{
	StClient* p = (StClient*)arg;

    	struct sockaddr_in my_address;
    	memset(&my_address, 0, sizeof(my_address));
    	my_address.sin_family = AF_INET;
    	my_address.sin_addr.s_addr = inet_addr(p->ip); 
    	my_address.sin_port = htons(p->port);
 
 	 // build event base
    	struct event_base* base = event_base_new();
 
    	// set TCP_NODELAY to let data arrive at the server side quickly
    	evutil_socket_t fd;
    	fd = socket(AF_INET, SOCK_STREAM, 0);
    	struct bufferevent* conn = bufferevent_socket_new(base,fd,BEV_OPT_CLOSE_ON_FREE);
	p->bev = conn;
	p->fd = fd;

	int enable = 1;
    	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));

    	//bufferevent_setcb(conn, NULL, NULL, event_cb, p); // For client, we don't need callback function
    	bufferevent_enable(conn, EV_WRITE);
    	bufferevent_socket_connect(conn,(struct sockaddr*)&my_address,sizeof(my_address));

	ThreadPoolJob job;
	TPJobInit( &job, ( start_routine) tvmpss_send_thread, p);
	TPJobSetFreeFunction( &job, ( free_routine ) NULL );
	TPJobSetPriority( &job, HIGH_PRIORITY );
	ThreadPoolAddPersistent( p->tp, &job, &job.jobId);
	//c->jobId = job.jobId;

	 event_base_dispatch(base);
	 
	return NULL;
}

