#include "ast/struct_declarator.h"

#include <stdlib.h>
#include <assert.h>

#include "error.h"
#include "util.h"

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
            set_error_file(ERR_PARSER,
                           s->it->file,
                           s->it->source_loc,
                           "Expected a declarator or a bit field specifier");
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
