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
#include <stdarg.h>

//#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

//typedef struct _TimaWebsock
//{
//	libwebsock_context*		websock;
//
//} TimaWebsock;

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
int tima_websock_callback_set(void *client, TimaWebsockFunc *cb);

int tima_websock_send_text(void *client, char *text);

//#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // TIMA_WEBSOCK_H
