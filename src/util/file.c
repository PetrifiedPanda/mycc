#include "util/file.h"

#include <string.h>
#include <assert.h>

#include "util/mem.h"

char* file_read_line(FILE* file,
                     char* static_buf,
                     size_t static_buf_len,
                     size_t* len) {
    assert(len);

    size_t i = 0;
    int c;
    while ((c = getc(file)) != '\n' && c != EOF) {
        static_buf[i] = (char)c;
        ++i;
        if (i == static_buf_len - 1) {
            break;
        }
    }

    if (i == 0 && c == EOF) { // only EOF read
        return NULL;
    }

    char* res = static_buf;

    if (c != '\n' && c != EOF) {
        size_t len = static_buf_len * 2;
        char* dyn_buf = xmalloc(sizeof(char) * len);
        memcpy(dyn_buf, static_buf, (static_buf_len - 1) * sizeof(char));

        while ((c = getc(file)) != '\n' && c != EOF) {
            if (i == len - 1) {
                grow_alloc((void**)&dyn_buf, &len, sizeof(char));
            }

            dyn_buf[i] = (char)c;
            ++i;
        }

        res = dyn_buf;
    }
    res[i] = '\0';
    *len = i;
    return res;
}

