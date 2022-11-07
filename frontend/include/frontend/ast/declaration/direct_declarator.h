#ifndef DIRECT_DECLARATOR_H
#define DIRECT_DECLARATOR_H

#include <stdbool.h>
#include <stddef.h>

#include "param_type_list.h"
#include "identifier_list.h"

#include "frontend/parser/parser_state.h"

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
    struct ast_node_info info;
    enum arr_or_func_suffix_type type;
    union {
        struct arr_suffix arr_suffix;
        struct param_type_list fun_types;
        struct identifier_list fun_params;
    };
};

struct direct_declarator {
    struct ast_node_info info;
    bool is_id;
    union {
        struct identifier* id;
        struct declarator* bracket_decl;
    };
    size_t len;
    struct arr_or_func_suffix* suffixes;
};

/**
 * @param s current parser_state
 * @param res direct_decl already initialized except suffixes, will be freed on
 *            failure
 */
bool parse_arr_or_func_suffixes(struct parser_state* s,
                                struct direct_declarator* res);

struct direct_declarator* parse_direct_declarator(struct parser_state* s);
struct direct_declarator* parse_direct_declarator_typedef(
    struct parser_state* s);

void free_direct_declarator(struct direct_declarator* d);

#include "declarator.h"

#include "frontend/ast/identifier.h"

#include "frontend/ast/expr/assign_expr.h"
#include "frontend/ast/expr/const_expr.h"

#endif

