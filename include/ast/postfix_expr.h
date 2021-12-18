#ifndef POSTFIX_EXPR_H
#define POSTFIX_EXPR_H

#include "token_type.h"

#include "ast/arg_expr_list.h"
#include "ast/init_list.h"

struct primary_expr;
struct expr;
struct identifier;

enum postfix_suffix_type {
    POSTFIX_INDEX,
    POSTFIX_BRACKET,
    POSTFIX_ACCESS,
    POSTFIX_PTR_ACCESS,
    POSTFIX_INC_DEC
};

struct postfix_suffix {
    enum postfix_suffix_type type;
    union {
        struct expr* index_expr;
        struct arg_expr_list bracket_list;
        struct identifier* identifier;
        enum token_type inc_dec;
    };
};

struct postfix_expr {
    bool is_primary;
    union {
        struct primary_expr* primary;
        struct {
            struct type_name* type_name;
            struct init_list init_list;
        };
    };
    size_t len;
    struct postfix_suffix* suffixes;
};

struct postfix_expr* parse_postfix_expr(struct parser_state* s);

struct postfix_expr* parse_postfix_expr_type_name(struct parser_state* s, struct type_name* type_name);

void free_postfix_expr(struct postfix_expr* p);

#include "ast/primary_expr.h"
#include "ast/expr.h"
#include "ast/identifier.h"

#endif

