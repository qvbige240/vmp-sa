/**
 * History:
 * ================================================================
 * 2019-08-16 qing.zou created
 *
 */

#ifndef VMP_SHARED_H
#define VMP_SHARED_H

#include "vmp_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif


#define IPC_PATH			"./ipc"

#define IPC_DEVSHM_PROJID		0x10
#define IPC_DEVSEM_PROJID		0x11

#define  shm_lock		vpk_sem_p
#define	 shm_unlock		vpk_sem_v

typedef struct vmp_shmdev_s
{
	int			semid;
	int			shmid;
	size_t		size;
	void*		shmbuf;
} vmp_shmdev_t;


int ipc_sem_open(const char *ipcpath, int id, int init);

int ipc_sem_close(vmp_shmdev_t *shm);

int ipc_shm_open(char *ipcpath, int id, vmp_shmdev_t *shm);

int ipc_shm_close(vmp_shmdev_t *shm, int destroy);


#ifdef __cplusplus
}
#endif

#endif // VMP_SHARED_H
