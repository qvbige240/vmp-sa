/**
 * History:
 * ================================================================
 * 2018-05-22 qing.zou created
 *
 */

#ifndef BLL_WEBSOCK_IOA_H
#define BLL_WEBSOCK_IOA_H

#include "bll_typedef.h"
#include "tima_server.h"

TIMA_BEGIN_DELS

#define BLL_WEBSOCK_IOA_CLASS		FOURCCLE('B','L','W','I')


typedef struct _WebsockIOAReq
{
	unsigned long		flowid;
	void				*client;
	void				*ws;
} WebsockIOAReq;

typedef struct _WebsockIOARep
{
	
} WebsockIOARep;

void bll_websockioa_init(void);
void bll_websockioa_done(void);

TIMA_END_DELS

#endif // BLL_WEBSOCK_IOA_H
