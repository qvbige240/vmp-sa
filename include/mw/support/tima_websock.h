/**
 * History:
 * ================================================================
 * 2019-05-21 qing.zou created
 *
 */

#ifndef TIMA_WEBSOCK_H
#define TIMA_WEBSOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <event2/event.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct relay_wserver_s
{
	unsigned char			id;

	struct event_base		*event_base;

	pthread_t				pth_id;
} relay_wserver_t;


typedef struct tima_wsmessage {
	unsigned int			opcode;
	unsigned long long		payload_len;
	char					*payload;
} tima_wsmessage_t;

typedef struct TimaWebsockFunc
{
	int (*onmessage)(void *client, tima_wsmessage_t *msg);
	//int (*onmessage)(void *client, void *msg);
	//int (*onopen)(void *client);
	int (*onclose)(void *client);
	int (*onpong)(void *client);
} TimaWebsockFunc;

//TimaWebsock* tima_websock_init();

int tima_websock_fd_get(void *client);

void* tima_websock_priv_get(void *client);

int tima_websock_callback_set(void *client, TimaWebsockFunc *cb);

int tima_websock_priv_set(void *client, void *priv);

int tima_websock_send_text(void *client, char *text);

int tima_websock_send_binary(void *client, char *data, unsigned int length);

int tima_websock_close(void *client);

relay_wserver_t* tima_websock_get_relay_server(void *client);


#ifdef __cplusplus
}
#endif

#endif // TIMA_WEBSOCK_H
