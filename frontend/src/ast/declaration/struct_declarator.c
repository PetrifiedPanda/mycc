#include "ast/declaration/struct_declarator.h"

#include <assert.h>

#include "util/mem.h"

bool parse_struct_declarator_inplace(struct parser_state* s,
                                     struct struct_declarator* res) {
    assert(res);

    if (s->it->type != COLON) {
        res->decl = parse_declarator(s);
        if (!res->decl) {
            return false;
        }
    } else {
        res->decl = NULL;
    }

    if (s->it->type == COLON) {
        accept_it(s);
        res->bit_field = parse_const_expr(s);
        if (!res->bit_field) {
            free_struct_declarator_children(res);
            return false;
        }
    } else {
        res->bit_field = NULL;
        if (!res->decl) {
            set_parser_err(s->err,
                           PARSER_ERR_EMPTY_STRUCT_DECLARATOR,
                           &s->it->loc);
            return false;
        }
    }

    return true;
}

void free_struct_declarator_children(struct struct_declarator* d) {
    if (d->decl) {
        free_declarator(d->decl);
    }
    if (d->bit_field) {
        free_const_expr(d->bit_field);
    }
}

