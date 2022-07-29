#ifndef PARAM_DECLARATION_H
#define PARAM_DECLARATION_H

#include <stdbool.h>

#include "frontend/parser/parser_state.h"

struct declaration_specs;
struct declarator;
struct abs_declarator;

enum param_decl_type {
    PARAM_DECL_DECL,
    PARAM_DECL_ABSTRACT_DECL,
    PARAM_DECL_NONE
};

struct param_declaration {
    struct declaration_specs* decl_specs;
    enum param_decl_type type;
    union {
        struct declarator* decl;
        struct abs_declarator* abstract_decl;
    };
};

bool parse_param_declaration_inplace(struct parser_state* s,
                                     struct param_declaration* res);

void free_param_declaration_children(struct param_declaration* d);

#include "declaration_specs.h"
#include "declarator.h"
#include "abs_declarator.h"

#endif

