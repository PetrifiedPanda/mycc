#ifndef STRING_CONSTANT_H
#define STRING_CONSTANT_H

#include <stdbool.h>

#include "frontend/ast/StringLiteralNode.h"

typedef struct {
    bool is_func;
    union {
        StringLiteralNode lit;
        AstNodeInfo info;
    };
} StringConstant;

StringConstant StringConstant_create(const StrLit* lit, SourceLoc loc);

StringConstant StringConstant_create_func_name(SourceLoc loc);

void StringConstant_free(StringConstant* c);

#endif

