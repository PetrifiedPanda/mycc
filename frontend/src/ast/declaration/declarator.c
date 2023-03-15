#include "frontend/ast/declaration/declarator.h"

#include "util/mem.h"

static struct declarator* parse_declarator_base(
    struct parser_state* s,
    struct direct_declarator* (*parse_func)(struct parser_state* s)) {
    struct declarator* res = mycc_alloc(sizeof *res);
    if (s->it->kind == ASTERISK) {
        res->ptr = parse_pointer(s);
        if (!res->ptr) {
            mycc_free(res);
            return NULL;
        }
    } else {
        res->ptr = NULL;
    }

    res->direct_decl = parse_func(s);
    if (!res->direct_decl) {
        if (res->ptr) {
            free_pointer(res->ptr);
        }
        mycc_free(res);
        return NULL;
    }

    return res;
}

struct declarator* parse_declarator_typedef(struct parser_state* s) {
    return parse_declarator_base(s, parse_direct_declarator_typedef);
}

struct declarator* parse_declarator(struct parser_state* s) {
    return parse_declarator_base(s, parse_direct_declarator);
}

static void free_children(struct declarator* d) {
    if (d->ptr) {
        free_pointer(d->ptr);
    }
    free_direct_declarator(d->direct_decl);
}

void free_declarator(struct declarator* d) {
    free_children(d);
    mycc_free(d);
}
