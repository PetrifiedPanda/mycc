#include "frontend/ast/StringLiteralNode.h"

StringLiteralNode StringLiteralNode_create(uint32_t idx) {
    return (StringLiteralNode){
        .info = AstNodeInfo_create(idx),
    };
}
