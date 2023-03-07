#ifndef PARAM_TYPE_LIST_H
#define PARAM_TYPE_LIST_H

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

struct param_list {
    size_t len;
    struct param_declaration* decls;
};

struct param_type_list {
    bool is_variadic;
    struct param_list param_list;
};

bool parse_param_type_list(struct parser_state* s, struct param_type_list* res);

void free_param_type_list(struct param_type_list* l);

void free_param_list(struct param_list* l);

void free_param_declaration_children(struct param_declaration* d);

#include "declaration_specs.h"
#include "declarator.h"
#include "abs_declarator.h"

#endif

