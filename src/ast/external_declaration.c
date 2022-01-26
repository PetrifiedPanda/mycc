#include "ast/external_declaration.h"

#include <assert.h>

#include "util.h"

bool parse_external_declaration_inplace(struct parser_state* s, struct external_declaration* res) {
    assert(res);

    if (s->it->type == STATIC_ASSERT) {
        res->is_func_def = false;
        res->decl.is_normal_decl = false;
        res->decl.static_assert_decl = parse_static_assert_declaration(s);
        if (!res->decl.static_assert_decl) {
            return false;
        }
        return true;
    }

    struct declaration_specs* decl_specs = parse_declaration_specs(s);
    if (!decl_specs) {
        return false;
    }

    if (s->it->type == SEMICOLON) {
        accept_it(s);
        res->is_func_def = false;
        res->decl.is_normal_decl = true;
        res->decl.decl_specs = decl_specs;
        res->decl.init_decls = (struct init_declarator_list){.len = 0, .decls = NULL};

        return true;
    }

    struct declarator* first_decl = parse_declarator(s);
    if (!first_decl) {
        free_declaration_specs(decl_specs);
        return false;
    }


    if (s->it->type == ASSIGN || s->it->type == COMMA) {
        res->is_func_def = false;

        struct declaration* decl = &res->decl;

        decl->is_normal_decl = true;
        decl->decl_specs = decl_specs;

        struct initializer* init = NULL;
        if (s->it->type == ASSIGN) {
            accept_it(s);
            init = parse_initializer(s);
            if (!init) {
                free_declaration_specs(decl_specs);
                free_declarator(first_decl);
                return false;
            }
        }

        struct init_declarator* init_decl = xmalloc(sizeof(struct init_declarator));
        init_decl->decl = first_decl;
        init_decl->init = init;

        decl->init_decls = parse_init_declarator_list_first(s, init_decl);
        if (decl->init_decls.len == 0) {
            free_init_declarator_children(init_decl);
            free(init_decl);
            free_declaration_children(decl);
            return false;
        }

        if (!accept(s, SEMICOLON)) {
            free_declaration_children(decl);
            return false;
        }
        return true;
    } else if (s->it->type == SEMICOLON) {
        accept_it(s);

        res->is_func_def = false;
        res->decl.is_normal_decl = true;
        res->decl.decl_specs = decl_specs;
        res->decl.init_decls = (struct init_declarator_list){.len = 0, .decls = NULL};
    } else {
        res->is_func_def = true;

        struct func_def* func_def = &res->func_def;

        func_def->specs = decl_specs;
        func_def->decl = first_decl;
        if (s->it->type != LBRACKET) {
            func_def->decl_list = parse_declaration_list(s);
            if (func_def->decl_list.len == 0) {
                free_declaration_specs(decl_specs);
                free_declarator(first_decl);
                return false;
            }
        } else {
            func_def->decl_list = (struct declaration_list) {
                .len = 0,
                .decls = NULL
            };
        }

        func_def->comp = parse_compound_statement(s);
        if (!func_def->comp) {
            free_declaration_specs(decl_specs);
            free_declarator(first_decl);
            free_declaration_list(&func_def->decl_list);
            return false;
        }
    }

    return true;
}

void free_external_declaration_children(struct external_declaration* d) {
    if (d->is_func_def) {
        free_func_def_children(&d->func_def);
    } else {
        free_declaration_children(&d->decl);
    }
}

