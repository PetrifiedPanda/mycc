#include "frontend/err_base.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

struct err_base create_err_base(struct source_loc loc) {
    return (struct err_base){
        .loc = loc
    };
}

void print_err_base(FILE* out, const struct file_info* file_info, const struct err_base* err) {
    assert(err->loc.file_idx < file_info->len);
    const char* path = file_info->paths[err->loc.file_idx];
    fprintf(out,
            "%s(%zu, %zu):\n",
            path,
            err->loc.file_loc.line,
            err->loc.file_loc.index);
}

