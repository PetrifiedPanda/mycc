#ifndef UTIL_FILE_H
#define UTIL_FILE_H

#include <stdio.h>

#include "Str.h"

const char* file_read_line(FILE* file,
                           Str* str,
                           size_t* res_len,
                           char* static_buf,
                           size_t static_buf_len);

#endif
