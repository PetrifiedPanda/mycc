#include "ast/expr/const_expr.h"

#include <stdlib.h>

#include "util/mem.h"

#include "ast/ast_visitor.h"

struct const_expr* parse_const_expr(struct parser_state* s) {
    struct const_expr* res = xmalloc(sizeof(struct const_expr));
    if (!parse_cond_expr_inplace(s, &res->expr)) {
        free(res);
        return NULL;
    }
    return res;
}

static void free_children(struct const_expr* e) {
    free_cond_expr_children(&e->expr);
}

void free_const_expr(struct const_expr* e) {
    free_children(e);
    free(e);
}

static bool visit_children(struct ast_visitor* visitor, struct const_expr* e) {
    return visit_cond_expr(visitor, &e->expr);
}

bool visit_const_expr(struct ast_visitor* visitor, struct const_expr* e) {
    AST_VISITOR_VISIT_TEMPLATE(visitor, e, visit_children, visitor->visit_const_expr); 
}

