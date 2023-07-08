#include "frontend/ast/declaration/Declarator.h"

#include "util/mem.h"

#include "frontend/ast/declaration/Pointer.h"
#include "frontend/ast/declaration/DirectDeclarator.h"

static Declarator* parse_declarator_base(
    ParserState* s,
    DirectDeclarator* (*parse_func)(ParserState* s)) {
    Declarator* res = mycc_alloc(sizeof *res);
    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
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
            Pointer_free(res->ptr);
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

static void Declarator_free_children(Declarator* d) {
    if (d->ptr) {
        Pointer_free(d->ptr);
    }
    DirectDeclarator_free(d->direct_decl);
}

void Declarator_free(Declarator* d) {
    Declarator_free_children(d);
    mycc_free(d);
}
