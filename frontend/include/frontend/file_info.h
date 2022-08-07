#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <stddef.h>

struct file_info {
    size_t len;
    char** paths;
};

struct file_info create_file_info(char* start_file);

void file_info_add(struct file_info* i, char* path);

void free_file_info(struct file_info* i);

#endif

