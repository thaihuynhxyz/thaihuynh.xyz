#include <sys/types.h>
#include <microhttpd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <model/http_result.h>
#include <sys/stat.h>
#include <util/util.h>

#include "model/connection_info.h"
#include "service/URLRouter.h"

#define PORT 443
#define SERVERKEYFILE "privkey.pem"
#define SERVERCERTFILE "fullchain.pem"

#define POST_BUFFER_SIZE  1024
#define SITE_PATH "/blog"

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
    (void) kind;                 /* Unused. Silent compiler warning. */
    (void) filename;             /* Unused. Silent compiler warning. */
    (void) content_type;         /* Unused. Silent compiler warning. */
    (void) transfer_encoding;    /* Unused. Silent compiler warning. */
    (void) off;                  /* Unused. Silent compiler warning. */
    if (size) {
        log_info("%s: %s\n", key, data);
        struct connection_info *info = info_cls;
        HMap_set(info->params, strdup(key), (void *) strdup(data));
    }
    return MHD_YES;
}

static int iterate_get(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
    (void) kind;                 /* Unused. Silent compiler warning. */
    if (key && value) {
        log_info("%s: %s\n", key, value);
        struct connection_info *info = cls;
        HMap_set(info->params, strdup(key), (void *) strdup(value));
    }
    return MHD_YES;
}

static int iterate_header(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
    (void) kind;               /* Unused. Silent compiler warning. */

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
    (void) cls;               /* Unused. Silent compiler warning. */
    (void) connection;        /* Unused. Silent compiler warning. */
    (void) toe;               /* Unused. Silent compiler warning. */
    struct connection_info *con_info = *con_cls;
    if (con_info) {
        if (con_info->connection_type == POST) MHD_destroy_post_processor(con_info->post_processor);
        if (con_info->params) HMap_destroy(con_info->params);
        if (con_info->body) {
            free(con_info->body);
            con_info->body = NULL;
        }
        if (con_info->result) {
            if (con_info->result->message) {
                free(con_info->result->message);
                con_info->result->message = NULL;
            }
            free(con_info->result);
            con_info->result = NULL;
        }
        free(con_info);

        *con_cls = NULL;
    }
}

static ssize_t file_reader(void *cls, uint64_t pos, char *buf, size_t max) {
    FILE *file = cls;

    (void) fseek(file, (long) pos, SEEK_SET);
    return fread(buf, 1, max, file);
}

static void file_free_callback(void *cls) {
    FILE *file = cls;
    fclose(file);
}

static long get_file_size(const char *filename) {
    FILE *fp;

    fp = fopen(filename, "rb");
    if (fp) {
        long size;

        if ((0 != fseek(fp, 0, SEEK_END)) || (-1 == (size = ftell(fp)))) size = 0;

        fclose(fp);

        return size;
    } else {
        return 0;
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "DanglingPointers"
static char *load_file(const char *filename) {
    FILE *fp;
    char *buffer;
    long size;

    size = get_file_size(filename);
    if (0 == size) return NULL;

    fp = fopen(filename, "rb");
    if (!fp) return NULL;

    buffer = malloc(size + 1);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }
    buffer[size] = '\0';

    if (size != (long) fread(buffer, 1, size, fp)) {
        free(buffer);
        buffer = NULL;
    }

    fclose(fp);
    return buffer;
}
#pragma clang diagnostic pop

static int on_handle_connection(void *cls,
                                struct MHD_Connection *connection,
                                const char *url,
                                const char *method,
                                const char *version,
                                const char *upload_data,
                                size_t *upload_data_size,
                                void **con_cls) {
    log_info ("New %s request for %s using version %s\n", method, url, version);

    (void) cls;               /* Unused. Silent compiler warning. */

    struct connection_info *con_info;

    int ret;
    struct MHD_Response *response;

    if (!*con_cls) {
        con_info = malloc(sizeof(struct connection_info));
        check_mem(con_info)
        con_info->params = HMap_create(NULL, NULL);
        if (!strcmp(method, MHD_HTTP_METHOD_POST)) {  // init #iterate_post func for params analysis
            con_info->connection_type = POST;
            MHD_get_connection_values(connection, MHD_HEADER_KIND, iterate_header, (void *) con_info);
            con_info->post_processor = MHD_create_post_processor(connection, POST_BUFFER_SIZE, iterate_post,
                                                                 (void *) con_info);
        } else if (!strcmp(method, MHD_HTTP_METHOD_GET)) {
            con_info->connection_type = GET;
            con_info->get_connection = connection;
            con_info->body = NULL;
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

        FILE *file;
        struct stat buf;

        char *current_dir = get_current_dir();
        char *path = malloc(strlen(current_dir) + strlen(SITE_PATH) + strlen(url) + 1);
        strcpy(path, current_dir);
        strcat(path, SITE_PATH);
        strcat(path, url);

        if ((0 == stat(path, &buf)) && (S_ISREG (buf.st_mode))) { // NOLINT(hicpp-signed-bitwise)
            file = fopen(path, "rb");
        } else {
            file = NULL;
        }

        free(path);

        if (file == NULL) {
            con_info->result = URLRouter_route(router, url, con_info);
            response = MHD_create_response_from_buffer(strlen(con_info->result->message),
                                                       (void *) con_info->result->message, MHD_RESPMEM_PERSISTENT);
            ret = MHD_queue_response(connection, con_info->result->http_code, response);
            MHD_destroy_response(response);
        } else {
            response = MHD_create_response_from_callback((uint64_t) buf.st_size,
                                                         32 * 1024,     /* 32k PAGE_NOT_FOUND size */
                                                         &file_reader, file,
                                                         &file_free_callback);
            con_info->result = NULL;
            if (response == NULL) {
                fclose(file);
                return MHD_NO;
            }
            ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
            MHD_destroy_response(response);
        }
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
    char *key_pem;
    char *cert_pem;

    key_pem = load_file(SERVERKEYFILE);
    cert_pem = load_file(SERVERCERTFILE);

    if ((key_pem == NULL) || (cert_pem == NULL)) {
        printf("The key/certificate files could not be read.\n");
        if (NULL != key_pem)
            free(key_pem);
        if (NULL != cert_pem)
            free(cert_pem);
        return 1;
    }

    uint16_t port;
#ifdef NDEBUG
    port = PORT;
#else
    port = 8080;
#endif
    log_info("start on: https://localhost:%d\n", port);

    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_TLS, port, NULL, // NOLINT(hicpp-signed-bitwise)
                              NULL, &on_handle_connection, NULL,
                              MHD_OPTION_NOTIFY_COMPLETED, on_handle_complete, NULL,
                              MHD_OPTION_HTTPS_MEM_KEY, key_pem,
                              MHD_OPTION_HTTPS_MEM_CERT, cert_pem,
                              MHD_OPTION_END);
    check_mem(daemon)

    // router direct url to services
    router = URLRouter_create();

    // wait for any key press to exit
    while (getchar() != 'q');

    error:
    free(key_pem);
    free(cert_pem);
    exit_nicely(daemon);
    return 0;
}
