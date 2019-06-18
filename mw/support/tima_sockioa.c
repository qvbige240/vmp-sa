/**
 * History:
 * ================================================================
 * 2019-05-29 qing.zou created
 *
 */

#include "vmp.h"
#include "tima_typedef.h"
#include "tima_sockioa.h"


VmpSocketIOA* vmp_unbound_relay_socket_create(void *e, int family, vpk_prototype_t type, sock_application_t atype)
{
	VmpSocketIOA *sock = VPK_CALLOC(1, sizeof(VmpSocketIOA));
	if (sock)
	{
		vpk_socket_ioa_create(e, family, type, &sock->abs);

		sock->abs.apptype = atype;
	}

	return sock;
}

void vmp_socket_release(VmpSocketIOA *s)
{
	if (s)
	{
		if (s->read_event) {
			event_free(s->read_event);
			s->read_event = NULL;
		}

		close(s->abs.fd);

		VPK_FREE(s);
	}
}

int vmp_relay_socket_create(void *e, sock_application_t atype, VmpSocketIOA **s)
{
	int ret = 0;
	VmpSocketIOA *sock = NULL;
	vpk_sockaddr local_addr = {0};
	vpk_sockaddr dest_addr = {0};

	unsigned short src_port=13000;
	unsigned short dst_port=13001;
	if (atype == VPK_APPTYPE_WEBSOCKET_RELAY) {
		src_port = 13001;
		dst_port = 13000;
	} else {
		src_port = 13000;
		dst_port = 13001;
	}

	local_addr.s4.sin_family = PF_INET;
	local_addr.s4.sin_port = htons(src_port);		//...
	local_addr.s4.sin_addr.s_addr = INADDR_ANY;

	dest_addr.s4.sin_family = PF_INET;
	dest_addr.s4.sin_port = htons(dst_port);		//...
	dest_addr.s4.sin_addr.s_addr = INADDR_ANY;

	sock = vmp_unbound_relay_socket_create(e, PF_INET, VPK_PROTOTYPE_UDP, atype /*VPK_APPTYPE_WEBSOCKET_RELAY*/);
	if (!sock) {
		VMP_LOGE("create relay socket error!");
		return -1;
	}

	ret = vpk_addr_bind(sock->abs.fd, &local_addr, 0);
	if (ret >= 0) {
		sock->bound = 1;
	} else {
		VMP_LOGE("socket bind error!");
		vmp_socket_release(sock);
		return -1;
	}

	vpk_addr_copy(&sock->local_addr, &local_addr);
	vpk_addr_copy(&sock->dest_addr, &dest_addr);

	*s = sock;

	return 0;
}

