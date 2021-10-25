#ifndef ERROR_H
#define ERROR_H

#include "token.h"

typedef enum {
    ERR_NONE,
    ERR_ALLOC_FAIL,
    ERR_TOKENIZER,
    ERR_PARSER
} ErrorType;

ErrorType get_last_error();
void clear_last_error();
const char* get_error_string();

void set_error(ErrorType type, const char* format, ...);
void set_error_file(ErrorType type, const char* filename, SourceLocation loc, const char* format, ...); 

#endif
