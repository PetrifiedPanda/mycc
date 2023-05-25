#ifndef AST_STRING_LITERAL_NODE_H
#define AST_STRING_LITERAL_NODE_H

#include "AstNodeInfo.h"

#include "frontend/StrLit.h"

typedef struct {
    AstNodeInfo info;
    StrLit lit;
} StringLiteralNode;

StringLiteralNode create_string_literal_node(const StrLit* lit, SourceLoc loc);

void free_string_literal(StringLiteralNode* l);

#endif

