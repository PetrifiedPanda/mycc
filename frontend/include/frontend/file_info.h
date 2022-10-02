#ifndef FILE_INFO_H
#define FILE_INFO_H

#include "util/str.h"

#include <stddef.h>

struct file_info {
    size_t len;
    struct str* paths;
};

struct file_info create_file_info(const struct str* start_file);

void file_info_add(struct file_info* i, const struct str* path);

void free_file_info(struct file_info* i);

#endif

