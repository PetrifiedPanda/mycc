#include "ast/init_declarator.h"

#include "error.h"

#include "util/mem.h"

bool parse_init_declarator_typedef_inplace(struct parser_state* s,
                                           struct init_declarator* res) {
    res->decl = parse_declarator_typedef(s);
    if (!res->decl) {
        return false;
    }

    if (s->it->type == ASSIGN) {
        set_error_file(ERR_PARSER,
                       s->it->file,
                       s->it->source_loc,
                       "Initializer not allowed in typedef");
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
