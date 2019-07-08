/**
 * History:
 * ================================================================
 * 2019-03-01 qing.zou created
 *
 */
#ifndef TIMA_TYPEDEF_H
#define TIMA_TYPEDEF_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vmp.h"

#ifndef TIMA_BEGIN_DELS
  #ifdef __cplusplus
	#define TIMA_BEGIN_DELS extern "C" {
	#define TIMA_END_DELS }
  #else
	#define TIMA_BEGIN_DELS
	#define TIMA_END_DELS
  #endif
#endif


#ifndef TIMA_LOGI
	#define TIMA_LOGD		VMP_LOGD
	#define TIMA_LOGI		VMP_LOGI
	#define TIMA_LOGW		VMP_LOGW
	#define TIMA_LOGE		VMP_LOGE
	#define TIMA_LOGF		VMP_LOGF
#else
	//#define TIMA_LOGD		printf
	//#define TIMA_LOGI		printf
	//#define TIMA_LOGW		printf
	//#define TIMA_LOGE		printf
	//#define TIMA_LOGF		printf
#endif


#define NODE_CLASS_REGISTER(nodeinfo) \
	do {		\
		context* ctx = context_get();	\
		node_register_class(&(nodeinfo), (void*)ctx->vector_node);  \
	} while(0)

#define NODE_CLASS_UNREGISTER(nclass) \
	do {		\
		context* ctx = context_get();	\
		node_unregister_class(nclass, (void*)ctx->vector_node);  \
	} while(0)


#define bs_server_object()		\
	char				cmsg[VPK_CMSG_MAX_SIZE+1];		\
	vmp_maps_t			*map;		\
	vmp_ports_t			*porter;		/* allocate ports for relay sock */		\
	vpk_sockaddr		local_addr


#endif // TIMA_TYPEDEF_H
