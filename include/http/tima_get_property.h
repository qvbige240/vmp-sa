/**
 * History:
 * ================================================================
 * 2019-03-15 wison.wei created
 *
 */

#ifndef TM_TIMA_GET_PROPERTY_H
#define TM_TIMA_GET_PROPERTY_H

//#include "node.h"

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

#define TIMA_SIM_NO_LEN		(16)
#define TIMA_GET_PROPERTY_CLASS		FOURCCLE('T','M','G','P')


typedef struct _TimaGetPropertyReq
{
	unsigned long	flowid;

	char			sim[TIMA_SIM_NO_LEN];
	int				ch;

	nodecb			pfncb;
} TimaGetPropertyReq;

typedef struct _TimaGetPropertyRsp
{
	char			uri[MAX_LEN];
	char			property[MAX_LEN];
} TimaGetPropertyRsp;

void tima_get_property_init(void);
void tima_get_property_done(void);

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // HTTP_TOKEN_H
