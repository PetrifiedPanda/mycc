#ifndef DIRECT_DECLARATOR_H
#define DIRECT_DECLARATOR_H

#include <stdbool.h>
#include <stddef.h>

#include "ast/param_type_list.h"
#include "ast/type_qual_list.h"
#include "ast/identifier_list.h"

struct assign_expr;
struct const_expr;
struct declarator;
struct identifier;

enum arr_or_func_suffix_type {
    ARR_OR_FUNC_ARRAY,
    ARR_OR_FUNC_FUN_TYPES,
    ARR_OR_FUNC_FUN_PARAMS
};

struct arr_suffix {
    bool is_static;
    struct type_qual_list type_quals;
    bool is_asterisk; // if this is true arr_len should be NULL
    struct assign_expr* arr_len;
};

struct arr_or_func_suffix {
    enum arr_or_func_suffix_type type;
    union {
        struct arr_suffix arr_suffix;
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
    struct arr_or_func_suffix* suffixes;
};

struct direct_declarator* create_direct_declarator(struct identifier* id, struct arr_or_func_suffix* suffixes, size_t len);
struct direct_declarator* create_direct_declarator_decl(struct declarator* decl, struct arr_or_func_suffix* suffixes, size_t len);

void free_direct_declarator(struct direct_declarator* d);

#include "ast/assign_expr.h"
#include "ast/const_expr.h"
#include "ast/declarator.h"
#include "ast/identifier.h"

#endif

