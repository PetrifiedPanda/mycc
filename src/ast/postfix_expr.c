#include "ast/postfix_expr.h"

#include <assert.h>

PostfixExpr* create_postfix_expr(PrimaryExpr* primary, PostfixSuffix* suffixes, size_t len) {
    assert(primary);
    if (len > 0) {
        assert(suffixes);
    } else {
        assert(suffixes == NULL);
    }
    PostfixExpr* res = malloc(sizeof(PostfixExpr));
    if (res) {
        res->primary = primary;
        res->len = len;
        res->suffixes = suffixes;
    }

    return res;
}

static void free_children(PostfixExpr* p) {
    free_primary_expr(p->primary);
    for (size_t i = 0; i < p->len; ++i) {
        PostfixSuffix* s = &p->suffixes[i];
        switch (s->type) {
            case POSTFIX_INDEX:
                free_expr(s->index_expr);
                break;
            case POSTFIX_BRACKET:
                free_arg_expr_list(&s->bracket_list);
                break;
            case POSTFIX_ACCESS:
            case POSTFIX_PTR_ACCESS:
                free(s->identifier);
                break;
        }
    }
    free(p->suffixes);
}

void free_postfix_expr(PostfixExpr* p) {
    free_children(p);
    free(p);
}
