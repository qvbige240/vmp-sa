/**
 * History:
 * ================================================================
 * 2019-06-27 qing.zou created
 *
 */

#include "tima_typedef.h"
#include "tima_ioamaps.h"


RelaySocketIO* tima_ioamaps_put(vmp_maps_t *vm, const char *key, void *s, SockMapsType type)
{
	char ret = -1;
	RelaySocketIO* value = NULL;
	return_val_if_fail(vm && vm->kh && key, NULL);

	VMP_MUTEX_LOCK(vm->mutex_lock, 0);

	value = vpk_hash_get(vm->kh, key);
	if (value) {
		if (value->flag & type) {
			VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);
			return value;
		} else {
			value->sock[type-MAPS_SOCK_STARTBIT] = s;
			value->flag |= type;
		}
	} else {
		value = calloc(1, sizeof(RelaySocketIO));
		if (!value) {
			VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);
			return NULL;
		}
		memcpy(value->sim, key, sizeof(value->sim));
		value->sock[type-MAPS_SOCK_STARTBIT] = s;
		value->flag |= type;
	}
	ret = vpk_hash_set(vm->kh, key, value);

	VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);

	if (ret < 0) {
		free(value);
		value = NULL;
	}

	return value;
}

RelaySocketIO* tima_ioamaps_get(vmp_maps_t *vm, const char *key)
{
	RelaySocketIO* value = NULL;
	return_val_if_fail(vm && vm->kh && key, NULL);

	VMP_MUTEX_LOCK(vm->mutex_lock, 0);

	value = vpk_hash_get(vm->kh, key);

	VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);

	return value;
}

void* tima_ioamaps_get_type(vmp_maps_t *vm, const char *key, SockMapsType type)
{
	void *s = NULL;
	RelaySocketIO* value = NULL;
	return_val_if_fail(vm && vm->kh && key, NULL);

	VMP_MUTEX_LOCK(vm->mutex_lock, 0);

	value = vpk_hash_get(vm->kh, key);

	VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);

	if ( value && (value->flag & type)) {
		s = value->sock[type-MAPS_SOCK_STARTBIT];
	}

	return s;
}

void* tima_ioamaps_exist(RelaySocketIO *value, SockMapsType type)
{
	void *s = NULL;
	return_val_if_fail(value && type > 0, NULL);

	if (value->flag & type) {
		s = value->sock[type-MAPS_SOCK_STARTBIT];
	}

	return s;
}
