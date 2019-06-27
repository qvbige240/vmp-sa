/**
 * History:
 * ================================================================
 * 2019-06-26 qing.zou created
 *
 */

#ifndef VMP_MAPS_H
#define VMP_MAPS_H

#include "vpk.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct vmp_maps_s
{
	vpk_hash_t		*kh;
		void			*mutex_lock;
} vmp_maps_t;


vmp_maps_t* vmp_maps_create(int bucket);
void vmp_maps_destroy(vmp_maps_t *vm);
int vmp_map_put(vmp_maps_t *vm, const char *key, void *val);
void* vmp_map_get(vmp_maps_t *vm, const char *key);
void* vmp_map_delkey(vmp_maps_t *vm, const char *key);

#ifdef __cplusplus
}
#endif

#endif //VMP_MAPS_H
