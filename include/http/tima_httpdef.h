/**
 * History:
 * ================================================================
 * 2019-03-29 qing.zou created
 *
 */
#ifndef TIMA_HTTPDEF_H
#define TIMA_HTTPDEF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vmp.h"
#include "tima_typedef.h"

#pragma pack(1)

TIMA_BEGIN_DELS

typedef enum _HttpReqType 
{
	HTTP_REQ_POST = 0,
	HTTP_REQ_GET,
	HTTP_REQ_PUT,
} HttpReqType;

typedef struct _TimaUri
{
	HttpReqType		type;
	char*			ip;
	unsigned int	port;
	char*			path;
} TimaUri;

typedef struct _TimaHttpReq
{
	int				id;
	HttpReqType		reqtype;
	char			host[32];
	unsigned int	port;
	char*			url;		// utf-8
	char*			data;		// post data utf-8
	void*			priv;

	int				retry;
} TimaHttpReq;

typedef struct _TimaHttpRsp
{
	int				id;
	int				status;
	char			reason[128]; // unicode
	char*			data;
	int				size;
	void*			priv;
} TimaHttpRsp;

#pragma pack()

TIMA_END_DELS

#endif // TIMA_HTTPDEF_H
