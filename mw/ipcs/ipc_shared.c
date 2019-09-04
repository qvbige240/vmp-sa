/**
 * History:
 * ================================================================
 * 2019-08-21 qing.zou created
 *
 */

#include <unistd.h>

#include "ipc_shared.h"


int ipc_shared_init(vmp_shmdev_t* shm)
{
	vmp_shmdev_t* idev = shm;
	idev->semid = ipc_sem_open(IPC_PATH, IPC_DEVSEM_PROJID, 1);
	idev->size = sizeof(ipc_shmdata_t);
	if (ipc_shm_open(IPC_PATH, IPC_DEVSHM_PROJID, idev) < 0) return -1;

	ipc_shmdata_t* g_data = (ipc_shmdata_t*)idev->shmbuf;
	if (!g_data) return -1;

	shm_lock(idev->semid);
	memset(g_data, 0x00, sizeof(ipc_shmdata_t));
	//g_data->appstatus.proc.dog = 0;
	shm_unlock(idev->semid);

	return 0;
}

int ipc_shared_done(vmp_shmdev_t* shm)
{
    ipc_sem_close(shm);
    return ipc_shm_close(shm, 1);
}

vmon_connection_t ipc_shared_vmon(vmp_shmdev_t* shm)
{
    vmon_connection_t vmon = {0};
    vmp_shmdev_t *idev = shm;
    ipc_shmdata_t* g_data = (ipc_shmdata_t*)idev->shmbuf;
    if (!g_data) return vmon;

    shm_lock(idev->semid);
    vmon = g_data->vmon;
    shm_unlock(idev->semid);

    return vmon;
}

int ipc_shared_proc(vmp_shmdev_t* shm, void* data)
{
	int watchdog = 0, ret = 0, cnt = 0;
	vmp_shmdev_t* idev = shm;
    vmon_connection_t* vmon = data;
	ipc_shmdata_t* g_data = (ipc_shmdata_t*)idev->shmbuf;
	if (!g_data) return -1;

	shm_lock(idev->semid);
	memset(g_data, 0x00, sizeof(ipc_shmdata_t));
	//watchdog = g_data->appstatus.proc.dog = 0;
	shm_unlock(idev->semid);

	while (1) 
	{
		sleep(1);
		ret = shm_lock(idev->semid);
		g_data->appstatus.proc.dog++;
		watchdog = g_data->appstatus.proc.dog;
        *vmon = g_data->vmon;
		shm_unlock(idev->semid);

		if (++cnt > 30) {
			VMP_LOGD("watchdog: %d, ret = %d", watchdog, ret);
			cnt = 0;
		}

		if (watchdog > 16)
			VMP_LOGW("watchdog: %d", watchdog);

		if (watchdog >= 60)
		{
			VMP_LOGE("vmp app did not feed dog %d", watchdog);
			break;
		}
	}
	
	return 0;
}

/** child process **/
int ipc_share_init(vmp_shmdev_t *shm)
{
    vmp_shmdev_t *idev = shm;
    idev->semid = ipc_sem_open(IPC_PATH, IPC_DEVSEM_PROJID, 0);
    idev->size = sizeof(ipc_shmdata_t);

    return ipc_shm_open(IPC_PATH, IPC_DEVSHM_PROJID, idev);
}

int ipc_watchdog_feed(vmp_shmdev_t *shm)
{
    int ret = 0;
    vmp_shmdev_t *idev = shm;
    ipc_shmdata_t *g_data = (ipc_shmdata_t *)idev->shmbuf;
    if (!g_data) return -1;

    ret = shm_lock(idev->semid);
    g_data->appstatus.proc.dog = 0;
    ret = shm_unlock(idev->semid);

    return ret;
}

int ipc_stream_conn(vmp_shmdev_t *shm, int op)
{
    int ret = 0;
    vmp_shmdev_t *idev = shm;
    ipc_shmdata_t *g_data = (ipc_shmdata_t *)idev->shmbuf;
    if (!g_data) return -1;

    shm_lock(idev->semid);
    if (op)
        g_data->vmon.number++;
    else
        g_data->vmon.number--;
    ret = g_data->vmon.number;
    shm_unlock(idev->semid);

    return ret;
}
