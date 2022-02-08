#include "error.h"

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

static enum error_type g_type = ERR_NONE;
enum { MSG_BUF_SIZE = 512 };
static char g_msg_buf[MSG_BUF_SIZE] = {0};
static size_t g_msg_len = 0;

enum error_type get_last_error() {
    return g_type;
}

void clear_last_error() {
    g_type = ERR_NONE;
    g_msg_len = 0;
    for (size_t i = 0; i < MSG_BUF_SIZE; ++i) {
        g_msg_buf[i] = '\0';
    }
}

const char* get_error_string() {
    assert(g_type != ERR_NONE);
    return g_msg_buf;
}

const char* get_error_type_str(enum error_type t) {
    switch (t) {
        case ERR_NONE:
            return "ERR_NONE";
        case ERR_TOKENIZER:
            return "ERR_TOKENIZER";
        case ERR_PARSER:
            return "ERR_PARSER";
        default:
            return "INVALID ERROR TYPE";
    }
}

static inline void append_error_msg_va_list(const char* format, va_list args) {
    assert(format);

    size_t can_print = MSG_BUF_SIZE - g_msg_len;
    g_msg_len += vsnprintf(&g_msg_buf[g_msg_len], can_print, format, args);

    assert(g_msg_len < MSG_BUF_SIZE);
}

void set_error(enum error_type type, const char* format, ...) {
    assert(g_type == ERR_NONE);

    g_type = type;
    va_list(args);
    va_start(args, format);
    append_error_msg_va_list(format, args);
    va_end(args);
}

void set_error_file(enum error_type type,
                    const char* filename,
                    struct source_location loc,
                    const char* format,
                    ...) {
    assert(g_type == ERR_NONE);

    g_type = type;
    append_error_msg("%s(%zu,%zu):\n", filename, loc.line, loc.index);

    va_list(args);
    va_start(args, format);
    append_error_msg_va_list(format, args);
    va_end(args);
}

void append_error_msg(const char* format, ...) {
    assert(format);
    assert(g_type != ERR_NONE);

    va_list(args);
    va_start(args, format);

    append_error_msg_va_list(format, args);
    va_end(args);
}
