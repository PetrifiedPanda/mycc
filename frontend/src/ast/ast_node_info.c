#include "frontend/ast/ast_node_info.h"

struct ast_node_info create_ast_node_info(struct source_loc loc) {
    return (struct ast_node_info) {
        .loc = loc,
    };
}
