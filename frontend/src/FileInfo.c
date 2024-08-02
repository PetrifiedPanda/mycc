#include "frontend/FileInfo.h"

#include <assert.h>

#include "util/mem.h"

FileInfo FileInfo_create(const StrBuf* start_file) {
    FileInfo res = {
        .len = 1,
        .paths = mycc_alloc(sizeof *res.paths),
    };
    res.paths[0] = *start_file;
    return res;
}

void FileInfo_add(FileInfo* info, const StrBuf* path) {
    assert(path);

    ++info->len;
    info->paths = mycc_realloc(info->paths, sizeof *info->paths * info->len);
    info->paths[info->len - 1] = *path;
}

Str FileInfo_get(const FileInfo* i, uint32_t file_idx) {
    assert(file_idx < i->len);
    return StrBuf_as_str(&i->paths[file_idx]);
}

void FileInfo_free(const FileInfo* info) {
    for (uint32_t i = 0; i < info->len; ++i) {
        StrBuf_free(&info->paths[i]);
    }
    mycc_free(info->paths);
}
