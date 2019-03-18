
#include <memory.h>
#include "tmHttp.h"
#include "Libevent_Http.h"
#include "context.h"
#include "ThreadPool.h"
#include <string.h>

void tmFreeHttp(tmHttp* pHttp)
{
	TIMA_LOGD("tmFreeHttp");
	if(pHttp)
	{
		if (pHttp->req.pPostData)
		{
			free(pHttp->req.pPostData);
		}

		if (pHttp->req.pUrl)
		{
			free(pHttp->req.pUrl);
		}
		if (pHttp->free)
		{
			pHttp->free(pHttp->stopArg);
		}
		free(pHttp);
		pHttp = NULL;
	}

}

void tmStopHttp(tmHttp* pHttp)
{
	TIMA_LOGD("tmStopHttp");
	if (pHttp->stopHttp)
	{
		pHttp->stopHttp(pHttp->stopArg);
	}
}

void tmHttpPost(IN void* uri, IN char* pPostData,IN void* pPriv, IN HttpCB pfnRespHandler,IN int nRetryTimes, IN RetryCB cbRetry,OUT int* pID)
{
	context* p = Context();

	if (p)
	{
		ThreadPool* pTP = p->tp;
		if (pTP)
		{
			ThreadPoolJob job;
			HttpUri* tima_uri = (HttpUri*)uri;
			
			tmHttp* pHttp = (tmHttp*)malloc(sizeof(tmHttp));
			memset(pHttp, 0, sizeof(tmHttp));

			pHttp->startHttp = (StartHttpFunc)Libevent_HttpReqThread;
			pHttp->stopHttp = (StopHttpFunc)Libevent_StopHttp;
			pHttp->malloc = (MallocCB)Libevent_malloc;
			pHttp->free = (FreeCB)Libevent_free;

			if (pHttp->malloc)
			{
				pHttp->stopArg = pHttp->malloc();
			}
			pHttp->cbHttp = pfnRespHandler;
			pHttp->req.reqtype = tima_uri->type;
			if (tima_uri->type == HTTP_REQ_GET)
			{
				int len1 = strlen(tima_uri->path);
				int len2 = strlen(pPostData);
				pHttp->req.pUrl =  (char*)malloc(len1+len2+1);
				memset(pHttp->req.pUrl, 0x00, len1+len2+1);
				strncpy(pHttp->req.pUrl, tima_uri->path, len1);
				strncpy(pHttp->req.pUrl+len1, pPostData, len2);
			}
			else
			{
				pHttp->req.pPostData = (char*)malloc(strlen(pPostData)+1);
				memset(pHttp->req.pPostData, 0x00, strlen(pPostData)+1);
				strcpy(pHttp->req.pPostData, pPostData);
				pHttp->req.pUrl =  (char*)malloc(strlen(tima_uri->path)+1);
				memset(pHttp->req.pUrl, 0x00, strlen(tima_uri->path)+1);
				strcpy(pHttp->req.pUrl, tima_uri->path);
			}
			strncpy(pHttp->req.ip, tima_uri->ip, sizeof(pHttp->req.ip));
			strncpy(pHttp->req.port, tima_uri->port, sizeof(pHttp->req.port));
			pHttp->req.pPriv  = pPriv;
			pHttp->rsp.pPriv  = pPriv;
			pHttp->nRetryTimes = nRetryTimes;
			pHttp->cbRetry = cbRetry;
			
			job.arg = pHttp;

			TPJobInit( &job, ( start_routine) pHttp->startHttp, pHttp);
			TPJobSetFreeFunction( &job, ( free_routine ) tmFreeHttp );
			TPJobSetStopFunction( &job, ( stop_routine ) tmStopHttp );
			TPJobSetPriority( &job, MED_PRIORITY );
			ThreadPoolAdd( pTP, &job, &job.jobId );

			pHttp->id = job.jobId;
			*pID = job.jobId;
		}
	}

}

void tmHttpCancel(int nID)
{
	context* p = Context();
	if (p)
	{
		ThreadPool* pTP = p->tp;
		if (pTP)
		{
			ThreadPoolJob* job = NULL;
			ThreadPoolStop(pTP, nID, job);
		}
	}
}

const char* tmHttp_Code2Reason(const int code)
{
	switch (code) 
	{
	case 100: return "Continue";
	case 101: return "Switching Protocols";
	case 200: return "OK";
	case 201: return "Created";
	case 202: return "Accepted";
	case 203: return "Non-Authoritative Information";
	case 204: return "No Content";
	case 205: return "Reset Content";
	case 206: return "Partial Content";
	case 300: return "Multiple Choices";
	case 301: return "Moved Permanently";
	case 302: return "Found";
	case 303: return "See Other";
	case 304: return "Not Modified";
	case 305: return "Use Proxy";
	case 307: return "Temporary Redirect";
	case 400: return "Bad Request";
	case 401: return "Unauthorized";
	case 402: return "Payment Required";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 405: return "Method Not Allowed";
	case 406: return "Not Acceptable";
	case 407: return "Proxy Authentication Required";
	case 408: return "Request Time-out";
	case 409: return "Conflict";
	case 410: return "Gone";
	case 411: return "Length Required";
	case 412: return "Precondition Failed";
	case 413: return "Request Entity Too Large";
	case 414: return "Request-URI Too Large";
	case 415: return "Unsupported Media Type";
	case 416: return "Requested range not satisfiable";
	case 417: return "Expectation Failed";
	case 500: return "Internal Server Error";
	case 501: return "Not Implemented";
	case 502: return "Bad Gateway";
	case 503: return "Service Unavailable";
	case 504: return "Gateway Time-out";
	case 505: return "HTTP Version not supported";
	case 506: return "User Canceled";
	case 507: return "Imsi Error";
	case 508: return "Ras NotConnected";
	default: return "Unknown";
	}
}

void tmHttp_GetHost(const char* pUri, char* pHost, int* pPort, char* pPath)
{
	Libevent_DnsParse(pUri, pHost, pPort, pPath);
}
