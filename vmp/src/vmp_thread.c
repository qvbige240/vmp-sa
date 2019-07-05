/**
 * History:
 * ================================================================
 * 2019-06-20 qing.zou created
 *
 */
 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

#include "vmp_thread.h"

static pthread_mutexattr_t attr_recursive;

static void *vpk_thread_lock_alloc(unsigned locktype)
{
	pthread_mutexattr_t *attr = NULL;
	pthread_mutex_t *lock = VPK_MALLOC(sizeof(pthread_mutex_t));
	if (!lock) {
		VMP_LOGE("lock malloc error.");
		return NULL;
	}
	if (locktype & VMP_THREAD_LOCKTYPE_RECURSIVE)
		attr = &attr_recursive;
	if (pthread_mutex_init(lock, attr)) {
		VMP_LOGE("vpk_thread_lock init error.");
		VPK_FREE(lock);
		return NULL;
	}
	return lock;
}

static void vpk_thread_lock_free(void *_lock, unsigned locktype)
{
	pthread_mutex_t *lock = _lock;
	pthread_mutex_destroy(lock);
	VPK_FREE(lock);
}

static int vpk_thread_lock(unsigned mode, void *_lock)
{
	pthread_mutex_t *lock = _lock;
	if (mode & VMP_THREAD_TRY)
		return pthread_mutex_trylock(lock);
	else
		return pthread_mutex_lock(lock);
}

static int vpk_thread_unlock(unsigned mode, void *_lock)
{
	pthread_mutex_t *lock = _lock;
	return pthread_mutex_unlock(lock);
}

//static unsigned long vpk_thread_get_id(void)
//{
//	union {
//		pthread_t		thr;
//		unsigned long	id;
//	} r;
//	memset(&r, 0, sizeof(r));
//	r.thr = pthread_self();
//	return (unsigned long)r.id;
//}

/** cond **/
static void *vpk_thread_cond_alloc(unsigned condflags)
{
	pthread_cond_t *cond = VPK_MALLOC(sizeof(pthread_cond_t));
	if (!cond) {
		VMP_LOGE("cond malloc error.");
		return NULL;
	}
	if (pthread_cond_init(cond, NULL)) {
		VMP_LOGE("vpk_thread_cond init error.");
		VPK_FREE(cond);
		return NULL;
	}
	return cond;
}

static void vpk_thread_cond_free(void *_cond)
{
	pthread_cond_t *cond = _cond;
	pthread_cond_destroy(cond);
	VPK_FREE(cond);
}

static int vpk_thread_cond_signal(void *_cond, int broadcast)
{
	pthread_cond_t *cond = _cond;
	int r;
	if (broadcast)
		r = pthread_cond_broadcast(cond);
	else
		r = pthread_cond_signal(cond);
	return r ? -1 : 0;
}

static int vpk_thread_cond_wait(void *_cond, void *_lock, const struct timeval *tv)
{
	int r;
	pthread_cond_t *cond = _cond;
	pthread_mutex_t *lock = _lock;

	if (tv) {
		struct timeval now, abstime;
		struct timespec ts;
		vpk_gettimeofday(&now, NULL);
		vpk_timeradd(&now, tv, &abstime);
		ts.tv_sec = abstime.tv_sec;
		ts.tv_nsec = abstime.tv_usec * 1000;
		r = pthread_cond_timedwait(cond, lock, &ts);
		if (r == ETIMEDOUT)
			return 1;
		else if (r)
			return -1;
		else
			return 0;
	} else {
		r = pthread_cond_wait(cond, lock);
		return r ? -1 : 0;
	}
}

static const struct vmp_thread_lock_callbacks threads_lock_ops = {
	VMP_THREAD_LOCK_API_VERSION,
	VMP_THREAD_LOCKTYPE_RECURSIVE,
	vpk_thread_lock_alloc,
	vpk_thread_lock_free,
	vpk_thread_lock,
	vpk_thread_unlock,
};

static const struct vmp_thread_condition_callbacks threads_cond_ops = {
	VMP_THREAD_CONDITION_API_VERSION,
	vpk_thread_cond_alloc,
	vpk_thread_cond_free,
	vpk_thread_cond_signal,
	vpk_thread_cond_wait,
};

//static void vpk_thread_set_id_callback(unsigned long (*id_fn)(void));
//static int vpk_thread_set_lock_callbacks(const struct vpk_thread_lock_callbacks *cbs);
//static int vpk_thread_set_condition_callbacks(const struct vpk_thread_condition_callbacks *cbs);

int vmp_use_pthreads_init(void)
{
	//struct vpk_thread_lock_callbacks cbs = {
	//	VMP_THREAD_LOCK_API_VERSION,
	//	VMP_THREAD_LOCKTYPE_RECURSIVE,
	//	vpk_thread_lock_alloc,
	//	vpk_thread_lock_free,
	//	vpk_thread_lock,
	//	vpk_thread_unlock,
	//};
	//struct vpk_thread_condition_callbacks cond_cbs = {
	//	VMP_THREAD_CONDITION_API_VERSION,
	//	vpk_thread_cond_alloc,
	//	vpk_thread_cond_free,
	//	vpk_thread_cond_signal,
	//	vpk_thread_cond_wait,
	//};

	if (pthread_mutexattr_init(&attr_recursive)) {
		VMP_LOGE("pthread_mutexattr_init error.");
		return -1;
	}
	if (pthread_mutexattr_settype(&attr_recursive, PTHREAD_MUTEX_RECURSIVE)) {
		VMP_LOGE("pthread_mutexattr_settype error.");
		return -1;
	}

	//vpk_thread_set_lock_callbacks(&cbs);
	//vpk_thread_set_condition_callbacks(&cond_cbs);
	//vpk_thread_set_id_callback(vpk_thread_get_id);

	return 0;
}

void *vmp_thread_setup_global_lock(void *lock, unsigned locktype, int enable_locks)
{
	if (enable_locks) {
		return_val_if_fail(lock == NULL, lock);
		return threads_lock_ops.alloc(locktype);
	} else {
		return lock;
	}
}

unsigned long vmp_thread_get_id(void)
{
	union {
		pthread_t		thr;
		unsigned long	id;
	} r;
	memset(&r, 0, sizeof(r));
	r.thr = pthread_self();
	return (unsigned long)r.id;
}

void *vmp_thread_lock_alloc(unsigned locktype)
{
	return threads_lock_ops.alloc ? threads_lock_ops.alloc(locktype) : NULL;
}

void vmp_thread_lock_free(void *lock, unsigned locktype)
{
	if (threads_lock_ops.free)
		threads_lock_ops.free(lock, locktype);
}

int vmp_thread_lock_lock(unsigned mode, void *lock)
{
	if (threads_lock_ops.lock)
		return threads_lock_ops.lock(mode, lock);
	else
		return 0;
}

int vmp_thread_lock_unlock(unsigned mode, void *lock)
{
	if (threads_lock_ops.unlock)
		return threads_lock_ops.unlock(mode, lock);
	else
		return 0;
}

void *vmp_thread_cond_alloc(unsigned condtype)
{
	if (threads_cond_ops.alloc_condition)
		return threads_cond_ops.alloc_condition(condtype);
	else
		return NULL;
}

void vmp_thread_cond_free(void *cond)
{
	if (threads_cond_ops.free_condition)
		threads_cond_ops.free_condition(cond);
}

int vmp_thread_cond_signal(void *cond, int broadcast)
{
	if (threads_cond_ops.signal_condition)
		return threads_cond_ops.signal_condition(cond, broadcast);
	else
		return 0;
}

int vmp_thread_cond_wait(void *cond, void *lock, const struct timeval *tv)
{
	if (threads_cond_ops.wait_condition)
		return threads_cond_ops.wait_condition(cond, lock, tv);
	else
		return 0;
}

int vmp_thread_locking_enabled(void)
{
	return threads_lock_ops.lock != NULL;
}
