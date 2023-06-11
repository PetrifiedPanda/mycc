#include "util/file.h"

#include <stdbool.h>
#include <assert.h>

#include "util/mem.h"

#ifndef _WIN32

static void handle_win_line_ending(int newline_char, FILE* file) {
    if (newline_char == '\r') {
        int next = getc(file);
        if (next != '\n') {
            ungetc(next, file);
        }
    }
}

#endif

Str file_read_line(FILE* file,
                   StrBuf* str,
                   size_t* res_len,
                   char* static_buf,
                   size_t static_buf_len) {
    assert(file);
    assert(str);
    assert(static_buf_len == 0 || static_buf != NULL);

    int c;
    if (*res_len < static_buf_len && StrBuf_cap(str) < static_buf_len) {
        bool copy_to_str = false;
        while ((c = getc(file)) != '\n' && c != '\r' && c != EOF) {
            static_buf[*res_len] = (char)c;
            ++*res_len;
            if (*res_len == static_buf_len - 1) {
                copy_to_str = true;
                break;
            }
        }
#ifndef _WIN32
        handle_win_line_ending(c, file);
#endif
        if (copy_to_str) {
            size_t new_cap = static_buf_len * 2;
            StrBuf_reserve(str, new_cap);
            StrBuf_append(str, (Str){*res_len, static_buf});
        } else if (*res_len == 0 && c == EOF) {
            return Str_null();
        } else {
            static_buf[*res_len] = '\0';
            return (Str){*res_len, static_buf};
        }
    }

    while ((c = getc(file)) != '\n' && c != '\r' && c != EOF) {
        StrBuf_push_back(str, (char)c);
    }
#ifndef _WIN32
    handle_win_line_ending(c, file);
#endif
    *res_len = StrBuf_len(str);
    if (*res_len == 0 && c == EOF) {
        return Str_null();
    }
    return StrBuf_as_str(str);
}

