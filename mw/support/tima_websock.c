/**
 * History:
 * ================================================================
 * 2019-05-21 qing.zou created
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vmp.h"
#include "tima_typedef.h"
#include "tima_websock.h"

#include "websock/websock.h"

int tima_websock_fd_get(void *client)
{
	libwebsock_client_state *c = client;
	return_val_if_fail(c, -1);

	return c->sockfd;
}

void* tima_websock_priv_get(void *client)
{
	libwebsock_client_state *c = client;
	return_val_if_fail(c, -1);

	return c->priv;
}

int tima_websock_callback_set(void *client, TimaWebsockFunc *cb)
{
	libwebsock_client_state *c = client;
	return_val_if_fail(c && cb, -1);

	c->onmessage	= cb->onmessage;
	c->onclose		= cb->onclose;
	c->onpong		= cb->onpong;

	return 0;
}

int tima_websock_priv_set(void *client, void *priv)
{
	libwebsock_client_state *c = client;
	return_val_if_fail(c && priv, -1);

	c->priv = priv;

	return 0;
}

int tima_websock_send_text(void *client, char *text)
{
	return libwebsock_send_text(client, text);
}

int tima_websock_send_binary(void *client, char *data, unsigned int length)
{
	return libwebsock_send_binary(client, data, length);
}

relay_wserver_t* tima_websock_get_relay_server(void *client)
{
	libwebsock_client_state *c = client;
	return_val_if_fail(c, NULL);

	return c->server;
}
