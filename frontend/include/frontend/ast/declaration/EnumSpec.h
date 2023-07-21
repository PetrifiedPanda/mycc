#ifndef MYCC_FRONTEND_DECLARATION_ENUM_SPEC_H
#define MYCC_FRONTEND_DECLARATION_ENUM_SPEC_H

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
    uint32_t len;
    Enumerator* enums;
} EnumList;

typedef struct EnumSpec {
    AstNodeInfo info;
    Identifier* identifier;
    EnumList enum_list;
} EnumSpec;

EnumSpec* parse_enum_spec(ParserState* s);

void Enumerator_free(Enumerator* e);
void EnumList_free(EnumList* l);
void EnumSpec_free(EnumSpec* s);


#endif

