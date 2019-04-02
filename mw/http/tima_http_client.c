/**
 * History:
 * ================================================================
 * 2019-03-29 qing.zou created
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tima_http_client.h"

#include "event2/buffer.h"
#include "event2/http.h"


static void http_handle_200(void* ctx, HttpClient* client);

static void http_request_done(struct evhttp_request *req, void *ctx)
{
	HttpClient *client = (HttpClient*)ctx;

	if (req == NULL) {
		/* If the OpenSSL error queue was empty, maybe it was a
		* socket error; let's try printing that. */
		int errcode = EVUTIL_SOCKET_ERROR();
		TIMA_LOGE("[%d]socket error = %s (%d)", client->id, evutil_socket_error_to_string(errcode), errcode);
		TIMA_LOGE("[%d]http_request_done, req null, arg: %p, retry = %d\n", client->id, ctx, client->retries);
		//retry++;
		//event_base_loopexit(base, 0);
		return;
	}

	//fprintf(stderr, "Response line: %d %s\n", req->response_code, req->response_code_line);
	int response_code = evhttp_request_get_response_code(req);
	const char* response_code_line = evhttp_request_get_response_code_line(req);
	fprintf(stderr, "Response line: %d %s\n", response_code, response_code_line);

	client->status = response_code;
	/*  response is ready */
	switch(response_code)
	{
	case HTTP_OK:
		evbuffer_add_buffer(client->input_buffer, evhttp_request_get_input_buffer(req));
		//event_base_loopexit(client->base, 0);
		http_handle_200(NULL, client);
		break;
	default:
		/*  FAILURE */
		TIMA_LOGW("[%d]http_request_done, response_code = %d, retry = %d\n", client->id, response_code, client->retries);
		//event_base_loopexit(client->base, 0);
		return;
	}

	//while ((nread = evbuffer_remove(evhttp_request_get_input_buffer(req),
	//	    buffer, sizeof(buffer)))
	//       > 0) {
	//	/* These are just arbitrary chunks of 256 bytes.
	//	 * They are not lines, so we can't treat them as such. */
	//	fwrite(buffer, nread, 1, stdout);
	//}
}

static void http_error_callback(enum evhttp_request_error e, void *arg)
{
	TIMA_LOGE("request error type = %d\n\n", e);
}


static void http_data_printf(int id, char* data)
{
	TIMA_LOGI("[%d]RESPONSE DUMP: %s", id, data);
}

static void http_handle_200(void* ctx, HttpClient* client)
{
	struct evbuffer* input = client->input_buffer;
	if (input)
	{
		int len = evbuffer_get_length(input);
		if (len > 0)
		{
			//pHttp->rsp.data = (char*)malloc(len+1);
			//pHttp->rsp.size = evbuffer_remove(input, pHttp->rsp.data, len);
			//pHttp->rsp.data[len] = '\0';
			char tmp[1024] = {0};
			evbuffer_remove(input, tmp, len);
			printf("\n====== respone: %s ", tmp);

		}
	}

	//http_data_printf(client->id, pHttp->rsp.data);

	//pHttp->cbHttp(&pHttp->rsp);

	//if (pHttp->rsp.data)
	//{
	//	free(pHttp->rsp.data);
	//	pHttp->rsp.data = NULL;
	//}

	if (client->evcon)
		evhttp_connection_free(client->evcon);

	if (client->input_buffer)
		evbuffer_free(client->input_buffer);

	//if (client->base)
	//	event_base_free(client->base);
}


//void* tima_http_handle(void *arg)
//int tima_http_handle(HttpClient *thiz, TimaHttpReq *req, struct event_base *base)
int tima_http_handle(void *client, TimaHttpReq *req, void *base)
{
	int ret = 0;
	int retries = 0;
	int timeout = HTTP_REQUEST_NONBLOCK_TIMEOUT;

	struct evkeyvalq *output_headers;
	struct evbuffer *output_buffer;
	enum { REQUEST_TYPE_HTTP, REQUEST_TYPE_HTTPS } type = REQUEST_TYPE_HTTP;

	HttpClient *thiz = (HttpClient*)client;

	int req_type = 0;
	switch (req->reqtype) {
		case HTTP_REQ_POST:
			req_type = EVHTTP_REQ_POST;
			break;
		case HTTP_REQ_GET:
			req_type = EVHTTP_REQ_GET;
			break;
		case HTTP_REQ_PUT:
			req_type = EVHTTP_REQ_PUT;
			break;
		default: break;
	}

	if (req->port == 0) {
		req->port = (type == REQUEST_TYPE_HTTP) ? 80 : 443;
	}

	TIMA_LOGI("[%d]--------------------------------------------------------------", req->id);
	TIMA_LOGI("[%d]Start Request[%d]: %s:%d%s", req->id, req_type, req->host, req->port, req->url);

	if (req_type == EVHTTP_REQ_POST || req_type == EVHTTP_REQ_PUT)
		TIMA_LOGI("[%d]REQUEST DUMP: %s", req->id, req->data);

	//thiz->base = event_base_new();
	//if (!thiz->base) {
	//	TIMA_LOGE("event_base_new() NULL");
	//	goto error;
	//}
	//
	//thiz->bev = bufferevent_socket_new(thiz->base, -1, BEV_OPT_CLOSE_ON_FREE);
	//thiz->bev = bufferevent_openssl_socket_new(thiz->base, -1, ssl,
	//		BUFFEREVENT_SSL_CONNECTING,
	//		BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS);
	//evcon = evhttp_connection_base_bufferevent_new(thiz->base, NULL, bthiz->ev, host, port);
	thiz->id = req->id;
	thiz->input_buffer = evbuffer_new();
	if (thiz->input_buffer == NULL) {
		TIMA_LOGE("[%d]evbuffer_new() NULL", thiz->id);
		goto error;
	}

	TIMA_LOGD("[%d]HOST: %s:%s", req->id, req->host, req->port);
	//thiz->evcon = evhttp_connection_new(req->host, req->port);
	thiz->evcon = evhttp_connection_base_new(base, NULL, req->host, req->port);
	if (thiz->evcon == NULL) {
		TIMA_LOGE("[%d]evhttp_connection_base_new() failed\n", thiz->id);
		goto error;
	}

	if (retries > 0) {
		evhttp_connection_set_retries(thiz->evcon, retries);
	}
	if (timeout >= 0) {
		evhttp_connection_set_timeout(thiz->evcon, timeout);
	}

	thiz->req = evhttp_request_new(http_request_done, thiz);
	if (thiz->req == NULL) {
		TIMA_LOGE("[%d]evhttp_request_new() failed\n", thiz->id);
		goto error;
	}
	evhttp_request_set_error_cb(thiz->req, http_error_callback);

	output_headers = evhttp_request_get_output_headers(thiz->req);
	evhttp_add_header(output_headers, "Accept", "*/*");
	evhttp_add_header(output_headers, "Content-Type", "application/json");
	evhttp_add_header(output_headers, "Host", req->host);
	
	output_buffer = evhttp_request_get_output_buffer(thiz->req);
	if (req_type == EVHTTP_REQ_POST || req_type == EVHTTP_REQ_PUT)
		evbuffer_add(output_buffer, req->data, strlen(req->data));

	int r = evhttp_make_request(thiz->evcon, thiz->req, req_type, req->url);
	if (r != 0) {
		TIMA_LOGE("[%d]evhttp_make_request() failed, %s\n", thiz->id, req->url);
		goto error;
	}

	return 0;
	//goto cleanup;

error:
	ret = -1;
cleanup:
	if (thiz->evcon)
		evhttp_connection_free(thiz->evcon);

	if (thiz->input_buffer)
		evbuffer_free(thiz->input_buffer);

	return ret;
}
