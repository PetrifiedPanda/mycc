#ifndef ENUM_SPEC_H
#define ENUM_SPEC_H

#include <stddef.h>

#include "frontend/ast/AstNodeInfo.h"

#include "frontend/parser/ParserState.h"

typedef struct Identifier Identifier;
typedef struct ConstExpr ConstExpr;

typedef struct {
    Identifier* identifier;
    ConstExpr* enum_val;
} Enumerator;

typedef struct {
    size_t len;
    Enumerator* enums;
} EnumList;

typedef struct EnumSpec {
    AstNodeInfo info;
    Identifier* identifier;
    EnumList enum_list;
} EnumSpec;

EnumSpec* parse_enum_spec(ParserState* s);

void EnumSpec_free(EnumSpec* s);

void EnumList_free(EnumList* l);

void Enumerator_free(Enumerator* e);

#include "frontend/ast/Identifier.h"

#include "frontend/ast/expr/ConstExpr.h"

#endif

