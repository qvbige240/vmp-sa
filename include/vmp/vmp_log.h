/**
 * History:
 * ================================================================
 * 2019-02-28 qing.zou created
 *
 */
#ifndef VMP_LOG_H
#define VMP_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_ZLOG
#include "zlog.h"
/* use zlog */
#ifndef VMP_LOGI
	/* tima log macros */
#define VMP_LOGF(format, args...) \
	dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_FATAL, format, ##args)
#define VMP_LOGE(format, args...) \
	dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_ERROR, format, ##args)
#define VMP_LOGW(format, args...) \
	dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_WARN, format, ##args)
#define VMP_LOGN(format, args...) \
	dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_NOTICE, format, ##args)
#define VMP_LOGI(format, args...) \
	dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_INFO, format, ##args)
#define VMP_LOGD(format, args...) \
	dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_DEBUG, format, ##args)
#endif
#else
#include "vpk.h"
#define VMP_LOGD(format, args...) LOG_D(format, ##args)
#define VMP_LOGI(format, args...) LOG_I(format, ##args)
#define VMP_LOGN(format, args...) LOG_I(format, ##args)
#define VMP_LOGW(format, args...) LOG_W(format, ##args)
#define VMP_LOGE(format, args...) LOG_E(format, ##args)
#define VMP_LOGF(format, args...) LOG_F(format, ##args)
#endif

#ifdef __cplusplus
}
#endif

#endif // VMP_LOG_H
