/**
 * History:
 * ================================================================
 * 2019-03-15 wison.wei created
 *
 */

#ifndef TM_TIMA_TOKEN_H
#define TM_TIMA_TOKEN_H

#include "node.h"

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

#define TIMA_TOKEN_CLASS	FOURCCLE('T','M','T','K')


typedef struct _TimaTokenReq
{
	char	devtype[16];		/* TACHOGRAPH */
	char	seriesno[64];
} TimaTokenReq;

typedef struct _TimaTokenRes
{
	char token[MAX_LEN];
} TimaTokenRes;

void tima_token_init(void);
void tima_token_done(void);

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // HTTP_TOKEN_H
