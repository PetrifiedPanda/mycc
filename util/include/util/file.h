#ifndef UTIL_FILE_H
#define UTIL_FILE_H

#include <stdio.h>

#include "StrBuf.h"

Str file_read_line(FILE* file,
                   StrBuf* str,
                   size_t* res_len,
                   char* static_buf,
                   size_t static_buf_len);

#endif
