#include "frontend/ErrBase.h"

#include <stdio.h>
#include <assert.h>

#include "util/mem.h"

ErrBase ErrBase_create(SourceLoc loc) {
    return (ErrBase){.loc = loc};
}

void ErrBase_print(File out, const FileInfo* file_info, const ErrBase* err) {
    assert(err->loc.file_idx < file_info->len);
    const Str path = StrBuf_as_str(&file_info->paths[err->loc.file_idx]);
    File_printf(out,
                "{Str}({size_t}, {size_t}):\n",
                path,
                err->loc.file_loc.line,
                err->loc.file_loc.index);
}

