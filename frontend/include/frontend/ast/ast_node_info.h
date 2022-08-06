#ifndef AST_NODE_INFO_H
#define AST_NODE_INFO_H

#include "frontend/token.h"

struct ast_node_info {
    struct source_loc loc; 
};

struct ast_node_info create_ast_node_info(struct source_loc loc);

void free_ast_node_info(struct ast_node_info* info);

#endif

