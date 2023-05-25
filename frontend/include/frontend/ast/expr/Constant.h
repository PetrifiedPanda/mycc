#ifndef AST_CONSTANT_H
#define AST_CONSTANT_H

#include "frontend/ast/AstNodeInfo.h"

typedef enum {
    CONSTANT_ENUM,
    CONSTANT_INT,
    CONSTANT_FLOAT,
} ConstantKind;

typedef struct {
    AstNodeInfo info;
    ConstantKind kind;
    union {
        Str spelling;
        IntValue int_val;
        FloatValue float_val;
    };
} Constant;

Constant create_int_constant(IntValue val,
                                    SourceLoc loc);
Constant create_float_constant(FloatValue val,
                                      SourceLoc loc);

Constant create_enum_constant(const Str* spelling,
                                     SourceLoc loc);

void free_constant(Constant* c);

#endif

