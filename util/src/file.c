#include "util/file.h"

#include <string.h>
#include <assert.h>

#include "util/mem.h"

#ifndef _WIN32

static void handle_win_line_ending(int newline_char, FILE* file) {
    if (newline_char == '\r') {
        int next = getc(file);
        if (next != '\n') {
            putc(next, file);
        }
    }
}

#endif

void file_read_line(FILE* file,
                    char** res,
                    size_t* res_len,
                    char* static_buf,
                    size_t static_buf_len) {
    assert(res);
    assert(res_len);

    int c;
    size_t cap;
    if (*res_len < static_buf_len) {
        *res = static_buf;
        bool copy_to_dyn_buf = false;
        while ((c = getc(file)) != '\n' && c != '\r' && c != EOF) {
            static_buf[*res_len] = (char)c;
            ++*res_len;
            if (*res_len == static_buf_len - 1) {
                copy_to_dyn_buf = true;
                break;
            }
        }
#ifndef _WIN32
        handle_win_line_ending(c, file);
#endif

        if (copy_to_dyn_buf) {
            cap = static_buf_len * 2;
            *res = xmalloc(sizeof **res * cap);
            memcpy(*res, static_buf, sizeof **res * (static_buf_len - 1));
        } else if (*res_len == 0 && c == EOF) {
            *res = NULL;
            return;
        } else {
            static_buf[*res_len] = '\0';
            *res = static_buf;
            return;
        }
    } else {
        cap = *res_len;
    }

    while ((c = getc(file)) != '\n' && c != '\r' && c != EOF) {
        if (*res_len == cap) {
            grow_alloc((void**)res, &cap, sizeof **res);
        }
        (*res)[*res_len] = (char)c;

        ++*res_len;
    }

#ifndef _WIN32
    handle_win_line_ending(c, file);
#endif

    if (*res_len == 0 && c == EOF) {
        assert(*res == NULL);
        return;
    }

    *res = xrealloc(*res, sizeof **res * (*res_len + 1));
    (*res)[*res_len] = '\0';
}

