#include "frontend/ast/declaration/declaration.h"

#include <assert.h>

#include "util/mem.h"

bool parse_declaration_inplace(struct parser_state* s,
                               struct declaration* res) {
    assert(res);
    if (s->it->kind == TOKEN_STATIC_ASSERT) {
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

        if (s->it->kind != TOKEN_SEMICOLON) {
            const bool success = found_typedef
                                     ? parse_init_declarator_list_typedef(
                                         s,
                                         &res->init_decls)
                                     : parse_init_declarator_list(
                                         s,
                                         &res->init_decls);
            if (!success) {
                free_declaration_specs(res->decl_specs);
                return false;
            }
        } else {
            res->init_decls = (struct init_declarator_list){
                .len = 0,
                .decls = NULL,
            };
        }
        if (!parser_accept(s, TOKEN_SEMICOLON)) {
            free_declaration_specs(res->decl_specs);
            free_init_declarator_list(&res->init_decls);
            return false;
        }
    }

    return true;
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
    mycc_free(d);
}

