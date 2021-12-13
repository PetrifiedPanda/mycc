#ifndef DIRECT_ABSTRACT_DECL_H
#define DIRECT_ABSTRACT_DECL_H

// TODO: update with new grammar

#include <stdbool.h>
#include <stddef.h>

#include "ast/param_type_list.h"

struct abstract_declarator;
struct const_expr;

enum direct_abstract_decl_type {
    DIRECT_ABSTRACT_DECL_ARRAY,
    DIRECT_ABSTRACT_DECL_ABSTRACT_DECL,
    DIRECT_ABSTRACT_DECL_PARAM_TYPE_LIST
};

struct arr_or_func_suffix_direct_abs_decl {
    bool is_array_decl;
    union {
        struct const_expr* array_size;
        struct param_type_list func_types;
    };
};

struct direct_abstract_declarator {
    enum direct_abstract_decl_type type;
    union {
        struct const_expr* array_size;
        struct abstract_declarator* bracket_decl;
        struct param_type_list func_types;
    };
    
    size_t len;
    struct arr_or_func_suffix_direct_abs_decl* following_suffixes;
};

struct direct_abstract_declarator* create_direct_abstract_declarator_arr(struct const_expr* array_size, struct arr_or_func_suffix_direct_abs_decl* following_suffixes, size_t len);
struct direct_abstract_declarator* create_direct_abstract_declarator_abs_decl(struct abstract_declarator* bracket_decl, struct arr_or_func_suffix_direct_abs_decl* following_suffixes, size_t len);
struct direct_abstract_declarator* create_direct_abstract_declarator_param_list(struct param_type_list func_types, struct arr_or_func_suffix_direct_abs_decl* following_suffixes, size_t len);

void free_direct_abstract_declarator(struct direct_abstract_declarator* d);

#include "ast/abstract_declarator.h"
#include "ast/const_expr.h"

#endif

