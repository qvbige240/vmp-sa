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

#ifdef __cplusplus
extern "C"
{
#endif



typedef void (*TimaHttpCB)(TimaHttpRsp* rsp);

int tima_http_post(void *uri, void *post_data,	void *node, TimaHttpCB callback, int retry, int *task_id);


#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // TIMA_HTTP_H
