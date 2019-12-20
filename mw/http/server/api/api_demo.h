/**
 * History:
 * ================================================================
 * 2019-12-16 qing.zou created
 *
 */

#ifndef API_DEMO_H
#define API_DEMO_H

#include "api_typedef.h"

VMP_BEGIN_DELS

typedef struct _ApiDemoReq
{
    unsigned int        id;
} ApiDemoReq;

typedef struct _ApiDemoRsp
{
    unsigned int        id;
    char                name[64];
    unsigned int        count;
} ApiDemoRsp;


VMP_END_DELS

#endif // API_DEMO_H
