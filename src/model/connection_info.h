#ifndef BACKEND_MODEL_CONNECTION_INFO_H
#define BACKEND_MODEL_CONNECTION_INFO_H

#include "util/HMap.h"

enum http_type {
    GET,
    POST,
    PUT,
    DELETE
};

struct connection_info {
    enum http_type connection_type;
    HMap *params;
    struct MHD_Connection *get_connection;
    struct MHD_PostProcessor *post_processor;
    char *body;
    unsigned long body_length;
    struct http_result *result;
};

#endif //BACKEND_MODEL_CONNECTION_INFO_H
