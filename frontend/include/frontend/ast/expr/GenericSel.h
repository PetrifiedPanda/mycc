#ifndef GENERIC_SEL_H
#define GENERIC_SEL_H

#include <stddef.h>
#include <stdbool.h>

#include "frontend/ast/AstNodeInfo.h"

#include "frontend/parser/ParserState.h"

typedef struct TypeName TypeName;
typedef struct AssignExpr AssignExpr;

typedef struct {
    AstNodeInfo info;
    TypeName* type_name; // if NULL this is the default case
    AssignExpr* assign;
} GenericAssoc;

typedef struct {
    AstNodeInfo info;
    size_t len;
    GenericAssoc* assocs;
} GenericAssocList;

typedef struct GenericSel {
    AstNodeInfo info;
    AssignExpr* assign;
    GenericAssocList assocs;
} GenericSel;

GenericSel* parse_generic_sel(ParserState* s);

void GenericSel_free(GenericSel* s);

void GenericAssocList_free(GenericAssocList* l);

void GenericAssoc_free_children(GenericAssoc* a);

#include "AssignExpr.h"

#endif

