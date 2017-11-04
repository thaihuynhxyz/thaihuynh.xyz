#ifndef BACKEND_URLROUTER_H
#define BACKEND_URLROUTER_H

#include "util/backend_util.h"
#include "util/TSTree.h"

typedef struct URLRouter {
    // store services info
    TSTree *routes;
} URLRouter;

void *URLRouter_create();

void URLRouter_destroy(URLRouter *self);

struct http_result *URLRouter_route(URLRouter *self, const char *url, struct connection_info *info);

#endif //BACKEND_URLROUTER_H
