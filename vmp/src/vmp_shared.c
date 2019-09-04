/**
 * History:
 * ================================================================
 * 2019-08-16 qing.zou created
 *
 */

#include <errno.h>
#include <unistd.h>

#include "vpk/vpk_ipc.h"
#include "vpk/vpk_filesys.h"

#include "vmp_log.h"
#include "vmp_shared.h"

union semun {
    int setval;
    struct semid_ds *buf;
    unsigned short *array;
};

static int vmp_filepath_check(char *filepath)
{
    return_val_if_fail(filepath, -1);

    if (!vpk_exists(filepath))
    {
        int ret = 0;
        char tmp[256] = {0};
        vpk_pathname_get(filepath, tmp);
        VMP_LOGI("full: %s, pathname: %s", filepath, tmp);
        ret = vpk_mkdir_mult(filepath);
        VMP_LOGI("vpk_mkdir_mult \'%s\' ret = %d\n", filepath, ret);
        if (vpk_create_file(filepath) < 0)
        {
            VMP_LOGE("create/touch file error!");
            return -1;
        }
    }

    return 0;
}

int ipc_sem_open(const char *ipcpath, int id, int init)
{
    return_val_if_fail(ipcpath, -1);

    vmp_filepath_check(IPC_PATH); /* IPC_PATH */

    key_t key_sem = ftok(IPC_PATH, IPC_DEVSEM_PROJID);
    if (key_sem < 0)
    {
        VMP_LOGE("[ipc]: ftok failed, error(%s)", strerror(errno));
        return -1;
    }

    int semid = vpk_semget(key_sem, 1, 0);
    if (semid < 0)
    {
        VMP_LOGE("semid error.");
        return -1;
    }

    if (init)
    {
        union semun sem_args;
        unsigned short array[1] = {1};
        sem_args.array = array;
        if (vpk_semctl(semid, 0, SETALL, sem_args) < 0)
        {
            VMP_LOGE("vpk_semctl error.");
            return -1;
        }
    }

    return semid;
}

int ipc_sem_close(vmp_shmdev_t *shm)
{
    int semid = shm->semid;
    if (-1 == vpk_semctl(semid, 0, IPC_RMID, 0))
        VMP_LOGE("semctl rm failed.");
    return 0;
}

int ipc_shm_open(char *ipcpath, int id, vmp_shmdev_t *shm)
{
    key_t key_shm = ftok(IPC_PATH, IPC_DEVSHM_PROJID);
    if (key_shm < 0)
    {
        VMP_LOGE("[ipc]: ftok failed, error(%s)", strerror(errno));
        return -1;
    }

    int shmid = vpk_shmget(key_shm, shm->size, 0);
    if (shmid < 0)
    {
        VMP_LOGE("semid error.");
        return -1;
    }

    shm->shmid = shmid;
    shm->shmbuf = vpk_shmat(shmid, NULL, 0);
    if (shm->shmbuf == (void *)-1)
    {
        VMP_LOGE("[ipc]: vpk_shmat(%d) failed, error(%s)", shmid, strerror(errno));
        return -1;
    }
    return 0;
}

int ipc_shm_close(vmp_shmdev_t *shm, int destroy)
{
    int ret = 0;
    if (vpk_shmdt((void *)shm->shmbuf) < 0)
        return -1;

    if (destroy)
        ret = vpk_shmctl(shm->shmid, IPC_RMID, 0); // don't rm it while other use

    return ret;
}

int ipc_open(void)
{
    key_t key_shm = ftok(IPC_PATH, IPC_DEVSHM_PROJID);
    key_t key_sem = ftok(IPC_PATH, IPC_DEVSEM_PROJID);
    if (key_shm < 0 || key_sem < 0)
    {
        VMP_LOGE("[ipc]: ftok failed, error(%s)", strerror(errno));
        return -1;
    }
    return 0;
}
