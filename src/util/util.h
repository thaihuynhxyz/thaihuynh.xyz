#ifndef BACKEND_UTIL_H
#define BACKEND_UTIL_H

#include <stdio.h>
#include <unistd.h>
#include <time.h>

/**
* Return The full path of current directory that application run on
*
* @return The current directory's full path
*/
static inline char *get_current_dir() {
    char currentPath[FILENAME_MAX];
    return getcwd(currentPath, sizeof(currentPath));
};

static inline char *get_cur_ctime() {
    time_t cur_time;
    time(&cur_time);
    return ctime(&cur_time);
}

#endif //BACKEND_UTIL_H
