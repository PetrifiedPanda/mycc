#include "frontend/ast/declaration/ExternalDeclaration.h"

#include <assert.h>

#include "util/mem.h"

static bool parse_external_decl_normal_decl(
    ParserState* s,
    ExternalDeclaration* res,
    DeclarationSpecs* decl_specs,
    Declarator* first_decl,
    bool found_typedef) {
    res->is_func_def = false;

    Declaration* decl = &res->decl;

    decl->is_normal_decl = true;
    decl->decl_specs = decl_specs;

    Initializer* init = NULL;
    if (s->it->kind == TOKEN_ASSIGN) {
        if (found_typedef) {
            ParserErr_set(s->err, PARSER_ERR_TYPEDEF_INIT, s->it->loc);
            DeclarationSpecs_free(decl_specs);
            Declarator_free(first_decl);
            return false;
        }
        parser_accept_it(s);
        init = parse_initializer(s);
        if (!init) {
            DeclarationSpecs_free(decl_specs);
            Declarator_free(first_decl);
            return false;
        }
    }

    InitDeclarator* init_decl = mycc_alloc(sizeof *init_decl);
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
        InitDeclarator_free_children(init_decl);
        mycc_free(init_decl);
        Declaration_free_children(decl);
        return false;
    }

    if (!parser_accept(s, TOKEN_SEMICOLON)) {
        Declaration_free_children(decl);
        return false;
    }
    return true;
}

static bool parse_external_declaration_func_def(
    ParserState* s,
    ExternalDeclaration* res,
    DeclarationSpecs* decl_specs,
    Declarator* first_decl,
    bool found_typedef) {
    if (found_typedef) {
        ParserErr_set(s->err, PARSER_ERR_TYPEDEF_FUNC_DEF, s->it->loc);
        DeclarationSpecs_free(decl_specs);
        Declarator_free(first_decl);
        return false;
    }
    res->is_func_def = true;

    FuncDef* func_def = &res->func_def;

    func_def->specs = decl_specs;
    func_def->decl = first_decl;
    if (s->it->kind != TOKEN_LBRACE) {
        if (!parse_declaration_list(s, &func_def->decl_list)) {
            DeclarationSpecs_free(decl_specs);
            Declarator_free(first_decl);
            return false;
        }
    } else {
        func_def->decl_list = (DeclarationList){
            .len = 0,
            .decls = NULL,
        };
    }

    if (!parse_compound_statement_inplace(s, &func_def->comp)) {
        DeclarationSpecs_free(decl_specs);
        Declarator_free(first_decl);
        DeclarationList_free(&func_def->decl_list);
        return false;
    }
    return true;
}

bool parse_external_declaration_inplace(ParserState* s, ExternalDeclaration* res) {
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
    DeclarationSpecs* decl_specs = parse_declaration_specs(s, &found_typedef);
    if (!decl_specs) {
        return false;
    }

    if (s->it->kind == TOKEN_SEMICOLON) {
        parser_accept_it(s);
        res->is_func_def = false;
        res->decl.is_normal_decl = true;
        res->decl.decl_specs = decl_specs;
        res->decl.init_decls = (InitDeclaratorList){
            .len = 0,
            .decls = NULL,
        };

        return true;
    }

    Declarator* first_decl;
    if (found_typedef) {
        first_decl = parse_declarator_typedef(s);
    } else {
        first_decl = parse_declarator(s);
    }

    if (!first_decl) {
        DeclarationSpecs_free(decl_specs);
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

void ExternalDeclaration_free_children(ExternalDeclaration* d) {
    if (d->is_func_def) {
        FuncDef_free_children(&d->func_def);
    } else {
        Declaration_free_children(&d->decl);
    }
}
