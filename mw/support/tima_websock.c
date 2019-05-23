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

int tima_websock_callback_set(void *client, TimaWebsockFunc *cb)
{
	libwebsock_client_state *c = client;
	return_val_if_fail(c && cb, -1);

	c->onmessage	= cb->onmessage;
	c->onclose		= cb->onclose;
	c->onpong		= cb->onpong;

	return 0;
}
