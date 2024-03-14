#ifndef MYCC_FRONTEND_FILE_INFO_H
#define MYCC_FRONTEND_FILE_INFO_H

#include "util/StrBuf.h"

#include <stdint.h>

typedef struct FileInfo {
    uint32_t len;
    StrBuf* paths;
} FileInfo;

FileInfo FileInfo_create(const StrBuf* start_file);

void FileInfo_add(FileInfo* i, const StrBuf* path);

Str FileInfo_get(const FileInfo* i, uint32_t file_idx);

void FileInfo_free(FileInfo* i);

#endif

