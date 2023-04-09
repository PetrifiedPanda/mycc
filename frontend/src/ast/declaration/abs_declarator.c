#include "frontend/ast/declaration/abs_declarator.h"

#include "util/mem.h"

struct abs_declarator* parse_abs_declarator(struct parser_state* s) {
    struct abs_declarator* res = mycc_alloc(sizeof *res);
    if (s->it->kind == TOKEN_ASTERISK) {
        res->ptr = parse_pointer(s);
        if (!res->ptr) {
            mycc_free(res);
            return NULL;
        }
    } else {
        res->ptr = NULL;
    }

    if (s->it->kind == TOKEN_LBRACKET || s->it->kind == TOKEN_LINDEX) {
        res->direct_abs_decl = parse_direct_abs_declarator(s);
        if (!res->direct_abs_decl) {
            if (res->ptr) {
                free_pointer(res->ptr);
            }
            mycc_free(res);
            return NULL;
        }
    } else {
        res->direct_abs_decl = NULL;
    }

    if (res->direct_abs_decl == NULL && res->ptr == NULL) {
        set_parser_err(s->err, PARSER_ERR_EMPTY_DIRECT_ABS_DECL, s->it->loc);
        mycc_free(res);
        return NULL;
    }

    return res;
}

static void free_abs_declarator_children(struct abs_declarator* d) {
    if (d->ptr) {
        free_pointer(d->ptr);
    }
    if (d->direct_abs_decl) {
        free_direct_abs_declarator(d->direct_abs_decl);
    }
}

void free_abs_declarator(struct abs_declarator* d) {
    free_abs_declarator_children(d);
    mycc_free(d);
}
