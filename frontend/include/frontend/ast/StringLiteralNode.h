#ifndef MYCC_FRONTEND_AST_STRING_LITERAL_NODE_H
#define MYCC_FRONTEND_AST_STRING_LITERAL_NODE_H

#include "AstNodeInfo.h"

#include "frontend/StrLit.h"

typedef struct {
    AstNodeInfo info;
} StringLiteralNode;

StringLiteralNode StringLiteralNode_create(uint32_t idx);

#endif

