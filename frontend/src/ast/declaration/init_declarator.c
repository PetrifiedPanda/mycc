#include "frontend/ast/declaration/init_declarator.h"

#include "util/mem.h"

bool parse_init_declarator_typedef_inplace(struct parser_state* s,
                                           struct init_declarator* res) {
    res->decl = parse_declarator_typedef(s);
    if (!res->decl) {
        return false;
    }

    if (s->it->kind == TOKEN_ASSIGN) {
        set_parser_err(s->err, PARSER_ERR_TYPEDEF_INIT, s->it->loc);
        return false;
    }

    res->init = NULL;

    return true;
}

bool parse_init_declarator_inplace(struct parser_state* s,
                                   struct init_declarator* res) {
    res->decl = parse_declarator(s);
    if (!res->decl) {
        return false;
    }

    if (s->it->kind == TOKEN_ASSIGN) {
        parser_accept_it(s);
        res->init = parse_initializer(s);
        if (!res->init) {
            free_declarator(res->decl);
            return false;
        }
    } else {
        res->init = NULL;
    }
    return true;
}

void free_init_declarator_children(struct init_declarator* d) {
    free_declarator(d->decl);
    if (d->init) {
        free_initializer(d->init);
    }
}

