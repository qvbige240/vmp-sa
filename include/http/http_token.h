/**
 * History:
 * ================================================================
 * 2019-03-29 qing.zou created
 *
 */

#ifndef HTTP_TOKEN_H
#define HTTP_TOKEN_H

#include "vmp_node.h"

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

#define HTTP_TOKEN_CLASS	FOURCCLE('H','T','T','K')


typedef struct _HttpTokenReq
{
	char	devtype[16];		/* TACHOGRAPH */
	char	seriesno[64];
} HttpTokenReq;

typedef struct _HttpTokenRep
{
	char	token[MAX_LEN];
} HttpTokenRep;

void http_token_init(void);
void http_token_done(void);

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // HTTP_TOKEN_H
