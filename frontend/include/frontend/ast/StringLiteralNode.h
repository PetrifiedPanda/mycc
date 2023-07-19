#ifndef MYCC_FRONTEND_AST_STRING_LITERAL_NODE_H
#define MYCC_FRONTEND_AST_STRING_LITERAL_NODE_H

#include "AstNodeInfo.h"

#include "frontend/StrLit.h"

typedef struct {
    AstNodeInfo info;
    StrLit lit;
} StringLiteralNode;

StringLiteralNode StringLiteralNode_create(const StrLit* lit, SourceLoc loc);

void StringLiteralNode_free(StringLiteralNode* l);

#endif

