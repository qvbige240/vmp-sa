/**
 * History:
 * ================================================================
 * 2019-03-29 qing.zou created
 *
 */
#ifndef TIMA_HTTP_H
#define TIMA_HTTP_H

#include "tima_httpdef.h"

#pragma pack(1)

TIMA_BEGIN_DELS

typedef void (*TimaHttpCB)(TimaHttpRsp* rsp);

int tima_http_post(void *uri, void *post_data,	void *node, TimaHttpCB callback, int retry, int *task_id);


#pragma pack()

TIMA_END_DELS

#endif // TIMA_HTTP_H
