#include "frontend/ast/AstNodeInfo.h"

#include <assert.h>

AstNodeInfo AstNodeInfo_create(uint32_t token_idx) {
    return (AstNodeInfo) {
        .token_idx = token_idx,
    };
}

