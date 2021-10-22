#include "error.h"

#include <stdio.h>

static ErrorType g_type = ERR_NONE;
enum {MSG_BUF_SIZE = 512};
static char g_msg_buf[MSG_BUF_SIZE] = {0};

ErrorType get_last_error() {
    return g_type;
}

void clear_last_error() {
    g_type = ERR_NONE;
}

const char* get_error_string() {
    return g_msg_buf;
}

void set_error(ErrorType type, const char* filename, SourceLocation loc, const char* format, ...) {
    g_type = type;
    sprintf(g_msg_buf, "%s(%zu,%zu):\n", filename, loc.line, loc.index);
    va_list(args);
    vsprintf(g_msg_buf, format, args);

}
