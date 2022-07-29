#include "frontend/err_base.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

struct err_base create_err_base(struct source_loc* loc) {
    assert(loc);

    char* file = loc->file;
    loc->file = NULL;
    return (struct err_base){
        .loc =
            {
                .file = file,
                .file_loc = loc->file_loc,
            },
    };
}

struct err_base create_err_base_copy(const struct source_loc* loc) {
    assert(loc);
    assert(loc->file);

    return (struct err_base){
        .loc =
            {
                .file = alloc_string_copy(loc->file),
                .file_loc = loc->file_loc,
            },
    };
}

void print_err_base(FILE* out, const struct err_base* err) {
    fprintf(out,
            "%s(%zu, %zu):\n",
            err->loc.file,
            err->loc.file_loc.line,
            err->loc.file_loc.index);
}

void free_err_base(struct err_base* err) {
    free_source_loc(&err->loc);
}

