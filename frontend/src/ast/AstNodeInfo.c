#include "frontend/ast/AstNodeInfo.h"

#include <assert.h>

AstNodeInfo AstNodeInfo_create(SourceLoc loc) {
    assert(loc.file_idx != (uint32_t)-1);
    assert(loc.file_loc.line != 0);
    assert(loc.file_loc.index != 0);
    return (AstNodeInfo) {
        .loc = loc,
    };
}

