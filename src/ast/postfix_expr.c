#include "ast/postfix_expr.h"

static void free_children(struct postfix_expr* p) {
    if (p->is_primary) {
        free_primary_expr(p->primary);
    } else {
        free_init_list_children(&p->init_list);
    }
    for (size_t i = 0; i < p->len; ++i) {
        struct postfix_suffix* s = &p->suffixes[i];
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
            case POSTFIX_INC_DEC:
                break;
        }
    }
    free(p->suffixes);
}

void free_postfix_expr(struct postfix_expr* p) {
    free_children(p);
    free(p);
}

