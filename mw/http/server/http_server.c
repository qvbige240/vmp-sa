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


typedef struct _PrivInfo
{
    HttpServerReq               req;
    HttpServerRsp               rsp;

    // onion                    *server;
    // onion_url                *urls;

    // web_handler_t            *sc_head;
    struct event_base           *base;
    struct evhttp               *http;
    struct evhttp_bound_socket  *handle;

    int                         cond;
    
    pthread_t                   thread;
} PrivInfo;


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

    printf("Received a %s request for %s\nHeaders:\n", cmdtype, evhttp_request_get_uri(req));

    headers = evhttp_request_get_input_headers(req);
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

    evhttp_send_reply(req, 200, "OK", NULL);
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

    evhttp_set_cb(thiz->http, "/carnet/sr/tg/demo", demo_request_cb, NULL);

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
