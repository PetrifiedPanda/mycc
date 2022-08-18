#include "util/file.h"

#include <string.h>
#include <assert.h>

#include "util/mem.h"

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
        while ((c = getc(file)) != '\n' && c != EOF) {
            static_buf[*res_len] = (char)c;
            ++*res_len;
            if (*res_len == static_buf_len - 1) {
                copy_to_dyn_buf = true;
                break;
            }
        }

        if (copy_to_dyn_buf) {
            cap = static_buf_len * 2;
            *res = xmalloc(sizeof(char) * cap);
            memcpy(*res, static_buf, sizeof(char) * (static_buf_len - 1));
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

    while ((c = getc(file)) != '\n' && c != EOF) {
        if (*res_len == cap) {
            grow_alloc((void**)res, &cap, sizeof(char));
        }
        (*res)[*res_len] = (char)c;

        ++*res_len;
    }

    if (*res_len == 0 && c == EOF) {
        assert(*res == NULL);
        return;
    }

    *res = xrealloc(*res, sizeof(char) * (*res_len + 1));
    (*res)[*res_len] = '\0';
}

