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
#include "tima_typedef.h"
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

#define JT1078_STREAM_PACKAGE_SIZE		980

typedef enum SockMatchState
{
	SOCK_MATCH_STATE_INIT	= 0x00,
	SOCK_MATCH_STATE_PUT	= 0X01,
	SOCK_MATCH_STATE_GET,
	SOCK_MATCH_STATE_SUCCESS,
	SOCK_MATCH_STATE_DISCONN,
	SOCK_MATCH_STATE_ERROR,
} SockMatchState;


#endif // BLL_TYPEDEF_H
