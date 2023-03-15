#ifndef POSTFIX_EXPR_H
#define POSTFIX_EXPR_H

#include "arg_expr_list.h"

#include "frontend/ast/initializer.h"

struct primary_expr;
struct expr;
struct identifier;

enum postfix_suffix_kind {
    POSTFIX_INDEX,
    POSTFIX_BRACKET,
    POSTFIX_ACCESS,
    POSTFIX_PTR_ACCESS,
    POSTFIX_INC,
    POSTFIX_DEC,
};

struct postfix_suffix {
    enum postfix_suffix_kind kind;
    union {
        struct expr* index_expr;
        struct arg_expr_list bracket_list;
        struct identifier* identifier;
    };
};

struct postfix_expr {
    bool is_primary;
    union {
        struct primary_expr* primary;
        struct {
            struct ast_node_info info;
            struct type_name* type_name;
            struct init_list init_list;
        };
    };
    size_t len;
    struct postfix_suffix* suffixes;
};

struct postfix_expr* parse_postfix_expr(struct parser_state* s);

/**
 *
 * @param s current state
 * @param type_name A type name that was already parsed by parse_unary_expr
 * @param start_bracket_loc Location of the bracket starting this expr
 *
 * @return A postfix_expr that uses the given type_name
 */
struct postfix_expr* parse_postfix_expr_type_name(struct parser_state* s,
                                                  struct type_name* type_name,
                                                  struct source_loc start_bracket_loc);

void free_postfix_expr(struct postfix_expr* p);

#include "primary_expr.h"
#include "expr.h"

#include "frontend/ast/identifier.h"

#endif

