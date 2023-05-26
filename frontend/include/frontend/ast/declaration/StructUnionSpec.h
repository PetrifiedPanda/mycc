#ifndef STRUCT_UNION_SPEC_H
#define STRUCT_UNION_SPEC_H

#include <stddef.h>
#include <stdbool.h>

#include "frontend/ast/AstNodeInfo.h"

#include "frontend/parser/ParserState.h"

typedef struct Declarator Declarator;
typedef struct ConstExpr ConstExpr;

typedef struct {
    Declarator* decl;
    ConstExpr* bit_field;
} StructDeclarator;

typedef struct {
    size_t len;
    StructDeclarator* decls;
} StructDeclaratorList;

typedef struct DeclarationSpecs DeclarationSpecs;
typedef struct StaticAssertDeclaration StaticAssertDeclaration;

typedef struct {
    bool is_static_assert;
    union {
        struct {
            DeclarationSpecs* decl_specs;
            StructDeclaratorList decls;
        };
        StaticAssertDeclaration* assert;
    };
} StructDeclaration;

typedef struct {
    size_t len;
    StructDeclaration* decls;
} StructDeclarationList;

typedef struct Identifier Identifier;

typedef struct StructUnionSpec {
    AstNodeInfo info;
    bool is_struct;
    Identifier* identifier;
    StructDeclarationList decl_list;
} StructUnionSpec;

StructUnionSpec* parse_struct_union_spec(ParserState* s);

void StructUnionSpec_free(StructUnionSpec* s);

void StructDeclarationList_free(StructDeclarationList* l);

void StructDeclaration_free_children(StructDeclaration* d);

void StructDeclaratorList_free(StructDeclaratorList* l);

void StructDeclarator_free_children(StructDeclarator* d);

#include "StaticAssertDeclaration.h"

#include "frontend/ast/Identifier.h"

#endif

