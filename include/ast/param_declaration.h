#ifndef PARAM_DECLARATION_H
#define PARAM_DECLARATION_H

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

struct param_declaration* create_param_declaration(struct declaration_specs* decl_specs);
struct param_declaration* create_param_declaration_declarator(struct declaration_specs* decl_specs, struct declarator* decl);
struct param_declaration* create_param_declaration_abstract(struct declaration_specs* decl_specs, struct abs_declarator* abstract_decl);

void free_param_declaration_children(struct param_declaration* d);
void free_param_declaration(struct param_declaration* d);

#include "ast/declaration_specs.h"
#include "ast/declarator.h"
#include "ast/abs_declarator.h"


#endif

