#include "ast/init_declarator.h"

#include "util.h"

bool parse_init_declarator_inplace(struct parser_state* s, struct init_declarator* res) {
    res->decl = parse_declarator(s);
    if (!res->decl) {
        return false;
    }

    if (s->it->type == ASSIGN) {
        accept_it(s);
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

