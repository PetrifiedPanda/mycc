#ifndef AST_CONSTANT_H
#define AST_CONSTANT_H

#include "frontend/ast/AstNodeInfo.h"

typedef enum {
    CONSTANT_ENUM,
    CONSTANT_VAL,
} ConstantKind;

typedef struct {
    AstNodeInfo info;
    ConstantKind kind;
    union {
        Str spelling;
        Value val;
    };
} Constant;

Constant Constant_create(Value val, SourceLoc loc);

Constant Constant_create_enum(const Str* spelling,
                                     SourceLoc loc);

void Constant_free(Constant* c);

#endif

