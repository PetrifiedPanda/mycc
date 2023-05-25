#ifndef AST_NODE_INFO_H
#define AST_NODE_INFO_H

#include "frontend/Token.h"

typedef struct {
    SourceLoc loc; 
} AstNodeInfo;

AstNodeInfo create_ast_node_info(SourceLoc loc);

#endif

