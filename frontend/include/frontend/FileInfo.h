#ifndef MYCC_FRONTEND_FILE_INFO_H
#define MYCC_FRONTEND_FILE_INFO_H

#include "util/StrBuf.h"

#include <stddef.h>

typedef struct {
    size_t len;
    StrBuf* paths;
} FileInfo;

FileInfo FileInfo_create(const StrBuf* start_file);

void FileInfo_add(FileInfo* i, const StrBuf* path);

void FileInfo_free(FileInfo* i);

#endif

