/**
 * History:
 * ================================================================
 * 2019-05-29 qing.zou created
 *
 */

#include "tima_typedef.h"
#include "tima_sockioa.h"


typedef struct vmp_bserver_s
{
	bs_server_object();

} vmp_bserver_t;

VmpSocketIOA* vmp_unbound_relay_socket_create(void *e, int family, vpk_prototype_t type, sock_application_t atype)
{
	VmpSocketIOA *sock = VPK_CALLOC(1, sizeof(VmpSocketIOA));
	if (sock)
	{
		vpk_socket_ioa_create(e, family, type, &sock->abs);

		sock->abs.apptype	= atype;
		sock->abs.e			= e;
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

		if (s->abs.e && s->src_port > 0) {
			vmp_bserver_t *bs = s->abs.e;
			vmp_ports_release(bs->porter, s->src_port);
		}

		close(s->abs.fd);

		VPK_FREE(s);
	}
}

static int vmp_socket_bind(VmpSocketIOA* sock, vpk_sockaddr* local_addr, int reusable)
{
	return_val_if_fail(sock && sock->abs.fd > 0 && local_addr, -1);

	int ret = vpk_addr_bind(sock->abs.fd, local_addr, 0);
	if (ret >= 0) {
		sock->bound = 1;
		if (vpk_sockaddr_get_port(local_addr) < 1) {
			vpk_sockaddr tmp_addr;
			vpk_addr_get_from_sock(sock->abs.fd, &tmp_addr);
			//vpk_addr_any(&(s->local_addr))
			vpk_sockaddr_set_port(local_addr, vpk_sockaddr_get_port(&tmp_addr));
		}

		return 0;
	}

	VMP_LOGE("addr bind failed.");
	return -1;
}

int vmp_relay_socket_create(void *server, sock_application_t atype, VmpSocketIOA **s)
{
	int ret = 0, i = 0;
	VmpSocketIOA *sock = NULL;
	vpk_sockaddr local_addr = {0};
	vpk_sockaddr dest_addr = {0};
	int port;

#if 1
	vmp_bserver_t *bs = (vmp_bserver_t*)server;

	for (i = 0; i < 0xFFFF; i++)
	{
		port = vmp_ports_allocate(bs->porter);
		if (port < 0) {
			VMP_LOGE("vmp_ports_allocate failed.");
			return -1;
		}

		VMP_LOGI("%sallocate port %d", i > 0 ? "re-" : "", port);
		local_addr.s4.sin_family = PF_INET;
		local_addr.s4.sin_port = htons(port);
		local_addr.s4.sin_addr.s_addr = INADDR_ANY;

		dest_addr.s4.sin_family = PF_INET;
		dest_addr.s4.sin_port = htons(0);		/* set dst port zero */
		dest_addr.s4.sin_addr.s_addr = INADDR_ANY;

		sock = vmp_unbound_relay_socket_create(server, PF_INET, VPK_PROTOTYPE_UDP, atype /*VPK_APPTYPE_WEBSOCKET_RELAY*/);
		if (!sock) {
			VMP_LOGE("create relay socket error!");
			vmp_ports_release(bs->porter, port);
			return -1;
		}

		ret = vmp_socket_bind(sock, &local_addr, 0);
		if (ret < 0) {
			VMP_LOGE("socket bind error!");
			vmp_socket_release(sock);
			vmp_ports_release(bs->porter, port);
		} else {
			break;
		}

	}

	sock->src_port = port;
	vpk_addr_copy(&sock->local_addr, &local_addr);
	vpk_addr_copy(&sock->dest_addr, &dest_addr);

#else
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

	sock = vmp_unbound_relay_socket_create(s, PF_INET, VPK_PROTOTYPE_UDP, atype /*VPK_APPTYPE_WEBSOCKET_RELAY*/);
	if (!sock) {
		VMP_LOGE("create relay socket error!");
		return -1;
	}

	ret = vpk_addr_bind(sock->abs.fd, &local_addr, 0);
	if (ret < 0) {
		VMP_LOGE("socket bind error!");
		vmp_socket_release(sock);
		return -1;

	} else {
		sock->bound = 1;
		if (vpk_sockaddr_get_port(&local_addr) < 1) {
			vpk_sockaddr tmp_addr;
			vpk_addr_get_from_sock(sock->abs.fd, &tmp_addr);
			//vpk_addr_any(&(s->local_addr))
			vpk_sockaddr_set_port(&local_addr, vpk_sockaddr_get_port(&tmp_addr));
		}
	}

	vpk_addr_copy(&sock->local_addr, &local_addr);
	vpk_addr_copy(&sock->dest_addr, &dest_addr);
#endif

	*s = sock;

	return 0;
}

