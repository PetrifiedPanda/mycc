#ifndef UTIL_FILE_H
#define UTIL_FILE_H

#include <stdio.h>

char* file_read_line(FILE* file,
                     char* static_buf,
                     size_t static_buf_len,
                     size_t* len);

#endif

