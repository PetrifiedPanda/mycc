#ifndef MYCC_FRONTEND_AST_AST_NODE_INFO_H
#define MYCC_FRONTEND_AST_AST_NODE_INFO_H

#include "frontend/Token.h"

typedef struct {
    uint32_t token_idx; 
} AstNodeInfo;

AstNodeInfo AstNodeInfo_create(uint32_t token_idx);

#endif

