#include "frontend/ast/AstNodeInfo.h"

#include <assert.h>

AstNodeInfo create_ast_node_info(SourceLoc loc) {
    assert(loc.file_idx != (size_t)-1);
    assert(loc.file_loc.line != 0);
    assert(loc.file_loc.index != 0);
    return (AstNodeInfo) {
        .loc = loc,
    };
}

