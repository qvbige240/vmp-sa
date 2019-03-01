/**
 * History:
 * ================================================================
 * 2018-02-05 qing.zou created
 *
 */

#ifndef BLL_DEMO_H
#define BLL_DEMO_H

//#include "node.h"
//#include "bll.h"
#include "bll_typedef.h"

#pragma pack(1)

TIMA_BEGIN_DELS

#define BLL_DEMO_CLASS		FOURCCLE('B','L','D','M')


typedef struct
{
	char*	data;
} DemoDataReq;

typedef struct
{
	
} DemoDataRep;

void bll_demo_init(void);
void bll_demo_done(void);

#pragma pack()

TIMA_END_DELS

#endif // BLL_DEMO_H
