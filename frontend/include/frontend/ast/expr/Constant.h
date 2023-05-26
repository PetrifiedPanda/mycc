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

Constant Constant_create_int(IntValue val,
                                    SourceLoc loc);
Constant Constant_create_float(FloatValue val,
                                      SourceLoc loc);

Constant Constant_create_enum(const Str* spelling,
                                     SourceLoc loc);

void Constant_free(Constant* c);

#endif

