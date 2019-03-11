/**
 * History:
 * ================================================================
 * 2017-10-19 qing.zou created
 *
 */
#ifndef TIMA_LOG_H
#define TIMA_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_ZLOG
#include "zlog.h"
/* use zlog */
#ifndef TIMA_LOGI
	/* tima log macros */
#define TIMA_LOGF(format, args...) \
	dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_FATAL, format, ##args)
#define TIMA_LOGE(format, args...) \
	dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_ERROR, format, ##args)
#define TIMA_LOGW(format, args...) \
	dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_WARN, format, ##args)
#define TIMA_LOGN(format, args...) \
	dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_NOTICE, format, ##args)
#define TIMA_LOGI(format, args...) \
	dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_INFO, format, ##args)
#define TIMA_LOGD(format, args...) \
	dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_DEBUG, format, ##args)
#endif
#else
#include "vpk.h"
#define TIMA_LOGD(format, args...) LOG_D(format, ##args)
#define TIMA_LOGI(format, args...) LOG_I(format, ##args)
#define TIMA_LOGN(format, args...) LOG_I(format, ##args)
#define TIMA_LOGW(format, args...) LOG_W(format, ##args)
#define TIMA_LOGE(format, args...) LOG_E(format, ##args)
#define TIMA_LOGF(format, args...) LOG_F(format, ##args)
#endif

#ifdef __cplusplus
}
#endif

#endif // TIMA_LOG_H
