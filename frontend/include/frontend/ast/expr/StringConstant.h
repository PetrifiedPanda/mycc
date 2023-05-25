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

StringConstant create_string_constant(const StrLit* lit, SourceLoc loc);

StringConstant create_func_name(SourceLoc loc);

void free_string_constant(StringConstant* c);

#endif

