#ifndef DIRECT_ABSTRACT_DECL_H
#define DIRECT_ABSTRACT_DECL_H

#include <stdbool.h>
#include <stddef.h>

#include "param_type_list.h"

#include "frontend/parser/parser_state.h"

struct abs_declarator;
struct assign_expr;

enum abs_arr_or_func_suffix_type {
    ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY, // either [] or [*]
    ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN,
    ABS_ARR_OR_FUNC_SUFFIX_FUNC
};

struct abs_arr_or_func_suffix {
    struct ast_node_info info;
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
    struct ast_node_info info;
    struct abs_declarator* bracket_decl;

    size_t len;
    struct abs_arr_or_func_suffix* following_suffixes;
};

struct direct_abs_declarator* parse_direct_abs_declarator(
    struct parser_state* s);

bool parse_abs_arr_or_func_suffixes(struct parser_state* s,
                                    struct direct_abs_declarator* res);

void free_direct_abs_declarator(struct direct_abs_declarator* d);

#include "abs_declarator.h"

#include "frontend/ast/expr/assign_expr.h"

#endif

