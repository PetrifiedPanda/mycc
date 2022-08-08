#include "frontend/ast/ast_node_info.h"

#include <assert.h>

struct ast_node_info create_ast_node_info(struct source_loc loc) {
    assert(loc.file_idx != (size_t)-1);
    assert(loc.file_loc.line != 0);
    assert(loc.file_loc.index != 0);
    return (struct ast_node_info) {
        .loc = loc,
    };
}

