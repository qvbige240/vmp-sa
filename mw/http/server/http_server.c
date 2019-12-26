/**
 * History:
 * ================================================================
 * 2019-12-16 qing.zou created
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

#include "vpk.h"
#include "http_server.h"
#include "http_handler.h"

typedef struct _PrivInfo
{
    HttpServerReq               req;
    HttpServerRsp               rsp;

    // onion                    *server;
    // onion_url                *urls;

    http_handler_t              *sc_head;

    struct event_base           *base;
    struct evhttp               *http;
    struct evhttp_bound_socket  *handle;

    int                         cond;
    
    pthread_t                   thread;
} PrivInfo;

#if 0
static void demo_request_cb(struct evhttp_request *req, void *arg)
{
	const char *cmdtype;
	struct evkeyvalq *headers;
	struct evkeyval *header;
	struct evbuffer *buf;

	switch (evhttp_request_get_command(req)) {
	case EVHTTP_REQ_GET: cmdtype = "GET"; break;
	case EVHTTP_REQ_POST: cmdtype = "POST"; break;
	case EVHTTP_REQ_HEAD: cmdtype = "HEAD"; break;
	case EVHTTP_REQ_PUT: cmdtype = "PUT"; break;
	case EVHTTP_REQ_DELETE: cmdtype = "DELETE"; break;
	case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
	case EVHTTP_REQ_TRACE: cmdtype = "TRACE"; break;
	case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
	case EVHTTP_REQ_PATCH: cmdtype = "PATCH"; break;
	default: cmdtype = "unknown"; break;
	}

    const char *uri = evhttp_request_get_uri(req);
    printf("Received a %s request for %s\nHeaders:\n", cmdtype, uri);

    headers = evhttp_request_get_input_headers(req);
    for (header = headers->tqh_first; header; header = header->next.tqe_next)
    {
        printf("  %s: %s\n", header->key, header->value);
    }

    struct evhttp_uri *decoded = NULL;
    const char *path = NULL;
    const char *query = NULL;
    const char *value = NULL;
    struct evkeyvalq params = {0};

    decoded = evhttp_uri_parse(uri);
    query = evhttp_uri_get_query(decoded);
    path = evhttp_uri_get_path(decoded);
    printf("path: %s, query: %s\n", path, query);
    evhttp_parse_query_str(query, &params);
    value = evhttp_find_header(&params, "id");
    printf("value: %s\n", value);

    headers = &params;
    for (header = headers->tqh_first; header; header = header->next.tqe_next)
    {
        printf("  %s: %s\n", header->key, header->value);
    }


    buf = evhttp_request_get_input_buffer(req);
    puts("Input data: <<<");
    while (evbuffer_get_length(buf))
    {
        int n;
        char cbuf[128];
        n = evbuffer_remove(buf, cbuf, sizeof(cbuf));
        if (n > 0)
            (void)fwrite(cbuf, 1, n, stdout);
    }
    puts("demo_request_cb >>>");

    printf("========== replay 200\n");
    struct evbuffer *reply_buffer = evbuffer_new();
    evbuffer_add_printf(reply_buffer, "hello http");
    evhttp_send_reply(req, 200, "OK", reply_buffer);
    evbuffer_free(reply_buffer);

    //evhttp_send_reply(req, 200, "OK", NULL);
}
#endif

static int http_uri_add(PrivInfo *thiz)
{
    http_handler_t *s;
    for (s = thiz->sc_head; s != NULL; s = s->sc_next)
    {
        if (s->sc_urls && s->handler)
            evhttp_set_cb(thiz->http, s->sc_urls, s->handler, s->sc_args);

    //evhttp_set_cb(thiz->http, "/carnet/sr/tg/demo", demo_request_cb, NULL);
    }

    return 0;
}

static void *http_server_thread(void *arg)
{
    PrivInfo *thiz = (PrivInfo *)arg;
    unsigned short port = thiz->req.port;

    VMP_LOGD("http server start...");

    thiz->base = event_base_new();
    if (!thiz->base) {
        VMP_LOGE("Couldn't create an event_base: exiting");
        goto end;
    }

    thiz->http = evhttp_new(thiz->base);
    if (!thiz->http) {
        VMP_LOGE("couldn't create evhttp. Exiting.");
        goto end;
    }

    if (thiz->req.func)
        thiz->req.func(thiz->req.ctx, 0, NULL);

    // evhttp_set_cb(thiz->http, "/carnet/sr/tg/demo", demo_request_cb, NULL);
    // evhttp_set_cb(thiz->http, "/carnet/sr/tg/test", demo_request_cb, NULL);
    http_uri_add(thiz);

    thiz->handle = evhttp_bind_socket_with_handle(thiz->http, "0.0.0.0", port);
    if (!thiz->handle) {
        VMP_LOGE("couldn't bind to port %d. Exiting.", (int)port);
        goto end;
    }

    {
        //struct sockaddr_storage ss;
        vpk_sockaddr addr;
        evutil_socket_t fd = evhttp_bound_socket_get_fd(thiz->handle);

        vpk_addr_get_from_sock(fd, &addr);

        char ip[INET_ADDRSTRLEN] = {0};
        vpk_inet_ntop(AF_INET, vpk_sockaddr_get_addr(&addr), ip, sizeof(ip));
        printf("http server listening on %s:%u\n", ip, vpk_sockaddr_get_port(&addr));
    }

    event_base_dispatch(thiz->base);

    // thiz->server = onion_new(O_POOL);
    // if (thiz->server)
    // {
    //     char port[8] = {0};
    //     sprintf(port, "%d", thiz->req.port);
    //     onion_set_timeout(thiz->server, 10000);
    //     onion_set_hostname(thiz->server, "0.0.0.0");
    //     onion_set_port(thiz->server, port);

    //     thiz->urls = onion_root_url(thiz->server);
    //     //onion_url_add(thiz->urls, "carnet/sm/lbs/hello", hello);
    //     if (thiz->req.func)
    //         thiz->req.func(thiz->req.ctx, 0, NULL);

    //     url_add(thiz);

    //     VMP_LOGI("Listening at http://0.0.0.0:%s", port);
    //     onion_listen(thiz->server);

    //     onion_free(thiz->server);
    // }
end:
    pthread_exit(0);

    return NULL;
}

int http_server_start(void *p)
{
    int ret = 0;
    PrivInfo *thiz = p;

    if (!thiz) return -1;

    // pthread_mutexattr_t attr;
    // if (pthread_mutexattr_init(&attr) != 0)
    //     goto fail;
    // if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0)
    //     goto fail;
    // if (pthread_mutex_init(&thiz->core_mutex, &attr) != 0)
    //     goto fail;

    ret = pthread_create(&thiz->thread, NULL, http_server_thread, (void *)thiz);
    if (ret != 0)
        VMP_LOGE("create thread 'http_server_thread' failed");

    pthread_detach(thiz->thread);
    
    //http_server_destroy(thiz);
    return 0;
}

void *http_server_create(void *parent, HttpServerReq *req)
{
    PrivInfo *priv = calloc(1, sizeof(PrivInfo));
    if (priv)
    {
        priv->cond  = 1;
        priv->req   = *req;
    }
    return priv;
}

void http_server_destroy(PrivInfo *thiz)
{
    if (thiz)
    {
        free(thiz);
    }
}

/** url handler regisger **/
static http_handler_t *http_handler_find(void *p, char *url)
{
    PrivInfo *thiz = p;
    http_handler_t *s;

    if (!url)
    {
        VMP_LOGE("null pointer");
        return NULL;
    }

    for (s = thiz->sc_head; s != NULL; s = s->sc_next)
    {
        if (strcmp(s->sc_urls, url) == 0)
            break;
    }

    return s;
}

int http_handler_register(void *http, char *url, void *handler, void *args)
{
    PrivInfo *thiz = http;
    http_handler_t *s;

    if (!http || !url || !handler)
    {
        VMP_LOGE("null pointer");
        return -1;
    }

    if ((s = http_handler_find(http, url)) != NULL)
    {
        if (s->handler == handler)
            return 0;
        VMP_LOGW("path: %s have been registered!", url);
        return 1;
    }
    s = calloc(1, sizeof(http_handler_t));
    if (s == NULL)
    {
        VMP_LOGE("memory alloc failed.");
        return -1;
    }
    s->sc_urls      = url;
    s->sc_args      = args;
    s->handler      = handler;
    s->sc_next      = thiz->sc_head;
    thiz->sc_head   = s;

    return 0;
}