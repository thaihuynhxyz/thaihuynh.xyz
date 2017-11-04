#include <model/http_result.h>
#include <microhttpd.h>

#include "URLRouter.h"

#define API "/"

struct http_result *Api_handle_get(struct connection_info *info) {
    log_info("func %s - conn", __func__);
    struct http_result *result = malloc(sizeof(struct http_result));
    result->http_code = MHD_HTTP_OK;
    result->message = "<html><body>Hi, my name is Thai Huynh!</body></html>";
    return result;
}

struct http_result *Api_handle(struct connection_info *info) {
    if (info->connection_type == GET) return Api_handle_get(info);

    struct http_result *result = malloc(sizeof(struct http_result));
    result->http_code = MHD_HTTP_METHOD_NOT_ALLOWED;
    result->message = "method not supported";
    return result;
}

void *URLRouter_create() {
    URLRouter *router = malloc(sizeof(URLRouter));

    router->routes = NULL;

    router->routes = TSTree_insert(router->routes, API, sizeof(API), Api_handle);

    return router;
}

void URLRouter_destroy(URLRouter *self) {
    if (self) {
        if (self->routes) TSTree_destroy(self->routes);
        free(self);
    }
}

struct http_result *URLRouter_route(URLRouter *self, const char *url, struct connection_info *info) {
    on_handle_request handler = TSTree_search(self->routes, url, strlen(url) + 1);
    if (handler) return handler(info);

    struct http_result *result = malloc(sizeof(struct http_result));
    result->http_code = MHD_HTTP_SERVICE_UNAVAILABLE;
    result->message = "service unavailable";
    return result;
}
