/**
 * History:
 * ================================================================
 * 2019-06-27 qing.zou created
 *
 */

#ifndef TIMA_IOAMAPS_H
#define TIMA_IOAMAPS_H


#include "vmp.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef enum SockMapsType
{
	MAPS_SOCK_STARTBIT	= 0x01,
	MAPS_SOCK_STREAM	= MAPS_SOCK_STARTBIT,
	MAPS_SOCK_WEBSKT	= 0x02,
} SockMapsType;

typedef struct RelaySocketIO
{
	unsigned char	flag;		/** bit0: sock stream, bit1: websock **/
	unsigned char	sim[6];
	void			*sock[2];
} RelaySocketIO;

static INLINE vmp_maps_t* tima_ioamaps_create(int bucket)
{
	return vmp_maps_create(bucket);
}
static INLINE void tima_ioamaps_destroy(vmp_maps_t *vm)
{
	vmp_maps_destroy(vm);
}

RelaySocketIO* tima_ioamaps_put(vmp_maps_t *vm, const char *key, void *s, SockMapsType type);
RelaySocketIO* tima_ioamaps_get(vmp_maps_t *vm, const char *key);
void* tima_ioamaps_get_type(vmp_maps_t *vm, const char *key, SockMapsType type);
void* tima_ioamaps_exist(RelaySocketIO *value, SockMapsType type);

int tima_ioamaps_clear(vmp_maps_t *vm, const char *key, SockMapsType type);
int tima_ioamaps_delete(vmp_maps_t *vm, const char *key);

#ifdef __cplusplus
}
#endif

#endif // TIMA_IOAMAPS_H
