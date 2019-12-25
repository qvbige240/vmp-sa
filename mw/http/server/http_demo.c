/**
 * History:
 * ================================================================
 * 2019-12-16 qing.zou created
 *
 */
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

#include "api/api_demo.h"
#include "http_server.h"
#include "http_handler.h"
#include "http_register.h"

#include "jansson.h"

typedef struct _PrivInfo
{
    ApiDemoReq          req;
    ApiDemoRsp          rsp;

    char                *path;

    char                *data;
    unsigned int        length;

    int                 cond;
} PrivInfo;

const char* http_request_get_fullpath(struct evhttp_request *req)
{
    const char *path = NULL;
    struct evhttp_uri *decoded = NULL;

    const char *uri = evhttp_request_get_uri(req);

    decoded = evhttp_uri_parse(uri);
    path = evhttp_uri_get_path(decoded);

    return path;
}

const char* http_request_get_query(struct evhttp_request *req, const char *key)
{
    const char *uri = evhttp_request_get_uri(req);
    struct evhttp_uri *decoded = evhttp_uri_parse(uri);

    const char *querys = NULL;
    const char *value = NULL;
    struct evkeyvalq params = {0};

    querys = evhttp_uri_get_query(decoded);
    evhttp_parse_query_str(querys, &params);
    value = evhttp_find_header(&params, key);

	struct evkeyvalq *headers;
	struct evkeyval *header;

    headers = &params;
    for (header = headers->tqh_first; header; header = header->next.tqe_next)
    {
        printf("  %s: %s\n", header->key, header->value);
    }

    return value;
}

static void http_handle_destroy(void *p)
{
    PrivInfo *thiz = p;
    if (thiz)
    {
        if (thiz->data)
            free(thiz->data);
        free(thiz);
    }
}

static int query_param_parse(void *p, struct evhttp_request *req)
{
    const char *value;
    PrivInfo *thiz = p;
    
    // struct evhttp_uri *decoded = NULL;

    // const char *uri = evhttp_request_get_uri(req);

    // decoded = evhttp_uri_parse(uri);
    // char *query = NULL;
    // char *value = NULL;
    // struct evkeyvalq params = {0};

    // query = evhttp_uri_get_query(decoded);
 
    // printf("query: %s\n", query);
    // evhttp_parse_query_str(query, &params);
    // value = (char*)evhttp_find_header(&params, "id");

    value = http_request_get_query(req, "id");
    if (!value) goto param_parse_end;
    thiz->req.id = atoi(value);
    VMP_LOGD("id: %d", thiz->req.id);

    return 0;

param_parse_end:
    VMP_LOGE("\n============ param parse error!!! ============\n");
    return -1;
}

static void response_json_create(PrivInfo *thiz)
{
    ApiDemoRsp *rsp = &thiz->rsp;
    json_t *json_root = json_object();

    json_object_set_new(json_root, "id", json_integer(rsp->id));
    json_object_set_new(json_root, "name", json_string(rsp->name));
    json_object_set_new(json_root, "count", json_integer(rsp->count));

    char *data_dump = json_dumps(json_root, 0);
    VMP_LOGD("response api demo:\n%s", data_dump);

    thiz->length = strlen(data_dump);
    thiz->data = calloc(1, thiz->length + 1);
    if (thiz->data)
        strcpy(thiz->data, data_dump);
    else
        VMP_LOGE("memory alloc failed: response_json_create");

    free(data_dump);
    json_decref(json_root);
}

static void handle_demo(struct evhttp_request *req, void *p)
{
    char *data = NULL;
    int length = 0;
    int rcode = 200;
    struct evbuffer *reply_buffer = evbuffer_new();
    PrivInfo *thiz = calloc(1, sizeof(PrivInfo));

    thiz->path = (char *)http_request_get_fullpath(req);
    VMP_LOGD("path: %s", thiz->path);

    int ret = query_param_parse(thiz, req);
    if (ret != 0) {
        VMP_LOGE("handle_demo failed.");
        rcode = 5001;
        goto end;
    }

    service_handler_t *service = (service_handler_t *)p;
    if (!service || !service->pfn_callback) {
        rcode = 5002;
        goto end;
    }

    ret = service->pfn_callback(service->ctx, &thiz->req, &thiz->rsp);
    if (ret != 0) {
        VMP_LOGE("get response failed");
        rcode = 5002;
        goto end;
    }

    response_json_create(thiz);

    printf("========== replay 200\n");
    evbuffer_add(reply_buffer, thiz->data, thiz->length);
    //evbuffer_add_printf(reply_buffer, "hello http");
    evhttp_send_reply(req, 200, "OK", reply_buffer);
    evbuffer_free(reply_buffer);
    http_handle_destroy(thiz);

    return ;

end:
    length = error_json_create(rcode, &data);
    evbuffer_add(reply_buffer, data, length);
    evhttp_send_reply(req, 200, "OK", reply_buffer);
    evbuffer_free(reply_buffer);
    http_handle_destroy(thiz);
    if (data) free(data);
}

int http_demo_register(void *server, void *args)
{
    return http_handler_register(server, HTTP_URL_DEMO, handle_demo, args);
}
