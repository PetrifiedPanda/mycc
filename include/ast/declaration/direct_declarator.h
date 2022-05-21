#ifndef DIRECT_DECLARATOR_H
#define DIRECT_DECLARATOR_H

#include <stdbool.h>
#include <stddef.h>

#include "ast/declaration/param_type_list.h"
#include "ast/declaration/identifier_list.h"

#include "parser/parser_state.h"

struct assign_expr;
struct const_expr;
struct declarator;
struct identifier;

enum arr_or_func_suffix_type {
    ARR_OR_FUNC_ARRAY,
    ARR_OR_FUNC_FUN_PARAMS,
    ARR_OR_FUNC_FUN_OLD_PARAMS,
    ARR_OR_FUNC_FUN_EMPTY
};

struct arr_suffix {
    bool is_static;
    struct type_quals type_quals;
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

struct direct_declarator* parse_direct_declarator(struct parser_state* s);
struct direct_declarator* parse_direct_declarator_typedef(
    struct parser_state* s);

void free_direct_declarator(struct direct_declarator* d);

#include "ast/declaration/declarator.h"

#include "ast/identifier.h"

#include "ast/expr/assign_expr.h"
#include "ast/expr/const_expr.h"

#endif