#include "ast/unary_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

static inline void assign_operators_before(struct unary_expr* res, enum token_type* operators_before, size_t len) {
    assert(res);
    if (len > 0) {
        assert(operators_before);
    } else {
        assert(operators_before == NULL);
    }
    for (size_t i = 0; i < len; ++i) {
        enum token_type op = operators_before[i];
        assert(op == SIZEOF || op == INC_OP || op == DEC_OP);
    }
    res->len = len;
    res->operators_before = operators_before;
}

struct unary_expr* create_unary_expr_postfix(enum token_type* operators_before, size_t len, struct postfix_expr* postfix) {
    assert(postfix);
    struct unary_expr* res = xmalloc(sizeof(struct unary_expr));
    assign_operators_before(res, operators_before, len);
    res->type = UNARY_POSTFIX;
    res->postfix = postfix;
    
    return res;
}

struct unary_expr* create_unary_expr_unary_op(enum token_type* operators_before, size_t len, enum token_type unary_op, struct cast_expr* cast_expr) {
    assert(is_unary_op(unary_op));
    assert(cast_expr);
    struct unary_expr* res = xmalloc(sizeof(struct unary_expr));
    assign_operators_before(res, operators_before, len);
    res->type = UNARY_UNARY_OP;
    res->unary_op = unary_op;
    res->cast_expr = cast_expr;
    
    return res;
}

struct unary_expr* create_unary_expr_sizeof_type(enum token_type* operators_before, size_t len, struct type_name* type_name) {
    assert(type_name);
    struct unary_expr* res = xmalloc(sizeof(struct unary_expr));
    assign_operators_before(res, operators_before, len);
    res->type = UNARY_SIZEOF_TYPE;
    res->type_name = type_name;
    
    return res;
}

struct unary_expr* create_unary_expr_alignof(enum token_type* operators_before, size_t len, struct type_name* type_name) {
    assert(type_name);
    struct unary_expr* res = xmalloc(sizeof(struct unary_expr));
    assign_operators_before(res, operators_before, len);
    res->type = UNARY_ALIGNOF_TYPE;
    res->type_name = type_name;

    return res;
}

void free_unary_expr_children(struct unary_expr* u) {
    free(u->operators_before);
    switch (u->type) {
        case UNARY_POSTFIX:
            free_postfix_expr(u->postfix);
            break;
        case UNARY_UNARY_OP:
            free_cast_expr(u->cast_expr);
            break;
        case UNARY_SIZEOF_TYPE:
        case UNARY_ALIGNOF_TYPE:
            free_type_name(u->type_name);
            break;
    }
}

void free_unary_expr(struct unary_expr* u) {
    free_unary_expr_children(u);
    free(u);
}

