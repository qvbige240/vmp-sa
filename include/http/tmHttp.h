#ifndef _TMHTTP_H
#define _TMHTTP_H

#include "tima_log.h"

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif


#define IN
#define OUT

#define HTTP_REQUEST_RETRYTIMES				(3)
#define HTTP_REQUEST_NONBLOCK_TIMEOUT		(21)

typedef enum _HttpReqType {
	HTTP_REQ_POST = 0,
	HTTP_REQ_GET,
	HTTP_REQ_PUT,
}HttpReqType;

typedef struct _HttpUri
{
	HttpReqType	type;
	char*		ip;
	char*		port;
	char*		path;
} HttpUri;

typedef struct 
{
	HttpReqType	reqtype;
	char		ip[32];
	char		port[8];
	char*		pUrl; // utf-8
	char*		pPostData; // utf-8
	void*		pPriv;
} HttpReq, *HttpReqPtr;

typedef struct 
{
	int status;
	char reason[128]; // unicode
	char* data; // utf-8
	int size;
	int id;
	void* pPriv;
} HttpRsp, *HttpRspPtr;

typedef void (*HttpCB)(HttpRsp*);
typedef void (*StartHttpFunc)(void* arg);
typedef void (*StopHttpFunc)(void* arg);
typedef void (*RetryCB)(int nTotal, int nCur);
typedef void* (*MallocCB)(void);
typedef void (*FreeCB)(void*);

typedef struct {
	HttpCB cbHttp;
	StartHttpFunc startHttp;
	StopHttpFunc stopHttp;
	MallocCB malloc;
	FreeCB free;
	HttpReq req;
	HttpRsp rsp;
	int id;
	int cond;
	void* stopArg;

	int nRetryTimes;
	RetryCB cbRetry;

} tmHttp;


/*
* Function:
*    发送HTTP请求
* Parameters:
*    pUrl	发送命令的URL
*    pPostData	发送内容
*    pfnRespHandler	对Response处理的回调函数
*    pPriv	私有数据指针
* Return:
*    tmTRUE             --命令发送成功
*    tmFALSE            --命令发送失败
*/
void tmHttpPost(void* uri, char* pPostData,void* pPriv, HttpCB pfnRespHandler,int nRetryTimes, RetryCB cbRetry, int* pID);

/*
* Function:
*    停止当前HTTP请求
* Return:
*    TRUE             --命令发送成功
*    FALSE            --命令发送失败
*/
void tmHttpCancel(int nID);

const char* tmHttp_Code2Reason(const int code);

void tmHttp_GetHost(const char* pUri, char* pHost, int* pPort, char* pPath);


#pragma pack()

#ifdef __cplusplus
}
#endif


#endif
