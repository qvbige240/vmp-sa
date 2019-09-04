/**
 * History:
 * ================================================================
 * 2019-08-15 qing.zou created
 *
 */
#ifndef VMP_TYPEDEF_H
#define VMP_TYPEDEF_H

#include "vpk.h"

#define VMP_BEGIN_DELS  VPK_BEGIN_DELS
#define VMP_END_DELS    VPK_END_DELS

typedef int (*vmp_callback_func)(void* p, int msg, void* arg);

#endif // VMP_TYPEDEF_H
