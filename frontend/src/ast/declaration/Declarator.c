#include "frontend/ast/declaration/Declarator.h"

#include "util/mem.h"

static Declarator* parse_declarator_base(ParserState* s, DirectDeclarator* (*parse_func)(ParserState* s)) {
    Declarator* res = mycc_alloc(sizeof *res);
    if (s->it->kind == TOKEN_ASTERISK) {
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

Declarator* parse_declarator_typedef(ParserState* s) {
    return parse_declarator_base(s, parse_direct_declarator_typedef);
}

Declarator* parse_declarator(ParserState* s) {
    return parse_declarator_base(s, parse_direct_declarator);
}

static void free_declarator_children(Declarator* d) {
    if (d->ptr) {
        free_pointer(d->ptr);
    }
    free_direct_declarator(d->direct_decl);
}

void free_declarator(Declarator* d) {
    free_declarator_children(d);
    mycc_free(d);
}
