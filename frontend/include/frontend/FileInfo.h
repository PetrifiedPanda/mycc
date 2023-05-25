#ifndef FILE_INFO_H
#define FILE_INFO_H

#include "util/Str.h"

#include <stddef.h>

typedef struct {
    size_t len;
    Str* paths;
} FileInfo;

FileInfo create_file_info(const Str* start_file);

void file_info_add(FileInfo* i, const Str* path);

void free_file_info(FileInfo* i);

#endif

