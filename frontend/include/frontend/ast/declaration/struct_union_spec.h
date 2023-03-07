#ifndef STRUCT_UNION_SPEC_H
#define STRUCT_UNION_SPEC_H

#include <stddef.h>
#include <stdbool.h>

#include "frontend/ast/ast_node_info.h"

#include "frontend/parser/parser_state.h"

struct declarator;
struct const_expr;

struct struct_declarator {
    struct declarator* decl;
    struct const_expr* bit_field;
};

struct struct_declarator;

struct struct_declarator_list {
    size_t len;
    struct struct_declarator* decls;
};

struct declaration_specs;
struct static_assert_declaration;

struct struct_declaration {
    bool is_static_assert;
    union {
        struct {
            struct declaration_specs* decl_specs;
            struct struct_declarator_list decls;
        };
        struct static_assert_declaration* assert;
    };
};

struct struct_declaration_list {
    size_t len;
    struct struct_declaration* decls;
};

struct identifier;

struct struct_union_spec {
    struct ast_node_info info;
    bool is_struct;
    struct identifier* identifier;
    struct struct_declaration_list decl_list;
};

struct struct_union_spec* parse_struct_union_spec(struct parser_state* s);

void free_struct_union_spec(struct struct_union_spec* s);

void free_struct_declaration_list(struct struct_declaration_list* l);

void free_struct_declaration_children(struct struct_declaration* d);

void free_struct_declarator_list(struct struct_declarator_list* l);

void free_struct_declarator_children(struct struct_declarator* d);

#include "static_assert_declaration.h"

#include "frontend/ast/identifier.h"

#endif

