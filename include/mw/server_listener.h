/**
 * History:
 * ================================================================
 * 2019-03-11 qing.zou created
 *
 */

#ifndef SERVER_LISTENER_H
#define SERVER_LISTENER_H

#include <pthread.h>

#include "tima_typedef.h"
#include "tima_server.h"

#pragma pack(1)

TIMA_BEGIN_DELS

#define SERVER_LISTENER_CLASS	FOURCCLE('S','V','R','L')


typedef enum VmpStreamType {
	VMP_STREAM_UNKNOWN = 0,
	VMP_STREAM_JT808,
	VMP_STREAM_JT1078,
	VMP_STREAM_RTP,
	VMP_STREAM_RECORDER,
} VmpStreamType;

//typedef union {
//	struct sockaddr ss;
//	struct sockaddr_in s4;
//	struct sockaddr_in6 s6;
//} vmp_addr;

struct stream_server;
typedef struct stream_server vmp_server_t;

struct vmp_connection_s;
typedef struct vmp_connection_s vmp_connection_t;

//typedef struct vmp_launcher_s
//{
//	//tima_memory_t			*mem;
//	struct event_base		*event_base;
//} vmp_launcher_t;

typedef int (*server_new_connection_handler)(vmp_launcher_t *e, vmp_connection_t *sm);

//typedef struct vmp_socket_s
//{
//	int				cached;
//	//buffer_list buff_list;
//	vmp_addr		peer_addr;
//
//	evutil_socket_t	fd;
//} vmp_socket_t;

struct stream_server {
	unsigned char			id;

	//tima_memory_t			*mem;
	void					*core;
	struct event_base		*event_base;
	vmp_launcher_t			*e;
	struct bufferevent		*in_buf;
	struct bufferevent		*out_buf;
	bufferevent_data_cb		read_cb;

	pthread_t				pth_id;

	unsigned int			client_cnt;		//need lock

	void					*priv;
};

struct vmp_connection_s {
	VmpStreamType		t;
	vmp_server_t		*stream_server;
	vmp_socket_t		sock;
};



typedef struct _ServerListenerReq
{
	void*					ctx;
	bufferevent_data_cb		read_cb;
} ServerListenerReq;

typedef struct _ServerListenerRep
{

} ServerListenerRep;

void server_listener_init(void);
void server_listener_done(void);

#pragma pack()

TIMA_END_DELS

#endif // SERVER_LISTENER_H
