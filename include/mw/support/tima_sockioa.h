/**
 * History:
 * ================================================================
 * 2019-05-29 qing.zou created
 *
 */

#ifndef TIMA_SOCKIOA_H
#define TIMA_SOCKIOA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <event2/event.h>

#include "vmp.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct _VmpSocketIOA
{
	vpk_socketio_t			abs;
	int						bound;

	int						dst_port;
	int						src_port;

	vpk_sockaddr			dest_addr;
	vpk_sockaddr			local_addr;
	struct bufferevent	   *bev;

	struct event		   *read_event;

} VmpSocketIOA;

VmpSocketIOA* vmp_unbound_relay_socket_create(void *e, int family, vpk_prototype_t type, sock_application_t atype);
int vmp_relay_socket_create(void *e, sock_application_t atype, VmpSocketIOA **s);
void vmp_socket_release(VmpSocketIOA *s);


#ifdef __cplusplus
}
#endif

#endif // TIMA_SOCKIOA_H
