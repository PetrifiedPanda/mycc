#include "frontend/ast/declaration/Declaration.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/ast/declaration/StaticAssertDeclaration.h"
#include "frontend/ast/declaration/DeclarationSpecs.h"

bool parse_declaration_inplace(ParserState* s, Declaration* res) {
    assert(res);
    if (ParserState_curr_kind(s) == TOKEN_STATIC_ASSERT) {
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

        if (ParserState_curr_kind(s) != TOKEN_SEMICOLON) {
            const bool success = found_typedef
                                     ? parse_init_declarator_list_typedef(
                                         s,
                                         &res->init_decls)
                                     : parse_init_declarator_list(
                                         s,
                                         &res->init_decls);
            if (!success) {
                DeclarationSpecs_free(res->decl_specs);
                return false;
            }
        } else {
            res->init_decls = (InitDeclaratorList){
                .len = 0,
                .decls = NULL,
            };
        }
        if (!parser_accept(s, TOKEN_SEMICOLON)) {
            DeclarationSpecs_free(res->decl_specs);
            InitDeclaratorList_free(&res->init_decls);
            return false;
        }
    }

    return true;
}

void Declaration_free_children(Declaration* d) {
    if (d->is_normal_decl) {
        DeclarationSpecs_free(d->decl_specs);
        InitDeclaratorList_free(&d->init_decls);
    } else {
        StaticAssertDeclaration_free(d->static_assert_decl);
    }
}

void Declaration_free(Declaration* d) {
    Declaration_free_children(d);
    mycc_free(d);
}

