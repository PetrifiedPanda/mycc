#ifndef ERROR_H
#define ERROR_H

typedef enum {
    NO_ERROR,
    ALLOC_FAIL,
    TOKENIZER,
    PARSER
} ErrorType;

ErrorType get_last_error();
void clear_last_error();
const char* get_error_string();

void set_error(ErrorType type, const char* format, ...);

#endif