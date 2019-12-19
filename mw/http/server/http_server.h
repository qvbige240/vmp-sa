/**
 * History:
 * ================================================================
 * 2019-12-16 qing.zou created
 *
 */

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "vmp.h"

VMP_BEGIN_DELS

typedef int (*web_register_func)(void *p, int msg, void *arg);

typedef struct _HttpServerReq
{
    unsigned short          port;

    web_register_func       func;

    void                    *ctx;
    vmp_callback_func       pfncb;
} HttpServerReq;

typedef struct _HttpServerRsp
{
    void                    *web;
} HttpServerRsp;

void *http_server_create(void *parent, HttpServerReq *req);
int http_server_start(void *p);

VMP_END_DELS

#endif // HTTP_SERVER_H
