#include "ast/struct_declarator.h"

#include <stdlib.h>

#include "error.h"
#include "util.h"

struct struct_declarator* parse_struct_declarator(struct parser_state* s) {
    struct struct_declarator* res = xmalloc(sizeof(struct struct_declarator));

    if (s->it->type != COLON) {
        res->decl = parse_declarator(s);
        if (!res->decl) {
            free(res);
            return NULL;
        }
    } else {
        res->decl = NULL;
    }

    if (s->it->type == COLON) {
        accept_it(s);
        res->bit_field = parse_const_expr(s);
        if (!res->bit_field) {
            free_struct_declarator(res);
            return NULL;
        }
    } else {
        res->bit_field = NULL;
        if (!res->decl) {
            set_error_file(ERR_PARSER, s->it->file, s->it->source_loc, "Expected a declarator or a bit field specifier");
            return NULL;
        }
    }

    return res;
}

void free_struct_declarator_children(struct struct_declarator* d) {
    if (d->decl) {
        free_declarator(d->decl);
    }
    if (d->bit_field) {
        free_const_expr(d->bit_field);
    }
}

void free_struct_declarator(struct struct_declarator* d) {
    free_struct_declarator_children(d);
    free(d);
}

