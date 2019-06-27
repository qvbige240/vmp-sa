/**
 * History:
 * ================================================================
 * 2019-06-26 qing.zou created
 *
 */

#include "vmp_maps.h"

#include "vmp_log.h"
#include "vmp_thread.h"


vmp_maps_t* vmp_maps_create(int bucket)
{
	vmp_maps_t* vm = VPK_CALLOC(1, sizeof(vmp_maps_t));
	if (vm)
	{
		//VMP_THREAD_SETUP_GLOBAL_LOCK(vm->mutex_lock, VMP_THREAD_LOCKTYPE_RECURSIVE);
		vm->mutex_lock = vmp_thread_setup_global_lock(vm->mutex_lock, VMP_THREAD_LOCKTYPE_RECURSIVE, 1);
		if (vm->mutex_lock == NULL) {
			VMP_LOGE("maps mutex_lock create failed");
			VPK_FREE(vm);
			return NULL;
		}

		vm->kh = vpk_hash_create(bucket);
		if (vm->kh == NULL) {
			VMP_LOGE("vpk_hash_create failed");
			vmp_thread_lock_free(vm->mutex_lock, VMP_THREAD_LOCKTYPE_RECURSIVE);
			VPK_FREE(vm);
			return NULL;
		}
	}

	return vm;
}

void vmp_maps_destroy(vmp_maps_t *vm)
{
	if (vm && vm->mutex_lock)
	{
		VMP_MUTEX_LOCK(vm->mutex_lock, 0);

		if (vm->kh)
			vpk_hash_destroy(vm->kh);

		VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);

		vmp_thread_lock_free(vm->mutex_lock, VMP_THREAD_LOCKTYPE_RECURSIVE);
		vm->mutex_lock = NULL;
		VPK_FREE(vm);
	}
}

int vmp_map_put(vmp_maps_t *vm, const char *key, void *val)
{
	int ret = -1;
	return_val_if_fail(vm && vm->kh && key, -1);

	VMP_MUTEX_LOCK(vm->mutex_lock, 0);

	ret = vpk_hash_set(vm->kh, key, val);

	VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);

	return ret;
}

void* vmp_map_get(vmp_maps_t *vm, const char *key)
{
	void* value;
	return_val_if_fail(vm && vm->kh && key, NULL);

	VMP_MUTEX_LOCK(vm->mutex_lock, 0);

	value = vpk_hash_get(vm->kh, key);

	VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);

	return value;
}

//int vmp_map_exist(vmp_maps_t *vm, const char *key)
//{
//	void* value;
//	return_val_if_fail(vm && vm->kh && key, -1);
//
//	VMP_MUTEX_LOCK(vm->mutex_lock, 0);
//
//	value = vpk_hash_get(vm->kh, key);
//
//	VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);
//
//	return value ? 1 : 0;
//}

void* vmp_map_delkey(vmp_maps_t *vm, const char *key)
{
	void* value;
	return_val_if_fail(vm && vm->kh && key, NULL);

	VMP_MUTEX_LOCK(vm->mutex_lock, 0);

	value = vpk_hash_get_and_del(vm->kh, key);

	VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);

	return value;
}
