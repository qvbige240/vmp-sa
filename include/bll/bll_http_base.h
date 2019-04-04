/**
 * History:
 * ================================================================
 * 2018-04-03 qing.zou created
 *
 */

#ifndef BLL_HTTPBASE_H
#define BLL_HTTPBASE_H

#include "bll_typedef.h"

#pragma pack(1)

TIMA_BEGIN_DELS

#define BLL_HTTPBASE_CLASS		FOURCCLE('B','L','H','B')


void bll_hbase_init(void);
void bll_hbase_done(void);

#pragma pack()

TIMA_END_DELS

#endif // BLL_HTTPBASE_H
