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

    const char *uri = evhttp_request_get_uri(req);
    printf("Received a %s request for %s\nHeaders:\n", cmdtype, uri);

    headers = evhttp_request_get_input_headers(req);
    for (header = headers->tqh_first; header; header = header->next.tqe_next)
    {
        printf("  %s: %s\n", header->key, header->value);
    }

    struct evhttp_uri *decoded = NULL;
    char *query = NULL;
    char *value = NULL;
    struct evkeyvalq params = {0};

    decoded = evhttp_uri_parse(uri);
    query = evhttp_uri_get_query(decoded);
    printf("query: %s\n", query);
    evhttp_parse_query_str(query, &params);
    value = (char*)evhttp_find_header(&params, "id");
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

#if 0
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
static const struct table_entry {
	const char *extension;
	const char *content_type;
} content_type_table[] = {
	{ "txt", "text/plain" },
	{ "c", "text/plain" },
	{ "h", "text/plain" },
	{ "html", "text/html" },
	{ "htm", "text/htm" },
	{ "css", "text/css" },
	{ "gif", "image/gif" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" },
	{ "pdf", "application/pdf" },
	{ "ps", "application/postscript" },
	{ NULL, NULL },
};
/* Try to guess a good content-type for 'path' */
static const char *guess_content_type(const char *path)
{
	const char *last_period, *extension;
	const struct table_entry *ent;
	last_period = strrchr(path, '.');
	if (!last_period || strchr(last_period, '/'))
		goto not_found; /* no exension */
	extension = last_period + 1;
	for (ent = &content_type_table[0]; ent->extension; ++ent) {
		if (!evutil_ascii_strcasecmp(ent->extension, extension))
			return ent->content_type;
	}

not_found:
	return "application/misc";
}
static void send_document_cb(struct evhttp_request *req, void *arg)
{
	struct evbuffer *evb = NULL;
	const char *docroot = arg;
	const char *uri = evhttp_request_get_uri(req);
	struct evhttp_uri *decoded = NULL;
	const char *path;
	char *decoded_path;
	char *whole_path = NULL;
	size_t len;
	int fd = -1;
	struct stat st;

	if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
		demo_request_cb(req, arg);
		return;
	}

	printf("Got a GET request for <%s>\n",  uri);

	/* Decode the URI */
	decoded = evhttp_uri_parse(uri);
	if (!decoded) {
		printf("It's not a good URI. Sending BADREQUEST\n");
		evhttp_send_error(req, HTTP_BADREQUEST, 0);
		return;
	}

	/* Let's see what path the user asked for. */
	path = evhttp_uri_get_path(decoded);
	if (!path) path = "/";

	/* We need to decode it, to see what path the user really wanted. */
	decoded_path = evhttp_uridecode(path, 0, NULL);
	if (decoded_path == NULL)
		goto err;
	/* Don't allow any ".."s in the path, to avoid exposing stuff outside
	 * of the docroot.  This test is both overzealous and underzealous:
	 * it forbids aceptable paths like "/this/one..here", but it doesn't
	 * do anything to prevent symlink following." */
	if (strstr(decoded_path, ".."))
		goto err;

	len = strlen(decoded_path)+strlen(docroot)+2;
	if (!(whole_path = malloc(len))) {
		perror("malloc");
		goto err;
	}
	evutil_snprintf(whole_path, len, "%s/%s", docroot, decoded_path);

	if (stat(whole_path, &st)<0) {
		goto err;
	}

	/* This holds the content we're sending. */
	evb = evbuffer_new();

	if (S_ISDIR(st.st_mode)) {
		/* If it's a directory, read the comments and make a little
		 * index page */

		DIR *d;
		struct dirent *ent;
		const char *trailing_slash = "";

		if (!strlen(path) || path[strlen(path)-1] != '/')
			trailing_slash = "/";

		if (!(d = opendir(whole_path)))
			goto err;

		evbuffer_add_printf(evb,
                    "<!DOCTYPE html>\n"
                    "<html>\n <head>\n"
                    "  <meta charset='utf-8'>\n"
		    "  <title>%s</title>\n"
		    "  <base href='%s%s'>\n"
		    " </head>\n"
		    " <body>\n"
		    "  <h1>%s</h1>\n"
		    "  <ul>\n",
		    decoded_path, /* XXX html-escape this. */
		    path, /* XXX html-escape this? */
		    trailing_slash,
		    decoded_path /* XXX html-escape this */);

		while ((ent = readdir(d))) {
			const char *name = ent->d_name;

			evbuffer_add_printf(evb,
			    "    <li><a href=\"%s\">%s</a>\n",
			    name, name);/* XXX escape this */
		}
		evbuffer_add_printf(evb, "</ul></body></html>\n");

		closedir(d);

		evhttp_add_header(evhttp_request_get_output_headers(req),
		    "Content-Type", "text/html");
	} else {
		/* Otherwise it's a file; add it to the buffer to get
		 * sent via sendfile */
		const char *type = guess_content_type(decoded_path);
		if ((fd = open(whole_path, O_RDONLY)) < 0) {
			perror("open");
			goto err;
		}

		if (fstat(fd, &st)<0) {
			/* Make sure the length still matches, now that we
			 * opened the file :/ */
			perror("fstat");
			goto err;
		}
		evhttp_add_header(evhttp_request_get_output_headers(req),
		    "Content-Type", type);
		evbuffer_add_file(evb, fd, 0, st.st_size);
	}

	evhttp_send_reply(req, 200, "OK", evb);
	goto done;
err:
	evhttp_send_error(req, 404, "Document was not found");
	if (fd>=0)
		close(fd);
done:
	if (decoded)
		evhttp_uri_free(decoded);
	if (decoded_path)
		free(decoded_path);
	if (whole_path)
		free(whole_path);
	if (evb)
		evbuffer_free(evb);
}
#endif

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

    //evhttp_set_gencb(thiz->http, send_document_cb, ".");

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
