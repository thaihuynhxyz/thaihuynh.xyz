---
layout: post
title:  "Web server"
date:   2020-02-29 23:54:27 +0700
categories: how-to
---
# [Web server](https://en.wikipedia.org/wiki/Web_server)
How to do a web server?

### I. Requirements
To do a web server we need to know:
- [Hypertext Transfer Protocol](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol) (HTTP) is the foundation of
data communication for the web.
- Uniform Resource Locator ([URL](https://en.wikipedia.org/wiki/URL)) to reference web page.
 
### II. Steps
There are two most popular web server: [Apache](https://httpd.apache.org/) and [Nginx](https://nginx.org/) but in this 
article, I would like to explain my code that built this website.

#### 1. GNU's [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/)
This is a small C library that run an HTTP server.

```c
#include <microhttpd.h>
#include <string.h>
#include <stdio.h>

#define PAGE "<html><head><title>My website</title>"\
             "</head><body>Hello world!</body></html>"

static int ahc_echo(void *cls,
                    struct MHD_Connection *connection,
                    const char *url,
                    const char *method,
                    const char *version,
                    const char *upload_data,
                    const size_t *upload_data_size,
                    void **ptr) {
  (void) url;         /* Unused. Silent compiler warning. */
  (void) version;     /* Unused. Silent compiler warning. */
  (void) upload_data; /* Unused. Silent compiler warning. */

  static int dummy;
  const char *page = cls;
  struct MHD_Response *response;
  int ret;

  if (0 != strcmp(method, MHD_HTTP_METHOD_GET)) return MHD_NO; /* unexpected method */
  if (&dummy != *ptr) {
    /* The first time only the headers are valid,
       do not respond in the first round... */
    *ptr = &dummy;
    return MHD_YES;
  }
  if (0 != *upload_data_size) return MHD_NO; /* upload data in a GET!? */
  *ptr = NULL; /* clear context pointer */
  response = MHD_create_response_from_buffer(strlen(page), (void *) page, MHD_RESPMEM_PERSISTENT);
  ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  return ret;
}

int main(int argc, char **argv) {
  struct MHD_Daemon *d;

  d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
                       8080,
                       NULL,
                       NULL,
                       (MHD_AccessHandlerCallback) &ahc_echo,
                       PAGE,
                       MHD_OPTION_END);
  if (d == NULL) return 1;
  (void) getc(stdin);
  MHD_stop_daemon(d);
  return 0;
}
```

Above code run an HTTP daemon to listen on port `8080`.
```c
MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
                         8080,
                         NULL,
                         NULL,
                         (MHD_AccessHandlerCallback) &ahc_echo,
                         PAGE,
                         MHD_OPTION_END);
``` 
If method is not `GET`, the server will response error `MHD_NO`
```c
if (0 != strcmp(method, MHD_HTTP_METHOD_GET)) return MHD_NO; /* unexpected method */
```

otherwise response the `PAGE` content with [HTML](https://en.wikipedia.org/wiki/HTML) format that is supported display 
in browsers.
```c
#define PAGE "<html><head><title>My website</title>"\
             "</head><body>Hello world!</body></html>"
```

#### 2. curl
To test the server, we can use run the application then open the browser with the address http://localhost:8080 or use
the [curl](https://curl.haxx.se/) tool.

```commandline
curl localhost:8080
```