#include "frontend/ast/declaration/EnumSpec.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

#include "frontend/ast/Identifier.h"

#include "frontend/ast/expr/AssignExpr.h"

static bool parse_enumerator_inplace(ParserState* s, Enumerator* res) {
    assert(res);

    if (ParserState_curr_kind(s) != TOKEN_IDENTIFIER) {
        expected_token_error(s, TOKEN_IDENTIFIER);
        return false;
    }

    const StrBuf spell = ParserState_take_curr_spell(s);
    const SourceLoc loc = ParserState_curr_loc(s);
    parser_accept_it(s);

    if (!parser_register_enum_constant(s, &spell, loc)) {
        StrBuf_free(&spell);
        return false;
    }

    ConstExpr* enum_val = NULL;
    if (ParserState_curr_kind(s) == TOKEN_ASSIGN) {
        parser_accept_it(s);
        enum_val = parse_const_expr(s);
        if (!enum_val) {
            StrBuf_free(&spell);
            return false;
        }
    }

    res->identifier = Identifier_create(&spell, loc);
    res->enum_val = enum_val;

    return true;
}

static bool parse_enum_list(ParserState* s, EnumList* res) {
    res->len = 1;
    res->enums = mycc_alloc(sizeof *res->enums);
    if (!parse_enumerator_inplace(s, &res->enums[0])) {
        mycc_free(res->enums);
        return false;
    }

    size_t alloc_len = 1;
    while (ParserState_curr_kind(s) == TOKEN_COMMA
           && ParserState_next_token_kind(s) == TOKEN_IDENTIFIER) {
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->enums,
                            &alloc_len,
                            sizeof *res->enums);
        }

        if (!parse_enumerator_inplace(s, &res->enums[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->enums = mycc_realloc(res->enums, res->len * sizeof *res->enums);

    return res;
fail:
    EnumList_free(res);
    return false;
}

static EnumSpec* EnumSpec_create(SourceLoc loc,
                                 Identifier* identifier,
                                 EnumList enum_list) {
    assert(identifier || enum_list.len > 0);
    EnumSpec* res = mycc_alloc(sizeof *res);
    res->info = AstNodeInfo_create(loc);
    res->identifier = identifier;
    res->enum_list = enum_list;

    return res;
}

EnumSpec* parse_enum_spec(ParserState* s) {
    const SourceLoc loc = ParserState_curr_loc(s);
    if (!parser_accept(s, TOKEN_ENUM)) {
        return NULL;
    }

    Identifier* id = NULL;
    if (ParserState_curr_kind(s) == TOKEN_IDENTIFIER) {
        const StrBuf spell = ParserState_take_curr_spell(s);
        const SourceLoc id_loc = ParserState_curr_loc(s);
        parser_accept_it(s);
        id = Identifier_create(&spell, id_loc);
    }

    EnumList enums = {.len = 0, .enums = NULL};
    if (ParserState_curr_kind(s) == TOKEN_LBRACE) {
        parser_accept_it(s);
        if (!parse_enum_list(s, &enums)) {
            goto fail;
        }
        assert(enums.len > 0);

        if (ParserState_curr_kind(s) == TOKEN_COMMA) {
            parser_accept_it(s);
        }
        if (!parser_accept(s, TOKEN_RBRACE)) {
            EnumList_free(&enums);
            goto fail;
        }
    } else if (id == NULL) {
        expected_token_error(s, TOKEN_LBRACE);
        goto fail;
    }

    return EnumSpec_create(loc, id, enums);
fail:
    if (id) {
        Identifier_free(id);
    }
    return NULL;
}

void Enumerator_free(Enumerator* e) {
    Identifier_free(e->identifier);
    if (e->enum_val) {
        ConstExpr_free(e->enum_val);
    }
}

void EnumList_free(EnumList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        Enumerator_free(&l->enums[i]);
    }
    mycc_free(l->enums);
}

static void EnumSpec_free_children(EnumSpec* s) {
    if (s->identifier) {
        Identifier_free(s->identifier);
    }
    EnumList_free(&s->enum_list);
}

void EnumSpec_free(EnumSpec* s) {
    EnumSpec_free_children(s);
    mycc_free(s);
}

