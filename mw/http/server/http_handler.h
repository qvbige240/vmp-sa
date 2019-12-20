/**
 * History:
 * ================================================================
 * 2019-12-19 qing.zou created
 *
 */

#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include "vmp.h"

VMP_BEGIN_DELS

#define HTTP_URL_DEMO       "/carnet/sr/tg/demo"    // http://localhost:9090/carnet/sr/tg/demo


typedef struct http_handler_s
{
    struct http_handler_s   *sc_next;
    char                    *sc_urls;
    void                    *sc_args;
    void                    *handler;
} http_handler_t;

int error_json_create(int ecode, char **data);

int http_handler_register(void *http, char *url, void *handler, void *args);

VMP_END_DELS

#endif // HTTP_HANDLER_H
