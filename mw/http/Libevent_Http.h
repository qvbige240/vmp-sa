#ifndef _LIBEVENT_HTTP_H
#define _LIBEVENT_HTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void* Libevent_HttpReqThread( void* arg);
void Libevent_StopHttp( void* arg);
void* Libevent_malloc(void);
void Libevent_free(void*);
void Libevent_DnsParse(const char* url, char* host, int* port,  char* path);

#endif
