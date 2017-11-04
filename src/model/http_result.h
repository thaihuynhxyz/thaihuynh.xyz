#ifndef BACKEND_HTTP_RESULT_H
#define BACKEND_HTTP_RESULT_H

struct http_result {
    char *message;
    unsigned int http_code;
};

#endif //BACKEND_HTTP_RESULT_H
