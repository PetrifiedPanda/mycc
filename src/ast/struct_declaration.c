#include "ast/struct_declaration.h"

#include <stdlib.h>

#include "util.h"
#include "error.h"

bool parse_struct_declaration_inplace(struct parser_state* s,
                                      struct struct_declaration* res) {
    if (s->it->type == STATIC_ASSERT) {
        res->is_static_assert = true;
        res->assert = parse_static_assert_declaration(s);
        if (!res->assert) {
            return NULL;
        }
    } else {
        res->is_static_assert = false;
        bool found_typedef = false;
        res->decl_specs = parse_declaration_specs(s, &found_typedef);
        if (!res->decl_specs) {
            return NULL;
        }

        if (found_typedef) {
            set_error_file(ERR_PARSER,
                           s->it->file,
                           s->it->source_loc,
                           "typedef in struct declaration");
        }

        if (s->it->type != SEMICOLON) {
            res->decls = parse_struct_declarator_list(s);
            if (res->decls.len == 0) {
                free_declaration_specs(res->decl_specs);
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
        free_declaration_specs(d->decl_specs);
        free_struct_declarator_list(&d->decls);
    }
}
