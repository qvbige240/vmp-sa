/**
 * History:
 * ================================================================
 * 2019-05-22 qing.zou created
 *
 */

#ifndef BLL_VOICE_TASK_H
#define BLL_VOICE_TASK_H

#include "bll_typedef.h"
#include "tima_server.h"

TIMA_BEGIN_DELS

#define BLL_VOICE_TASK_CLASS		FOURCCLE('B','L','V','T')


typedef struct _VoiceTaskReq
{
	char*	data;
} VoiceTaskReq;

typedef struct _VoiceTaskRep
{
	
} VoiceTaskRep;

void bll_voice_init(void);
void bll_voice_done(void);

TIMA_END_DELS

#endif // BLL_VOICE_TASK_H
