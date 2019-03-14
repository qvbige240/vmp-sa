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


typedef union {
	struct sockaddr ss;
	struct sockaddr_in s4;
	struct sockaddr_in6 s6;
} vmp_addr;


typedef struct vmp_socket_s
{
	int				cached;
	//buffer_list buff_list;
	vmp_addr		peer_addr;

	evutil_socket_t	fd;
} vmp_socket_t;



#endif // TIMA_SERVER_H
