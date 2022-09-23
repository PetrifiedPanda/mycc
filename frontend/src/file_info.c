#include "frontend/file_info.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

struct file_info create_file_info(char* start_file) {
    struct file_info res = {
        .len = 1,
        .paths = xmalloc(sizeof *res.paths),
    };
    res.paths[0] = start_file;
    return res;
}

void file_info_add(struct file_info* info, char* path) {
    assert(path);

    ++info->len;
    info->paths = xrealloc(info->paths, sizeof *info->paths * info->len);
    info->paths[info->len - 1] = path;
}

void free_file_info(struct file_info* info) {
    for (size_t i = 0;  i < info->len; ++i) {
        free(info->paths[i]); 
    }
    free(info->paths);
}

