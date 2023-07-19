#include "frontend/ast/declaration/StructUnionSpec.h"

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

#include "frontend/ast/Identifier.h"
#include "frontend/ast/AssignExpr.h"

#include "frontend/ast/declaration/Declarator.h"
#include "frontend/ast/declaration/DeclarationSpecs.h"
#include "frontend/ast/declaration/StaticAssertDeclaration.h"

static bool parse_struct_declarator_inplace(ParserState* s,
                                            StructDeclarator* res) {
    assert(res);

    if (ParserState_curr_kind(s) != TOKEN_COLON) {
        res->decl = parse_declarator(s);
        if (!res->decl) {
            return false;
        }
    } else {
        res->decl = NULL;
    }

    if (ParserState_curr_kind(s) == TOKEN_COLON) {
        ParserState_accept_it(s);
        res->bit_field = parse_const_expr(s);
        if (!res->bit_field) {
            StructDeclarator_free_children(res);
            return false;
        }
    } else {
        res->bit_field = NULL;
        if (!res->decl) {
            ParserErr_set(s->err,
                          PARSER_ERR_EMPTY_STRUCT_DECLARATOR,
                          ParserState_curr_loc(s));
            return false;
        }
    }

    return true;
}

static bool parse_struct_declarator_list(ParserState* s,
                                         StructDeclaratorList* res) {
    *res = (StructDeclaratorList){
        .len = 1,
        .decls = mycc_alloc(sizeof *res->decls),
    };

    if (!parse_struct_declarator_inplace(s, &res->decls[0])) {
        mycc_free(res->decls);
        return false;
    }

    size_t alloc_len = res->len;
    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->decls,
                            &alloc_len,
                            sizeof *res->decls);
        }

        if (!parse_struct_declarator_inplace(s, &res->decls[res->len])) {
            StructDeclaratorList_free(res);
            return false;
        }

        ++res->len;
    }

    res->decls = mycc_realloc(res->decls, sizeof *res->decls * res->len);

    return res;
}

static bool parse_struct_declaration_inplace(ParserState* s,
                                             StructDeclaration* res) {
    if (ParserState_curr_kind(s) == TOKEN_STATIC_ASSERT) {
        res->is_static_assert = true;
        res->assert = parse_static_assert_declaration(s);
        if (!res->assert) {
            return false;
        }
    } else {
        res->is_static_assert = false;
        bool found_typedef = false;
        if (!parse_declaration_specs(s, &res->decl_specs, &found_typedef)) {
            return false;
        }

        if (found_typedef) {
            ParserErr_set(s->err,
                          PARSER_ERR_TYPEDEF_STRUCT,
                          ParserState_curr_loc(s));
        }

        if (ParserState_curr_kind(s) != TOKEN_SEMICOLON) {
            if (!parse_struct_declarator_list(s, &res->decls)) {
                DeclarationSpecs_free(&res->decl_specs);
                return false;
            }
        } else {
            res->decls = (StructDeclaratorList){
                .len = 0,
                .decls = NULL,
            };
        }

        if (!ParserState_accept(s, TOKEN_SEMICOLON)) {
            StructDeclaration_free_children(res);
            return false;
        }
    }

    return true;
}

static StructUnionSpec* StructUnionSpec_create(
    SourceLoc loc,
    bool is_struct,
    Identifier* identifier,
    StructDeclarationList decl_list) {
    StructUnionSpec* res = mycc_alloc(sizeof *res);
    res->info = AstNodeInfo_create(loc);
    res->is_struct = is_struct;
    res->identifier = identifier;
    res->decl_list = decl_list;

    return res;
}

static bool parse_struct_declaration_list(ParserState* s,
                                          StructDeclarationList* res) {
    *res = (StructDeclarationList){
        .len = 1,
        .decls = mycc_alloc(sizeof *res->decls),
    };

    if (!parse_struct_declaration_inplace(s, &res->decls[0])) {
        mycc_free(res->decls);
        return false;
    }

    size_t alloc_len = res->len;
    while (is_declaration(s)
           || ParserState_curr_kind(s) == TOKEN_STATIC_ASSERT) {
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->decls,
                            &alloc_len,
                            sizeof *res->decls);
        }

        if (!parse_struct_declaration_inplace(s, &res->decls[res->len])) {
            StructDeclarationList_free(res);
            return false;
        }

        ++res->len;
    }

    res->decls = mycc_realloc(res->decls, sizeof *res->decls * res->len);

    return res;
}

StructUnionSpec* parse_struct_union_spec(ParserState* s) {
    assert(ParserState_curr_kind(s) == TOKEN_STRUCT
           || ParserState_curr_kind(s) == TOKEN_UNION);
    const SourceLoc loc = ParserState_curr_loc(s);
    bool is_struct;
    if (ParserState_curr_kind(s) == TOKEN_STRUCT) {
        is_struct = true;
        ParserState_accept_it(s);
    } else {
        is_struct = false;
        ParserState_accept_it(s);
    }
    Identifier* id = NULL;
    if (ParserState_curr_kind(s) == TOKEN_IDENTIFIER) {
        const StrBuf spell = ParserState_take_curr_spell(s);
        const SourceLoc id_loc = ParserState_curr_loc(s);
        ParserState_accept_it(s);
        id = Identifier_create(&spell, id_loc);
    }

    StructDeclarationList list = {.len = 0, .decls = NULL};
    if (ParserState_curr_kind(s) == TOKEN_LBRACE) {
        ParserState_accept_it(s);
        if (!parse_struct_declaration_list(s, &list)) {
            goto fail;
        }

        if (!ParserState_accept(s, TOKEN_RBRACE)) {
            StructDeclarationList_free(&list);
            goto fail;
        }
    }
    return StructUnionSpec_create(loc, is_struct, id, list);

fail:
    if (id) {
        Identifier_free(id);
    }
    return NULL;
}

void StructDeclarator_free_children(StructDeclarator* d) {
    if (d->decl) {
        Declarator_free(d->decl);
    }
    if (d->bit_field) {
        ConstExpr_free(d->bit_field);
    }
}

void StructDeclaratorList_free(StructDeclaratorList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        StructDeclarator_free_children(&l->decls[i]);
    }
    mycc_free(l->decls);
}

void StructDeclaration_free_children(StructDeclaration* d) {
    if (d->is_static_assert) {
        StaticAssertDeclaration_free(d->assert);
    } else {
        DeclarationSpecs_free(&d->decl_specs);
        StructDeclaratorList_free(&d->decls);
    }
}

void StructDeclarationList_free(StructDeclarationList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        StructDeclaration_free_children(&l->decls[i]);
    }
    mycc_free(l->decls);
}

static void StructUnionSpec_free_children(StructUnionSpec* s) {
    if (s->identifier) {
        Identifier_free(s->identifier);
    }
    StructDeclarationList_free(&s->decl_list);
}

void StructUnionSpec_free(StructUnionSpec* s) {
    StructUnionSpec_free_children(s);
    mycc_free(s);
}

