
#ifndef CACHE_TIMA_H
#define CACHE_TIMA_H

#include "cache_typedef.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CACHE_TIMA_CLASS	FOURCCLE('C','T','I','M')


typedef struct _TimaNetworkConfig
{
	char	http_ip[64];
	char	http_port[16];

	char ss_ip[64];
	char ss_rtmp_port[16];
	char ss_http_port[16];
} CacheNetworkConfig;


void cache_tima_init(void);
void cache_tima_done(void);


#ifdef __cplusplus
}
#endif

#endif //CACHE_TIMA_H
