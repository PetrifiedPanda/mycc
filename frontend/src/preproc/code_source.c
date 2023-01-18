#include "frontend/preproc/code_source.h"

#include <assert.h>
#include <string.h>

#include "util/mem.h"
#include "util/file.h"

#ifdef MYCC_TEST_FUNCTIONALITY
struct code_source create_code_source_string(const char* str, const char* path) {
    assert(str);
    assert(path);
    return (struct code_source){
        ._is_str = true,
        ._str = str,
        .path = path,
        .current_line = 1,
        .comment_not_terminated = false,
    };
}

static void string_read_line(const char** str,
                             char** res,
                             size_t* res_len,
                             char* static_buf,
                             size_t static_buf_len) {
    const char* start = *str;
    const char* it = *str;
    if (*res_len < static_buf_len) {
        *res = static_buf;
        bool use_dyn_buf = false;
        size_t new_res_len = *res_len;
        while (*it != '\n' && *it != '\0') {
            ++it;
            ++new_res_len;
            if (new_res_len == static_buf_len - 1) {
                use_dyn_buf = true;
                *res = NULL;
                break;
            }
        }

        if (!use_dyn_buf) {
            if (it == start && *res_len == 0) {
                *res = NULL;
            } else {
                const size_t len = it - start;
                *str = *it == '\0' ? it : it + 1;
                memcpy(static_buf + *res_len, start, sizeof *static_buf * len);
                *res_len += len;
                static_buf[len] = '\0';
            }
            return;
        }
    }

    while (*it != '\n' && *it != '\0') {
        ++it;
    }

    *str = *it == '\0' ? it : it + 1;
    const size_t len = it - start;
    const size_t prev_res_len = *res_len;
    *res_len += len;
    if (len == 0) {
        return;
    }
    *res = xrealloc(*res, sizeof **res * (*res_len + 1));
    memcpy(*res + prev_res_len, start, sizeof **res * len);
    (*res)[*res_len] = '\0';
}

#endif // MYCC_TEST_FUNCTIONALITY

struct code_source create_code_source_file(FILE* file, const char* path) {
    assert(file);
    assert(path);
    return (struct code_source){
#ifdef MYCC_TEST_FUNCTIONALITY
        ._is_str = false,
#endif
        ._file = file,
        .path = path,
        .current_line = 1,
        .comment_not_terminated = false,
    };
}

bool code_source_over(struct code_source* src) {
#ifdef MYCC_TEST_FUNCTIONALITY
    if (src->_is_str) {
        return *src->_str == '\0';
    } else
#endif
    {
        return feof(src->_file);
    }
}

char* code_source_read_line(struct code_source* src,
                                   size_t static_buf_len,
                                   char* static_buf) {
    char* res = NULL;
    bool escaped_newline = false;
    size_t len = 0;
    do {
#ifdef MYCC_TEST_FUNCTIONALITY
        if (src->_is_str) {
            string_read_line(&src->_str,
                             &res,
                             &len,
                             static_buf,
                             static_buf_len);
        } else 
#endif
        {
            file_read_line(src->_file,
                           &res,
                           &len,
                           static_buf,
                           static_buf_len);
        }

        if (res != NULL && len > 0) {
            escaped_newline = res[len - 1] == '\\';
            // TODO: newlines not contained when escaped newline is found
        }
        ++src->current_line;
    } while (escaped_newline);

    return res;
}

