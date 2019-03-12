/**
 * History:
 * ================================================================
 * 2019-03-01 qing.zou created
 *
 */
#ifndef BLL_TYPEDEF_H
#define BLL_TYPEDEF_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vmp.h"
#include "tima_support.h"

#ifndef TIMA_BEGIN_DELS
  #ifdef __cplusplus
	#define TIMA_BEGIN_DELS extern "C" {
	#define TIMA_END_DELS }
  #else
	#define TIMA_BEGIN_DELS
	#define TIMA_END_DELS
  #endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif


#ifdef __cplusplus
}
#endif

#endif // BLL_TYPEDEF_H
