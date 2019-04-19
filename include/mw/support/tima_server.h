/**
 * History:
 * ================================================================
 * 2019-03-13 qing.zou created
 *
 */
#ifndef TIMA_SERVER_H
#define TIMA_SERVER_H

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "tima_typedef.h"

#define VMP_BUFFEREVENTS_OPTIONS (BEV_OPT_DEFER_CALLBACKS | BEV_OPT_THREADSAFE | BEV_OPT_UNLOCK_CALLBACKS | BEV_OPT_CLOSE_ON_FREE)

#define vmp_addr	vpk_sockaddr

typedef struct vmp_launcher_s
{
	//tima_memory_t		*mem;
	struct event_base	*event_base;
} vmp_launcher_t;

typedef struct vmp_socket_s
{
	int					cached;
	vmp_addr			peer_addr;

	evutil_socket_t		fd;
	struct bufferevent *bev;

	struct event_base	*event_base;

	vmp_launcher_t		*e;

	void				*priv;
} vmp_socket_t;



#endif // TIMA_SERVER_H
