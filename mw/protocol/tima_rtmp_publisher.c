/**
 * History:
 * ================================================================
 * 2019-02-28 qing.zou created
 *
 */

#include "librtmp/log.h"
#include "tima_rtmp_publisher.h"

typedef struct _PrivInfo
{
	RTMP		rtmp;
} PrivInfo;

TimaRTMPPublisher* tima_rtmp_create(const char *url)
{
	TimaRTMPPublisher* thiz = TIMA_CALLOC(1, sizeof(TimaRTMPPublisher) + sizeof(PrivInfo));
	if (!thiz) {
		VMP_LOGE("rtmp publisher create failed.");
		return NULL;
	}

	DECL_PRIV(thiz, priv);
	RTMP_Init(&priv->rtmp);
	RTMP_LogSetLevel(RTMP_LOGWARNING);
	//RTMP_LogSetLevel(RTMP_LOGDEBUG);
	thiz->url = strdup(url);

	return thiz;
}

int tima_rtmp_connect(TimaRTMPPublisher *publisher)
{
	TimaRTMPPublisher* thiz = publisher;
	DECL_PRIV(thiz, priv);
	if (!thiz) {
		VMP_LOGE("rtmp publisher null.");
		return -1;
	}

	if (!RTMP_SetupURL(&priv->rtmp, thiz->url)) {
		goto ret_error;
	}

	RTMP_EnableWrite(&priv->rtmp);

	if (!RTMP_Connect(&priv->rtmp, NULL)) {
		goto ret_error;
	}

	if (!RTMP_ConnectStream(&priv->rtmp, 0)) {
		RTMP_Close(&priv->rtmp);
		goto ret_error;
	}

	VMP_LOGI("connected[%d]: %s", thiz->id, thiz->url);
	return 0;
ret_error:
	VMP_LOGE("rtmp[%s] connect error.", thiz->url);
	return -1;
}

int tima_rtmp_send(TimaRTMPPublisher *publisher, RTMPPacket *packet, unsigned int timestamp)
{
	TimaRTMPPublisher* thiz = publisher;
	DECL_PRIV(thiz, priv);
	if (!thiz) {
		VMP_LOGE("rtmp publisher null.");
		return -1;
	}

	packet->m_nInfoField2 = priv->rtmp.m_stream_id;

	packet->m_nTimeStamp = RTMP_GetTime() & 0xffffff;
	//packet->m_nTimeStamp = timestamp & 0xffffff;
	//packet->m_nTimeStamp = timestamp;

	if (!RTMP_IsConnected(&priv->rtmp)) {
		VMP_LOGE("can not connect to server, url[%s].", thiz->url);
		return -1;
	}

	if (!RTMP_SendPacket(&priv->rtmp, packet, 1)) {
		VMP_LOGE("failed to send, url[%s].", thiz->url);
		return -1;
	}

	return 0;
}

void tima_rtmp_destory(TimaRTMPPublisher *publisher)
{
	TimaRTMPPublisher* thiz = publisher;
	DECL_PRIV(thiz, priv);
	if (thiz)
	{

		RTMP_Close(&priv->rtmp);
		VMP_LOGI("closed[%d]: %s.", thiz->id, thiz->url);
		if (thiz->url)
			free(thiz->url);

		TIMA_FREE(thiz);
	}
}
