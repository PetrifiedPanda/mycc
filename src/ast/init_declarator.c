#include "ast/init_declarator.h"

#include "util.h"

struct init_declarator* parse_init_declarator(struct parser_state* s) {
    struct init_declarator* res = xmalloc(sizeof(struct init_declarator));

    res->decl = parse_declarator(s);
    if (!res->decl) {
        free(res);
        return NULL;
    }

    if (s->it->type == ASSIGN) {
        accept_it(s);
        res->init = parse_initializer(s);
        if (!res->init) {
            free_declarator(res->decl);
            free(res);
            return NULL;
        }
    } else {
        res->init = NULL;
    }
    return res;
}

void free_init_declarator_children(struct init_declarator* d) {
    free_declarator(d->decl);
    free_initializer(d->init);
}

