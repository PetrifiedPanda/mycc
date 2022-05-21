#ifndef DIRECT_ABSTRACT_DECL_H
#define DIRECT_ABSTRACT_DECL_H

#include <stdbool.h>
#include <stddef.h>

#include "ast/declaration/param_type_list.h"

#include "parser/parser_state.h"

struct abs_declarator;
struct assign_expr;

enum abs_arr_or_func_suffix_type {
    ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY, // either [] or [*]
    ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN,
    ABS_ARR_OR_FUNC_SUFFIX_FUNC
};

struct abs_arr_or_func_suffix {
    enum abs_arr_or_func_suffix_type type;
    union {
        bool has_asterisk;
        struct {
            bool is_static; // only if assign != NULL
            struct type_quals type_quals;
            struct assign_expr* assign;
        };
        struct param_type_list func_types;
    };
};

struct direct_abs_declarator {
    struct abs_declarator* bracket_decl;

    size_t len;
    struct abs_arr_or_func_suffix* following_suffixes;
};

struct direct_abs_declarator* parse_direct_abs_declarator(
    struct parser_state* s);

void free_direct_abs_declarator(struct direct_abs_declarator* d);

#include "ast/declaration/abs_declarator.h"

#include "ast/expr/assign_expr.h"

#endif