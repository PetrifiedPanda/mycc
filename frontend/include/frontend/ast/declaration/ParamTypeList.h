#ifndef PARAM_TYPE_LIST_H
#define PARAM_TYPE_LIST_H

#include <stdbool.h>

#include "frontend/parser/ParserState.h"

typedef struct DeclarationSpecs DeclarationSpecs;
typedef struct Declarator Declarator;
typedef struct AbsDeclarator AbsDeclarator;

typedef enum {
    PARAM_DECL_DECL,
    PARAM_DECL_ABSTRACT_DECL,
    PARAM_DECL_NONE
} ParamDeclKind;

typedef struct {
    DeclarationSpecs* decl_specs;
    ParamDeclKind kind;
    union {
        Declarator* decl;
        AbsDeclarator* abstract_decl;
    };
} ParamDeclaration;

typedef struct {
    size_t len;
    ParamDeclaration* decls;
} ParamList;

typedef struct {
    bool is_variadic;
    ParamList param_list;
} ParamTypeList;

bool parse_param_type_list(ParserState* s, ParamTypeList* res);

void free_param_type_list(ParamTypeList* l);

void free_param_list(ParamList* l);

void free_param_declaration_children(ParamDeclaration* d);

#include "DeclarationSpecs.h"
#include "Declarator.h"
#include "AbsDeclarator.h"

#endif

