#include "frontend/ast/declaration/external_declaration.h"

#include <assert.h>

#include "util/mem.h"

static bool parse_external_decl_normal_decl(
    struct parser_state* s,
    struct external_declaration* res,
    struct declaration_specs* decl_specs,
    struct declarator* first_decl,
    bool found_typedef) {
    res->is_func_def = false;

    struct declaration* decl = &res->decl;

    decl->is_normal_decl = true;
    decl->decl_specs = decl_specs;

    struct initializer* init = NULL;
    if (s->it->kind == TOKEN_ASSIGN) {
        if (found_typedef) {
            set_parser_err(s->err, PARSER_ERR_TYPEDEF_INIT, s->it->loc);
            free_declaration_specs(decl_specs);
            free_declarator(first_decl);
            return false;
        }
        accept_it(s);
        init = parse_initializer(s);
        if (!init) {
            free_declaration_specs(decl_specs);
            free_declarator(first_decl);
            return false;
        }
    }

    struct init_declarator* init_decl = mycc_alloc(sizeof *init_decl);
    init_decl->decl = first_decl;
    init_decl->init = init;

    const bool success = found_typedef
                             ? parse_init_declarator_list_typedef_first(
                                 s,
                                 &decl->init_decls,
                                 init_decl)
                             : parse_init_declarator_list_first(
                                 s,
                                 &decl->init_decls,
                                 init_decl);
    if (!success) {
        free_init_declarator_children(init_decl);
        mycc_free(init_decl);
        free_declaration_children(decl);
        return false;
    }

    if (!accept(s, TOKEN_SEMICOLON)) {
        free_declaration_children(decl);
        return false;
    }
    return true;
}

static bool parse_external_declaration_func_def(
    struct parser_state* s,
    struct external_declaration* res,
    struct declaration_specs* decl_specs,
    struct declarator* first_decl,
    bool found_typedef) {
    if (found_typedef) {
        set_parser_err(s->err, PARSER_ERR_TYPEDEF_FUNC_DEF, s->it->loc);
        free_declaration_specs(decl_specs);
        free_declarator(first_decl);
        return false;
    }
    res->is_func_def = true;

    struct func_def* func_def = &res->func_def;

    func_def->specs = decl_specs;
    func_def->decl = first_decl;
    if (s->it->kind != TOKEN_LBRACE) {
        if (!parse_declaration_list(s, &func_def->decl_list)) {
            free_declaration_specs(decl_specs);
            free_declarator(first_decl);
            return false;
        }
    } else {
        func_def->decl_list = (struct declaration_list){
            .len = 0,
            .decls = NULL,
        };
    }

    if (!parse_compound_statement_inplace(s, &func_def->comp)) {
        free_declaration_specs(decl_specs);
        free_declarator(first_decl);
        free_declaration_list(&func_def->decl_list);
        return false;
    }
    return true;
}

bool parse_external_declaration_inplace(struct parser_state* s,
                                        struct external_declaration* res) {
    assert(res);

    if (s->it->kind == TOKEN_STATIC_ASSERT) {
        res->is_func_def = false;
        res->decl.is_normal_decl = false;
        res->decl.static_assert_decl = parse_static_assert_declaration(s);
        if (!res->decl.static_assert_decl) {
            return false;
        }
        return true;
    }
    bool found_typedef = false;
    struct declaration_specs* decl_specs = parse_declaration_specs(
        s,
        &found_typedef);
    if (!decl_specs) {
        return false;
    }

    if (s->it->kind == TOKEN_SEMICOLON) {
        accept_it(s);
        res->is_func_def = false;
        res->decl.is_normal_decl = true;
        res->decl.decl_specs = decl_specs;
        res->decl.init_decls = (struct init_declarator_list){
            .len = 0,
            .decls = NULL,
        };

        return true;
    }

    struct declarator* first_decl;
    if (found_typedef) {
        first_decl = parse_declarator_typedef(s);
    } else {
        first_decl = parse_declarator(s);
    }

    if (!first_decl) {
        free_declaration_specs(decl_specs);
        return false;
    }

    if (s->it->kind == TOKEN_ASSIGN || s->it->kind == TOKEN_COMMA
        || s->it->kind == TOKEN_SEMICOLON) {
        return parse_external_decl_normal_decl(s,
                                               res,
                                               decl_specs,
                                               first_decl,
                                               found_typedef);
    } else {
        return parse_external_declaration_func_def(s,
                                                   res,
                                                   decl_specs,
                                                   first_decl,
                                                   found_typedef);
    }
}

void free_external_declaration_children(struct external_declaration* d) {
    if (d->is_func_def) {
        free_func_def_children(&d->func_def);
    } else {
        free_declaration_children(&d->decl);
    }
}

