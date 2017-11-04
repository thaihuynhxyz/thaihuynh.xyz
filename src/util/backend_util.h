#ifndef BACKEND_API_H
#define BACKEND_API_H

#include <stdarg.h>

#include "model/connection_info.h"

#define MAX_PARAM_SIZE 256

typedef struct http_result *(*on_handle_request)(struct connection_info *info);

#endif //BACKEND_API_H
