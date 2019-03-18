/*
 ============================================================================
 Name        : context.h
 Author      : wison.wei
 Copyright   : 2017(c) Timanetworks Company
 Description : 
 ============================================================================
 */

#ifndef TM_HTTP_DEFINE_H
#define TM_HTTP_DEFINE_H

#include <stdbool.h>

#include "http_network.h"

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

#define HTTP_MAX_LEN	256

typedef enum
{
	HTTP_RES_SUCCESS = 0,
	
	HTTP_RES_TIMEOUT,
	HTTP_RES_NETERROR,
	
	HTTP_RES_UNKNOWN,
	
} EHttpResult;

typedef struct
{
	EHttpResult eResult;
	int  netCode;
	char netMsg[HTTP_MAX_LEN];
	char errCode[HTTP_MAX_LEN];
	char errMsg[HTTP_MAX_LEN];
	
} StHttpResult;

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // TM_HTTP_DEFINE_H

