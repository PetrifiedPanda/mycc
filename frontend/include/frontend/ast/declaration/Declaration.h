#ifndef MYCC_FRONTEND_DECLARATION_DECLARATION_H
#define MYCC_FRONTEND_DECLARATION_DECLARATION_H

#include "InitDeclaratorList.h"

#include "frontend/parser/ParserState.h"

typedef struct DeclarationSpecs DeclarationSpecs;
typedef struct StaticAssertDeclaration StaticAssertDeclaration;

typedef struct Declaration {
    bool is_normal_decl;
    union {
        struct {
            DeclarationSpecs* decl_specs;
            InitDeclaratorList init_decls;
        };
        StaticAssertDeclaration* static_assert_decl;
    };
} Declaration;

bool parse_declaration_inplace(ParserState* s, Declaration* res);

void Declaration_free_children(Declaration* d);
void Declaration_free(Declaration* d);

#endif

