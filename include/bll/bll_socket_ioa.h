/**
 * History:
 * ================================================================
 * 2019-05-29 qing.zou created
 *
 */

#ifndef BLL_SOCKETIOA_H
#define BLL_SOCKETIOA_H

#include "bll_typedef.h"
#include "tima_server.h"

TIMA_BEGIN_DELS

#define BLL_SOCKETIOA_CLASS		FOURCCLE('B','L','S','I')

#define VOI_BUFFEREVENT_LOW_WATERMARK	(200)
#define VOI_BUFFEREVENT_HIGH_WATERMARK	(826)

typedef struct _SocketIOAReq
{
	unsigned long		flowid;
	vmp_launcher_t		*e;
	vmp_socket_t		client;

	void				*ss;
} SocketIOAReq;

typedef struct SocketIOARep
{
	
} SocketIOARep;

void bll_sockioa_init(void);
void bll_sockioa_done(void);


TIMA_END_DELS

#endif // BLL_SOCKETIOA_H
