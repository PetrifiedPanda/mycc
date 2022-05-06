#include "ast/declaration.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

bool parse_declaration_inplace(struct parser_state* s,
                               struct declaration* res) {
    assert(res);
    if (s->it->type == STATIC_ASSERT) {
        res->is_normal_decl = false;
        res->static_assert_decl = parse_static_assert_declaration(s);
        if (!res->static_assert_decl) {
            return false;
        }
    } else {
        res->is_normal_decl = true;

        bool found_typedef = false;
        res->decl_specs = parse_declaration_specs(s, &found_typedef);
        if (!res->decl_specs) {
            return false;
        }

        if (s->it->type != SEMICOLON) {
            if (found_typedef) {
                res->init_decls = parse_init_declarator_list_typedef(s);
            } else {
                res->init_decls = parse_init_declarator_list(s);
            }
            if (res->init_decls.len == 0) {
                free_declaration_specs(res->decl_specs);
                return false;
            }
        } else {
            res->init_decls = (struct init_declarator_list){
                .len = 0,
                .decls = NULL,
            };
        }
        if (!accept(s, SEMICOLON)) {
            free_declaration_specs(res->decl_specs);
            free_init_declarator_list(&res->init_decls);
            return false;
        }
    }

    return true;
}

struct declaration* parse_declaration(struct parser_state* s) {
    struct declaration* res = xmalloc(sizeof(struct declaration));
    if (!parse_declaration_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

void free_declaration_children(struct declaration* d) {
    if (d->is_normal_decl) {
        free_declaration_specs(d->decl_specs);
        free_init_declarator_list(&d->init_decls);
    } else {
        free_static_assert_declaration(d->static_assert_decl);
    }
}

void free_declaration(struct declaration* d) {
    free_declaration_children(d);
    free(d);
}
