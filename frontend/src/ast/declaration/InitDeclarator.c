#include "frontend/ast/declaration/InitDeclarator.h"

#include "util/mem.h"

bool parse_init_declarator_typedef_inplace(ParserState* s, InitDeclarator* res) {
    res->decl = parse_declarator_typedef(s);
    if (!res->decl) {
        return false;
    }

    if (s->it->kind == TOKEN_ASSIGN) {
        ParserErr_set(s->err, PARSER_ERR_TYPEDEF_INIT, s->it->loc);
        return false;
    }

    res->init = NULL;

    return true;
}

bool parse_init_declarator_inplace(ParserState* s, InitDeclarator* res) {
    res->decl = parse_declarator(s);
    if (!res->decl) {
        return false;
    }

    if (s->it->kind == TOKEN_ASSIGN) {
        parser_accept_it(s);
        res->init = parse_initializer(s);
        if (!res->init) {
            Declarator_free(res->decl);
            return false;
        }
    } else {
        res->init = NULL;
    }
    return true;
}

void InitDeclarator_free_children(InitDeclarator* d) {
    Declarator_free(d->decl);
    if (d->init) {
        Initializer_free(d->init);
    }
}

