#ifndef MYCC_FRONTEND_DECLARATION_ATOMIC_TYPE_SPEC_H
#define MYCC_FRONTEND_DECLARATION_ATOMIC_TYPE_SPEC_H

#include "frontend/parser/ParserState.h"

#include "frontend/ast/AstNodeInfo.h"

typedef struct TypeName TypeName;

typedef struct AtomicTypeSpec {
    AstNodeInfo info;
    TypeName* type_name;
} AtomicTypeSpec;

AtomicTypeSpec* parse_atomic_type_spec(ParserState* s);

void AtomicTypeSpec_free(AtomicTypeSpec* s);

#endif

