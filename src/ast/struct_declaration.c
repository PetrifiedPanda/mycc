#include "ast/struct_declaration.h"

#include <stdlib.h>

#include "util.h"

bool parse_struct_declaration_inplace(struct parser_state* s, struct struct_declaration* res) {
    if (s->it->type == STATIC_ASSERT) {
        res->is_static_assert = true;
        res->assert = parse_static_assert_declaration(s);
        if (!res->assert) {
            return NULL;
        }
    } else {
        res->is_static_assert = false;
        res->spec_qual_list = parse_spec_qual_list(s);
        if (res->spec_qual_list.len == 0) {
            return NULL;
        }

        if (s->it->type != SEMICOLON) {
            res->decls = parse_struct_declarator_list(s);
            if (res->decls.len == 0) {
                free_spec_qual_list(&res->spec_qual_list);
                return NULL;
            }
        }

        if (!accept(s, SEMICOLON)) {
            free_struct_declaration_children(res);
            return NULL;
        }
    }

    return res;
}

void free_struct_declaration_children(struct struct_declaration* d) {
    if (d->is_static_assert) {
        free_static_assert_declaration(d->assert);
    } else {
        free_spec_qual_list(&d->spec_qual_list);
        free_struct_declarator_list(&d->decls);
    }
}

void free_struct_declaration(struct struct_declaration* d) {
    free_struct_declaration_children(d);
    free(d);
}

