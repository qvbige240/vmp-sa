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

 
static void server_on_read(struct bufferevent* bev,void* arg){
    struct evbuffer* input = bufferevent_get_input(bev);
    size_t len = 0;
    len = evbuffer_get_length(input);
    printf("There Is %u Bytes Data In The Buffer In Total.\n",
            (unsigned)len);
    
    // read
    char* buf;
    buf = (char*)malloc(sizeof(char)*len);
    if(NULL==buf){return;}
    //evbuffer_copyout(input,buf,len); // it do not clear the input buffer
    evbuffer_remove(input,buf,len); // clear the buffer
    printf("Server gets the message from client: %s\n", buf);    
   
    // check
    len = 0;
    len = evbuffer_get_length(input);
    printf("After the first reading, there Is %u Bytes Data Left In The Buffer.\n",
            (unsigned)len);
    
    free(buf);
    buf=NULL;
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
    bufferevent_setcb(new_buff_event,server_on_read,NULL,NULL,NULL);
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

