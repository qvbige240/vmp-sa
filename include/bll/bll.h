/*
 ============================================================================
 Name        : bll.h
 Author      : wison.wei
 Copyright   : 2019(c) Timanetworks Company
 Description : 
 ============================================================================
 */

#ifndef TM_BLL_H
#define TM_BLL_H

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>


#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

void bll_init(char* strConf);
int bll_cond(void);
void bll_idle(void);
void bll_done(void);

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // TM_BLL_H

