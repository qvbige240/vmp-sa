
#ifndef CONFIG_H
#define CONFIG_H

#include "cache_tima.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CACHE_CLASS		FOURCCLE('C','A','C','H')


void cache_init(void);
void cache_done(void);


#ifdef __cplusplus
}
#endif

#endif //CONFIG_H
