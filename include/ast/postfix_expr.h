#ifndef POSTFIX_EXPR_H
#define POSTFIX_EXPR_H

#include "token_type.h"

#include "ast/arg_expr_list.h"

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
    struct primary_expr* primary;
    size_t len;
    struct postfix_suffix* suffixes;
};

void free_postfix_expr(struct postfix_expr* p);

#include "ast/primary_expr.h"
#include "ast/expr.h"
#include "ast/identifier.h"

#endif

