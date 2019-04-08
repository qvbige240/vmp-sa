/**
 * History:
 * ================================================================
 * 2017-06-05 qing.zou created
 *
 */
#ifndef TIMA_CONFIG_H
#define TIMA_CONFIG_H

#include "vpk_typedef.h"

TIMA_BEGIN_DELS

#if defined(_X86_)
  #define ROOT_CFG				"./"
  #define ROOT_RUN				"./"
#else
  #define ROOT_CFG				"/etc/cloud/"
  #define ROOT_RUN				"/bin/"
#endif

#define APP_ROOT_DVR_DEFAULT	"DCDVR"

/* _PRODUCT_ */
#define SOCKET_IP_PRODUCE			"60.195.250.188"
#define SOCKET_PORT_PRODUCE			8888

#define HTTP_IP_PRODUCE				"tg2.91carnet.com"
#define PORT_TG_PRODUCE				"80"
#define PORT_MG_PRODUCE				"80"
#define PORT_GETPROFILE_PRODUCE		"80"

#define HTTP_BDA_IP_PRODUCE			"bda.91carnet.com"
#define PORT_BDA_PRODUCE			"80"

/* _TEST_ */
#define SOCKET_IP_TEST				"115.182.105.71"
#define SOCKET_PORT_TEST			8888

#define HTTP_IP_TEST				"tg.test.timanetwork.cn"
#define PORT_TG_TEST				"8114"
#define PORT_MG_TEST				"8112"
#define PORT_GETPROFILE_TEST		"8114"

#define HTTP_BDA_IP_TEST			"bdatest.91carnet.com"
#define PORT_BDA_TEST				"80"

#if defined(_PRODUCT_)
#define SOCKET_IP_DEFAULT				SOCKET_IP_PRODUCE
#define SOCKET_PORT_DEFAULT				SOCKET_PORT_PRODUCE

#define HTTP_IP_DEFAULT					HTTP_IP_PRODUCE
#define PORT_TG_DEFAULT					PORT_TG_PRODUCE
#define PORT_MG_DEFAULT					PORT_MG_PRODUCE
#define PORT_GETPROFILE_DEFAULT			PORT_GETPROFILE_PRODUCE

#define HTTP_BDA_IP_DEFAULT				HTTP_BDA_IP_PRODUCE
#define PORT_BDA_DEFAULT				PORT_BDA_PRODUCE
#else
#define SOCKET_IP_DEFAULT				SOCKET_IP_TEST
#define SOCKET_PORT_DEFAULT				SOCKET_PORT_TEST

#define HTTP_IP_DEFAULT					HTTP_IP_TEST
#define PORT_TG_DEFAULT					PORT_TG_TEST		// device
#define PORT_MG_DEFAULT					PORT_MG_TEST		// smart phone
#define PORT_GETPROFILE_DEFAULT			PORT_GETPROFILE_TEST

#define HTTP_BDA_IP_DEFAULT				HTTP_BDA_IP_TEST
#define PORT_BDA_DEFAULT				PORT_BDA_TEST
#endif

#define TIMA_MEDIA_DATABASE				".tima.db"
#define TIMA_DATA_FIELD_IMEI			"imei"

#define TIMA_DATA_FIELD_VIDEO			"videotime"
#define TIMA_DATA_FIELD_PHOTO			"phototime"

#define TIMA_DATA_FIELD_ACCON			"acc_on"

#define TIMA_DATA_FIELD_LON				"longitude"
#define TIMA_DATA_FIELD_LAT				"latitude"

typedef struct _TimaLogConfig
{
	char	data_path[MAX_PATH_SIZE+1];
	char	log_path[MAX_PATH_SIZE+1];
	int		log_mode;
	int		log_level;  /* @level  All level: "FATAL", "ERROR", "WARN", "INFO", "DEBUG". */

	int		file_len;
	int		file_cnt;

	//char	network_file[MAX_PATH_SIZE+1];
} TimaLogConfig;

TIMA_END_DELS

#endif // TIMA_CONFIG_H
