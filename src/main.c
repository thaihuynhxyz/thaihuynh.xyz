#include <sys/types.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <model/http_result.h>

#include "model/connection_info.h"
#include "service/URLRouter.h"
#include "util/dbg.h"
#include "util/HMap.h"

#define PORT 8080
#define POST_BUFFER_SIZE  1024

URLRouter *router;

/**
 * safe exit: stop MHD, finish PQ, free memory
 */
static void exit_nicely(struct MHD_Daemon *daemon) {
    MHD_stop_daemon(daemon);
    URLRouter_destroy(router);
    exit(1);
}

/**
 * POST params {"key":value} analysis
 *
 * @params key: key of param
 * @params data: value of param
 * @params size: size of data
 */
static int iterate_post(void *info_cls,
                        enum MHD_ValueKind kind,
                        const char *key,
                        const char *filename,
                        const char *content_type,
                        const char *transfer_encoding,
                        const char *data,
                        uint64_t off,
                        size_t size) {
    if (size) {
        log_info("%s: %s\n", key, data);
        struct connection_info *info = info_cls;
        HMap_set(info->params, strdup(key), (void *) strdup(data));
    }
    return MHD_YES;
}

static int iterate_get(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
    if (key && value) {
        log_info("%s: %s\n", key, value);
        struct connection_info *info = cls;
        HMap_set(info->params, strdup(key), (void *) strdup(value));
    }
    return MHD_YES;
}

static int iterate_header(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
    if (key && value) {
        log_info("%s: %s\n", key, value);
        struct connection_info *info = cls;
        HMap_set(info->params, strdup(key), (void *) strdup(value));
    }
    return MHD_YES;
}

/**
 * handle a request complete
 * free memory con_info
 *
 * @params cls:
 * @params connection:
 * @params con_cls:
 * @params toe:
 */
static void on_handle_complete(void *cls,
                               struct MHD_Connection *connection,
                               void **con_cls,
                               enum MHD_RequestTerminationCode toe) {
    struct connection_info *con_info = *con_cls;
    if (con_info) {
        if (con_info->connection_type == POST) MHD_destroy_post_processor(con_info->post_processor);
        if (con_info->params) HMap_destroy(con_info->params);
        if (con_info->body) free(con_info->body);
        if (con_info->result) free(con_info->result);
        free(con_info);

        *con_cls = NULL;
    }
}

static int on_handle_connection(void *cls,
                                struct MHD_Connection *connection,
                                const char *url,
                                const char *method,
                                const char *version,
                                const char *upload_data,
                                size_t *upload_data_size,
                                void **con_cls) {
    log_info ("New %s request for %s using version %s\n", method, url, version);
    struct connection_info *con_info;

    int ret;
    struct MHD_Response *response;

    if (!*con_cls) {
        con_info = malloc(sizeof(struct connection_info));
        check_mem(con_info);
        con_info->params = HMap_create(NULL, NULL);
        if (!strcmp(method, MHD_HTTP_METHOD_POST)) {  // init #iterate_post func for params analysis
            con_info->connection_type = POST;
            MHD_get_connection_values(connection, MHD_HEADER_KIND, iterate_header, (void *) con_info);
            con_info->post_processor = MHD_create_post_processor(connection, POST_BUFFER_SIZE, iterate_post,
                                                                 (void *) con_info);
        } else if (!strcmp(method, MHD_HTTP_METHOD_GET)) {
            con_info->connection_type = GET;
            con_info->get_connection = connection;
            MHD_get_connection_values(connection, MHD_HEADER_KIND, iterate_header, (void *) con_info);
            MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, iterate_get, (void *) con_info);
        }

        *con_cls = (void *) con_info;
        return MHD_YES;

        error:
        return MHD_NO;
    }

    con_info = *con_cls;
    if (!strcmp(method, MHD_HTTP_METHOD_GET)) {    // handle GET method
        con_info->result = URLRouter_route(router, url, con_info);
        response = MHD_create_response_from_buffer(strlen(con_info->result->message),
                                                   (void *) con_info->result->message, MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response(connection, con_info->result->http_code, response);
        MHD_destroy_response(response);
        return ret;
    }
    if (!strcmp(method, MHD_HTTP_METHOD_POST)) {   // handle POST method
        if (*upload_data_size) {    // get params and process
            if (con_info->post_processor) {
                MHD_post_process(con_info->post_processor, upload_data, *upload_data_size);
            } else {
                con_info->body = strdup(upload_data);
                con_info->body_length = *upload_data_size;
            }

            *upload_data_size = 0;

            con_info->result = URLRouter_route(router, url, con_info);
            return MHD_YES;
        }

        if (con_info->result) {  // return result
            response = MHD_create_response_from_buffer(strlen(con_info->result->message),
                                                       (void *) con_info->result->message, MHD_RESPMEM_PERSISTENT);
            ret = MHD_queue_response(connection, con_info->result->http_code, response);
            MHD_destroy_response(response);
            return ret;
        }
    }

    // return error page
    const char *page = "<html><body>An internal server error has occurred.</body></html>";
    response = MHD_create_response_from_buffer(strlen(page), (void *) page, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
    MHD_destroy_response(response);
    return ret;
}

int main() {
    // start micro HTTP daemon (MHD). when a request coming, we will handle it in #on_handle_connection
    struct MHD_Daemon *daemon;

    unsigned int flag;
    if (MHD_is_feature_supported(MHD_FEATURE_EPOLL))
        flag = MHD_USE_EPOLL_INTERNALLY_LINUX_ONLY;
    else if (MHD_is_feature_supported(MHD_FEATURE_POLL))
        flag = MHD_USE_POLL_INTERNALLY;
    else
        flag = MHD_USE_SELECT_INTERNALLY;
    daemon = MHD_start_daemon(flag, PORT, NULL, NULL, &on_handle_connection, NULL,
                              MHD_OPTION_NOTIFY_COMPLETED, on_handle_complete, NULL, MHD_OPTION_END);
    check_mem(daemon);

    // router direct url to services
    router = URLRouter_create();

    // wait for any key press to exit
    (void) getchar();

    error:
    exit_nicely(daemon);
    return 0;
}