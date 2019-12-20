/**
 * History:
 * ================================================================
 * 2019-12-16 qing.zou created
 *
 */

#ifndef API_TYPEDEF_H
#define API_TYPEDEF_H

#include "vmp.h"
//#include "module_support.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*api_service_func)(void *p, void *req, void *rsp);

typedef struct service_handler_s
{
    void               *ctx;
    api_service_func    pfn_callback;
} service_handler_t;

#ifdef __cplusplus
}
#endif

#endif // API_TYPEDEF_H
