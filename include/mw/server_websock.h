/**
 * History:
 * ================================================================
 * 2018-05-21 qing.zou created
 *
 */

#ifndef SERVER_WEBSOCK_H
#define SERVER_WEBSOCK_H

#include "tima_typedef.h"
#include "tima_server.h"

//#pragma pack(1)

TIMA_BEGIN_DELS

#define SERVER_WEBSOCK_CLASS		FOURCCLE('S','V','W','S')


//struct websock_server;
//typedef struct websock_server vmp_wserver_t;
//
//struct websock_server {
//	unsigned char			id;
//
//	//tima_memory_t			*mem;
//	void					*core;
//	struct event_base		*event_base;
//	//vmp_launcher_t			*e;
//	//struct bufferevent		*in_buf;
//	//struct bufferevent		*out_buf;
//	//bufferevent_data_cb		read_cb;
//
//	pthread_t				pth_id;
//
//	unsigned int			client_cnt;		//need lock
//
//	void					*priv;
//};

typedef struct vmp_wserver_s
{
	bs_server_object();

	unsigned char			id;

	struct event_base		*event_base;

	//pthread_t				pth_id;
} vmp_wserver_t;

typedef int (*websock_connect_func)(void *ctx, void *rep);

typedef struct _ServerWebsockReq
{
	unsigned short			port;
	void*					ctx;

	websock_connect_func	on_connect;
} ServerWebsockReq;

typedef struct _ServerWebsockRep
{
	void					*client;
	void					*ws;	/* vmp_wserver_t */
} ServerWebsockRep;

void server_websock_init(void);
void server_websock_done(void);

//#pragma pack()

TIMA_END_DELS

#endif // SERVER_WEBSOCK_H
