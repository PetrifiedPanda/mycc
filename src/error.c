#include "error.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>

static ErrorType g_type = ERR_NONE;
enum {MSG_BUF_SIZE = 512};
static char g_msg_buf[MSG_BUF_SIZE] = {0};

ErrorType get_last_error() {
    return g_type;
}

void clear_last_error() {
    g_type = ERR_NONE;
    for (size_t i = 0; i < MSG_BUF_SIZE; ++i) {
        g_msg_buf[i] = '\0';
    }
}

const char* get_error_string() {
    return g_msg_buf;
}

static bool check_chars_printed(size_t num_printed, size_t can_print) {
    if (num_printed >= MSG_BUF_SIZE) {
        fprintf(stderr, "Error message was too large for message buffer\n");     
        return false;
    } else {
        return true;
    }
}

void set_error(ErrorType type, const char* format, ...) {
    assert(g_type == ERR_NONE);
    g_type = type;
    va_list(args);
    va_start(args, format);
    size_t num_printed = vsnprintf(g_msg_buf, MSG_BUF_SIZE, format, args);
    va_end(args);
    check_chars_printed(num_printed, MSG_BUF_SIZE);
}

void set_error_file(ErrorType type, const char* filename, SourceLocation loc, const char* format, ...) {
    assert(g_type == ERR_NONE);
    g_type = type;
    size_t num_printed = snprintf(g_msg_buf, MSG_BUF_SIZE, "%s(%zu,%zu):\n", filename, loc.line, loc.index);
    if (!check_chars_printed(num_printed, MSG_BUF_SIZE)) {
        return;
    }

    va_list(args);
    va_start(args, format);
    size_t can_print = MSG_BUF_SIZE - num_printed;
    num_printed = vsnprintf(&g_msg_buf[num_printed], can_print, format, args);
    va_end(args);

    check_chars_printed(num_printed, can_print);
}
