#include <model/http_result.h>
#include <microhttpd.h>
#include <util/util.h>

#include "URLRouter.h"

#define API "/"
#define ABOUT "/res/index.html"

struct http_result *Api_handle_get() {
    log_info("func %s - conn", __func__);
    struct http_result *result = malloc(sizeof(struct http_result));
    result->http_code = MHD_HTTP_OK;
    char *buffer = NULL;
    size_t size = 0;

    char *current_dir = get_current_dir();
    char *path = malloc(strlen(current_dir) + strlen(ABOUT) + 1);
    strcpy(path, current_dir);
    strcat(path, ABOUT);

    /* Open your_file in read-only mode */
    FILE *fp = fopen(path, "r");
    free(path);

    /* Get the buffer size */
    fseek(fp, 0, SEEK_END); /* Go to end of file */
    size = (size_t) ftell(fp); /* How many bytes did we pass ? */

    /* Set position of stream to the beginning */
    rewind(fp);

    /* Allocate the buffer (no need to initialize it with calloc) */
    buffer = malloc((size + 1) * sizeof(*buffer)); /* size + 1 byte for the \0 */

    /* Read the file into the buffer */
    fread(buffer, size, 1, fp); /* Read 1 chunk of size bytes from fp into buffer */

    /* NULL-terminate the buffer */
    buffer[size] = '\0';

    result->message = buffer;

    return result;
}

struct http_result *Api_handle(struct connection_info *info) {
    if (info->connection_type == GET) return Api_handle_get();

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
    result->http_code = MHD_HTTP_FORBIDDEN;
    result->message = "Forbidden";
    return result;
}
