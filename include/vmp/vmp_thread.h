/**
 * History:
 * ================================================================
 * 2019-06-20 qing.zou created
 *
 */

#ifndef VMP_THREAD_H
#define VMP_THREAD_H


#include "vpk.h"
#include "vmp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/** A flag passed to a locking callback when the lock was allocated as a
 * read-write lock, and we want to acquire or release the lock for writing. */
#define VMP_THREAD_WRITE	0x04
/** A flag passed to a locking callback when the lock was allocated as a
 * read-write lock, and we want to acquire or release the lock for reading. */
#define VMP_THREAD_READ		0x08
/** A flag passed to a locking callback when we don't want to block waiting
 * for the lock; if we can't get the lock immediately, we will instead
 * return nonzero from the locking callback. */
#define VMP_THREAD_TRY		0x10


#define VMP_THREAD_LOCK_API_VERSION		1
/** A recursive lock is one that can be acquired multiple times at once by the
 * same thread.  No other process can allocate the lock until the thread that
 * has been holding it has unlocked it as many times as it locked it. */
#define VMP_THREAD_LOCKTYPE_RECURSIVE	1
/* A read-write lock is one that allows multiple simultaneous readers, but
 * where any one writer excludes all other writers and readers. */
#define VMP_THREAD_LOCKTYPE_READWRITE	2


struct vmp_thread_lock_callbacks {
	/** The current version of the locking API.  Set this to
	 * VMP_THREAD_LOCK_API_VERSION */
	int lock_api_version;
	/** Which kinds of locks does this version of the locking API
	 * support?  A bitfield of VMP_THREAD_LOCKTYPE_RECURSIVE and
	 * VMP_THREAD_LOCKTYPE_READWRITE.
	 *
	 * (Note that RECURSIVE locks are currently mandatory, and
	 * READWRITE locks are not currently used.)
	 **/
	unsigned supported_locktypes;
	/** Function to allocate and initialize new lock of type 'locktype'.
	 * Returns NULL on failure. */
	void *(*alloc)(unsigned locktype);
	/** Funtion to release all storage held in 'lock', which was created
	 * with type 'locktype'. */
	void (*free)(void *lock, unsigned locktype);
	/** Acquire an already-allocated lock at 'lock' with mode 'mode'.
	 * Returns 0 on success, and nonzero on failure. */
	int (*lock)(unsigned mode, void *lock);
	/** Release a lock at 'lock' using mode 'mode'.  Returns 0 on success,
	 * and nonzero on failure. */
	int (*unlock)(unsigned mode, void *lock);
};

#define VMP_THREAD_CONDITION_API_VERSION 1

struct vmp_thread_condition_callbacks {
	/** The current version of the conditions API.  Set this to
	 * VMP_THREAD_CONDITION_API_VERSION */
	int condition_api_version;
	/** Function to allocate and initialize a new condition variable.
	 * Returns the condition variable on success, and NULL on failure.
	 * The 'condtype' argument will be 0 with this API version.
	 */
	void *(*alloc_condition)(unsigned condtype);
	/** Function to free a condition variable. */
	void (*free_condition)(void *cond);
	/** Function to signal a condition variable.  If 'broadcast' is 1, all
	 * threads waiting on 'cond' should be woken; otherwise, only on one
	 * thread is worken.  Should return 0 on success, -1 on failure.
	 * This function will only be called while holding the associated
	 * lock for the condition.
	 */
	int (*signal_condition)(void *cond, int broadcast);
	/** Function to wait for a condition variable.  The lock 'lock'
	 * will be held when this function is called; should be released
	 * while waiting for the condition to be come signalled, and
	 * should be held again when this function returns.
	 * If timeout is provided, it is interval of seconds to wait for
	 * the event to become signalled; if it is NULL, the function
	 * should wait indefinitely.
	 *
	 * The function should return -1 on error; 0 if the condition
	 * was signalled, or 1 on a timeout. */
	int (*wait_condition)(void *cond, void *lock, const struct timeval *timeout);
};

int vmp_use_pthreads_init(void);

unsigned long vmp_thread_get_id();
void *vmp_thread_lock_alloc(unsigned locktype);
void vmp_thread_lock_free(void *lock, unsigned locktype);
int vmp_thread_lock_lock(unsigned mode, void *lock);
int vmp_thread_lock_unlock(unsigned mode, void *lock);

void *vmp_thread_cond_alloc(unsigned condtype);
void vmp_thread_cond_free(void *cond);
int vmp_thread_cond_signal(void *cond, int broadcast);
int vmp_thread_cond_wait(void *cond, void *lock, const struct timeval *tv);
int vmp_thread_locking_enabled(void);


#define VMP_THREAD_ALLOC_LOCK(lockvar, locktype)		\
	((lockvar) = vmp_thread_lock_alloc(locktype))

#define VMP_THREAD_FREE_LOCK(lockvar, locktype)			\
	do {								\
		void *_lock_tmp_ = (lockvar);				\
		if (_lock_tmp_)						\
			vmp_thread_lock_free(_lock_tmp_, (locktype)); \
	} while (0)

/** Acquire a lock. */
#define VMP_MUTEX_LOCK(lockvar, mode)					\
	do {								\
		if (lockvar)						\
			vmp_thread_lock_lock(mode, lockvar);		\
	} while (0)

/** Release a lock */
#define VMP_MUTEX_UNLOCK(lockvar, mode)					\
	do {								\
		if (lockvar)						\
			vmp_thread_lock_unlock(mode, lockvar);	\
	} while (0)

#if 0
/** Lock an event base. */
#define VMP_EVACQUIRE_LOCK(base, lockvar) do {				\
	VMP_MUTEX_LOCK((base)->lockvar, 0);			\
	VMP_LOGD(("%s[%d]: id[%lu]  lock ====", __FILE__, __LINE__, vmp_thread_get_id()));          \
} while (0)

/** Unlock an event base. */
#define VMP_EVRELEASE_LOCK(base, lockvar) do {				\
	VMP_LOGD(("%s[%d]: id[%lu] unlock ====\n", __FILE__, __LINE__, vmp_thread_get_id()));        \
	VMP_MUTEX_UNLOCK((base)->lockvar, 0);			\
} while (0)
#endif

void *vmp_thread_setup_global_lock(void *lock, unsigned locktype, int enable_locks);

#define VMP_THREAD_SETUP_GLOBAL_LOCK(lockvar, locktype)			\
	do {								\
		lockvar = vmp_thread_setup_global_lock(lockvar, (locktype), 1); \
		if (!lockvar) {						\
			VMP_LOGW("Couldn't allocate lock %s", #lockvar);	\
			return -1;					\
		}							\
	} while (0)


#ifdef __cplusplus
}
#endif

#endif //VMP_THREAD_H
