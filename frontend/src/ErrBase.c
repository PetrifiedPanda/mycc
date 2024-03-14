#include "frontend/ErrBase.h"

#include <assert.h>

ErrBase ErrBase_create(SourceLoc loc) {
    return (ErrBase){.loc = loc};
}

void ErrBase_print(File out, const FileInfo* file_info, const ErrBase* err) {
    assert(err->loc.file_idx < file_info->len);
    const Str path = FileInfo_get(file_info, err->loc.file_idx);
    File_printf(out,
                "{Str}({u32}, {u32}):\n",
                path,
                err->loc.file_loc.line,
                err->loc.file_loc.index);
}

