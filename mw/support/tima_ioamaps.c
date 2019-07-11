/**
 * History:
 * ================================================================
 * 2019-06-27 qing.zou created
 *
 */

#include "tima_typedef.h"
#include "tima_ioamaps.h"

static int ioamaps_flush(SockHashValue* value, former_sock_proc proc)
{
	if (!value) {
		return -1;
	}

	if (value->flag & MAPS_SOCK_STREAM) {
		if (proc) proc(&value->data[MAPS_SOCK_STREAM-MAPS_SOCK_STARTBIT]);

		memset(&value->data[MAPS_SOCK_STREAM-MAPS_SOCK_STARTBIT], 0x00, sizeof(VoiceSockData));
		value->flag &= ~MAPS_SOCK_STREAM;
	}

	if (value->flag & MAPS_SOCK_WEBSKT) {
		if (proc) proc(&value->data[MAPS_SOCK_WEBSKT-MAPS_SOCK_STARTBIT]);

		memset(&value->data[MAPS_SOCK_WEBSKT-MAPS_SOCK_STARTBIT], 0x00, sizeof(VoiceSockData));
		value->flag &= ~MAPS_SOCK_WEBSKT;
	}

	return 0;
}

SockHashValue* tima_ioamaps_put(vmp_maps_t *vm, const char *key, VoiceSockData *data, SockMapsType type, former_sock_proc proc)
{
	int ret = -1, print = 0;
	SockHashValue* value = NULL;
	return_val_if_fail(vm && vm->kh && key, NULL);

	VMP_MUTEX_LOCK(vm->mutex_lock, 0);

	value = vpk_hash_get(vm->kh, key);
	if (value) {
		if (value->flag & type) {
			ioamaps_flush(value, proc);
			print = 1;
			//VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);
			//return value;
		} /*else {
			//value->data[type-MAPS_SOCK_STARTBIT].sock = data->sock;
			//value->data[type-MAPS_SOCK_STARTBIT].job  = data->job;
			memcpy(&value->data[type-MAPS_SOCK_STARTBIT], data, sizeof(VoiceSockData));
			value->flag |= type;
		}*/
		memcpy(&value->data[type-MAPS_SOCK_STARTBIT], data, sizeof(VoiceSockData));
		value->flag |= type;

	} else {
		value = calloc(1, sizeof(SockHashValue));
		if (!value) {
			VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);
			return NULL;
		}
		memcpy(value->sim, key, sizeof(value->sim));
		//value->sock[type-MAPS_SOCK_STARTBIT] = s;
		memcpy(&value->data[type-MAPS_SOCK_STARTBIT], data, sizeof(VoiceSockData));
		value->flag |= type;
	}
	ret = vpk_hash_set(vm->kh, key, value);

	VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);

	if (ret < 0) {
		free(value);
		value = NULL;
	}
	
	if (print)
		TIMA_LOGW("the key[%s] have been connected and new connection will replace it\n", key);

	return value;
}

SockHashValue* tima_ioamaps_get(vmp_maps_t *vm, const char *key)
{
	SockHashValue* value = NULL;
	return_val_if_fail(vm && vm->kh && key, NULL);

	VMP_MUTEX_LOCK(vm->mutex_lock, 0);

	value = vpk_hash_get(vm->kh, key);

	VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);

	return value;
}

VoiceSockData tima_ioamaps_get_data(vmp_maps_t *vm, const char *key, SockMapsType type)
{
	VoiceSockData data = {0};
	SockHashValue* value = NULL;
	return_val_if_fail(vm && vm->kh && key, data);

	VMP_MUTEX_LOCK(vm->mutex_lock, 0);

	value = vpk_hash_get(vm->kh, key);

	VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);

	if ( value && (value->flag & type)) {
		return value->data[type-MAPS_SOCK_STARTBIT];
	}

	return data;
}

//void* tima_ioamaps_get_type(vmp_maps_t *vm, const char *key, SockMapsType type)
//{
//	void *s = NULL;
//	SockHashValue* value = NULL;
//	return_val_if_fail(vm && vm->kh && key, NULL);
//
//	VMP_MUTEX_LOCK(vm->mutex_lock, 0);
//
//	value = vpk_hash_get(vm->kh, key);
//
//	VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);
//
//	if ( value && (value->flag & type)) {
//		s = value->sock[type-MAPS_SOCK_STARTBIT];
//	}
//
//	return s;
//}

void* tima_ioamaps_exist(SockHashValue *value, SockMapsType type)
{
	void *s = NULL;
	return_val_if_fail(value && type > 0, NULL);

	if (value->flag & type) {
		s = value->data[type-MAPS_SOCK_STARTBIT].sock;
	}

	return s;
}

int tima_ioamaps_delete(vmp_maps_t *vm, const char *key)
{
	SockHashValue* value = NULL;
	return_val_if_fail(vm && vm->kh && key, -1);

	VMP_MUTEX_LOCK(vm->mutex_lock, 0);

	value = vpk_hash_get_and_del(vm->kh, key);

	VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);

	if (value) {
		free(value);
	}
	return 0;
}

int tima_ioamaps_clear(vmp_maps_t *vm, const char *key, SockMapsType type)
{
	char ret = -1;
	SockHashValue* value = NULL;
	return_val_if_fail(vm && vm->kh && key, -1);

	VMP_MUTEX_LOCK(vm->mutex_lock, 0);

	value = vpk_hash_get(vm->kh, key);
	if (value) {
		if (value->flag & type) {
			//value->sock[type-MAPS_SOCK_STARTBIT] = NULL;
			memset(&value->data[type-MAPS_SOCK_STARTBIT], 0x0, sizeof(VoiceSockData));
			value->flag &= ~type;
		}

		if (!value->flag) {
			vpk_hash_del(vm->kh, key);
			VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);
			free(value);
			return 0;
		}

		ret = vpk_hash_set(vm->kh, key, value);
	}

	VMP_MUTEX_UNLOCK(vm->mutex_lock, 0);

	if (ret < 0 && value) {
		free(value);
		value = NULL;
	}

	return 0;
}
