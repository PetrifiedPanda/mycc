#include "frontend/ErrBase.h"

#include <stdio.h>
#include <assert.h>

#include "util/mem.h"

ErrBase create_err_base(SourceLoc loc) {
    return (ErrBase){
        .loc = loc
    };
}

void print_err_base(FILE* out, const FileInfo* file_info, const ErrBase* err) {
    assert(err->loc.file_idx < file_info->len);
    const char* path = str_get_data(&file_info->paths[err->loc.file_idx]);
    fprintf(out,
            "%s(%zu, %zu):\n",
            path,
            err->loc.file_loc.line,
            err->loc.file_loc.index);
}

