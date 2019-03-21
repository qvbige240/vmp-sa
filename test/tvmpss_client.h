#ifndef TM_TVMPSS_CLIENT_H
#define TM_TVMPSS_CLIENT_H

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>


#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct _StClient
{
	char* data;
	int len;

	char* ip;
	int port;

	int jobId;
	void* tp;
	void* bev;

	int fd;
	int delay;
}StClient;

void* tvmpss_client_thread(void* arg);


#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // TM_TVMPSS_CLIENT_H

