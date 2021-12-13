#ifndef DIRECT_DECLARATOR_H
#define DIRECT_DECLARATOR_H

// TODO: update with new grammar

#include <stdbool.h>
#include <stddef.h>

#include "ast/param_type_list.h"
#include "ast/identifier_list.h"

struct const_expr;
struct declarator;
struct identifier;

enum arr_or_func_suffix_type {
    ARR_OR_FUNC_ARRAY,
    ARR_OR_FUNC_FUN_TYPES,
    ARR_OR_FUNC_FUN_PARAMS
};

struct arr_or_func_suffix_direct_decl {
    enum arr_or_func_suffix_type type;
    union {
        struct const_expr* arr_len;
        struct param_type_list fun_types;
        struct identifier_list fun_params;
    };
};

struct direct_declarator {
    bool is_id;
    union {
        struct identifier* id;
        struct declarator* decl;
    };
    size_t len;
    struct arr_or_func_suffix_direct_decl* suffixes;
};

struct direct_declarator* create_direct_declarator(struct identifier* id, struct arr_or_func_suffix_direct_decl* suffixes, size_t len);
struct direct_declarator* create_direct_declarator_decl(struct declarator* decl, struct arr_or_func_suffix_direct_decl* suffixes, size_t len);

void free_direct_declarator(struct direct_declarator* d);

#include "ast/const_expr.h"
#include "ast/declarator.h"
#include "ast/identifier.h"

#endif

