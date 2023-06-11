#include "frontend/ErrBase.h"

#include <stdio.h>
#include <assert.h>

#include "util/mem.h"

ErrBase ErrBase_create(SourceLoc loc) {
    return (ErrBase){
        .loc = loc
    };
}

void ErrBase_print(FILE* out, const FileInfo* file_info, const ErrBase* err) {
    assert(err->loc.file_idx < file_info->len);
    const char* path = StrBuf_data(&file_info->paths[err->loc.file_idx]);
    fprintf(out,
            "%s(%zu, %zu):\n",
            path,
            err->loc.file_loc.line,
            err->loc.file_loc.index);
}

