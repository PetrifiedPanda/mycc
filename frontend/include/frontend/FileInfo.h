#ifndef FILE_INFO_H
#define FILE_INFO_H

#include "util/Str.h"

#include <stddef.h>

typedef struct {
    size_t len;
    Str* paths;
} FileInfo;

FileInfo FileInfo_create(const Str* start_file);

void FileInfo_add(FileInfo* i, const Str* path);

void FileInfo_free(FileInfo* i);

#endif

