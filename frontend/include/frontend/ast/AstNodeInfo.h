#ifndef MYCC_FRONTEND_AST_AST_NODE_INFO_H
#define MYCC_FRONTEND_AST_AST_NODE_INFO_H

#include "frontend/Token.h"

typedef struct {
    SourceLoc loc; 
} AstNodeInfo;

AstNodeInfo AstNodeInfo_create(SourceLoc loc);

#endif

