#ifndef ALIGN_SPEC_H
#define ALIGN_SPEC_H

#include <stdbool.h>

#include "frontend/parser/ParserState.h"

#include "frontend/ast/AstNodeInfo.h"

typedef struct TypeName TypeName;
typedef struct ConstExpr ConstExpr;

typedef struct AlignSpec {
    AstNodeInfo info;
    bool is_type_name;
    union {
        TypeName* type_name;
        ConstExpr* const_expr;
    };
} AlignSpec;

bool parse_align_spec_inplace(ParserState* s, AlignSpec* res);

void AlignSpec_free_children(AlignSpec* s);

#include "frontend/ast/TypeName.h"

#include "frontend/ast/expr/ConstExpr.h"

#endif

