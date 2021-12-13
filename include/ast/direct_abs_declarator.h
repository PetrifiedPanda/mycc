#ifndef DIRECT_ABSTRACT_DECL_H
#define DIRECT_ABSTRACT_DECL_H

#include <stdbool.h>
#include <stddef.h>

#include "ast/param_type_list.h"

struct abs_declarator;
struct const_expr;
struct assign_expr;

enum abs_arr_or_func_suffix_type {
    ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY, // either [] or [*]
    ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN,
    ABS_ARR_OR_FUNC_SUFFIX_ARRAY,
    ABS_ARR_OR_FUNC_SUFFIX_FUNC
};

struct abs_arr_or_func_suffix {
    enum abs_arr_or_func_suffix_type type;
    union {
        bool has_asterisk;
        struct {
            bool is_static; // only if assign != NULL
            struct type_qual_list type_quals;
            struct assign_expr* assign;
        };
        struct const_expr* array_size;
        struct param_type_list func_types;
    };
};

struct direct_abs_declarator {
    struct abs_declarator* bracket_decl;
    
    size_t len;
    struct abs_arr_or_func_suffix* following_suffixes;
};

struct direct_abs_declarator* create_direct_abs_declarator(struct abs_declarator* bracket_decl, struct abs_arr_or_func_suffix* following_suffixes, size_t len);

void free_direct_abs_declarator(struct direct_abs_declarator* d);

#include "ast/abs_declarator.h"
#include "ast/const_expr.h"
#include "ast/assign_expr.h"

#endif

