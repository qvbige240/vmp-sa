/**
 * History:
 * ================================================================
 * 2019-08-21 qing.zou created
 *
 */

#ifndef IPC_SHARED_H
#define IPC_SHARED_H

#include "vmp.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IPC_PATH			    "./ipc"

#define IPC_DEVSHM_PROJID		0x10
#define IPC_DEVSEM_PROJID		0x11


struct network_status
{
	unsigned char dog;
};

struct proc_status
{
	unsigned char dog;
};

typedef struct app_status_s
{
	struct network_status   network;
	struct proc_status      proc;
} app_status_t;

// struct vmon_detail
// {
//     unsigned char           channel;
//     unsigned char		    simbcd[6];
//     //unsigned long long      simno;
// };

typedef struct vmon_connection
{
    unsigned int            number;
} vmon_connection_t;

typedef struct ipc_shmdata_t
{
	union {
		app_status_t        appstatus;
		char                tp1[64];
	};
    
    vmon_connection_t       vmon;       /* video */

} ipc_shmdata_t;

int ipc_shared_init(vmp_shmdev_t* shm);
int ipc_shared_done(vmp_shmdev_t* shm);
int ipc_shared_proc(vmp_shmdev_t* shm, void* data);

vmon_connection_t ipc_shared_vmon(vmp_shmdev_t* shm);

/** child process **/
int ipc_share_init(vmp_shmdev_t *shm);
int ipc_watchdog_feed(vmp_shmdev_t *shm);
int ipc_stream_conn(vmp_shmdev_t *shm, int op);

#ifdef __cplusplus
}
#endif

#endif // IPC_SHARED_H
